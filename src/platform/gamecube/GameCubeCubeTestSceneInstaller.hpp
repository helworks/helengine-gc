#pragma once

class Entity;

namespace helengine::gamecube {
    /// Installs the first rotating cube validation scene directly into the generated runtime.
    class GameCubeCubeTestSceneInstaller {
    public:
        /// Creates camera, light, and mesh runtime entities that mirror the city cube-test scene.
        static void Install();
    private:
        /// Creates the runtime camera entity used by the first GameCube cube-test milestone.
        static Entity* CreateCameraEntity();

        /// Creates the runtime directional light entity that mirrors the authored cube-test sun.
        static Entity* CreateDirectionalLightEntity();

        /// Creates the rotating cube mesh entity using an in-memory runtime model and material.
        static Entity* CreateCubeEntity();
    };
}
