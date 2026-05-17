# Platform-Owned Asset Cook Work Items Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a generic editor-emitted platform-owned asset cook work-item contract, then use it for GameCube textures and font atlas textures so GameCube-native texture bytes are cooked by the GameCube builder, not by the shared editor or runtime.

**Architecture:** Extend the shared build manifest with a first-class work-item collection that describes builder-owned asset transforms. Teach the editor build graph to emit those work items from platform capabilities and resolved processor settings, then teach `helengine-gc` to execute GameCube texture/font work items and stage the outputs as final runtime artifacts. Runtime will load those final artifacts directly.

**Tech Stack:** C# (.NET 9), shared `helengine.baseplatform` manifest/build graph code, `helengine.editor` scene packaging/build graph, `helengine-gc` builder/runtime, focused xUnit tests, editor-owned GameCube CLI build.

---

### Task 1: Add Shared Platform Cook Work Item Manifest Types

**Files:**
- Create: `C:/dev/helworks/helengine/engine/helengine.baseplatform/Manifest/PlatformCookWorkItem.cs`
- Create: `C:/dev/helworks/helengine/engine/helengine.baseplatform/Manifest/PlatformCookWorkItemMetadata.cs`
- Modify: `C:/dev/helworks/helengine/engine/helengine.baseplatform/Manifest/PlatformBuildManifest.cs`
- Test: `C:/dev/helworks/helengine/engine/helengine.baseplatform.tests/Builders/PlatformBuildManifestTests.cs`
- Test: `C:/dev/helworks/helengine/engine/helengine.baseplatform.tests/Builders/PlatformBuildRequestCodegenTests.cs`
- Test: `C:/dev/helworks/helengine/engine/helengine.baseplatform.tests/Builders/PlatformBuildRequestMultiTargetTests.cs`

- [ ] **Step 1: Write the failing manifest test for the new work-item collection**

```csharp
[Fact]
public void PlatformBuildManifest_preserves_platform_cook_work_items() {
    PlatformCookWorkItem workItem = new PlatformCookWorkItem(
        "gc-texture:logo",
        "Images/Menu/helengine-logo.png",
        "texture",
        "gamecube",
        "runtime-texture",
        "cooked/imported/helengine-logo.gc.hasset",
        "texture:helengine-logo",
        "sha256:source",
        "sha256:settings",
        "{\"colorFormat\":\"GxRgb5A3\",\"maxResolution\":256}",
        [new PlatformCookWorkItemMetadata("source-asset-id", "Images/Menu/helengine-logo.png")]);

    PlatformBuildManifest manifest = new(
        1,
        "city",
        "1.0.0",
        "1.0.0",
        "gamecube",
        "1.0.0",
        "Scenes/DemoDiscMainMenu.helen",
        Array.Empty<PlatformBuildScene>(),
        Array.Empty<PlatformBuildAsset>(),
        Array.Empty<PlatformBuildArtifact>(),
        Array.Empty<PlatformBuildCodeModule>(),
        Array.Empty<PlatformArtifactPlacement>(),
        new PlatformContainerWritePlan(string.Empty, Array.Empty<PlatformContainerArtifact>()),
        [workItem]);

    Assert.Single(manifest.PlatformCookWorkItems);
    Assert.Equal("gc-texture:logo", manifest.PlatformCookWorkItems[0].WorkItemId);
    Assert.Equal("texture", manifest.PlatformCookWorkItems[0].SourceAssetKind);
    Assert.Equal("runtime-texture", manifest.PlatformCookWorkItems[0].TargetArtifactKind);
}
```

- [ ] **Step 2: Run the baseplatform manifest tests to verify the new API is missing**

Run: `rtk dotnet test C:/dev/helworks/helengine/engine/helengine.baseplatform.tests/helengine.baseplatform.tests.csproj --filter "FullyQualifiedName~PlatformBuildManifestTests|FullyQualifiedName~PlatformBuildRequestCodegenTests|FullyQualifiedName~PlatformBuildRequestMultiTargetTests" -v minimal -p:BuildInParallel=false -p:UseSharedCompilation=false`

