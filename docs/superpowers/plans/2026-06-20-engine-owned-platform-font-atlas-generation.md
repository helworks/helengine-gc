# Engine-Owned Platform Font Atlas Generation Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Move platform-specific font atlas generation into the shared engine/editor pipeline, cache per-platform font variants in project cache, and remove raw-font atlas generation from `helengine-gc`.

**Architecture:** Add one editor-side platform font variant cache service that materializes per-platform cached font assets plus cached atlas texture assets on demand from authored fonts. Rewire scene/build packaging to consume those generated cache artifacts and emit only generic `texture` cook work items for atlas textures, then delete the GameCube builder’s special `font-atlas-texture` path and builder-side font normalization fallback.

**Tech Stack:** C# (.NET 9), HelEngine editor/import pipeline, shared build manifest/work-item flow, HelEngine editor xUnit tests, helengine-gc builder xUnit tests

---

## File Structure

### Shared engine/editor files

- Create: `C:/dev/helworks/helengine/engine/helengine.editor/managers/project/EditorPlatformFontVariantCacheKey.cs`
  - Immutable cache-key type that captures the target platform id plus all inputs that invalidate a platform font variant.
- Create: `C:/dev/helworks/helengine/engine/helengine.editor/managers/project/EditorPlatformFontVariantCacheResult.cs`
  - Immutable result type that returns the generated cached font path, generated cached atlas texture path, and canonical runtime-relative output paths used by packaging.
- Create: `C:/dev/helworks/helengine/engine/helengine.editor/managers/project/EditorPlatformFontVariantCacheService.cs`
  - Service that resolves authored font sources into cached per-platform generated font assets and generated atlas texture assets.
- Modify: `C:/dev/helworks/helengine/engine/helengine.editor/managers/asset/AssetImportManager.cs`
  - Expose or add the minimum helpers needed to load font import settings and import source fonts without directly staging runtime build outputs.
- Modify: `C:/dev/helworks/helengine/engine/helengine.editor/managers/project/SceneComponentPackagingTransformService.cs`
  - Replace builder-owned raw-font work-item emission with cached platform-font generation plus generic texture work-item emission.
- Modify: `C:/dev/helworks/helengine/engine/helengine.editor/managers/project/EditorWindowsBuildScenePackager.cs`
  - Apply the same cached platform-font flow for the editor build-scene packager path.
- Modify: `C:/dev/helworks/helengine/engine/helengine.editor/managers/project/EditorPlatformCookWorkItemFactory.cs`
  - Remove special `font-atlas-texture` work-item creation and ensure generated atlas textures use the normal `texture` work-item path.
- Modify: `C:/dev/helworks/helengine/engine/helengine.editor/managers/project/EditorPlatformAssetCookService.cs`
  - Preserve generated font assets in cooked outputs and treat generated atlas textures as generic builder-owned texture outputs.
- Modify: `C:/dev/helworks/helengine/engine/helengine.editor/components/ui/AssetImportSettingsView.cs`
  - Stop treating fonts as a separate `font-atlas-texture` capability source kind and route font texture settings through the normal texture capability path.

### Shared engine/editor tests

- Create: `C:/dev/helworks/helengine/engine/helengine.editor.tests/managers/project/EditorPlatformFontVariantCacheServiceTests.cs`
  - Covers cache creation, reuse, and invalidation.
- Modify: `C:/dev/helworks/helengine/engine/helengine.editor.tests/managers/project/SceneComponentPackagingTransformServiceTests.cs`
  - Assert packaged font references use cached platform variants and generic texture work items.
- Modify: `C:/dev/helworks/helengine/engine/helengine.editor.tests/managers/project/EditorWindowsBuildScenePackagerTests.cs`
  - Assert build-scene packaging emits generic texture work items for generated font atlases and no longer uses the raw-font path.
