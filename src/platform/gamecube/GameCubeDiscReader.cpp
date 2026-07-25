#include "platform/gamecube/GameCubeDiscReader.hpp"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <malloc.h>

#include <ogc/cache.h>
#include <ogc/dvd.h>
#include <ogc/irq.h>
#include <ogc/system.h>

/// Transfers control from the PI/DI interrupt dispatcher through the Nintendont-recognised bridge into the packaged-disc completion routine.
extern "C" void GameCubeDiscInterruptTrampoline(uint32_t interruptId, void* context);

namespace helengine::gamecube {
    /// Completes the active packaged-disc request after the physical DI interrupt or Nintendont's replacement bridge runs.
    extern "C" void GameCubeDiscInterruptComplete(
        volatile uint32_t* discInterface,
        uint32_t originalContext,
        uint32_t interruptMask,
        uint32_t interruptStatusMask);
}

asm(
    ".global GameCubeDiscInterruptTrampoline\n"
    "GameCubeDiscInterruptTrampoline:\n"
    "lis 3, 0xCC00\n"
    "addi 3, 3, 0x6000\n"
    "li 5, 0x002A\n"
    "li 6, 0x0054\n"
    "b GameCubeDiscInterruptComplete\n");

namespace helengine::gamecube {
    namespace {
        /// Defines the byte size of one GameCube disc sector.
        constexpr std::size_t DiscSectorSize = 2048U;

        /// Defines the memory alignment required by the PowerPC data cache and DI DMA.
        constexpr std::size_t CacheLineSize = 32U;

        /// Defines the maximum byte count accepted by one native DI DMA request.
        constexpr std::size_t MaximumTransferLength = 0x80000U;

        /// Defines the physical GameCube DI register address before Nintendont redirects it.
        constexpr uint32_t DiscInterfaceBaseAddress = 0xCC006000U;

        /// Defines the unencrypted GameCube disc-sector read command.
        constexpr uint32_t DiscReadCommand = 0xA8000000U;

        /// Defines the DI status bit raised when a DMA request completes.
        constexpr uint32_t DiscTransferCompleteInterrupt = 1U << 4U;

        /// Defines the DI status bit raised when a DMA request encounters a device error.
        constexpr uint32_t DiscDeviceErrorInterrupt = 1U << 2U;

        /// Defines the enabled DI status bits whose paired interrupt bits complete a synchronous read.
        constexpr uint32_t DiscCompletionInterruptEnableMask = (1U << 1U) | (1U << 3U);

        /// Defines the DI interrupt bits that report either transfer completion or device failure.
        constexpr uint32_t DiscCompletionInterruptMask = DiscDeviceErrorInterrupt | DiscTransferCompleteInterrupt;

        /// Defines every DI status mask bit that must remain enabled while a request is active.
        constexpr uint32_t DiscInterruptEnableMask = 0x2AU;

        /// Stores whether the executable has registered its owned DI completion bridge.
        bool IsInitialized = false;

        /// Stores whether one synchronous caller currently owns the DI hardware request.
        volatile bool IsReadPending = false;

        /// Stores whether the DI completion bridge observed a terminal interrupt for the active request.
        volatile bool IsReadComplete = false;

        /// Stores whether the terminal interrupt reported a transfer completion without a device error.
        volatile bool DidReadSucceed = false;

        /// Returns whether an address satisfies the required PowerPC cache-line alignment.
        bool IsCacheLineAligned(std::uintptr_t value) {
            return (value & (CacheLineSize - 1U)) == 0U;
        }

        /// Returns the physical or Nintendont-patched DI register window used by the current executable.
        volatile uint32_t* GetDiscInterface() {
            return reinterpret_cast<volatile uint32_t*>(DiscInterfaceBaseAddress);
        }

        /// Observes and acknowledges one enabled terminal DI status so synchronous callers can complete without relying on a platform-specific interrupt dispatcher.
        bool TryCompleteRead(volatile uint32_t* discInterface) {
            if (discInterface == nullptr || !IsReadPending) {
                return false;
            }

            const uint32_t status = discInterface[0];
            const uint32_t enabledInterrupts = (status & DiscCompletionInterruptEnableMask) << 1U;
            const uint32_t terminalInterrupts = (status & DiscCompletionInterruptMask) & enabledInterrupts;
            if (terminalInterrupts == 0U) {
                return false;
            }

            discInterface[0] = (status & DiscInterruptEnableMask) | terminalInterrupts;
            DidReadSucceed = (terminalInterrupts & DiscTransferCompleteInterrupt) != 0U
                && (terminalInterrupts & DiscDeviceErrorInterrupt) == 0U;
            IsReadPending = false;
            IsReadComplete = true;
            return true;
        }