Expected: FAIL because `PlatformCookWorkItem` and the new manifest constructor/property do not exist yet.

- [ ] **Step 3: Add the shared manifest types and manifest collection**

```csharp
namespace helengine.baseplatform.Manifest;

/// <summary>
/// Describes one builder-owned platform cook task emitted by the editor-owned build graph.
/// </summary>
public sealed class PlatformCookWorkItem {
    public PlatformCookWorkItem(
        string workItemId,
        string sourceAssetPath,
        string sourceAssetKind,
        string targetPlatformId,
        string targetArtifactKind,
        string outputRelativePath,
        string outputLogicalArtifactId,
        string sourceContentHash,
        string settingsHash,
        string serializedPlatformSettings,
        PlatformCookWorkItemMetadata[] metadata) {
        // validate all required strings and null collections
    }

    public string WorkItemId { get; }
    public string SourceAssetPath { get; }
    public string SourceAssetKind { get; }
    public string TargetPlatformId { get; }
    public string TargetArtifactKind { get; }
    public string OutputRelativePath { get; }
    public string OutputLogicalArtifactId { get; }
    public string SourceContentHash { get; }
    public string SettingsHash { get; }
    public string SerializedPlatformSettings { get; }
    public PlatformCookWorkItemMetadata[] Metadata { get; }
}
```

```csharp
public class PlatformBuildManifest {
    public PlatformBuildManifest(
        int manifestVersion,
        string projectId,
        string projectVersion,
        string requiredEngineVersion,
        string platformName,
        string platformVersion,
        string startupSceneId,
        PlatformBuildScene[] scenes,
        PlatformBuildAsset[] looseAssets,
        PlatformBuildArtifact[] cookedArtifacts,
        PlatformBuildCodeModule[] codeModules,
        PlatformArtifactPlacement[] artifactPlacements,
        PlatformContainerWritePlan containerWritePlan,
        PlatformCookWorkItem[] platformCookWorkItems) {
        PlatformCookWorkItems = [.. platformCookWorkItems];
    }

    public PlatformCookWorkItem[] PlatformCookWorkItems { get; }
}
```

- [ ] **Step 4: Update existing manifest/request tests to use the new constructor shape and assert empties by default**

```csharp
Assert.Empty(manifest.PlatformCookWorkItems);
```

- [ ] **Step 5: Run the shared baseplatform test slice**

Run: `rtk dotnet test C:/dev/helworks/helengine/engine/helengine.baseplatform.tests/helengine.baseplatform.tests.csproj --filter "FullyQualifiedName~PlatformBuildManifestTests|FullyQualifiedName~PlatformBuildRequestCodegenTests|FullyQualifiedName~PlatformBuildRequestMultiTargetTests" -v minimal -p:BuildInParallel=false -p:UseSharedCompilation=false`

Expected: PASS.

- [ ] **Step 6: Commit the shared manifest contract**

```bash
git -C C:/dev/helworks/helengine add engine/helengine.baseplatform/Manifest/PlatformCookWorkItem.cs engine/helengine.baseplatform/Manifest/PlatformCookWorkItemMetadata.cs engine/helengine.baseplatform/Manifest/PlatformBuildManifest.cs engine/helengine.baseplatform.tests/Builders/PlatformBuildManifestTests.cs engine/helengine.baseplatform.tests/Builders/PlatformBuildRequestCodegenTests.cs engine/helengine.baseplatform.tests/Builders/PlatformBuildRequestMultiTargetTests.cs

git -C C:/dev/helworks/helengine commit -m "Add platform cook work item manifest contract"
```

### Task 2: Add Generic Platform Capability Metadata For Builder-Owned Asset Kinds

