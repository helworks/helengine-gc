#include "platform/gamecube/GameCubeSceneBootstrap.hpp"

#include <cstdlib>

#include <ogc/dvd.h>
#include <ogc/system.h>

#include "InputControlId.hpp"
#include "InputControlKind.hpp"
#include "InputDeviceKind.hpp"
#include "RuntimeSceneCatalog.hpp"
#include "RuntimeSceneCatalogEntry.hpp"
#include "runtime/gamecube_runtime_scene_manifest.hpp"
#include "runtime/runtime_standard_platform_input_manifest.hpp"
#include "runtime/array.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_exceptions.hpp"
#include "StandardPlatformAction.hpp"
#include "StandardPlatformActionBinding.hpp"
#include "StandardPlatformInputConfiguration.hpp"
#include "system/io/file.hpp"
#include "system/io/path.hpp"

namespace helengine::gamecube {
    std::string GameCubeSceneBootstrap::BundledContentRootPath = "tmp/city-demo-disc-main-menu-content";

    std::string GameCubeSceneBootstrap::BundledContentRootWindowsHostPath = "C:/dev/helworks/helengine-gc/tmp/city-demo-disc-main-menu-content";

    std::string GameCubeSceneBootstrap::BundledContentRootWslPath = "/mnt/c/dev/helworks/helengine-gc/tmp/city-demo-disc-main-menu-content";

    std::string GameCubeSceneBootstrap::StartupSceneId = "Scenes/DemoDiscMainMenu.helen";

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

