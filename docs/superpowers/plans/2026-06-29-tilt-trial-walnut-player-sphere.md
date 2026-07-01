# Tilt Trial Walnut Player Sphere Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace the current `Tilt Trial` marble ball with a new texture-backed medium polished walnut sphere that reaches GameCube through the normal authored material, cook, and packaged runtime path.

**Architecture:** Keep the change content-owned. Add one City-side walnut material factory that writes the authored `PlayerSphereWalnut` material settings, switch the live `Tilt Trial` scene generation and runtime asset preparation from marble to walnut, add one real walnut source bitmap, and validate the result through the existing GameCube cooked material path with no native `PlayerSphere` special case.

**Tech Stack:** C#, City editor tooling, generated authored material settings, helengine editor tests, GameCube builder tests, headless editor CLI, packaged GameCube runtime

---

## File Structure

**City authored content and generators**

- Modify: `C:\dev\helprojs\city\assets\codebase\game.tools\GameSceneFactory.cs`
  Responsibility: assign the dedicated Tilt Trial sphere material to `PlayerSphere`.
- Modify: `C:\dev\helprojs\city\assets\codebase\game.tools\GameSceneGenerator.cs`
  Responsibility: ensure the walnut material asset is written before the Tilt Trial scene is regenerated.
- Modify: `C:\dev\helprojs\city\assets\codebase\rendering.tools\RenderingSceneGenerationAssets.cs`
  Responsibility: carry the renamed walnut runtime material through scene generation.
- Modify: `C:\dev\helprojs\city\assets\codebase\rendering.tools\RenderingSceneAssetPreparationService.cs`
  Responsibility: load the walnut material asset into the runtime preparation bundle.
- Create: `C:\dev\helprojs\city\assets\codebase\rendering.tools\TiltTrialPlayerSphereWalnutMaterialFactory.cs`
  Responsibility: generate the authored walnut material settings and derive imported texture ids from the real walnut bitmap.

**City authored assets**

- Create: `C:\dev\helprojs\city\assets\Textures\rendering\tilt_trial\PlayerSphereWalnut.bmp`
  Responsibility: source bitmap for the medium polished walnut grain.
- Generate: `C:\dev\helprojs\city\assets\Textures\rendering\tilt_trial\PlayerSphereWalnut.bmp.hasset`
  Responsibility: texture import settings sidecar created by the editor import pipeline when first resolved.
- Generate: `C:\dev\helprojs\city\assets\materials\rendering\tilt_trial\PlayerSphereWalnut.hasset`
  Responsibility: generated authored material settings that point all runtime platforms at the walnut texture.

**Tests**

- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\CityGameSceneSourceTests.cs`
  Responsibility: lock the live Tilt Trial source wiring to the walnut material path.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\CityGameCubeImportedTextureResolutionTests.cs`
  Responsibility: verify the imported walnut texture id resolves back to the authored source bitmap under GameCube packaging rules.
- Reuse: `C:\dev\helworks\helengine-gc\builder.tests\GameCubePlatformAssetBuilderTests.cs`
  Responsibility: existing builder regression that proves cooked GameCube textured materials preserve `TextureRelativePath`.

**Generated and packaged outputs**

- Generate: `C:\dev\helprojs\city\assets\scenes\games\tilt_trial.helen`
- Output: `C:\dev\helprojs\output\gamecube-tilt-trial-walnut\game.gcm`

### Task 1: Rename Tilt Trial Material Wiring From Marble To Walnut