**Files:**
- Create: `C:/dev/helworks/helengine/engine/helengine.baseplatform/Definitions/PlatformAssetCookOwnershipKind.cs`
- Create: `C:/dev/helworks/helengine/engine/helengine.baseplatform/Definitions/PlatformAssetCookCapabilityDefinition.cs`
- Modify: `C:/dev/helworks/helengine/engine/helengine.baseplatform/Definitions/PlatformDefinition.cs`
- Modify: `builder/GameCubePlatformDefinitionFactory.cs`
- Test: `C:/dev/helworks/helengine/engine/helengine.baseplatform.tests/Builders/IPlatformAssetBuilderMetadataTests.cs`
- Test: `builder.tests/GameCubePackagedBuildWorkspaceTests.cs`

- [ ] **Step 1: Write the failing metadata test for builder-owned texture capability**

```csharp
[Fact]
public void Builder_contract_exposes_platform_asset_cook_capabilities() {
    IPlatformAssetBuilder builder = new GameCubePlatformAssetBuilder();
    PlatformDefinition definition = builder.Definition;

    PlatformAssetCookCapabilityDefinition textureCapability = Assert.Single(
        definition.AssetCookCapabilities.Where(capability => capability.SourceAssetKind == "texture"));

    Assert.Equal(PlatformAssetCookOwnershipKind.BuilderOwned, textureCapability.OwnershipKind);
    Assert.Equal("runtime-texture", textureCapability.TargetArtifactKind);
}
```

- [ ] **Step 2: Run the focused metadata tests to verify capability types are missing**

Run: `rtk dotnet test C:/dev/helworks/helengine/engine/helengine.baseplatform.tests/helengine.baseplatform.tests.csproj --filter "FullyQualifiedName~IPlatformAssetBuilderMetadataTests" -v minimal -p:BuildInParallel=false -p:UseSharedCompilation=false`

Expected: FAIL because `AssetCookCapabilities` and the new capability types do not exist.

- [ ] **Step 3: Add the shared capability model to `PlatformDefinition`**

```csharp
public sealed class PlatformAssetCookCapabilityDefinition {
    public PlatformAssetCookCapabilityDefinition(
        string sourceAssetKind,
        string targetArtifactKind,
        PlatformAssetCookOwnershipKind ownershipKind,
        string settingsContractId) {
        // validate
    }

    public string SourceAssetKind { get; }
    public string TargetArtifactKind { get; }
    public PlatformAssetCookOwnershipKind OwnershipKind { get; }
    public string SettingsContractId { get; }
}
```

```csharp
public class PlatformDefinition {
    public PlatformDefinition(
        string id,
        string name,
        PlatformBuildProfileDefinition[] buildProfiles,
        PlatformGraphicsProfileDefinition[] graphicsProfiles,
        PlatformAssetRequirementDefinition[] assetRequirements,
        PlatformMaterialSchemaDefinition[] materialSchemas,
        PlatformComponentSupportRule[] componentSupportRules,
        PlatformCodegenProfileDefinition[] codegenProfiles,
        PlatformStorageProfileDefinition[] storageProfiles,
        PlatformMediaProfileDefinition[] mediaProfiles,
        RuntimeGenerationContract runtimeGenerationContract,
        PlatformAssetCookCapabilityDefinition[] assetCookCapabilities) {
        AssetCookCapabilities = [.. assetCookCapabilities];
    }

    public PlatformAssetCookCapabilityDefinition[] AssetCookCapabilities { get; }
}
```

- [ ] **Step 4: Add GameCube capability declarations without implementing execution yet**

```csharp
new PlatformAssetCookCapabilityDefinition(
    "texture",
    "runtime-texture",
    PlatformAssetCookOwnershipKind.BuilderOwned,
    "gamecube-texture"),
new PlatformAssetCookCapabilityDefinition(
    "font-atlas-texture",
    "runtime-font-atlas-texture",
    PlatformAssetCookOwnershipKind.BuilderOwned,
    "gamecube-texture")
```

- [ ] **Step 5: Run metadata tests plus the focused GC builder contract slice**