- Modify: `C:/dev/helworks/helengine/engine/helengine.editor.tests/managers/project/EditorPlatformAssetCookServiceTests.cs`
  - Assert generated atlas texture outputs are treated as builder-owned generic textures and that generated platform font assets stay in cooked outputs.
- Modify: `C:/dev/helworks/helengine/engine/helengine.editor.tests/managers/project/EditorPlatformCookWorkItemFactoryTests.cs`
  - Remove `font-atlas-texture` expectations and add generated-texture work-item expectations for platform font atlases.
- Modify: `C:/dev/helworks/helengine/engine/helengine.editor.tests/AssetImportSettingsViewTests.cs`
  - Assert font import settings use normal texture capability metadata instead of a special font-atlas capability.

### GameCube builder files

- Modify: `C:/dev/helworks/helengine-gc/builder/GameCubePlatformDefinitionFactory.cs`
  - Remove the `font-atlas-texture` asset cook capability.
- Modify: `C:/dev/helworks/helengine-gc/builder/GameCubePlatformCookWorkItemExecutor.cs`
  - Delete the raw-font import path and accept only generic `texture` work items.
- Modify: `C:/dev/helworks/helengine-gc/builder/GameCubeBuildWorkspace.cs`
  - Remove embedded-font-atlas normalization because staged font artifacts should already reference external cooked atlas textures.
- Modify: `C:/dev/helworks/helengine-gc/builder/helengine.gamecube.builder.csproj`
  - Remove the dedicated GDI font importer dependency after the raw-font path is deleted.

### GameCube builder tests

- Modify: `C:/dev/helworks/helengine-gc/builder.tests/GameCubePlatformAssetBuilderTests.cs`
  - Remove `font-atlas-texture` capability assertions.
- Modify: `C:/dev/helworks/helengine-gc/builder.tests/GameCubePlatformCookWorkItemExecutorTests.cs`
  - Replace font-atlas work-item tests with generic generated-texture work-item tests and assert raw-font inputs are rejected.
- Modify: `C:/dev/helworks/helengine-gc/builder.tests/GameCubePackagedBuildWorkspaceTests.cs`
  - Assert packaged builds do not rely on builder-side font-atlas normalization.
- Modify: `C:/dev/helworks/helengine-gc/builder.tests/GameCubePackagedRuntimeSourceTests.cs`
  - Assert `helengine-gc` no longer references `GdiFontImporter`, `font-atlas-texture`, or the dedicated GDI font importer project.

---

### Task 1: Add Editor-Side Cached Platform Font Variant Generation

**Files:**
- Create: `C:/dev/helworks/helengine/engine/helengine.editor/managers/project/EditorPlatformFontVariantCacheKey.cs`
- Create: `C:/dev/helworks/helengine/engine/helengine.editor/managers/project/EditorPlatformFontVariantCacheResult.cs`
- Create: `C:/dev/helworks/helengine/engine/helengine.editor/managers/project/EditorPlatformFontVariantCacheService.cs`
- Modify: `C:/dev/helworks/helengine/engine/helengine.editor/managers/asset/AssetImportManager.cs`
- Test: `C:/dev/helworks/helengine/engine/helengine.editor.tests/managers/project/EditorPlatformFontVariantCacheServiceTests.cs`
- Test: `C:/dev/helworks/helengine/engine/helengine.editor.tests/AssetImportManagerTests.cs`

- [ ] **Step 1: Write the failing cache-generation tests**

Add tests that prove:

- the editor can generate one cached platform font asset and one cached atlas texture asset from an authored `.ttf`
- repeated requests with the same target platform id and same settings reuse the cached files
- changing pixel size invalidates the cache and regenerates the font variant
- changing texture format settings invalidates the cache and regenerates the font variant

Use existing test helpers such as `TestFontImporter`, `ConfigurableFontImporter`, and `WriteSourceFont(...)` so the tests stay deterministic and do not require platform GDI.

- [ ] **Step 2: Run the new tests to verify they fail**

Run:

