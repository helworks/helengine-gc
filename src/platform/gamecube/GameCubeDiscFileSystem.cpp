#include "platform/gamecube/GameCubeDiscFileSystem.hpp"

#include <cctype>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <limits>
#include <malloc.h>
#include <stdexcept>
#include <string>
#include <vector>

#include <ogc/system.h>

#include "platform/gamecube/GameCubeApplication.hpp"
#include "platform/gamecube/GameCubeDiscReader.hpp"
#include "system/io/file-stream.hpp"

namespace helengine::gamecube {
    namespace {
        constexpr const char* BuildStamp = __DATE__ " " __TIME__;
        constexpr std::size_t DiscHeaderReadLength = 0x440;
        constexpr std::size_t DiscSectorSize = 2048;
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
        /// Limits each diagnostic DVD request to one physical GameCube sector so a multi-sector DI stall is distinguishable from an invalid request context.
        constexpr std::size_t MaximumSectorsPerRead = 1;
#else
        constexpr std::size_t MaximumSectorsPerRead = 32;
#endif
        constexpr std::size_t FstEntrySize = 12;
        constexpr uint32_t DiscMagic = 0xC2339F3D;

        struct GameCubeDiscFileEntry {
            std::string Path;
            uint32_t DiscOffset;
            uint32_t FileSize;
        };

        std::vector<uint8_t> FstBytes;
        std::vector<GameCubeDiscFileEntry> FileEntries;
        bool IndexLoaded = false;
        std::size_t Align32(std::size_t value) {
            return (value + 31U) & ~static_cast<std::size_t>(31U);
        }

        uint32_t ReadBigEndianU32(const uint8_t* bytes) {
            return (static_cast<uint32_t>(bytes[0]) << 24)
                | (static_cast<uint32_t>(bytes[1]) << 16)
                | (static_cast<uint32_t>(bytes[2]) << 8)
                | static_cast<uint32_t>(bytes[3]);
        }

        /// Reads an arbitrary byte range through the GameCube DI DMA interface using aligned sector transfers.
        bool ReadDiscRange(void* destination, std::size_t offset, std::size_t length) {
            if (destination == nullptr) {
                return false;
            } else if (length == 0U) {
                return true;
            }

            const std::size_t firstSectorIndex = offset / DiscSectorSize;
            const std::size_t lastByteOffsetExclusive = offset + length;
            const std::size_t lastSectorIndex = (lastByteOffsetExclusive + (DiscSectorSize - 1U)) / DiscSectorSize;
            const std::size_t sectorCount = lastSectorIndex - firstSectorIndex;
            const std::size_t scratchBufferLength = MaximumSectorsPerRead * DiscSectorSize;
            uint8_t* scratchBuffer = static_cast<uint8_t*>(memalign(32, Align32(scratchBufferLength)));
            if (scratchBuffer == nullptr) {
                SYS_Report("[GC] DVD scratch-buffer allocation failed.\n");
                return false;
            }

            bool readSucceeded = true;
            for (std::size_t sectorsRead = 0U; readSucceeded && sectorsRead < sectorCount;) {
                const std::size_t sectorsToRead = std::min(MaximumSectorsPerRead, sectorCount - sectorsRead);
                const std::size_t sectorIndex = firstSectorIndex + sectorsRead;
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
                constexpr std::size_t DirectDiagnosticSectorMask = 0x0FFFU;
                ReportDirectFrameDiagnosticCode(static_cast<uint16_t>(0xF000U | (sectorIndex & DirectDiagnosticSectorMask)));
#endif
                if (!GameCubeDiscReader::ReadBytes(scratchBuffer, sectorIndex * DiscSectorSize, sectorsToRead * DiscSectorSize)) {
                    SYS_Report(
                        "[GC] DI sector read failed sector=%lu count=%lu\n",
                        static_cast<unsigned long>(sectorIndex),
                        static_cast<unsigned long>(sectorsToRead));
                    readSucceeded = false;
                    break;
                }
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
                ReportDirectFrameDiagnosticCode(static_cast<uint16_t>(0xF240U + sectorsToRead));
                ReportDirectFrameDiagnosticCode(0xF101U);
#endif

                const std::size_t chunkStartOffset = sectorIndex * DiscSectorSize;
                const std::size_t chunkEndOffset = chunkStartOffset + (sectorsToRead * DiscSectorSize);
                const std::size_t copyStartOffset = std::max(offset, chunkStartOffset);
                const std::size_t copyEndOffset = std::min(lastByteOffsetExclusive, chunkEndOffset);
                if (copyStartOffset < copyEndOffset) {
                    std::memcpy(
                        static_cast<uint8_t*>(destination) + (copyStartOffset - offset),
                        scratchBuffer + (copyStartOffset - chunkStartOffset),
                        copyEndOffset - copyStartOffset);
                }

                sectorsRead += sectorsToRead;
            }

            free(scratchBuffer);
            return readSucceeded;
        }
    }