Run: `rtk dotnet test C:/dev/helworks/helengine/engine/helengine.baseplatform.tests/helengine.baseplatform.tests.csproj --filter "FullyQualifiedName~IPlatformAssetBuilderMetadataTests" -v minimal -p:BuildInParallel=false -p:UseSharedCompilation=false`

Run: `rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter "FullyQualifiedName~GameCubePackagedBuildWorkspaceTests" -v minimal -p:BuildInParallel=false -p:UseSharedCompilation=false`

Expected: PASS.

- [ ] **Step 6: Commit the platform capability contract**

```bash
git -C C:/dev/helworks/helengine add engine/helengine.baseplatform/Definitions/PlatformAssetCookOwnershipKind.cs engine/helengine.baseplatform/Definitions/PlatformAssetCookCapabilityDefinition.cs engine/helengine.baseplatform/Definitions/PlatformDefinition.cs engine/helengine.baseplatform.tests/Builders/IPlatformAssetBuilderMetadataTests.cs

git -C C:/dev/helworks/helengine-gc add builder/GameCubePlatformDefinitionFactory.cs builder.tests/GameCubePackagedBuildWorkspaceTests.cs

git -C C:/dev/helworks/helengine commit -m "Add platform asset cook capabilities"

git -C C:/dev/helworks/helengine-gc commit -m "Declare GameCube builder-owned asset cook kinds"
```

### Task 3: Emit Generic Platform Cook Work Items From the Editor Build Graph

**Files:**
- Create: `C:/dev/helworks/helengine/engine/helengine.editor/managers/project/EditorPlatformCookWorkItemEmitter.cs`
- Modify: `C:/dev/helworks/helengine/engine/helengine.editor/managers/project/EditorPlatformBuildGraphRunner.cs`
- Modify: `C:/dev/helworks/helengine/engine/helengine.editor/managers/project/EditorWindowsBuildScenePackager.cs`
- Modify: `C:/dev/helworks/helengine/engine/helengine.editor/managers/project/SceneComponentPackagingTransformService.cs`
- Test: `C:/dev/helworks/helengine/engine/helengine.editor.tests/managers/project/EditorWindowsBuildScenePackagerTests.cs`
- Test: `C:/dev/helworks/helengine/engine/helengine.baseplatform.tests/Definitions/EditorPlatformBuildScenePackagerMaterialCookTests.cs`

- [ ] **Step 1: Write the failing editor build-graph test that expects explicit GameCube work items for the menu logo and font atlas**

```csharp
[Fact]
public void Package_WhenGameCubeUsesBuilderOwnedTextureKinds_EmitsPlatformCookWorkItems() {
    EditorPlatformBuildScenePackager packager = CreatePackager("gamecube");

    EditorPlatformBuildScenePackagerResult result = packager.Package(
        ["Scenes/DemoDiscMainMenu.helen"],
        BuildRootPath);

    PlatformBuildManifest manifest = result.CookedManifest;

    Assert.Contains(manifest.PlatformCookWorkItems, item => item.SourceAssetKind == "texture" && item.SourceAssetPath.EndsWith("helengine-logo.png"));
    Assert.Contains(manifest.PlatformCookWorkItems, item => item.SourceAssetKind == "font-atlas-texture" && item.SourceAssetPath.EndsWith("DemoDiscBody.hefont"));
}
```

- [ ] **Step 2: Run the focused editor packager tests to verify the work items are not emitted yet**

Run: `rtk dotnet test C:/dev/helworks/helengine/engine/helengine.editor.tests/helengine.editor.tests.csproj --filter "FullyQualifiedName~EditorWindowsBuildScenePackagerTests|FullyQualifiedName~EditorPlatformBuildScenePackagerMaterialCookTests" -v minimal -p:BuildInParallel=false -p:UseSharedCompilation=false`

Expected: FAIL because `PlatformCookWorkItems` is empty or not emitted.

- [ ] **Step 3: Add a dedicated emitter that converts resolved dependencies plus platform capabilities into generic work items**

