#pragma once

#include "RenderManager3D.hpp"

class RuntimeModel;

namespace helengine::gamecube {
    /// Implements the minimal 3D render bridge needed to let the generated core initialize on GameCube.
    class GameCubeRenderManager3D : public RenderManager3D {
    public:
        /// Creates the GameCube 3D render bridge with the generated base state.
        GameCubeRenderManager3D();

        /// Builds a placeholder runtime model from generated model asset metadata.
        RuntimeModel* BuildModelFromRaw(ModelAsset* data) override;
    };
}