```powershell
dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~EditorPlatformFontVariantCacheServiceTests|FullyQualifiedName~AssetImportManagerTests" -v minimal -p:BuildInParallel=false -p:UseSharedCompilation=false
```

Expected:

- `EditorPlatformFontVariantCacheServiceTests` fails because the service and cache-key/result types do not exist
- any invalidation assertions fail because the current import path only writes one canonical font cache artifact

- [ ] **Step 3: Add the cache key and result types**

Implement:

- one immutable key type that includes target platform id, source file hash, glyph/rasterization inputs, and texture-settings hash
- one immutable result type that returns:
  - cached platform font asset path
  - cached platform atlas texture asset path
  - runtime-relative cooked font path
  - runtime-relative cooked atlas texture path

Validate all required constructor inputs and keep the types small and explicit.

- [ ] **Step 4: Implement the cache service with minimal editor-side generation**

Implement the service so it:

- loads the authored font source through existing editor import infrastructure
- resolves the per-platform settings that affect font metrics and atlas pixels
- materializes a cached font asset and cached atlas texture asset under project `cache/`
- reuses the cached outputs when the full key matches

Do not stage cooked runtime outputs in this task; only generate the project-cache artifacts.

- [ ] **Step 5: Add the minimum AssetImportManager hooks**

Expose only the helpers required by the cache service, such as:

- loading font import settings for a source path
- importing a font from source without directly binding the result to the single canonical cache output
- computing or reusing the platform texture settings that affect the source atlas contract

Keep the changes narrow and avoid rewriting the broader asset-import manager structure.

- [ ] **Step 6: Run the focused tests to verify they pass**

Run:

```powershell
dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~EditorPlatformFontVariantCacheServiceTests|FullyQualifiedName~AssetImportManagerTests" -v minimal -p:BuildInParallel=false -p:UseSharedCompilation=false
```

Expected: PASS

- [ ] **Step 7: Commit**

```powershell
git add C:\dev\helworks\helengine\engine\helengine.editor\managers\project\EditorPlatformFontVariantCacheKey.cs C:\dev\helworks\helengine\engine\helengine.editor\managers\project\EditorPlatformFontVariantCacheResult.cs C:\dev\helworks\helengine\engine\helengine.editor\managers\project\EditorPlatformFontVariantCacheService.cs C:\dev\helworks\helengine\engine\helengine.editor\managers\asset\AssetImportManager.cs C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\project\EditorPlatformFontVariantCacheServiceTests.cs C:\dev\helworks\helengine\engine\helengine.editor.tests\AssetImportManagerTests.cs
git commit -m "feat: cache platform font variants in editor"
```

### Task 2: Rewire Packaging To Consume Cached Platform Font Variants

**Files:**
- Modify: `C:/dev/helworks/helengine/engine/helengine.editor/managers/project/SceneComponentPackagingTransformService.cs`
- Modify: `C:/dev/helworks/helengine/engine/helengine.editor/managers/project/EditorWindowsBuildScenePackager.cs`
- Modify: `C:/dev/helworks/helengine/engine/helengine.editor/managers/project/EditorPlatformCookWorkItemFactory.cs`
- Modify: `C:/dev/helworks/helengine/engine/helengine.editor/managers/project/EditorPlatformAssetCookService.cs`
- Test: `C:/dev/helworks/helengine/engine/helengine.editor.tests/managers/project/SceneComponentPackagingTransformServiceTests.cs`
- Test: `C:/dev/helworks/helengine/engine/helengine.editor.tests/managers/project/EditorWindowsBuildScenePackagerTests.cs`
- Test: `C:/dev/helworks/helengine/engine/helengine.editor.tests/managers/project/EditorPlatformAssetCookServiceTests.cs`
- Test: `C:/dev/helworks/helengine/engine/helengine.editor.tests/managers/project/EditorPlatformCookWorkItemFactoryTests.cs`

- [ ] **Step 1: Write the failing packaging and work-item tests**