```csharp
public sealed class EditorPlatformCookWorkItemEmitter {
    public PlatformCookWorkItem[] Emit(
        PlatformDefinition platformDefinition,
        IReadOnlyList<ResolvedBuildAssetReference> assetReferences,
        string targetPlatformId) {
        // filter asset references by AssetCookCapabilities
        // resolve settings payloads
        // build stable work-item ids and output paths
    }
}
```

- [ ] **Step 4: Thread the emitted work items into the manifest produced by the editor build graph**

```csharp
PlatformCookWorkItem[] platformCookWorkItems = cookWorkItemEmitter.Emit(
    selectionModel.PlatformDefinition,
    resolvedAssetReferences,
    selectionModel.PlatformId);

PlatformBuildManifest manifest = new(
    manifestVersion,
    projectId,
    projectVersion,
    requiredEngineVersion,
    platformId,
    platformVersion,
    startupSceneId,
    scenes,
    looseAssets,
    cookedArtifacts,
    codeModules,
    artifactPlacements,
    containerWritePlan,
    platformCookWorkItems);
```

- [ ] **Step 5: Ensure the emitter uses editor-owned settings resolution instead of crawling staged cooked outputs**

```csharp
TextureAssetImportSettings settings = TextureAssetSettingsService.LoadOrCreate(
    sourceTexturePath,
    [targetPlatformId],
    ResolveSelectionModelForTextureSettings);
```

- [ ] **Step 6: Run the editor build-graph test slice**

Run: `rtk dotnet test C:/dev/helworks/helengine/engine/helengine.editor.tests/helengine.editor.tests.csproj --filter "FullyQualifiedName~EditorWindowsBuildScenePackagerTests|FullyQualifiedName~EditorPlatformBuildScenePackagerMaterialCookTests" -v minimal -p:BuildInParallel=false -p:UseSharedCompilation=false`

Expected: PASS.

- [ ] **Step 7: Commit the editor work-item emission layer**

```bash
git -C C:/dev/helworks/helengine add engine/helengine.editor/managers/project/EditorPlatformCookWorkItemEmitter.cs engine/helengine.editor/managers/project/EditorPlatformBuildGraphRunner.cs engine/helengine.editor/managers/project/EditorWindowsBuildScenePackager.cs engine/helengine.editor/managers/project/SceneComponentPackagingTransformService.cs engine/helengine.editor.tests/managers/project/EditorWindowsBuildScenePackagerTests.cs engine/helengine.baseplatform.tests/Definitions/EditorPlatformBuildScenePackagerMaterialCookTests.cs

git -C C:/dev/helworks/helengine commit -m "Emit platform cook work items from editor build graph"
```

### Task 4: Execute GameCube Texture and Font Work Items in the Builder

**Files:**
- Create: `builder/GameCubePlatformCookWorkItemExecutor.cs`
- Create: `builder/GameCubeTextureCooker.cs`
- Create: `builder/GameCubeTextureCookSettings.cs`
- Modify: `builder/GameCubeBuildWorkspace.cs`
- Modify: `builder/GameCubeBuilderPaths.cs`
- Modify: `builder.tests/GameCubePackagedBuildWorkspaceTests.cs`
- Modify: `builder.tests/GameCubePackagedRuntimeSourceTests.cs`
- Test: `builder.tests/GameCubeTextureCookerTests.cs`
- Test: `builder.tests/GameCubePlatformCookWorkItemExecutorTests.cs`

- [ ] **Step 1: Write the failing GameCube builder test for executing a texture work item into a final runtime artifact**

```csharp
[Fact]
public void Execute_WhenTextureWorkItemIsProvided_WritesGameCubeTextureArtifact() {
    PlatformCookWorkItem workItem = CreateGameCubeTextureWorkItem(
        sourceAssetPath: LogoTexturePath,
        outputRelativePath: "cooked/imported/helengine-logo.gc.hasset");

    GameCubePlatformCookWorkItemExecutor executor = new();
    executor.Execute([workItem], WorkingRootPath, OutputRootPath);

    Assert.True(File.Exists(Path.Combine(WorkingRootPath, "gamecube-staging", "cooked", "imported", "helengine-logo.gc.hasset")));
}
```

