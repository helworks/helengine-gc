#include "platform/gamecube/GameCubeFramePlan.hpp"

#include "IDrawable3D.hpp"
#include "LightComponent.hpp"

namespace helengine::gamecube {
    /// Releases the temporary scene snapshots used to build this frame plan.
    GameCubeFramePlan::~GameCubeFramePlan() {
        if (Cameras != nullptr) {
            delete Cameras;
            Cameras = nullptr;
        }

        if (Drawables != nullptr) {
            delete Drawables;
            Drawables = nullptr;
        }

        if (Lights != nullptr) {
            delete Lights;
            Lights = nullptr;
        }

        ExtractionResult = nullptr;
        DrawableSubmissions = nullptr;
        LightSubmissions = nullptr;
    }
}