Update the tests so they require:

- file-backed font references to generate a cached platform font variant before packaging
- staged packaged `.hefont` outputs to reference external cooked atlas textures and not embed source atlas payloads
- generic `texture` work items for generated atlas textures
- no `font-atlas-texture` work items anywhere in the manifest or packaging flow

Add explicit assertions for the generated atlas texture input path so the test proves the builder is now consuming a generated texture asset instead of a raw font source.

- [ ] **Step 2: Run the focused tests to verify they fail**

Run:

```powershell
dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~SceneComponentPackagingTransformServiceTests|FullyQualifiedName~EditorWindowsBuildScenePackagerTests|FullyQualifiedName~EditorPlatformAssetCookServiceTests|FullyQualifiedName~EditorPlatformCookWorkItemFactoryTests" -v minimal -p:BuildInParallel=false -p:UseSharedCompilation=false
```

Expected:

- the packaging tests fail because `font-atlas-texture` work items are still emitted
- the work-item factory tests fail because the factory still has a font-atlas-specific path

- [ ] **Step 3: Replace the raw-font packaging path with cached platform font variants**

Modify both packaging services so they:

- request the cached platform font variant from `EditorPlatformFontVariantCacheService`
- copy or serialize the generated platform font asset into the build output
- emit one generic texture work item that points at the generated atlas texture asset
- stop serializing packaged fonts from raw imported source fonts during packaging

Keep path-generation logic canonical and forward-slash normalized.

- [ ] **Step 4: Collapse the work-item factory to the generic texture path**

Delete `CreateFontAtlasTextureWorkItem(...)` and `CreateGeneratedFontAtlasTextureWorkItem(...)` after call sites are moved.

Ensure generated platform atlas textures use the existing generic generated-texture work-item path with `sourceAssetKind = "texture"`.

- [ ] **Step 5: Update cooked-artifact accounting**

Ensure `EditorPlatformAssetCookService`:

- keeps the generated platform `.hefont` outputs in cooked artifacts
- excludes only the generated atlas texture outputs that will later be produced by builder-owned generic texture work items

- [ ] **Step 6: Run the focused tests to verify they pass**

Run:

```powershell
dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~SceneComponentPackagingTransformServiceTests|FullyQualifiedName~EditorWindowsBuildScenePackagerTests|FullyQualifiedName~EditorPlatformAssetCookServiceTests|FullyQualifiedName~EditorPlatformCookWorkItemFactoryTests" -v minimal -p:BuildInParallel=false -p:UseSharedCompilation=false
```

Expected: PASS

- [ ] **Step 7: Commit**

```powershell
git add C:\dev\helworks\helengine\engine\helengine.editor\managers\project\SceneComponentPackagingTransformService.cs C:\dev\helworks\helengine\engine\helengine.editor\managers\project\EditorWindowsBuildScenePackager.cs C:\dev\helworks\helengine\engine\helengine.editor\managers\project\EditorPlatformCookWorkItemFactory.cs C:\dev\helworks\helengine\engine\helengine.editor\managers\project\EditorPlatformAssetCookService.cs C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\project\SceneComponentPackagingTransformServiceTests.cs C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\project\EditorWindowsBuildScenePackagerTests.cs C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\project\EditorPlatformAssetCookServiceTests.cs C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\project\EditorPlatformCookWorkItemFactoryTests.cs
git commit -m "refactor: package platform fonts through cached generated textures"
```

### Task 3: Remove Special Font-Atlas Capability Metadata From Shared Editor Paths

**Files:**
- Modify: `C:/dev/helworks/helengine/engine/helengine.editor/components/ui/AssetImportSettingsView.cs`
- Modify: `C:/dev/helworks/helengine/engine/helengine.editor.tests/AssetImportSettingsViewTests.cs`
- Modify: `C:/dev/helworks/helengine/engine/helengine.editor.tests/managers/project/EditorPlatformBuildGraphRunnerTests.cs`