- [ ] **Step 2: Run the focused GC builder tests to verify the executor path is missing**

Run: `rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter "FullyQualifiedName~GameCubeTextureCookerTests|FullyQualifiedName~GameCubePlatformCookWorkItemExecutorTests|FullyQualifiedName~GameCubePackagedBuildWorkspaceTests" -v minimal -p:BuildInParallel=false -p:UseSharedCompilation=false`

Expected: FAIL because the executor and cooker do not exist yet.

- [ ] **Step 3: Implement the builder-owned GameCube texture cooker from source asset + resolved settings**

```csharp
public sealed class GameCubeTextureCooker {
    public byte[] CookTexture(TextureAsset sourceAsset, GameCubeTextureCookSettings settings) {
        // resize to max resolution
        // convert to GX RGB5A3 tiled payload
        // serialize final GameCube runtime texture asset bytes
    }
}
```

- [ ] **Step 4: Implement a generic GameCube work-item executor that dispatches on work-item kind**

```csharp
public sealed class GameCubePlatformCookWorkItemExecutor {
    public void Execute(PlatformCookWorkItem[] workItems, string projectRootPath, string stagingRootPath) {
        foreach (PlatformCookWorkItem workItem in workItems) {
            if (string.Equals(workItem.SourceAssetKind, "texture", StringComparison.OrdinalIgnoreCase)) {
                ExecuteTextureWorkItem(workItem, stagingRootPath);
            } else if (string.Equals(workItem.SourceAssetKind, "font-atlas-texture", StringComparison.OrdinalIgnoreCase)) {
                ExecuteFontAtlasTextureWorkItem(workItem, stagingRootPath);
            } else {
                throw new InvalidOperationException($"GameCube builder does not support platform cook work item kind '{workItem.SourceAssetKind}'.");
            }
        }
    }
}
```

- [ ] **Step 5: Invoke the executor in `GameCubeBuildWorkspace` before normal cooked-artifact staging**

```csharp
WritePhaseMarker(phaseMarkerPath, "platform cook work items begin");
GameCubePlatformCookWorkItemExecutor executor = new();
executor.Execute(request.Manifest.PlatformCookWorkItems ?? [], repositoryRootPath, stagingRootPath);
WritePhaseMarker(phaseMarkerPath, "platform cook work items completed");
```

- [ ] **Step 6: Keep `CookedArtifacts` clean by having the editor emit final output paths while the builder writes those outputs directly**

```csharp
string destinationPath = Path.Combine(stagingRootPath, NormalizeRelativePath(workItem.OutputRelativePath));
File.WriteAllBytes(destinationPath, cookedBytes);
```

- [ ] **Step 7: Run the focused GC builder test slice**

Run: `rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter "FullyQualifiedName~GameCubeTextureCookerTests|FullyQualifiedName~GameCubePlatformCookWorkItemExecutorTests|FullyQualifiedName~GameCubePackagedBuildWorkspaceTests|FullyQualifiedName~GameCubePackagedRuntimeSourceTests" -v minimal -p:BuildInParallel=false -p:UseSharedCompilation=false`

Expected: PASS.

- [ ] **Step 8: Commit the GameCube builder execution path**

```bash
git -C C:/dev/helworks/helengine-gc add builder/GameCubePlatformCookWorkItemExecutor.cs builder/GameCubeTextureCooker.cs builder/GameCubeTextureCookSettings.cs builder/GameCubeBuildWorkspace.cs builder/GameCubeBuilderPaths.cs builder.tests/GameCubeTextureCookerTests.cs builder.tests/GameCubePlatformCookWorkItemExecutorTests.cs builder.tests/GameCubePackagedBuildWorkspaceTests.cs builder.tests/GameCubePackagedRuntimeSourceTests.cs

git -C C:/dev/helworks/helengine-gc commit -m "Cook GameCube platform-owned textures in builder"
```