    /// Returns whether the supplied path should be resolved from the packaged GameCube disc image.
    bool GameCubeDiscFileSystem::CanHandlePath(const char* path) {
        return path != nullptr && std::strncmp(path, "dvd:/", 5) == 0;
    }

    /// Returns whether the supplied packaged GameCube disc path resolves to a file entry in the indexed disc FST.
    bool GameCubeDiscFileSystem::Exists(const char* path) {
        std::size_t discOffset = 0;
        std::size_t fileSize = 0;
        const bool exists = TryResolveFile(path, discOffset, fileSize);
        SYS_Report("[GC] GameCubeDiscFileSystem::Exists path=%s result=%d offset=%lu size=%lu\n", path != nullptr ? path : "<null>", exists ? 1 : 0, static_cast<unsigned long>(discOffset), static_cast<unsigned long>(fileSize));
        return exists;
    }

    /// Opens one packaged GameCube disc file as a read-only memory-backed stream loaded from DVD sectors.
    FileStream* GameCubeDiscFileSystem::OpenRead(const char* path) {
        std::size_t discOffset = 0;
        std::size_t fileSize = 0;
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
        ReportDirectFrameDiagnosticCode(0xF001U);
#endif
        if (!TryResolveFile(path, discOffset, fileSize)) {
            throw std::runtime_error(std::string("Packaged GameCube disc path was not found: ") + (path != nullptr ? path : "<null>"));
        }

        if (fileSize > static_cast<std::size_t>(std::numeric_limits<uint32_t>::max())) {
            throw std::runtime_error("Packaged GameCube disc file exceeds the supported read size.");
        }

        const std::size_t alignedSize = Align32(fileSize);
        uint8_t* buffer = static_cast<uint8_t*>(memalign(32, alignedSize));
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
        ReportDirectFrameDiagnosticCode(0xF002U);
#endif
        if (buffer == nullptr) {
            SYS_Report(
                "[GC] Packaged file allocation failed path=%s size=%lu alignedSize=%lu\n",
                path != nullptr ? path : "<null>",
                static_cast<unsigned long>(fileSize),
                static_cast<unsigned long>(alignedSize));
            throw std::runtime_error("Could not allocate a packaged GameCube disc read buffer.");
        }

        std::memset(buffer, 0, alignedSize);
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
        ReportDirectFrameDiagnosticCode(0xF003U);
#endif
        SYS_Report("[GC] Build stamp before dvd read: %s\n", BuildStamp);
        SYS_Report("[GC] GameCubeDiscFileSystem reading indexed path=%s offset=%lu size=%lu\n", path != nullptr ? path : "<null>", static_cast<unsigned long>(discOffset), static_cast<unsigned long>(fileSize));
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
        ReportDirectFrameDiagnosticCode(0xF004U);
#endif
        if (!ReadDiscRange(buffer, discOffset, fileSize)) {
            free(buffer);
            throw std::runtime_error("GameCube DVD sector read failed for a packaged content file.");
        }
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
        ReportDirectFrameDiagnosticCode(0xF005U);
#endif
        if (path != nullptr
            && (std::strcmp(path, "dvd:/cooked/engine/models/cube.hasset") == 0
                || std::strcmp(path, "dvd:/cooked/engine/materials/standard.hasset") == 0)) {
            const unsigned int valueKindLowByte = fileSize > 11U ? static_cast<unsigned int>(buffer[11]) : 0U;
            SYS_Report(
                "[GC] Packaged asset stream head path=%s bytes=%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X valueKindLow=%u\n",
                path,
                fileSize > 0U ? static_cast<unsigned int>(buffer[0]) : 0U,
                fileSize > 1U ? static_cast<unsigned int>(buffer[1]) : 0U,
                fileSize > 2U ? static_cast<unsigned int>(buffer[2]) : 0U,
                fileSize > 3U ? static_cast<unsigned int>(buffer[3]) : 0U,
                fileSize > 4U ? static_cast<unsigned int>(buffer[4]) : 0U,
                fileSize > 5U ? static_cast<unsigned int>(buffer[5]) : 0U,
                fileSize > 6U ? static_cast<unsigned int>(buffer[6]) : 0U,
                fileSize > 7U ? static_cast<unsigned int>(buffer[7]) : 0U,
                fileSize > 8U ? static_cast<unsigned int>(buffer[8]) : 0U,
                fileSize > 9U ? static_cast<unsigned int>(buffer[9]) : 0U,
                fileSize > 10U ? static_cast<unsigned int>(buffer[10]) : 0U,
                fileSize > 11U ? static_cast<unsigned int>(buffer[11]) : 0U,
                valueKindLowByte);
        }

        FileStream* stream = nullptr;
        try {
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
            ReportDirectFrameDiagnosticCode(0xF006U);
#endif
            stream = new FileStream(buffer, fileSize);
        } catch (...) {
            free(buffer);
            throw;
        }

        // FileStream copies memory-backed input, so this DVD staging allocation must not outlive the call.
        free(buffer);
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
        ReportDirectFrameDiagnosticCode(0xF007U);
#endif
        return stream;
    }

