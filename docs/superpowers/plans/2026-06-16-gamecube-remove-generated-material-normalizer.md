# GameCube Remove Generated Material Normalizer Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Remove the GameCube generated-core material/runtime rewriter and let the packaged build consume the cooked-platform material contract already emitted by shared codegen.

**Architecture:** Keep the shared `helengine` generated-core contract as the source of truth. Simplify the GameCube builder so it compiles generated output as-is, and replace local tests that depended on rewrite behavior with tests that lock the builder to the declared platform contract and generator output seam.

**Tech Stack:** C#, xUnit, .NET 9, GameCube builder, shared helengine generated-core pipeline

---

### Task 1: Remove the obsolete generated-core normalizer hook

**Files:**
- Delete: `builder/GameCubeGeneratedCoreCompatibilityNormalizer.cs`
- Delete: `builder.tests/GameCubeGeneratedCoreCompatibilityNormalizerTests.cs`
- Modify: `builder/GameCubeDockerNativeBuildExecutor.cs`
- Test: `builder.tests/GameCubeDockerNativeBuildExecutorTests.cs`

- [ ] **Step 1: Write the failing test**

```csharp
[Fact]
public void GameCubeDockerNativeBuildExecutor_Source_DoesNotUseGeneratedCoreNormalizer() {
    string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
    string source = File.ReadAllText(Path.Combine(repositoryRootPath, "builder", "GameCubeDockerNativeBuildExecutor.cs"));

    Assert.DoesNotContain("GameCubeGeneratedCoreCompatibilityNormalizer", source, StringComparison.Ordinal);
    Assert.DoesNotContain(".Normalize(paths.GeneratedCoreRootPath)", source, StringComparison.Ordinal);
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `rtk dotnet test builder.tests\\helengine.gamecube.builder.tests.csproj --no-restore --filter "GameCubeDockerNativeBuildExecutorTests"`

Expected: FAIL because `GameCubeDockerNativeBuildExecutor.cs` still instantiates and invokes `GameCubeGeneratedCoreCompatibilityNormalizer`.

- [ ] **Step 3: Write minimal implementation**

```csharp
public sealed class GameCubeDockerNativeBuildExecutor : IGameCubeNativeBuildExecutor {
    public void Build(GameCubeBuilderPaths paths, CancellationToken cancellationToken) {
        if (paths == null) {
            throw new ArgumentNullException(nameof(paths));
        }

        ProcessStartInfo startInfo = CreateStartInfo(paths);
        // existing Docker invocation remains unchanged
    }
}
```

Also remove the obsolete normalizer source file and its dedicated test file.

- [ ] **Step 4: Run test to verify it passes**

Run: `rtk dotnet test builder.tests\\helengine.gamecube.builder.tests.csproj --no-restore --filter "GameCubeDockerNativeBuildExecutorTests"`

Expected: PASS

- [ ] **Step 5: Commit**

```bash
git add builder/GameCubeDockerNativeBuildExecutor.cs builder.tests/GameCubeDockerNativeBuildExecutorTests.cs builder/GameCubeGeneratedCoreCompatibilityNormalizer.cs builder.tests/GameCubeGeneratedCoreCompatibilityNormalizerTests.cs
git commit -m "refactor: remove gamecube generated core normalizer"
```

### Task 2: Replace rewrite-based assertions with contract-based assertions

**Files:**
- Modify: `builder.tests/GameCubePackagedRuntimeSourceTests.cs`
- Modify: `builder/GameCubePlatformDefinitionFactory.cs`
- Test: `builder.tests/GameCubePackagedRuntimeSourceTests.cs`

- [ ] **Step 1: Write the failing test**

```csharp
[Fact]
public void PackagedDiscBootSource_PublishesCookedMaterialContractWithoutGeneratedRewriter() {
    string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
    string platformDefinitionSource = File.ReadAllText(Path.Combine(repositoryRootPath, "builder", "GameCubePlatformDefinitionFactory.cs"));
    string dockerExecutorSource = File.ReadAllText(Path.Combine(repositoryRootPath, "builder", "GameCubeDockerNativeBuildExecutor.cs"));

    Assert.Contains("RuntimeMaterialResolutionMode.CookedPlatformOwned", platformDefinitionSource, StringComparison.Ordinal);
    Assert.Contains("PackagedPathPolicy.ContentRelativeOnly", platformDefinitionSource, StringComparison.Ordinal);
    Assert.DoesNotContain("GameCubeGeneratedCoreCompatibilityNormalizer", dockerExecutorSource, StringComparison.Ordinal);
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `rtk dotnet test builder.tests\\helengine.gamecube.builder.tests.csproj --no-restore --filter "PackagedDiscBootSource_PublishesCookedMaterialContractWithoutGeneratedRewriter"`

Expected: FAIL because the Docker executor still references the normalizer.

- [ ] **Step 3: Write minimal implementation**

```csharp
Assert.Contains("RuntimeMaterialResolutionMode.CookedPlatformOwned", platformDefinitionSource, StringComparison.Ordinal);
Assert.Contains("PackagedPathPolicy.ContentRelativeOnly", platformDefinitionSource, StringComparison.Ordinal);
Assert.DoesNotContain("GameCubeGeneratedCoreCompatibilityNormalizer", dockerExecutorSource, StringComparison.Ordinal);
```

Remove the old assertions that read `builder/GameCubeGeneratedCoreCompatibilityNormalizer.cs` and treated generated-code rewriting as the expected material/textured-runtime seam.

- [ ] **Step 4: Run test to verify it passes**

Run: `rtk dotnet test builder.tests\\helengine.gamecube.builder.tests.csproj --no-restore --filter "GameCubePackagedRuntimeSourceTests"`

Expected: PASS

- [ ] **Step 5: Commit**

```bash
git add builder.tests/GameCubePackagedRuntimeSourceTests.cs builder/GameCubePlatformDefinitionFactory.cs
git commit -m "test: lock gamecube packaged runtime to platform contract"
```

### Task 3: Verify the packaged city GameCube build still completes

**Files:**
- Modify: none expected
- Verify: `tmp/gamecube-build-current.log`

- [ ] **Step 1: Rebuild the GameCube builder**

Run: `rtk dotnet build builder\\helengine.gamecube.builder.csproj --no-restore`

Expected: `0 Error(s)`

- [ ] **Step 2: Run the packaged city build**

Run:

```powershell
$env:HELENGINE_GAMECUBE_APPLOADER_PATH='C:\dev\helprojs\output\gamecube-mainmenu-smoke\disc\sys\apploader.img'
rtk dotnet C:\dev\helworks\helengine\helengine.ui\helengine.editor.app\bin\Debug\net9.0-windows\helengine.editor.app.dll --project C:\dev\helprojs\city\project.heproj --build gamecube --output C:\dev\helprojs\city\gamecube-build
```

Expected: build reaches image output and writes `C:\dev\helprojs\city\gamecube-build\game.gcm`

- [ ] **Step 3: Verify the artifact exists**

Run: `rtk powershell -NoProfile -Command "Get-Item 'C:\\dev\\helprojs\\city\\gamecube-build\\game.gcm' | Format-List FullName,Length,LastWriteTime"`

Expected: file metadata for `game.gcm`

- [ ] **Step 4: Launch in Dolphin**

Run: `powershell -ExecutionPolicy Bypass -File .\\scripts\\launch_gamecube_image_in_dolphin.ps1 -IsoPath C:\\dev\\helprojs\\city\\gamecube-build\\game.gcm`

Expected: Dolphin opens the newly built image

- [ ] **Step 5: Commit**

```bash
git add tmp/gamecube-build-current.log
git commit -m "chore: verify gamecube packaged build without generated rewriter"
```
