#include "platform/gamecube/GameCubeRenderManager3D.hpp"

#include "RuntimeModel.hpp"

namespace helengine::gamecube {
    /// Creates the GameCube 3D render bridge with the generated base state.
    GameCubeRenderManager3D::GameCubeRenderManager3D()
        : RenderManager3D() {
    }

    /// Builds a placeholder runtime model from generated model asset metadata.
    RuntimeModel* GameCubeRenderManager3D::BuildModelFromRaw(ModelAsset* data) {
        (void)data;
        return new RuntimeModel();
    }
}