    /// Ensures the packaged disc FST has been loaded into memory before path resolution occurs.
    bool GameCubeDiscFileSystem::EnsureIndexLoaded() {
        if (IndexLoaded) {
            return true;
        }

        return LoadIndex();
    }

    /// Reads the disc header and FST, then indexes all packaged file entries.
    bool GameCubeDiscFileSystem::LoadIndex() {
        alignas(32) uint8_t discHeader[DiscHeaderReadLength];
        if (!ReadDiscRange(discHeader, 0U, DiscHeaderReadLength)) {
            SYS_Report("[GC] Failed to read the GameCube disc header for FST indexing.\n");
            return false;
        }

        const uint32_t discMagic = ReadBigEndianU32(discHeader + 0x1C);
        SYS_Report(
            "[GC] Disc header bytes @1C = %02X %02X %02X %02X magic=%08lX expected=%08lX\n",
            static_cast<unsigned int>(discHeader[0x1C]),
            static_cast<unsigned int>(discHeader[0x1D]),
            static_cast<unsigned int>(discHeader[0x1E]),
            static_cast<unsigned int>(discHeader[0x1F]),
            static_cast<unsigned long>(discMagic),
            static_cast<unsigned long>(DiscMagic));
        if (discMagic != DiscMagic) {
            SYS_Report("[GC] GameCube disc magic did not match while loading the packaged FST.\n");
            return false;
        }

        const uint32_t fstOffset = ReadBigEndianU32(discHeader + 0x424);
        const uint32_t fstSize = ReadBigEndianU32(discHeader + 0x428);
        if (fstOffset == 0U || fstSize < FstEntrySize) {
            SYS_Report("[GC] Packaged GameCube disc FST header fields were invalid.\n");
            return false;
        }

        uint8_t* alignedFstBuffer = static_cast<uint8_t*>(memalign(32, Align32(fstSize)));
        if (alignedFstBuffer == nullptr) {
            SYS_Report("[GC] Could not allocate an aligned packaged GameCube FST buffer.\n");
            return false;
        }

        if (!ReadDiscRange(alignedFstBuffer, fstOffset, fstSize)) {
            free(alignedFstBuffer);
            SYS_Report("[GC] Failed to read the packaged GameCube FST.\n");
            return false;
        }

        FstBytes.assign(alignedFstBuffer, alignedFstBuffer + fstSize);
        free(alignedFstBuffer);
        FileEntries.clear();
        if (!IndexDirectory(0U, "dvd:/")) {
            FileEntries.clear();
            SYS_Report("[GC] Packaged GameCube FST contains an invalid entry or name range.\n");
            return false;
        }

        IndexLoaded = true;
        SYS_Report("[GC] Indexed %lu packaged GameCube disc files.\n", static_cast<unsigned long>(FileEntries.size()));
        return true;
    }