        /// Waits for an interrupt-driven or directly observed terminal DI status after issuing one read request.
        bool WaitForReadCompletion(volatile uint32_t* discInterface) {
            while (!IsReadComplete) {
                TryCompleteRead(discInterface);
                asm volatile("sync" ::: "memory");
            }

            return DidReadSucceed;
        }

        /// Submits one aligned DI DMA request whose byte count fits the native GameCube transfer limit.
        bool ReadAlignedBytes(void* destination, std::size_t offset, std::size_t length) {
            if (!IsCacheLineAligned(reinterpret_cast<std::uintptr_t>(destination))
                || (length % CacheLineSize) != 0U
                || length == 0U
                || length > MaximumTransferLength
                || offset > std::numeric_limits<uint32_t>::max()
                || length > std::numeric_limits<uint32_t>::max()) {
                return false;
            }

            const uint32_t interruptLevel = IRQ_Disable();
            if (!IsInitialized || IsReadPending) {
                IRQ_Restore(interruptLevel);
                return false;
            }

            IsReadPending = true;
            IsReadComplete = false;
            DidReadSucceed = false;
            volatile uint32_t* const discInterface = GetDiscInterface();
            DCInvalidateRange(destination, static_cast<uint32_t>(length));
            discInterface[0] = (discInterface[0] & DiscInterruptEnableMask) | DiscInterruptEnableMask;
            discInterface[2] = DiscReadCommand;
            discInterface[3] = static_cast<uint32_t>(offset >> 2U);
            discInterface[4] = static_cast<uint32_t>(length);
            discInterface[5] = MEM_VIRTUAL_TO_PHYSICAL(destination);
            discInterface[6] = static_cast<uint32_t>(length);
            discInterface[7] = 0x03U;
            IRQ_Restore(interruptLevel);

            return WaitForReadCompletion(discInterface);
        }
    }

    /// Installs the owned DI completion bridge after libogc has configured the PI interrupt subsystem.
    bool GameCubeDiscReader::Initialize() {
        if (IsInitialized) {
            return true;
        }

        DVD_Init();
        IRQ_Request(IRQ_PI_DI, GameCubeDiscInterruptTrampoline, nullptr);
        IsInitialized = true;
        return true;
    }

    /// Reads a byte range through aligned native DI transfers, expanding unaligned requests to complete sectors.
    bool GameCubeDiscReader::ReadBytes(void* destination, std::size_t offset, std::size_t length) {
        if (destination == nullptr) {
            return false;
        } else if (length == 0U) {
            return true;
        } else if (!IsCacheLineAligned(reinterpret_cast<std::uintptr_t>(destination))
            || offset > std::numeric_limits<std::size_t>::max() - length) {
            return false;
        }

        const std::size_t lastByteOffsetInclusive = offset + length - 1U;
        const std::size_t firstSectorIndex = offset / DiscSectorSize;
        const std::size_t lastSectorIndex = (lastByteOffsetInclusive / DiscSectorSize) + 1U;
        const std::size_t sectorCount = lastSectorIndex - firstSectorIndex;
        if (sectorCount > std::numeric_limits<std::size_t>::max() / DiscSectorSize) {
            return false;
        }

        const bool readsWholeSectors = (offset % DiscSectorSize) == 0U && (length % DiscSectorSize) == 0U;
        if (readsWholeSectors) {
            uint8_t* destinationBytes = static_cast<uint8_t*>(destination);
            std::size_t currentOffset = offset;
            std::size_t remainingLength = length;
            while (remainingLength > 0U) {
                const std::size_t transferLength = std::min(remainingLength, MaximumTransferLength);
                if (!ReadAlignedBytes(destinationBytes, currentOffset, transferLength)) {
                    return false;
                }

                destinationBytes += transferLength;
                currentOffset += transferLength;
                remainingLength -= transferLength;
            }

            return true;
        }

        const std::size_t scratchBufferLength = sectorCount * DiscSectorSize;
        uint8_t* scratchBuffer = static_cast<uint8_t*>(memalign(CacheLineSize, scratchBufferLength));
        if (scratchBuffer == nullptr) {
            return false;
        }

        const bool readSucceeded = ReadBytes(
            scratchBuffer,
            firstSectorIndex * DiscSectorSize,
            scratchBufferLength);
        if (readSucceeded) {
            const std::size_t firstSectorByteOffset = offset % DiscSectorSize;
            std::memcpy(destination, scratchBuffer + firstSectorByteOffset, length);
        }

        free(scratchBuffer);
        return readSucceeded;
    }

    /// Completes the active synchronous transfer after acknowledging physical or Nintendont DI interrupt state.
    extern "C" void GameCubeDiscInterruptComplete(
        volatile uint32_t* discInterface,
        uint32_t originalContext,
        uint32_t interruptMask,
        uint32_t interruptStatusMask) {
        static_cast<void>(originalContext);
        static_cast<void>(interruptMask);
        static_cast<void>(interruptStatusMask);
        TryCompleteRead(discInterface);
    }
}