- [ ] **Step 1: Write the failing UI and build-graph contract tests**

Update the relevant tests so they require:

- font assets to resolve texture-format controls through the normal `texture` capability contract
- build-graph outputs to contain generic generated atlas texture work items instead of a special `font-atlas-texture` source kind

Keep the assertions explicit so the tests fail on source-kind names, not just on artifact counts.

- [ ] **Step 2: Run the focused tests to verify they fail**

Run:

```powershell
dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~AssetImportSettingsViewTests|FullyQualifiedName~EditorPlatformBuildGraphRunnerTests" -v minimal -p:BuildInParallel=false -p:UseSharedCompilation=false
```

Expected: FAIL because the view and build-graph still publish the special font-atlas source kind.

- [ ] **Step 3: Remove the special font-atlas capability path**

Modify the view and related build-graph logic so fonts use:

- normal texture capability metadata for format selection
- platform font generation settings owned by the editor-side cached variant service

Do not leave fallback branches for the legacy capability name.

- [ ] **Step 4: Run the focused tests to verify they pass**

Run:

```powershell
dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~AssetImportSettingsViewTests|FullyQualifiedName~EditorPlatformBuildGraphRunnerTests" -v minimal -p:BuildInParallel=false -p:UseSharedCompilation=false
```

Expected: PASS

- [ ] **Step 5: Commit**

```powershell
git add C:\dev\helworks\helengine\engine\helengine.editor\components\ui\AssetImportSettingsView.cs C:\dev\helworks\helengine\engine\helengine.editor.tests\AssetImportSettingsViewTests.cs C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\project\EditorPlatformBuildGraphRunnerTests.cs
git commit -m "refactor: remove special font atlas capability metadata"
```

### Task 4: Simplify helengine-gc To Generic Texture Cooking Only

**Files:**
- Modify: `C:/dev/helworks/helengine-gc/builder/GameCubePlatformDefinitionFactory.cs`
- Modify: `C:/dev/helworks/helengine-gc/builder/GameCubePlatformCookWorkItemExecutor.cs`
- Modify: `C:/dev/helworks/helengine-gc/builder/GameCubeBuildWorkspace.cs`
- Modify: `C:/dev/helworks/helengine-gc/builder/helengine.gamecube.builder.csproj`
- Test: `C:/dev/helworks/helengine-gc/builder.tests/GameCubePlatformAssetBuilderTests.cs`
- Test: `C:/dev/helworks/helengine-gc/builder.tests/GameCubePlatformCookWorkItemExecutorTests.cs`
- Test: `C:/dev/helworks/helengine-gc/builder.tests/GameCubePackagedBuildWorkspaceTests.cs`
- Test: `C:/dev/helworks/helengine-gc/builder.tests/GameCubePackagedRuntimeSourceTests.cs`

- [ ] **Step 1: Write the failing builder tests**

Update the builder tests so they require:

- no `font-atlas-texture` capability in the GameCube platform definition
- `GameCubePlatformCookWorkItemExecutor` to accept only `texture` work items for atlas cooking inputs
- raw `.ttf` and raw `.hefont` builder atlas-generation inputs to be rejected
- `GameCubeBuildWorkspace` not to normalize embedded font atlases during staging
- the builder project file not to reference the dedicated GDI font importer project

- [ ] **Step 2: Run the focused builder tests to verify they fail**

Run:

```powershell
dotnet test C:\dev\helworks\helengine-gc\builder.tests\helengine.gamecube.builder.tests.csproj --filter "FullyQualifiedName~GameCubePlatformAssetBuilderTests|FullyQualifiedName~GameCubePlatformCookWorkItemExecutorTests|FullyQualifiedName~GameCubePackagedBuildWorkspaceTests|FullyQualifiedName~GameCubePackagedRuntimeSourceTests" -v minimal -p:BuildInParallel=false -p:UseSharedCompilation=false
```