**Files:**
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\CityGameSceneSourceTests.cs`
- Modify: `C:\dev\helprojs\city\assets\codebase\game.tools\GameSceneFactory.cs`
- Modify: `C:\dev\helprojs\city\assets\codebase\game.tools\GameSceneGenerator.cs`
- Modify: `C:\dev\helprojs\city\assets\codebase\rendering.tools\RenderingSceneGenerationAssets.cs`
- Modify: `C:\dev\helprojs\city\assets\codebase\rendering.tools\RenderingSceneAssetPreparationService.cs`
- Create: `C:\dev\helprojs\city\assets\codebase\rendering.tools\TiltTrialPlayerSphereWalnutMaterialFactory.cs`

- [ ] **Step 1: Rewrite the source test so it expects walnut wiring instead of marble**

```csharp
    /// <summary>
    /// Ensures the Tilt Trial player sphere source uses the dedicated walnut material asset and generator path.
    /// </summary>
    [Fact]
    public void City_tilt_trial_player_sphere_source_uses_walnut_material() {
        string gameSceneFactoryPath = @"C:\dev\helprojs\city\assets\codebase\game.tools\GameSceneFactory.cs";
        string gameSceneFactorySource = File.ReadAllText(gameSceneFactoryPath);
        string gameSceneGeneratorPath = @"C:\dev\helprojs\city\assets\codebase\game.tools\GameSceneGenerator.cs";
        string gameSceneGeneratorSource = File.ReadAllText(gameSceneGeneratorPath);
        string preparationSourcePath = @"C:\dev\helprojs\city\assets\codebase\rendering.tools\RenderingSceneAssetPreparationService.cs";
        string preparationSource = File.ReadAllText(preparationSourcePath);

        Assert.Contains("Materials.rendering.tilt_trial.PlayerSphereWalnut", gameSceneFactorySource, StringComparison.Ordinal);
        Assert.Contains("TiltTrialPlayerSphereWalnutMaterialFactory", gameSceneGeneratorSource, StringComparison.Ordinal);
        Assert.Contains("materials/rendering/tilt_trial/PlayerSphereWalnut.hasset", preparationSource, StringComparison.Ordinal);
    }
```

- [ ] **Step 2: Run the source test to verify it fails on the current marble-only code**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj -c Debug --filter FullyQualifiedName~CityGameSceneSourceTests.City_tilt_trial_player_sphere_source_uses_walnut_material
```

Expected:

```text
FAIL
Assert.Contains() Failure: Sub-string not found
Materials.rendering.tilt_trial.PlayerSphereWalnut
```

- [ ] **Step 3: Add the walnut material factory and switch the live Tilt Trial code paths to it**

