#pragma once

#include "runtime/native_string.hpp"

class RuntimeSceneCatalog;

namespace helengine::gamecube {
    /// Declares the authored startup scene and staged content root used by the first GameCube 3D renderer milestone.
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
    private:
        /// Returns whether all required staged files exist under the candidate content root.
        static bool HasRequiredFiles(std::string rootPath);

        /// Verifies one required staged content file exists under the bundle root.
        static void ValidateRequiredFile(std::string rootPath, std::string relativePath);
    };
}