    /// Recursively indexes one validated FST directory entry and all of its children.
    bool GameCubeDiscFileSystem::IndexDirectory(std::size_t directoryEntryIndex, std::string directoryPath) {
        if (FstBytes.size() < FstEntrySize) {
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
            ReportDirectFrameDiagnosticCode(0xF801U);
#endif
            return false;
        }

        const std::size_t entryCount = ReadBigEndianU32(FstBytes.data() + 8);
        if (entryCount == 0U
            || entryCount > FstBytes.size() / FstEntrySize
            || directoryEntryIndex >= entryCount) {
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
            ReportDirectFrameDiagnosticCode(0xF802U);
#endif
            return false;
        }

        const std::size_t directoryOffset = directoryEntryIndex * FstEntrySize;
        const std::size_t directoryEndIndex = ReadBigEndianU32(FstBytes.data() + directoryOffset + 8);
        if (directoryEndIndex <= directoryEntryIndex || directoryEndIndex > entryCount) {
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
            ReportDirectFrameDiagnosticCode(0xF803U);
#endif
            return false;
        }

        for (std::size_t entryIndex = directoryEntryIndex + 1; entryIndex < directoryEndIndex; entryIndex++) {
            const std::size_t entryOffset = entryIndex * FstEntrySize;
            const bool isDirectory = FstBytes[entryOffset] != 0;
            std::string entryName;
            if (!TryReadEntryName(entryIndex, entryName)) {
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
                ReportDirectFrameDiagnosticCode(0xF804U);
#endif
                return false;
            }

            const std::string entryPath = directoryPath == "dvd:/"
                ? directoryPath + entryName
                : directoryPath + "/" + entryName;

            if (isDirectory) {
                const uint32_t parentDirectoryIndex = ReadBigEndianU32(FstBytes.data() + entryOffset + 4);
                if (parentDirectoryIndex == directoryEntryIndex) {
                    if (!IndexDirectory(entryIndex, NormalizePath(entryPath.c_str()))) {
                        return false;
                    }

                    entryIndex = ReadBigEndianU32(FstBytes.data() + entryOffset + 8) - 1U;
                }

                continue;
            }

            FileEntries.push_back(GameCubeDiscFileEntry {
                NormalizePath(entryPath.c_str()),
                ReadBigEndianU32(FstBytes.data() + entryOffset + 4),
                ReadBigEndianU32(FstBytes.data() + entryOffset + 8)
            });
        }

        return true;
    }

    /// Reads one FST entry name only when its bounded byte range and terminating character are present in the loaded FST.
    bool GameCubeDiscFileSystem::TryReadEntryName(std::size_t entryIndex, std::string& entryName) {
        entryName.clear();
        if (FstBytes.size() < FstEntrySize) {
            return false;
        }

        const std::size_t entryCount = ReadBigEndianU32(FstBytes.data() + 8);
        if (entryCount == 0U || entryCount > FstBytes.size() / FstEntrySize || entryIndex >= entryCount) {
            return false;
        }

        const std::size_t stringTableOffset = entryCount * FstEntrySize;
        if (stringTableOffset >= FstBytes.size()) {
            return false;
        }

        const std::size_t entryOffset = entryIndex * FstEntrySize;
        const uint32_t nameOffset = (static_cast<uint32_t>(FstBytes[entryOffset + 1]) << 16)
            | (static_cast<uint32_t>(FstBytes[entryOffset + 2]) << 8)
            | static_cast<uint32_t>(FstBytes[entryOffset + 3]);
        if (nameOffset >= FstBytes.size() - stringTableOffset) {
            return false;
        }

        const char* const entryNameStart = reinterpret_cast<const char*>(FstBytes.data() + stringTableOffset + nameOffset);
        const std::size_t entryNameMaximumLength = FstBytes.size() - stringTableOffset - nameOffset;
        const void* const entryNameTerminator = std::memchr(entryNameStart, '\0', entryNameMaximumLength);
        if (entryNameTerminator == nullptr) {
            return false;
        }

        entryName.assign(entryNameStart, static_cast<const char*>(entryNameTerminator) - entryNameStart);
        return true;
    }

    /// Normalizes one `dvd:/...` path into the slash form used by indexed entries.
    std::string GameCubeDiscFileSystem::NormalizePath(const char* path) {
        std::string normalizedPath = path != nullptr ? path : "";
        std::replace(normalizedPath.begin(), normalizedPath.end(), '\\', '/');
        std::transform(
            normalizedPath.begin(),
            normalizedPath.end(),
            normalizedPath.begin(),
            [](unsigned char character) {
                return static_cast<char>(std::tolower(character));
            });
        while (normalizedPath.size() > 5U && normalizedPath.back() == '/') {
            normalizedPath.pop_back();
        }

        return normalizedPath;
    }

    /// Resolves one packaged file path to its disc offset and byte length.
    bool GameCubeDiscFileSystem::TryResolveFile(const char* path, std::size_t& discOffset, std::size_t& fileSize) {
        discOffset = 0;
        fileSize = 0;
        if (!CanHandlePath(path) || !EnsureIndexLoaded()) {
            return false;
        }

        const std::string normalizedPath = NormalizePath(path);
        for (std::size_t entryIndex = 0; entryIndex < FileEntries.size(); entryIndex++) {
            if (FileEntries[entryIndex].Path != normalizedPath) {
                continue;
            }

            discOffset = FileEntries[entryIndex].DiscOffset;
            fileSize = FileEntries[entryIndex].FileSize;
            return true;
        }

        return false;
    }
}
