#include "platform/gamecube/GameCubeMemoryCardDiagnosticJournal.hpp"

#include <algorithm>
#include <cstring>

namespace helengine::gamecube {
    /// Creates one disabled journal that has not yet touched the GameCube CARD subsystem.
    GameCubeMemoryCardDiagnosticJournal::GameCubeMemoryCardDiagnosticJournal()
        : MountWorkArea {}
        , JournalData {}
        , JournalFile {}
        , SectorSize(0U)
        , IsMounted(false)
        , IsFileOpen(false)
        , IsAvailableValue(false)
        , InitializationFailure(GameCubeMemoryCardDiagnosticFailure::None)
        , InitializationResultCode(CARD_ERROR_READY) {
    }

    /// Closes and unmounts the journal card file when it was opened successfully.
    GameCubeMemoryCardDiagnosticJournal::~GameCubeMemoryCardDiagnosticJournal() {
        ReleaseCard();
    }

    /// Mounts Slot A and opens or creates the journal's one-sector backing file.
    bool GameCubeMemoryCardDiagnosticJournal::Initialize() {
        InitializationFailure = GameCubeMemoryCardDiagnosticFailure::None;
        InitializationResultCode = CARD_ERROR_READY;
        const s32 cardInitializeResult = CARD_Init(JournalGameCode, JournalCompanyCode);
        if (cardInitializeResult < CARD_ERROR_READY) {
            InitializationFailure = GameCubeMemoryCardDiagnosticFailure::CardInit;
            InitializationResultCode = cardInitializeResult;
            Disable();
            return false;
        }

        const s32 cardProbeResult = CARD_Probe(CARD_SLOTA);
        if (cardProbeResult < CARD_ERROR_READY) {
            InitializationFailure = GameCubeMemoryCardDiagnosticFailure::CardProbe;
            InitializationResultCode = cardProbeResult;
            Disable();
            return false;
        }

        const s32 cardMountResult = CARD_Mount(CARD_SLOTA, MountWorkArea.data(), HandleCardDetach);
        if (cardMountResult < CARD_ERROR_READY) {
            InitializationFailure = GameCubeMemoryCardDiagnosticFailure::CardMount;
            InitializationResultCode = cardMountResult;
            Disable();
            return false;
        }
        IsMounted = true;

        if (CARD_GetSectorSize(CARD_SLOTA, &SectorSize) < CARD_ERROR_READY || SectorSize < sizeof(JournalHeader) || SectorSize > MaximumSectorSize) {
            InitializationFailure = GameCubeMemoryCardDiagnosticFailure::CardSectorSize;
            Disable();
            return false;
        }

        s32 openResult = CARD_Open(CARD_SLOTA, JournalFileName, &JournalFile);
        if (openResult == CARD_ERROR_NOFILE) {
            if (CARD_Create(CARD_SLOTA, JournalFileName, SectorSize, &JournalFile) < CARD_ERROR_READY) {
                InitializationFailure = GameCubeMemoryCardDiagnosticFailure::CardCreate;
                Disable();
                return false;
            }
            IsFileOpen = true;
            ResetJournalData();
            if (!WriteJournalData()) {
                InitializationFailure = GameCubeMemoryCardDiagnosticFailure::CardWrite;
                Disable();
                return false;
            }
        } else if (openResult < CARD_ERROR_READY) {
            InitializationFailure = GameCubeMemoryCardDiagnosticFailure::CardOpen;
            Disable();
            return false;
        } else {
            IsFileOpen = true;
            if (CARD_Read(&JournalFile, JournalData.data(), SectorSize, 0U) < CARD_ERROR_READY) {
                InitializationFailure = GameCubeMemoryCardDiagnosticFailure::CardRead;
                Disable();
                return false;
            }

            const JournalHeader* header = reinterpret_cast<const JournalHeader*>(JournalData.data());
            if (std::memcmp(header->Magic, "HEDBG001", sizeof(header->Magic)) != 0 || header->Version != JournalVersion) {
                ResetJournalData();
                if (!WriteJournalData()) {
                    InitializationFailure = GameCubeMemoryCardDiagnosticFailure::CardWrite;
                    Disable();
                    return false;
                }
            }
        }

        IsAvailableValue = true;
        Record(GameCubeMemoryCardDiagnosticStage::JournalReady, CARD_ERROR_READY);
        return true;
    }

