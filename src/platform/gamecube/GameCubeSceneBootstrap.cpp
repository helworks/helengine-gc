#include "platform/gamecube/GameCubeSceneBootstrap.hpp"

#include "RuntimeSceneCatalog.hpp"
#include "RuntimeSceneCatalogEntry.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"
#include "system/io/file.hpp"
#include "system/io/path.hpp"

namespace helengine::gamecube {
    std::string GameCubeSceneBootstrap::BundledContentRootPath = "tmp/city-cube-test-content";

    std::string GameCubeSceneBootstrap::BundledContentRootWindowsHostPath = "C:/dev/helworks/helengine-gc/tmp/city-cube-test-content";

    std::string GameCubeSceneBootstrap::BundledContentRootWslPath = "/mnt/c/dev/helworks/helengine-gc/tmp/city-cube-test-content";

    std::string GameCubeSceneBootstrap::StartupSceneId = "scenes/rendering/cube_test.helen";

    /// Returns the staged content root and fails if the bundle has not been prepared.
    std::string GameCubeSceneBootstrap::GetValidatedContentRootPath() {
        const std::string relativeRootPath = Path::GetFullPath(BundledContentRootPath);
        if (HasRequiredFiles(relativeRootPath)) {
            return relativeRootPath;
        }

        const std::string windowsHostRootPath = Path::GetFullPath(BundledContentRootWindowsHostPath);
        if (HasRequiredFiles(windowsHostRootPath)) {
            return windowsHostRootPath;
        }

        const std::string wslRootPath = Path::GetFullPath(BundledContentRootWslPath);
        if (HasRequiredFiles(wslRootPath)) {
            return wslRootPath;
        }

        ValidateRequiredFile(relativeRootPath, "cooked/scenes/rendering/cube_test.hasset");
        ValidateRequiredFile(relativeRootPath, "cooked/engine/models/cube.hasset");
        ValidateRequiredFile(relativeRootPath, "cooked/engine/materials/standard.hasset");
        ValidateRequiredFile(relativeRootPath, "cooked/shaders/ForwardStandardShader.dx11.hasset");
        return relativeRootPath;
    }

    /// Creates the single-scene runtime catalog used by the first GameCube renderer milestone.
    RuntimeSceneCatalog* GameCubeSceneBootstrap::CreateSceneCatalog() {
        Array<RuntimeSceneCatalogEntry*>* entries = new Array<RuntimeSceneCatalogEntry*>(1);
        (*entries)[0] = new RuntimeSceneCatalogEntry(StartupSceneId, "cooked/scenes/rendering/cube_test.hasset");
        return new RuntimeSceneCatalog(entries);
    }

    /// Returns whether all required staged files exist under the candidate content root.
    bool GameCubeSceneBootstrap::HasRequiredFiles(std::string rootPath) {
        if (String::IsNullOrWhiteSpace(rootPath)) {
            return false;
        }

        return File::Exists(Path::GetFullPath(Path::Combine(rootPath, "cooked/scenes/rendering/cube_test.hasset")))
            && File::Exists(Path::GetFullPath(Path::Combine(rootPath, "cooked/engine/models/cube.hasset")))
            && File::Exists(Path::GetFullPath(Path::Combine(rootPath, "cooked/engine/materials/standard.hasset")))
            && File::Exists(Path::GetFullPath(Path::Combine(rootPath, "cooked/shaders/ForwardStandardShader.dx11.hasset")));
    }

    /// Verifies one required staged content file exists under the bundle root.
    void GameCubeSceneBootstrap::ValidateRequiredFile(std::string rootPath, std::string relativePath) {
        const std::string fullPath = Path::GetFullPath(Path::Combine(rootPath, relativePath));
        if (!File::Exists(fullPath)) {
            throw new InvalidOperationException(std::string("Required staged GameCube content file is missing: ") + fullPath);
        }
    }
}
