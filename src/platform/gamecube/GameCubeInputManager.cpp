#include "platform/gamecube/GameCubeInputManager.hpp"
#if HELENGINE_GAMECUBE_MEMORY_CARD_DIAGNOSTIC_JOURNAL
#include "platform/gamecube/GameCubeMemoryCardDiagnosticJournal.hpp"
#endif

#include <gccore.h>

#include "InputGamepadButton.hpp"
#include "InputGamepadState.hpp"
#include "runtime/array.hpp"

namespace helengine::gamecube {
    /// Creates the GameCube input backend with background input disabled.
    GameCubeInputManager::GameCubeInputManager(GameCubeMemoryCardDiagnosticJournal* diagnosticJournal) {
        static_cast<void>(diagnosticJournal);
    }

    /// Releases the GameCube input backend.
    GameCubeInputManager::~GameCubeInputManager() {
    }

    /// Initializes the standard controller transport used by GameCube software and Nintendont's patched hardware interface.
    void GameCubeInputManager::InitializePlatformInput(GameCubeMemoryCardDiagnosticJournal* diagnosticJournal) {
#if HELENGINE_GAMECUBE_MEMORY_CARD_DIAGNOSTIC_JOURNAL
        if (diagnosticJournal != nullptr) {
            diagnosticJournal->Record(GameCubeMemoryCardDiagnosticStage::InputTransportSelectionBegin, 0);
        }
#else
        static_cast<void>(diagnosticJournal);
#endif

        PAD_Init();

#if HELENGINE_GAMECUBE_MEMORY_CARD_DIAGNOSTIC_JOURNAL
        if (diagnosticJournal != nullptr) {
            diagnosticJournal->Record(GameCubeMemoryCardDiagnosticStage::InputTransportInitializationComplete, 0);
        }
#endif
    }

    /// Captures one bootstrap input frame with one shared gamepad state populated from controller port zero.
    InputFrameState GameCubeInputManager::CaptureFrame() {
        PADStatus padStatus {};
        const uint32_t connectedChannels = PAD_ScanPads();
        padStatus.button = PAD_ButtonsHeld(0);
        padStatus.stickX = PAD_StickX(0);
        padStatus.stickY = PAD_StickY(0);
        padStatus.substickX = PAD_SubStickX(0);
        padStatus.substickY = PAD_SubStickY(0);
        padStatus.triggerL = PAD_TriggerL(0);
        padStatus.triggerR = PAD_TriggerR(0);
        const bool hasActivePort0State = padStatus.button != 0U || padStatus.stickX != 0 || padStatus.stickY != 0 || padStatus.substickX != 0 || padStatus.substickY != 0 || padStatus.triggerL != 0 || padStatus.triggerR != 0;
        const bool port0Connected = (connectedChannels & PAD_CHAN0_BIT) != 0U || hasActivePort0State;
        padStatus.err = port0Connected ? PAD_ERR_NONE : PAD_ERR_NO_CONTROLLER;

#if HELENGINE_GAMECUBE_INPUT_TRACE_DIAGNOSTIC
        static bool hasReportedInput = false;
        if (!hasReportedInput && hasActivePort0State) {
            SYS_Report("[GC] Input trace port0 scan=0x%08X buttons=0x%04X stick=(%d,%d) cstick=(%d,%d) triggers=(%d,%d) connected=%d\n", connectedChannels, padStatus.button, padStatus.stickX, padStatus.stickY, padStatus.substickX, padStatus.substickY, padStatus.triggerL, padStatus.triggerR, padStatus.err == PAD_ERR_NONE ? 1 : 0);
            hasReportedInput = true;
        }
#endif

        InputFrameState frame;
        frame.set_GamepadCount(1);

        Array<InputGamepadState>* gamepads = new Array<InputGamepadState>(1);
        InputGamepadState gamepadState;
        gamepadState.set_Connected(padStatus.err == PAD_ERR_NONE);

        gamepadState.SetButtonDown(InputGamepadButton::DPadUp, (padStatus.button & PAD_BUTTON_UP) != 0);
        gamepadState.SetButtonDown(InputGamepadButton::DPadDown, (padStatus.button & PAD_BUTTON_DOWN) != 0);
        gamepadState.SetButtonDown(InputGamepadButton::DPadLeft, (padStatus.button & PAD_BUTTON_LEFT) != 0);
        gamepadState.SetButtonDown(InputGamepadButton::DPadRight, (padStatus.button & PAD_BUTTON_RIGHT) != 0);
        gamepadState.SetButtonDown(InputGamepadButton::South, (padStatus.button & PAD_BUTTON_A) != 0);
        gamepadState.SetButtonDown(InputGamepadButton::East, (padStatus.button & PAD_BUTTON_B) != 0);
        gamepadState.SetButtonDown(InputGamepadButton::West, (padStatus.button & PAD_BUTTON_X) != 0);
        gamepadState.SetButtonDown(InputGamepadButton::North, (padStatus.button & PAD_BUTTON_Y) != 0);
        gamepadState.SetButtonDown(InputGamepadButton::LeftShoulder, (padStatus.button & PAD_TRIGGER_L) != 0);
        gamepadState.SetButtonDown(InputGamepadButton::RightShoulder, (padStatus.button & PAD_TRIGGER_R) != 0);
        gamepadState.SetButtonDown(InputGamepadButton::Start, (padStatus.button & PAD_BUTTON_START) != 0);
        gamepadState.SetButtonDown(InputGamepadButton::Select, (padStatus.button & PAD_TRIGGER_Z) != 0);
        gamepadState.set_LeftStickX(static_cast<int16_t>(padStatus.stickX * 256));
        gamepadState.set_LeftStickY(static_cast<int16_t>(-padStatus.stickY * 256));
        gamepadState.set_RightStickX(static_cast<int16_t>(padStatus.substickX * 256));
        gamepadState.set_RightStickY(static_cast<int16_t>(padStatus.substickY * 256));
        gamepadState.set_LeftTrigger(static_cast<int16_t>(padStatus.triggerL * 256));
        gamepadState.set_RightTrigger(static_cast<int16_t>(padStatus.triggerR * 256));

        (*gamepads)[0] = gamepadState;
        frame.set_Gamepads(gamepads);
        return frame;
    }
}
