#pragma once

#include <array>
#include <cstdint>

#include <ogc/card.h>

namespace helengine::gamecube {
    /// <summary>
    /// Identifies one durable GameCube host checkpoint written to the optional memory-card diagnostic journal.
    /// </summary>
    enum class GameCubeMemoryCardDiagnosticStage : uint32_t {
        /// <summary>
        /// Indicates that the diagnostic card journal has mounted and opened its backing file.
        /// </summary>
        JournalReady = 1,

        /// <summary>
        /// Indicates that the GameCube controller transport selection has begun.
        /// </summary>
        InputTransportSelectionBegin = 2,

        /// <summary>
        /// Indicates that the GameCube controller transport initialization has completed.
        /// </summary>
        InputTransportInitializationComplete = 3,

        /// <summary>
        /// Indicates that the first Nintendont virtual-pad stub invocation is about to begin.
        /// </summary>
        NintendontPadReadBegin = 4,

        /// <summary>
        /// Indicates that the first Nintendont virtual-pad stub invocation returned.
        /// </summary>
        NintendontPadReadComplete = 5,

        /// <summary>
        /// Indicates that the first generated-core update invocation is about to begin.
        /// </summary>
        CoreUpdateBegin = 6,

        /// <summary>
        /// Indicates that the first generated-core update invocation returned.
        /// </summary>
        CoreUpdateComplete = 7,

        /// <summary>
        /// Indicates that the first generated-core draw invocation is about to begin.
        /// </summary>
        CoreDrawBegin = 8,

        /// <summary>
        /// Indicates that the first generated-core draw invocation returned.
        /// </summary>
        CoreDrawComplete = 9
    };

    /// <summary>
    /// Identifies the first CARD operation that prevented the diagnostic journal from becoming writable.
    /// </summary>
    enum class GameCubeMemoryCardDiagnosticFailure : uint32_t {
        /// <summary>
        /// Indicates that no initialization failure has occurred.
        /// </summary>
        None = 0,

        /// <summary>
        /// Indicates that the CARD subsystem initialization failed.
        /// </summary>
        CardInit = 1,

        /// <summary>
        /// Indicates that Slot A could not be probed.
        /// </summary>
        CardProbe = 2,

        /// <summary>
        /// Indicates that Slot A could not be mounted.
        /// </summary>
        CardMount = 3,

        /// <summary>
        /// Indicates that the mounted card reported an unsupported sector size.
        /// </summary>
        CardSectorSize = 4,

        /// <summary>
        /// Indicates that the existing journal file could not be opened.
        /// </summary>
        CardOpen = 5,

        /// <summary>
        /// Indicates that the journal file could not be created.
        /// </summary>
        CardCreate = 6,

        /// <summary>
        /// Indicates that the existing journal sector could not be read.
        /// </summary>
        CardRead = 7,

        /// <summary>
        /// Indicates that a journal sector could not be written.
        /// </summary>
        CardWrite = 8
    };

    /// <summary>
    /// Persists a bounded sequence of GameCube startup checkpoints to a standard Slot-A memory-card save file.
    /// </summary>
    class GameCubeMemoryCardDiagnosticJournal {
    public:
        /// <summary>
        /// Creates one disabled journal that has not yet touched the GameCube CARD subsystem.
        /// </summary>
        GameCubeMemoryCardDiagnosticJournal();

        /// <summary>
        /// Closes and unmounts the journal card file when it was opened successfully.
        /// </summary>
        ~GameCubeMemoryCardDiagnosticJournal();

        /// <summary>
        /// Mounts Slot A and opens or creates the journal's one-sector backing file.
        /// </summary>
        /// <returns>True when a durable journal is available; otherwise false without preventing game startup.</returns>
        bool Initialize();

        /// <summary>
        /// Appends one diagnostic checkpoint by overwriting the next record in the bounded circular journal.
        /// </summary>
        /// <param name="stage">Host checkpoint that was reached.</param>
        /// <param name="resultCode">Native result code associated with the checkpoint.</param>
        void Record(GameCubeMemoryCardDiagnosticStage stage, int32_t resultCode);

        /// <summary>
        /// Permanently disables journal writes and releases any CARD resources already acquired.
        /// </summary>
        void Disable();

        /// <summary>
        /// Returns whether the journal currently has an open, writable card file.
        /// </summary>
        /// <returns>True when records can be persisted; otherwise false.</returns>
        bool IsAvailable() const;

        /// <summary>
        /// Returns the first CARD operation that prevented journal initialization from succeeding.
        /// </summary>
        /// <returns>The initialization failure, or <see cref="GameCubeMemoryCardDiagnosticFailure::None"/> when initialization succeeded.</returns>
        GameCubeMemoryCardDiagnosticFailure GetInitializationFailure() const;