Expected: FAIL because the builder still advertises `font-atlas-texture`, still contains raw-font import logic, and still references the GDI importer project.

- [ ] **Step 3: Remove the special GameCube font-atlas capability**

Modify `GameCubePlatformDefinitionFactory` so the platform definition advertises only the generic builder-owned `texture` capability needed for generated atlas textures.

- [ ] **Step 4: Delete raw-font import and builder-side font normalization**

Modify:

- `GameCubePlatformCookWorkItemExecutor` to delete:
  - `ExecuteFontAtlasTextureWorkItem(...)`
  - `ImportSourceFont(...)`
  - `LoadFontAtlasSourceTexture(...)`
  - serialized-font fallback paths that exist only to recover atlas textures from font sources
- `GameCubeBuildWorkspace` to delete embedded-font-atlas normalization because staged font artifacts must already be externalized by the editor

Keep the existing generic texture cook path intact.

- [ ] **Step 5: Remove the now-unused GDI importer project reference**

Delete the `helengine.editor.windows.gdiimporter` project reference from `helengine.gamecube.builder.csproj` if no remaining builder code depends on it.

- [ ] **Step 6: Run the focused builder tests to verify they pass**

Run:

```powershell
dotnet test C:\dev\helworks\helengine-gc\builder.tests\helengine.gamecube.builder.tests.csproj --filter "FullyQualifiedName~GameCubePlatformAssetBuilderTests|FullyQualifiedName~GameCubePlatformCookWorkItemExecutorTests|FullyQualifiedName~GameCubePackagedBuildWorkspaceTests|FullyQualifiedName~GameCubePackagedRuntimeSourceTests" -v minimal -p:BuildInParallel=false -p:UseSharedCompilation=false
```

Expected: PASS

- [ ] **Step 7: Run the combined verification slice**

Run:

```powershell
dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~EditorPlatformFontVariantCacheServiceTests|FullyQualifiedName~SceneComponentPackagingTransformServiceTests|FullyQualifiedName~EditorWindowsBuildScenePackagerTests|FullyQualifiedName~EditorPlatformAssetCookServiceTests|FullyQualifiedName~EditorPlatformCookWorkItemFactoryTests|FullyQualifiedName~AssetImportSettingsViewTests|FullyQualifiedName~EditorPlatformBuildGraphRunnerTests" -v minimal -p:BuildInParallel=false -p:UseSharedCompilation=false
dotnet test C:\dev\helworks\helengine-gc\builder.tests\helengine.gamecube.builder.tests.csproj --filter "FullyQualifiedName~GameCubePlatformAssetBuilderTests|FullyQualifiedName~GameCubePlatformCookWorkItemExecutorTests|FullyQualifiedName~GameCubePackagedBuildWorkspaceTests|FullyQualifiedName~GameCubePackagedRuntimeSourceTests" -v minimal -p:BuildInParallel=false -p:UseSharedCompilation=false
```

Expected: both commands PASS

- [ ] **Step 8: Commit**

```powershell
git add C:\dev\helworks\helengine-gc\builder\GameCubePlatformDefinitionFactory.cs C:\dev\helworks\helengine-gc\builder\GameCubePlatformCookWorkItemExecutor.cs C:\dev\helworks\helengine-gc\builder\GameCubeBuildWorkspace.cs C:\dev\helworks\helengine-gc\builder\helengine.gamecube.builder.csproj C:\dev\helworks\helengine-gc\builder.tests\GameCubePlatformAssetBuilderTests.cs C:\dev\helworks\helengine-gc\builder.tests\GameCubePlatformCookWorkItemExecutorTests.cs C:\dev\helworks\helengine-gc\builder.tests\GameCubePackagedBuildWorkspaceTests.cs C:\dev\helworks\helengine-gc\builder.tests\GameCubePackagedRuntimeSourceTests.cs
git commit -m "refactor: remove builder-owned raw font atlas generation"
```
