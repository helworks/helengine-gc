#include "platform/gamecube/GameCubeSceneBootstrap.hpp"

#include <ogc/dvd.h>
#include <ogc/system.h>

#include "RuntimeSceneCatalog.hpp"
#include "RuntimeSceneCatalogEntry.hpp"
#include "runtime/gamecube_runtime_scene_manifest.hpp"
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

    /// Returns the packaged GameCube content root used by disc-backed startup.
    std::string GameCubeSceneBootstrap::GetPackagedContentRootPath() {
        return "dvd:/";
    }

    /// Initializes the packaged GameCube DVD interface before the runtime attempts any packaged file access.
    bool GameCubeSceneBootstrap::InitializePackagedDisc() {
        DVD_Init();
        const s32 mountResult = DVD_Mount();
        SYS_Report("[GC] DVD_Mount result: %ld\n", static_cast<long>(mountResult));
        return mountResult >= 0;
    }

    /// Creates the packaged runtime scene catalog emitted by the GameCube builder.
    RuntimeSceneCatalog* GameCubeSceneBootstrap::CreatePackagedSceneCatalog() {
        std::size_t entryCount = 0;
        const HEGameCubeRuntimeSceneEntry* entries = he_get_runtime_gamecube_scene_entries(&entryCount);
        SYS_Report("[GC] Packaged manifest entry count: %u\n", static_cast<unsigned int>(entryCount));
        Array<RuntimeSceneCatalogEntry*>* runtimeEntries = new Array<RuntimeSceneCatalogEntry*>(static_cast<int32_t>(entryCount));
        for (std::size_t index = 0; index < entryCount; index++) {
            SYS_Report(
                "[GC] Packaged manifest entry[%u] scene=%s path=%s\n",
                static_cast<unsigned int>(index),
                entries[index].SceneId,
                entries[index].CookedRelativePath);
            (*runtimeEntries)[static_cast<int32_t>(index)] = new RuntimeSceneCatalogEntry(entries[index].SceneId, entries[index].CookedRelativePath);
        }

        return new RuntimeSceneCatalog(runtimeEntries);
    }

    /// Returns the packaged startup scene id emitted by the GameCube builder.
    std::string GameCubeSceneBootstrap::GetPackagedStartupSceneId() {
        return he_get_runtime_gamecube_startup_scene_id();
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
