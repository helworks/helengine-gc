#pragma once

#include "float3.hpp"
#include "float4.hpp"

class Entity;
class Core;

namespace helengine::gamecube {
    /// Installs the first rotating cube validation scene directly into the generated runtime.
    class GameCubeCubeTestSceneInstaller {
    public:
        /// Creates camera, light, and mesh runtime entities that mirror the city cube-test scene.
        static void Install(Core* ownerCore);

        /// Creates one render-only slope comparison scene that exercises rotated cube transforms without any physics runtime involvement.
        static void InstallSlopeScene(Core* ownerCore);
    private:
        /// Creates the runtime camera entity used by the first GameCube cube-test milestone.
        static Entity* CreateCameraEntity(Core* ownerCore);

        /// Creates the runtime camera entity used by the render-only slope comparison scene.
        static Entity* CreateSlopeCameraEntity(Core* ownerCore);

        /// Creates the runtime directional light entity that mirrors the authored cube-test sun.
        static Entity* CreateDirectionalLightEntity(Core* ownerCore);

        /// Creates the rotating cube mesh entity using an in-memory runtime model and material.
        static Entity* CreateCubeEntity(Core* ownerCore);

        /// Creates one transformed cube mesh entity for render-only transform validation.
        static Entity* CreateCubeEntity(Core* ownerCore, float3 localPosition, float3 localScale, float4 localOrientation, const char* materialId);
    };
}