```csharp
namespace city.rendering.tools {
    /// <summary>
    /// Writes the authored walnut material assigned to the Tilt Trial player sphere.
    /// </summary>
    public sealed class TiltTrialPlayerSphereWalnutMaterialFactory {
        /// <summary>
        /// Stable project-relative material path used by the Tilt Trial player sphere.
        /// </summary>
        public const string MaterialRelativePath = "materials/rendering/tilt_trial/PlayerSphereWalnut.hasset";

        /// <summary>
        /// Stable material asset identifier used by the Tilt Trial player sphere.
        /// </summary>
        public const string MaterialAssetId = "Materials.rendering.tilt_trial.PlayerSphereWalnut";

        /// <summary>
        /// Stable project-relative walnut source texture path.
        /// </summary>
        public const string TextureRelativePath = "Textures/rendering/tilt_trial/PlayerSphereWalnut.bmp";

        const string WindowsMaterialSchemaId = "standard-shader";
        const string Ps2MaterialSchemaId = "ps2-simple-lit-textured";
        const string GameCubeMaterialSchemaId = "gamecube-standard-textured";
        const string DsMaterialSchemaId = "ds-standard-textured";
        const string StandardShaderAssetId = "ForwardStandardShader";
        const string StandardVertexProgramName = "ForwardStandardShader.vs";
        const string StandardPixelProgramName = "ForwardStandardShader.ps";
        const string MeshVariantName = "Mesh";
        const string UseCustomShaderFieldId = "use-custom-shader";
        const string ShaderAssetIdFieldId = "shader-asset-id";
        const string VertexProgramFieldId = "vertex-program";
        const string PixelProgramFieldId = "pixel-program";
        const string VariantFieldId = "variant";
        const string TextureIdFieldId = "texture-id";
        const string TextureRelativePathFieldId = "texture-relative-path";
        const string BaseColorFieldId = "base-color";
        const string CastsShadowFieldId = "casts-shadow";
        const string ReceivesShadowFieldId = "receives-shadow";
        const string DoubleSidedFieldId = "double-sided";
        const string VertexColorModeFieldId = "vertex-color-mode";
        const string LightingModeFieldId = "lighting-mode";

        readonly GeneratedMaterialAssetWriteService MaterialWriteService;

        /// <summary>
        /// Initializes one walnut material factory.
        /// </summary>
        public TiltTrialPlayerSphereWalnutMaterialFactory() {
            MaterialWriteService = new GeneratedMaterialAssetWriteService();
        }

        /// <summary>
        /// Writes the authored walnut material asset for the Tilt Trial player sphere.
        /// </summary>
        /// <param name="projectRootPath">Absolute or relative city project root path.</param>
        public void WriteMaterialAsset(string projectRootPath) {
            string importedTextureAssetId = ResolveTextureAssetId(projectRootPath);
            MaterialWriteService.WriteMaterial(projectRootPath, MaterialRelativePath, CreateDefinition(importedTextureAssetId));
        }

        string ResolveTextureAssetId(string projectRootPath) {
            string fullProjectRootPath = Path.GetFullPath(projectRootPath);
            string assetsRootPath = Path.Combine(fullProjectRootPath, "assets");
            string sourceTexturePath = Path.Combine(assetsRootPath, TextureRelativePath.Replace('/', Path.DirectorySeparatorChar));

            ContentManager contentManager = new ContentManager(assetsRootPath);
            AssetImportManager importManager = new AssetImportManager(fullProjectRootPath, contentManager);
            Assembly appAssembly = Assembly.Load("helengine.editor.app");
            Type importerFactoryType = appAssembly.GetType("helengine.editor.app.EditorHostImporterFactory", throwOnError: true);
            MethodInfo createDefaultMethod = importerFactoryType.GetMethod("CreateDefault", BindingFlags.Public | BindingFlags.Static)
                ?? throw new InvalidOperationException("EditorHostImporterFactory.CreateDefault was not found.");
            IReadOnlyList<IAssetImporterRegistration> importers = (IReadOnlyList<IAssetImporterRegistration>)createDefaultMethod.Invoke(null, Array.Empty<object>());
            for (int index = 0; index < importers.Count; index++) {
                importers[index].Register(importManager);
            }

            TextureAssetImportSettings settings = importManager.LoadOrCreateTextureImportSettings(sourceTexturePath);
            return string.IsNullOrWhiteSpace(settings.Importer.AssetId)
                ? throw new InvalidOperationException("Tilt Trial walnut material requires a persisted imported texture asset id.")
                : settings.Importer.AssetId;
        }

        GeneratedMaterialAssetDefinition CreateDefinition(string textureAssetId) {
            GeneratedMaterialAssetDefinition definition = new GeneratedMaterialAssetDefinition();
            definition.MaterialAsset = new ShaderMaterialAsset {
                Id = MaterialAssetId,
                RenderState = new MaterialRenderState(),
                CastsShadows = true,
                ReceivesShadows = true
            };

            GeneratedMaterialPlatformDefinition windowsSettings = definition.GetOrCreatePlatform("windows");
            windowsSettings.SchemaId = WindowsMaterialSchemaId;
            windowsSettings.SetFieldValue(UseCustomShaderFieldId, "false");
            windowsSettings.SetFieldValue(ShaderAssetIdFieldId, StandardShaderAssetId);
            windowsSettings.SetFieldValue(VertexProgramFieldId, StandardVertexProgramName);
            windowsSettings.SetFieldValue(PixelProgramFieldId, StandardPixelProgramName);
            windowsSettings.SetFieldValue(VariantFieldId, MeshVariantName);
            windowsSettings.SetFieldValue(TextureIdFieldId, textureAssetId);
            windowsSettings.SetFieldValue(BaseColorFieldId, "#FFE3D2B8");
            windowsSettings.SetFieldValue(CastsShadowFieldId, "true");
            windowsSettings.SetFieldValue(ReceivesShadowFieldId, "true");

            GeneratedMaterialPlatformDefinition gameCubeSettings = definition.GetOrCreatePlatform("gamecube");
            gameCubeSettings.SchemaId = GameCubeMaterialSchemaId;
            gameCubeSettings.SetFieldValue(TextureIdFieldId, textureAssetId);
            gameCubeSettings.SetFieldValue(TextureRelativePathFieldId, "cooked/imported/" + textureAssetId);
            gameCubeSettings.SetFieldValue(DoubleSidedFieldId, "false");
            gameCubeSettings.SetFieldValue(VertexColorModeFieldId, "ignore");
            gameCubeSettings.SetFieldValue(BaseColorFieldId, "#FFFFFFFF");
            gameCubeSettings.SetFieldValue(LightingModeFieldId, "lit");

            return definition;
        }
    }
}
```