### Task 5: Switch GameCube Runtime to Direct Platform-Owned Loads and Remove the Wrong Shared Final-Cook Path

**Files:**
- Modify: `src/platform/gamecube/GameCubeRuntimeTexture.cpp`
- Modify: `src/platform/gamecube/GameCubeRenderManager2D.cpp`
- Modify: `src/platform/gamecube/GameCubeRenderManager3D.cpp`
- Modify: `src/platform/gamecube/GameCubeRuntimeTexture.hpp`
- Modify: `C:/dev/helworks/helengine/engine/helengine.editor/managers/asset/TextureAssetProcessor.cs`
- Modify: `C:/dev/helworks/helengine/engine/helengine.editor.tests/managers/asset/TextureAssetProcessorTests.cs`
- Modify: `tools/build_and_launch_editor_gamecube_probe.ps1`
- Test: `builder.tests/GameCubePackagedRuntimeSourceTests.cs`
- Test: `C:/dev/helworks/helengine/engine/helengine.editor.tests/managers/asset/TextureAssetProcessorTests.cs`

- [ ] **Step 1: Write the failing runtime source-contract assertion that editor-owned GameCube builds no longer depend on shared final GxRgb5A3 editor conversion**

```csharp
Assert.DoesNotContain("ConvertToGxRgb5A3", sharedTextureProcessorSource, StringComparison.Ordinal);
Assert.Contains("PlatformCookWorkItems", buildGraphSource, StringComparison.Ordinal);
```

- [ ] **Step 2: Run the focused runtime/source tests to verify the old conversion path is still present**

Run: `rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter "FullyQualifiedName~GameCubePackagedRuntimeSourceTests" -v minimal -p:BuildInParallel=false -p:UseSharedCompilation=false`

Run: `rtk dotnet test C:/dev/helworks/helengine/engine/helengine.editor.tests/helengine.editor.tests.csproj --filter "FullyQualifiedName~TextureAssetProcessorTests" -v minimal -p:BuildInParallel=false -p:UseSharedCompilation=false`

Expected: FAIL because the shared editor processor still contains GameCube final-byte cooking.

- [ ] **Step 3: Reduce the shared editor texture processor back to generic editor-owned formats only**

```csharp
if (targetFormat == TextureAssetColorFormat.GxRgb5A3) {
    throw new InvalidOperationException("Final GameCube texture conversion is builder-owned and must not run in the shared editor processor.");
}
```

- [ ] **Step 4: Keep the GameCube runtime on direct platform-owned payload loads**

```c++
if (data->ColorFormat == TextureAssetColorFormat::GxRgb5A3) {
    LoadPrepackedRgb5A3(data);
    return;
}
```

- [ ] **Step 5: Verify the rebuild helper fails hard on editor prebuild failure so stale images cannot be silently launched**

```powershell
& rtk dotnet build $EditorApplicationProjectPath -v minimal -p:BuildInParallel=false -p:UseSharedCompilation=false
if ($LASTEXITCODE -ne 0) {
    throw "Editor application build failed with exit code $LASTEXITCODE."
}
```

- [ ] **Step 6: Run the focused test slices again**

Run: `rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter "FullyQualifiedName~GameCubePackagedRuntimeSourceTests" -v minimal -p:BuildInParallel=false -p:UseSharedCompilation=false`

Run: `rtk dotnet test C:/dev/helworks/helengine/engine/helengine.editor.tests/helengine.editor.tests.csproj --filter "FullyQualifiedName~TextureAssetProcessorTests" -v minimal -p:BuildInParallel=false -p:UseSharedCompilation=false`

Expected: PASS.

- [ ] **Step 7: Run the end-to-end editor-owned GameCube build and verify the menu logo + timing**

Run: `rtk powershell -NoProfile -ExecutionPolicy Bypass -File "C:/dev/helworks/helengine-gc/tools/build_and_launch_editor_gamecube_probe.ps1"`

Expected in Dolphin log:

