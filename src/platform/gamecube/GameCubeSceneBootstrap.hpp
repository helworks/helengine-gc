#pragma once

#include "runtime/native_string.hpp"

class RuntimeSceneCatalog;

namespace helengine::gamecube {
    /// Declares the authored startup scene and content-root helpers used by direct-DOL and packaged GameCube boot flows.
    class GameCubeSceneBootstrap {
    public:
        /// Relative repo path that must contain the staged cooked content bundle before Dolphin verification.
        static std::string BundledContentRootPath;

        /// Absolute Windows host path used when Dolphin does not launch with the repo root as its working directory.
        static std::string BundledContentRootWindowsHostPath;

        /// Absolute WSL path used for local non-Dolphin validation in the shared workspace.
        static std::string BundledContentRootWslPath;

        /// Stable scene id authored by `city` and expected by the generated runtime scene catalog.
        static std::string StartupSceneId;

        /// Returns the staged content root and fails if the bundle has not been prepared.
        static std::string GetValidatedContentRootPath();

        /// Creates the single-scene runtime catalog used by the first GameCube renderer milestone.
        static RuntimeSceneCatalog* CreateSceneCatalog();

        /// Returns the packaged GameCube content root used by disc-backed startup.
        static std::string GetPackagedContentRootPath();

        /// Initializes the packaged GameCube disc interface before any packaged content file access occurs.
        static bool InitializePackagedDisc();

        /// Creates the packaged runtime scene catalog emitted by the GameCube builder.
        static RuntimeSceneCatalog* CreatePackagedSceneCatalog();

        /// Returns the packaged startup scene id emitted by the GameCube builder.
        static std::string GetPackagedStartupSceneId();
    private:
        /// Returns whether all required staged files exist under the candidate content root.
        static bool HasRequiredFiles(std::string rootPath);

        /// Verifies one required staged content file exists under the bundle root.
        static void ValidateRequiredFile(std::string rootPath, std::string relativePath);
    };
}