    /// Appends one diagnostic checkpoint by overwriting the next record in the bounded circular journal.
    void GameCubeMemoryCardDiagnosticJournal::Record(GameCubeMemoryCardDiagnosticStage stage, int32_t resultCode) {
        if (!IsAvailableValue) {
            return;
        }

        JournalHeader* header = reinterpret_cast<JournalHeader*>(JournalData.data());
        const uint32_t recordCapacity = (SectorSize - sizeof(JournalHeader)) / sizeof(JournalRecord);
        if (recordCapacity == 0U || header->NextRecordIndex >= recordCapacity) {
            ResetJournalData();
            header = reinterpret_cast<JournalHeader*>(JournalData.data());
        }

        JournalRecord* records = reinterpret_cast<JournalRecord*>(JournalData.data() + sizeof(JournalHeader));
        records[header->NextRecordIndex] = JournalRecord {
            header->NextSequence,
            static_cast<uint32_t>(stage),
            resultCode,
            0U
        };
        header->NextRecordIndex = (header->NextRecordIndex + 1U) % recordCapacity;
        header->NextSequence++;

        if (!WriteJournalData()) {
            InitializationFailure = GameCubeMemoryCardDiagnosticFailure::CardWrite;
            Disable();
        }
    }

    /// Permanently disables journal writes and releases any CARD resources already acquired.
    void GameCubeMemoryCardDiagnosticJournal::Disable() {
        IsAvailableValue = false;
        ReleaseCard();
    }

    /// Returns whether the journal currently has an open, writable card file.
    bool GameCubeMemoryCardDiagnosticJournal::IsAvailable() const {
        return IsAvailableValue;
    }

    /// Returns the first CARD operation that prevented journal initialization from succeeding.
    GameCubeMemoryCardDiagnosticFailure GameCubeMemoryCardDiagnosticJournal::GetInitializationFailure() const {
        return InitializationFailure;
    }

    /// Returns the native CARD result produced by the operation that prevented journal initialization.
    s32 GameCubeMemoryCardDiagnosticJournal::GetInitializationResultCode() const {
        return InitializationResultCode;
    }

    /// Initializes the in-memory sector contents with an empty valid journal header.
    void GameCubeMemoryCardDiagnosticJournal::ResetJournalData() {
        std::fill(JournalData.begin(), JournalData.end(), 0U);
        JournalHeader* header = reinterpret_cast<JournalHeader*>(JournalData.data());
        std::memcpy(header->Magic, "HEDBG001", sizeof(header->Magic));
        header->Version = JournalVersion;
        header->NextRecordIndex = 0U;
        header->NextSequence = 1U;
    }

    /// Writes the complete aligned journal sector to the open memory-card file.
    bool GameCubeMemoryCardDiagnosticJournal::WriteJournalData() {
        return IsFileOpen && CARD_Write(&JournalFile, JournalData.data(), SectorSize, 0U) >= CARD_ERROR_READY;
    }

    /// Releases the open file and mounted Slot-A card without changing the disabled state.
    void GameCubeMemoryCardDiagnosticJournal::ReleaseCard() {
        if (IsFileOpen) {
            CARD_Close(&JournalFile);
            IsFileOpen = false;
        }

        if (IsMounted) {
            CARD_Unmount(CARD_SLOTA);
            IsMounted = false;
        }
    }

    /// Receives Slot-A detach notifications required by the synchronous CARD mount API.
    void GameCubeMemoryCardDiagnosticJournal::HandleCardDetach(s32 channel, s32 result) {
        (void)channel;
        (void)result;
    }
}