```csharp
            TiltTrialPlayerSphereWalnutMaterialFactory materialFactory = new TiltTrialPlayerSphereWalnutMaterialFactory();
            materialFactory.WriteMaterialAsset(projectRootPath);

            RenderingSceneAssetPreparationService assetPreparationService = new RenderingSceneAssetPreparationService();
            RenderingSceneGenerationAssets assets = assetPreparationService.Prepare(projectRootPath);
```

```csharp
        const string TiltTrialPlayerSphereWalnutMaterialAssetId = "Materials.rendering.tilt_trial.PlayerSphereWalnut";
        readonly RuntimeMaterial TiltTrialPlayerSphereWalnutMaterial;
```

```csharp
            RuntimeMaterial tiltTrialPlayerSphereWalnutMaterial = LoadRuntimeMaterial(
                bootstrap,
                projectRootPath,
                TiltTrialPlayerSphereWalnutMaterialFactory.MaterialRelativePath);
```

- [ ] **Step 4: Run the source suite again to verify the walnut references pass**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj -c Debug --filter FullyQualifiedName~CityGameSceneSourceTests
```

Expected:

```text
PASS
```

- [ ] **Step 5: Commit the source wiring checkpoint**

```bash
git -C C:\dev\helprojs\city add assets/codebase/game.tools/GameSceneFactory.cs assets/codebase/game.tools/GameSceneGenerator.cs assets/codebase/rendering.tools/RenderingSceneGenerationAssets.cs assets/codebase/rendering.tools/RenderingSceneAssetPreparationService.cs assets/codebase/rendering.tools/TiltTrialPlayerSphereWalnutMaterialFactory.cs
git -C C:\dev\helworks\helengine add engine/helengine.editor.tests/CityGameSceneSourceTests.cs
git -C C:\dev\helprojs\city commit -m "feat: switch tilt trial sphere to walnut material wiring"
git -C C:\dev\helworks\helengine commit -m "test: lock tilt trial walnut source wiring"
```

### Task 2: Add The Walnut Source Bitmap And Imported-Texture Regression

**Files:**
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\CityGameCubeImportedTextureResolutionTests.cs`
- Create: `C:\dev\helprojs\city\assets\Textures\rendering\tilt_trial\PlayerSphereWalnut.bmp`
- Generate: `C:\dev\helprojs\city\assets\Textures\rendering\tilt_trial\PlayerSphereWalnut.bmp.hasset`

- [ ] **Step 1: Update the imported-texture resolution test to resolve the walnut source texture through generated import settings**

```csharp
    /// <summary>
    /// Absolute authored source texture path that should back the Tilt Trial player sphere material.
    /// </summary>
    const string TiltTrialPlayerSphereTextureSourcePath = @"C:\dev\helprojs\city\assets\Textures\rendering\tilt_trial\PlayerSphereWalnut.bmp";

    /// <summary>
    /// Ensures the Tilt Trial player sphere imported texture id can still resolve back to its authored source bitmap under the GameCube packaging context.
    /// </summary>
    [Fact]
    public void Tilt_trial_player_sphere_imported_texture_id_resolves_to_authored_source_for_gamecube() {
        AssetImportManager manager = CreateManager();
        TextureAssetImportSettings settings = manager.LoadOrCreateTextureImportSettings(TiltTrialPlayerSphereTextureSourcePath);

        Assert.False(string.IsNullOrWhiteSpace(settings.Importer.AssetId));

        bool resolved = manager.TryResolveImportedTextureSourcePath(settings.Importer.AssetId, out string sourcePath);

        Assert.True(resolved);
        Assert.True(
            string.Equals(Path.GetFullPath(TiltTrialPlayerSphereTextureSourcePath), sourcePath, StringComparison.OrdinalIgnoreCase),
            $"Expected resolved Tilt Trial texture source path '{Path.GetFullPath(TiltTrialPlayerSphereTextureSourcePath)}' but found '{sourcePath}'.");
    }
```