        /// <summary>
        /// Returns the native CARD result produced by the operation that prevented journal initialization.
        /// </summary>
        /// <returns>The CARD result code, or <c>CARD_ERROR_READY</c> when initialization succeeded.</returns>
        s32 GetInitializationResultCode() const;

    private:
        /// <summary>
        /// Describes the journal file header stored at the beginning of the card sector.
        /// </summary>
        struct JournalHeader {
            /// <summary>
            /// Stores the eight-byte format identifier used to recognize journal data.
            /// </summary>
            char Magic[8];

            /// <summary>
            /// Stores the journal binary format version.
            /// </summary>
            uint32_t Version;

            /// <summary>
            /// Stores the next circular record position.
            /// </summary>
            uint32_t NextRecordIndex;

            /// <summary>
            /// Stores the next monotonically increasing record sequence number.
            /// </summary>
            uint32_t NextSequence;

            /// <summary>
            /// Reserves remaining header bytes for future compatible format revisions.
            /// </summary>
            uint8_t Reserved[12];
        };

        /// <summary>
        /// Describes one fixed-size card journal checkpoint entry.
        /// </summary>
        struct JournalRecord {
            /// <summary>
            /// Stores the monotonically increasing write sequence number.
            /// </summary>
            uint32_t Sequence;

            /// <summary>
            /// Stores the numeric startup stage identifier.
            /// </summary>
            uint32_t Stage;

            /// <summary>
            /// Stores the native result code associated with the stage.
            /// </summary>
            int32_t ResultCode;

            /// <summary>
            /// Reserves bytes for future compatible record fields.
            /// </summary>
            uint32_t Reserved;
        };

        /// <summary>
        /// Initializes the in-memory sector contents with an empty valid journal header.
        /// </summary>
        void ResetJournalData();

        /// <summary>
        /// Writes the complete aligned journal sector to the open memory-card file.
        /// </summary>
        /// <returns>True when the sector write succeeds; otherwise false.</returns>
        bool WriteJournalData();

        /// <summary>
        /// Releases the open file and mounted Slot-A card without changing the disabled state.
        /// </summary>
        void ReleaseCard();

        /// <summary>
        /// Receives Slot-A detach notifications required by the synchronous CARD mount API.
        /// </summary>
        /// <param name="channel">Detached CARD channel.</param>
        /// <param name="result">CARD subsystem detach result.</param>
        static void HandleCardDetach(s32 channel, s32 result);

        /// <summary>
        /// Stores the maximum supported card sector size used by the fixed journal payload buffer.
        /// </summary>
        static constexpr uint32_t MaximumSectorSize = 8192U;

        /// <summary>
        /// Stores the fixed one-file card save name visible to memory-card tooling.
        /// </summary>
        static constexpr const char* JournalFileName = "HELDBG";

        /// <summary>
        /// Stores the four-character owner game code used for the diagnostic save entry.
        /// </summary>
        static constexpr const char* JournalGameCode = "HELD";

        /// <summary>
        /// Stores the two-character owner company code used for the diagnostic save entry.
        /// </summary>
        static constexpr const char* JournalCompanyCode = "HC";

        /// <summary>
        /// Stores the current journal binary format version.
        /// </summary>
        static constexpr uint32_t JournalVersion = 1U;

        /// <summary>
        /// Stores the mount workspace required by libogc's synchronous CARD API.
        /// </summary>
        alignas(32) std::array<uint8_t, CARD_WORKAREA_SIZE> MountWorkArea;

        /// <summary>
        /// Stores the full one-sector journal payload required by aligned CARD reads and writes.
        /// </summary>
        alignas(32) std::array<uint8_t, MaximumSectorSize> JournalData;

        /// <summary>
        /// Stores the currently open Slot-A card file handle.
        /// </summary>
        card_file JournalFile;

        /// <summary>
        /// Stores the mounted card sector size used for every journal read and write.
        /// </summary>
        uint32_t SectorSize;

        /// <summary>
        /// Tracks whether Slot A is currently mounted by this journal instance.
        /// </summary>
        bool IsMounted;

        /// <summary>
        /// Tracks whether the journal file is currently open.
        /// </summary>
        bool IsFileOpen;

        /// <summary>
        /// Tracks whether checkpoint writes remain available.
        /// </summary>
        bool IsAvailableValue;

        /// <summary>
        /// Stores the first CARD operation that prevented this journal from becoming writable.
        /// </summary>
        GameCubeMemoryCardDiagnosticFailure InitializationFailure;

        /// <summary>
        /// Stores the native CARD result produced by the first failed initialization operation.
        /// </summary>
        s32 InitializationResultCode;
    };
}