```text
[GC] Runtime build stamp: ...
[GC] ContentManager opening asset: dvd:/cooked/scenes/DemoDiscMainMenu.hasset
[GC] ContentManager opening asset: dvd:/cooked/Fonts/DemoDiscBody.hefont
[GC] ContentManager opening asset: dvd:/cooked/imported/...
[GC] Scene load to first draw scene=DemoDiscMainMenu elapsedMs=...
```

Expected visually:
- menu logo colors correct
- no green eyes
- first-draw timing better than the old runtime-conversion path

- [ ] **Step 8: Commit the runtime/boundary cleanup**

```bash
git -C C:/dev/helworks/helengine add engine/helengine.editor/managers/asset/TextureAssetProcessor.cs engine/helengine.editor.tests/managers/asset/TextureAssetProcessorTests.cs

git -C C:/dev/helworks/helengine-gc add src/platform/gamecube/GameCubeRuntimeTexture.cpp src/platform/gamecube/GameCubeRuntimeTexture.hpp src/platform/gamecube/GameCubeRenderManager2D.cpp src/platform/gamecube/GameCubeRenderManager3D.cpp builder.tests/GameCubePackagedRuntimeSourceTests.cs tools/build_and_launch_editor_gamecube_probe.ps1

git -C C:/dev/helworks/helengine commit -m "Remove shared final GameCube texture conversion"

git -C C:/dev/helworks/helengine-gc commit -m "Load builder-cooked GameCube textures directly"
```

### Task 6: Final Cross-Repo Verification and Documentation

**Files:**
- Modify: `docs/superpowers/plans/2026-05-17-helengine-gamecube-platform-cooked-textures.md`
- Optional Docs: `C:/dev/helworks/helengine/engine/helengine.baseplatform/Manifest/PlatformBuildManifest.cs` XML comments if new collection wording needs refinement

- [ ] **Step 1: Run the shared manifest/editor/build slices together**

Run: `rtk dotnet test C:/dev/helworks/helengine/engine/helengine.baseplatform.tests/helengine.baseplatform.tests.csproj --filter "FullyQualifiedName~PlatformBuildManifestTests|FullyQualifiedName~PlatformBuildRequestCodegenTests|FullyQualifiedName~PlatformBuildRequestMultiTargetTests|FullyQualifiedName~IPlatformAssetBuilderMetadataTests|FullyQualifiedName~EditorPlatformBuildScenePackagerMaterialCookTests" -v minimal -p:BuildInParallel=false -p:UseSharedCompilation=false`

Expected: PASS.

- [ ] **Step 2: Run the focused GameCube builder/runtime slices together**

Run: `rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter "FullyQualifiedName~GameCubeTextureCookerTests|FullyQualifiedName~GameCubePlatformCookWorkItemExecutorTests|FullyQualifiedName~GameCubePackagedBuildWorkspaceTests|FullyQualifiedName~GameCubePackagedRuntimeSourceTests" -v minimal -p:BuildInParallel=false -p:UseSharedCompilation=false`

Expected: PASS.

- [ ] **Step 3: Re-run the editor-owned GameCube probe build from a clean boot**

Run: `rtk powershell -NoProfile -ExecutionPolicy Bypass -File "C:/dev/helworks/helengine-gc/tools/build_and_launch_editor_gamecube_probe.ps1"`

Expected:
- fresh `[GC] Runtime build stamp: ...`
- correct menu logo colors
- faster or at least no-regression first-draw timing

- [ ] **Step 4: Update the plan file with execution notes if any commands or file paths had to change during implementation**

```markdown
- Verified final editor-owned build path uses manifest-emitted platform cook work items.
- Verified GameCube builder no longer crawls menu asset references for texture/font cooking.
```

- [ ] **Step 5: Commit any final docs-only cleanup if needed**

```bash
git -C C:/dev/helworks/helengine-gc add docs/superpowers/plans/2026-05-17-helengine-gamecube-platform-cooked-textures.md

git -C C:/dev/helworks/helengine-gc commit -m "Document platform cook work item rollout notes"
```