- [ ] **Step 2: Run the walnut texture resolution test to verify it fails before the source bitmap exists**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj -c Debug --filter FullyQualifiedName~CityGameCubeImportedTextureResolutionTests.Tilt_trial_player_sphere_imported_texture_id_resolves_to_authored_source_for_gamecube
```

Expected:

```text
FAIL
Could not find file 'C:\dev\helprojs\city\assets\Textures\rendering\tilt_trial\PlayerSphereWalnut.bmp'
```

- [ ] **Step 3: Generate the walnut source bitmap with a deterministic medium-brown wood-grain script**

Run:

```powershell
@'
Add-Type -AssemblyName System.Drawing

$outputPath = 'C:\dev\helprojs\city\assets\Textures\rendering\tilt_trial\PlayerSphereWalnut.bmp'
$directoryPath = Split-Path -Parent $outputPath
if (-not (Test-Path -LiteralPath $directoryPath)) {
    New-Item -ItemType Directory -Path $directoryPath | Out-Null
}

$width = 512
$height = 512
$bitmap = New-Object System.Drawing.Bitmap($width, $height)
$random = [System.Random]::new(290629)

for ($y = 0; $y -lt $height; $y++) {
    for ($x = 0; $x -lt $width; $x++) {
        $waveA = [Math]::Sin(($x / 17.0) + ([Math]::Sin($y / 43.0) * 1.7))
        $waveB = [Math]::Sin((($x + ($y * 0.35)) / 51.0) + 0.9)
        $waveC = [Math]::Sin((($x * 0.6) - $y) / 83.0)
        $noise = ($random.NextDouble() - 0.5) * 0.10
        $tone = (($waveA * 0.58) + ($waveB * 0.24) + ($waveC * 0.10) + $noise + 0.72)
        $tone = [Math]::Clamp($tone, 0.0, 1.0)

        $red = [int](74 + ($tone * 94))
        $green = [int](42 + ($tone * 58))
        $blue = [int](24 + ($tone * 34))
        $bitmap.SetPixel($x, $y, [System.Drawing.Color]::FromArgb(255, $red, $green, $blue))
    }
}

$bitmap.Save($outputPath, [System.Drawing.Imaging.ImageFormat]::Bmp)
$bitmap.Dispose()
'@ | powershell -NoProfile
```

- [ ] **Step 4: Run the texture resolution test again to verify the new walnut bitmap now resolves through the GameCube import path**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj -c Debug --filter FullyQualifiedName~CityGameCubeImportedTextureResolutionTests
```

Expected:

```text
PASS
```

- [ ] **Step 5: Commit the walnut source texture checkpoint**

```bash
git -C C:\dev\helprojs\city add assets/Textures/rendering/tilt_trial/PlayerSphereWalnut.bmp assets/Textures/rendering/tilt_trial/PlayerSphereWalnut.bmp.hasset
git -C C:\dev\helworks\helengine add engine/helengine.editor.tests/CityGameCubeImportedTextureResolutionTests.cs
git -C C:\dev\helprojs\city commit -m "feat: add tilt trial walnut sphere texture"
git -C C:\dev\helworks\helengine commit -m "test: verify tilt trial walnut texture resolution"
```

### Task 3: Regenerate The Authored Tilt Trial Assets And Re-Run Focused Validation