        ValidateRequiredFile(relativeRootPath, "cooked/scenes/DemoDiscMainMenu.hasset");
        ValidateRequiredFile(relativeRootPath, "cooked/fonts/DemoDiscBody.hefont");
        ValidateRequiredFile(relativeRootPath, "cooked/fonts/default.hefont");
        ValidateRequiredFile(relativeRootPath, "cooked/imported/3df8b122ea9a854acc1138581a579c38858b774c8b72e02117d059539873897a");
        return relativeRootPath;
    }

    /// Creates the single-scene runtime catalog used by the current authored GameCube startup milestone.
    RuntimeSceneCatalog* GameCubeSceneBootstrap::CreateSceneCatalog() {
        Array<RuntimeSceneCatalogEntry*>* entries = new Array<RuntimeSceneCatalogEntry*>(1);
        (*entries)[0] = new RuntimeSceneCatalogEntry(StartupSceneId, "cooked/scenes/DemoDiscMainMenu.hasset");
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
        const std::string startupSceneAliasId = "DemoDiscMainMenu";
        bool startupSceneAliasExists = false;
        bool startupSceneSourceExists = false;
        for (std::size_t index = 0; index < entryCount; index++) {
            if (startupSceneAliasId == entries[index].SceneId) {
                startupSceneAliasExists = true;
            }

            if (StartupSceneId == entries[index].SceneId) {
                startupSceneSourceExists = true;
            }
        }

        const bool shouldAddStartupSceneAlias = !startupSceneAliasExists && startupSceneSourceExists;
        const std::size_t runtimeEntryCount = shouldAddStartupSceneAlias ? entryCount + 1U : entryCount;
        Array<RuntimeSceneCatalogEntry*>* runtimeEntries = new Array<RuntimeSceneCatalogEntry*>(static_cast<int32_t>(runtimeEntryCount));
        std::size_t runtimeEntryIndex = 0;
        for (std::size_t index = 0; index < entryCount; index++) {
            SYS_Report(
                "[GC] Packaged manifest entry[%u] scene=%s path=%s\n",
                static_cast<unsigned int>(index),
                entries[index].SceneId,
                entries[index].CookedRelativePath);
            (*runtimeEntries)[static_cast<int32_t>(runtimeEntryIndex)] = new RuntimeSceneCatalogEntry(entries[index].SceneId, entries[index].CookedRelativePath);
            runtimeEntryIndex++;
            if (shouldAddStartupSceneAlias && StartupSceneId == entries[index].SceneId) {
                (*runtimeEntries)[static_cast<int32_t>(runtimeEntryIndex)] = new RuntimeSceneCatalogEntry(startupSceneAliasId, entries[index].CookedRelativePath);
                runtimeEntryIndex++;
            }
        }

        return new RuntimeSceneCatalog(runtimeEntries);
    }

    /// Creates the packaged standard platform input configuration emitted by the GameCube builder.
    StandardPlatformInputConfiguration* GameCubeSceneBootstrap::CreatePackagedStandardPlatformInputConfiguration() {
        std::size_t entryCount = 0;
        const HERuntimeStandardPlatformActionEntry* entries = he_runtime_standard_platform_action_entries(&entryCount);
        SYS_Report("[GC] Packaged standard action entry count: %u\n", static_cast<unsigned int>(entryCount));

        List<StandardPlatformActionBinding*>* bindings = new List<StandardPlatformActionBinding*>(static_cast<int32_t>(entryCount));
        for (std::size_t index = 0; index < entryCount; index++) {
            SYS_Report(
                "[GC] Packaged standard action entry[%u] action=%d deviceKind=%d controlKind=%d deviceIndex=%d controlIndex=%d\n",
                static_cast<unsigned int>(index),
                entries[index].ActionId,
                entries[index].DeviceKind,
                entries[index].ControlKind,
                entries[index].DeviceIndex,
                entries[index].ControlIndex);

            bindings->Add(new StandardPlatformActionBinding(
                static_cast<StandardPlatformAction>(entries[index].ActionId),
                InputControlId(
                    static_cast<InputDeviceKind>(entries[index].DeviceKind),
                    static_cast<InputControlKind>(entries[index].ControlKind),
                    entries[index].DeviceIndex,
                    entries[index].ControlIndex)));
        }

        return new StandardPlatformInputConfiguration(bindings);
    }

    /// Returns the packaged startup scene id emitted by the GameCube builder.
    std::string GameCubeSceneBootstrap::GetPackagedStartupSceneId() {
        const std::string overrideSceneId = GetPackagedStartupSceneIdOverride();
        if (!String::IsNullOrWhiteSpace(overrideSceneId)) {
            SYS_Report("[GC] Packaged startup scene override active: %s\n", overrideSceneId.c_str());
            return overrideSceneId;
        }

        return he_get_runtime_gamecube_startup_scene_id();
    }

    /// Returns one optional packaged startup scene override supplied through the host environment.
    std::string GameCubeSceneBootstrap::GetPackagedStartupSceneIdOverride() {
        const char* environmentValue = std::getenv("HELENGINE_GAMECUBE_PACKAGED_STARTUP_SCENE_ID");
        if (environmentValue == nullptr) {
            return std::string();
        }

        const std::string overrideSceneId = environmentValue;
        if (String::IsNullOrWhiteSpace(overrideSceneId)) {
            return std::string();
        }

        return overrideSceneId;
    }

    /// Returns whether all required staged files exist under the candidate content root.
    bool GameCubeSceneBootstrap::HasRequiredFiles(std::string rootPath) {
        if (String::IsNullOrWhiteSpace(rootPath)) {
            return false;
        }

        return File::Exists(Path::GetFullPath(Path::Combine(rootPath, "cooked/scenes/DemoDiscMainMenu.hasset")))
            && File::Exists(Path::GetFullPath(Path::Combine(rootPath, "cooked/fonts/DemoDiscBody.hefont")))
            && File::Exists(Path::GetFullPath(Path::Combine(rootPath, "cooked/fonts/default.hefont")))
            && File::Exists(Path::GetFullPath(Path::Combine(rootPath, "cooked/imported/3df8b122ea9a854acc1138581a579c38858b774c8b72e02117d059539873897a")));
    }

    /// Verifies one required staged content file exists under the bundle root.
    void GameCubeSceneBootstrap::ValidateRequiredFile(std::string rootPath, std::string relativePath) {
        const std::string fullPath = Path::GetFullPath(Path::Combine(rootPath, relativePath));
        if (!File::Exists(fullPath)) {
            throw new InvalidOperationException(std::string("Required staged GameCube content file is missing: ") + fullPath);
        }
    }
}