**Files:**
- Generate: `C:\dev\helprojs\city\assets\materials\rendering\tilt_trial\PlayerSphereWalnut.hasset`
- Generate: `C:\dev\helprojs\city\assets\scenes\games\tilt_trial.helen`
- Verify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj`
- Verify: `C:\dev\helworks\helengine-gc\builder.tests\helengine.gamecube.builder.tests.csproj`

- [ ] **Step 1: Regenerate the live Tilt Trial scene so the walnut material file and scene asset are rewritten from the updated generators**

Run:

```powershell
rtk dotnet run --project C:\dev\helworks\helengine\helengine.ui\helengine.editor.app\helengine.editor.app.csproj -c Debug -- --project C:\dev\helprojs\city\project.heproj --editor-command menu.generate-game-scenes
```

Expected:

```text
The command completes successfully and writes PlayerSphereWalnut.hasset plus an updated tilt_trial.helen.
```

- [ ] **Step 2: Verify the generated walnut material and scene files exist where the runtime expects them**

Run:

```powershell
rtk rg -n "PlayerSphereWalnut|Materials.rendering.tilt_trial.PlayerSphereWalnut" C:\dev\helprojs\city\assets\materials\rendering\tilt_trial C:\dev\helprojs\city\assets\scenes\games\tilt_trial.helen
```

Expected:

```text
Matches are reported in PlayerSphereWalnut.hasset and the generated tilt_trial.helen scene.
```

- [ ] **Step 3: Run the focused editor tests that cover source wiring and imported texture resolution**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj -c Debug --filter "FullyQualifiedName~CityGameSceneSourceTests|FullyQualifiedName~CityGameCubeImportedTextureResolutionTests"
```

Expected:

```text
PASS
```

- [ ] **Step 4: Run the existing GameCube builder regression that proves cooked textured materials preserve TextureRelativePath**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine-gc\builder.tests\helengine.gamecube.builder.tests.csproj -c Debug --filter FullyQualifiedName~GameCubePlatformAssetBuilderTests.CookMaterial_WhenUsingGameCubeStandardSchema_ReturnsPlatformMaterialAssetWithoutShaderReferences
```

Expected:

```text
PASS
```

- [ ] **Step 5: Commit the regenerated material and scene checkpoint**

```bash
git -C C:\dev\helprojs\city add assets/materials/rendering/tilt_trial/PlayerSphereWalnut.hasset assets/scenes/games/tilt_trial.helen
git -C C:\dev\helprojs\city commit -m "feat: regenerate tilt trial with walnut sphere material"
```

### Task 4: Build And Smoke-Test The Real GameCube Tilt Trial Path

**Files:**
- Output: `C:\dev\helprojs\output\gamecube-tilt-trial-walnut`
- Verify: `C:\dev\helprojs\output\gamecube-tilt-trial-walnut\game.gcm`
- Verify: `C:\dev\helworks\helengine-gc\scripts\launch_in_emulator.ps1`

- [ ] **Step 1: Build a packaged GameCube image with Tilt Trial forced as the packaged startup scene**

Run:

```powershell
$env:HELENGINE_GAMECUBE_PACKAGED_STARTUP_SCENE_ID = 'tilt_trial'
rtk dotnet run --project C:\dev\helworks\helengine\helengine.ui\helengine.editor.app\helengine.editor.app.csproj -c Debug -- --project C:\dev\helprojs\city\project.heproj --build gamecube --output C:\dev\helprojs\output\gamecube-tilt-trial-walnut --common-output
```

Expected:

```text
The build completes successfully and writes C:\dev\helprojs\output\gamecube-tilt-trial-walnut\game.gcm.
```

- [ ] **Step 2: Launch the packaged GameCube image in Dolphin through the shared launcher script**

Run:

```powershell
powershell -ExecutionPolicy Bypass -File C:\dev\helworks\helengine-gc\scripts\launch_in_emulator.ps1 -ArtifactPath C:\dev\helprojs\output\gamecube-tilt-trial-walnut\game.gcm
```

Expected:

```text
The packaged runtime boots directly into Tilt Trial and the PlayerSphere renders with the new walnut grain instead of the old marble texture.
```

- [ ] **Step 3: Clear the temporary startup-scene override after verification**

Run:

```powershell
Remove-Item Env:HELENGINE_GAMECUBE_PACKAGED_STARTUP_SCENE_ID
```

Expected:

```text
The environment override is removed and later packaged boots return to their normal startup rules.
```

## Notes

- Do not add native GameCube code that checks for `Tilt Trial`, `PlayerSphere`, or walnut asset ids. The existing cooked textured-material path is already in place and should remain generic.
- Keep the walnut source bitmap deterministic and script-generated for the first pass. If visual polish is needed later, replace the bitmap content without changing the material or runtime contract.
- If the generated `PlayerSphereWalnut.bmp.hasset` sidecar appears after the editor test or scene-generation run, keep it. That sidecar is part of the import contract and should be committed with the new source texture.
