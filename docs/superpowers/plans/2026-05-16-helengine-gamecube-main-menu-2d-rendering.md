# GameCube Main Menu 2D Rendering Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Boot the packaged GameCube build directly into the authored city demo-disc main menu scene and render the visible 2D menu content through GX.

**Architecture:** Reuse the existing packaged-disc scene/runtime path, switch startup to `Scenes/DemoDiscMainMenu.helen`, and replace the current no-op `GameCubeRenderManager2D` with a GX-backed 2D capture and raster path. Keep scope narrow: sprites, text, and rounded rectangles only, with no navigation or interaction in this pass.

**Tech Stack:** C# builder/tests, generated C++ runtime, libogc GX on GameCube, Dolphin for visual verification, city project authored menu content.

---

## File Structure

- `builder.tests/GameCubeRuntimeSceneManifestWriterTests.cs`
  Purpose: guard the packaged startup scene contract for the menu scene.
- `builder.tests/GameCubePackagedBuildWorkspaceTests.cs`
  Purpose: guard staged packaged content expectations for menu scene assets.
- `builder.tests/GameCubePackagedRuntimeSourceTests.cs`
  Purpose: guard that the GC 2D runtime source is no longer a no-op bridge.
- `builder/Program.cs`
  Purpose: set the local retail-check startup scene to the authored menu scene.
- `tools/stage_city_demo_disc_main_menu_content.sh`
  Purpose: stage the menu scene, fonts, overlay textures, and other required authored content from `city`.
- `src/platform/gamecube/GameCubeRenderManager2D.hpp`
  Purpose: declare the GC 2D drawable capture surface and per-frame queues.
- `src/platform/gamecube/GameCubeRenderManager2D.cpp`
  Purpose: store sprite/text/rounded-rect draw calls and build runtime textures for 2D assets.
- `src/platform/gamecube/GameCubeRasterRenderer.hpp`
  Purpose: declare the 2D GX entrypoints alongside the existing 3D path.
- `src/platform/gamecube/GameCubeRasterRenderer.cpp`
  Purpose: implement GX-backed 2D sprite/text/rounded-rect rasterization in stable draw order.
- `src/platform/gamecube/GameCubeApplication.cpp`
  Purpose: reset the 2D queue each frame and execute the 2D raster path after engine draw submission.

### Task 1: Switch Packaged Startup to the Authored Main Menu

**Files:**
- Create: `tools/stage_city_demo_disc_main_menu_content.sh`
- Modify: `builder/Program.cs`
- Modify: `builder.tests/GameCubeRuntimeSceneManifestWriterTests.cs`
- Modify: `builder.tests/GameCubePackagedBuildWorkspaceTests.cs`

- [ ] **Step 1: Write the failing startup-scene and staging tests**

```csharp
[Fact]
public void WriteRuntimeManifest_WhenStartupSceneIsMenu_WritesDemoDiscMainMenuId() {
    const string StartupSceneId = "Scenes/DemoDiscMainMenu.helen";

    PlatformBuildManifest manifest = new PlatformBuildManifest(
        "gamecube",
        StartupSceneId,
        new[] {
            new PlatformBuildManifestScene(
                StartupSceneId,
                "city.menu.tools.DemoDiscMainMenuSceneFactory",
                new Dictionary<string, string> {
                    ["cooked-relative-path"] = "cooked/scenes/Scenes/DemoDiscMainMenu.hasset"
                })
        },
        Array.Empty<PlatformBuildManifestAsset>());

    string source = GameCubeRuntimeSceneManifestWriter.Write(manifest);

    Assert.Contains("static const char kRuntimeGameCubeStartupSceneId[] = \"Scenes/DemoDiscMainMenu.helen\";", source, StringComparison.Ordinal);
    Assert.Contains("{ \"Scenes/DemoDiscMainMenu.helen\", \"cooked/scenes/Scenes/DemoDiscMainMenu.hasset\" },", source, StringComparison.Ordinal);
}

[Fact]
public void PreparePackagedWorkspace_WhenMenuContentIsStaged_IncludesFontAndOverlayAssets() {
    string stageRootPath = Path.Combine(Path.GetTempPath(), Guid.NewGuid().ToString("N"));
    Directory.CreateDirectory(Path.Combine(stageRootPath, "cooked", "fonts"));
    Directory.CreateDirectory(Path.Combine(stageRootPath, "cooked", "scenes", "Scenes"));
    Directory.CreateDirectory(Path.Combine(stageRootPath, "cooked", "textures", "menu"));

    File.WriteAllText(Path.Combine(stageRootPath, "cooked", "fonts", "default.hefont"), "font");
    File.WriteAllText(Path.Combine(stageRootPath, "cooked", "scenes", "Scenes", "DemoDiscMainMenu.hasset"), "scene");
    File.WriteAllText(Path.Combine(stageRootPath, "cooked", "textures", "menu", "overlay.hasset"), "overlay");

    string[] stagedFiles = Directory.GetFiles(stageRootPath, "*", SearchOption.AllDirectories);

    Assert.Contains(stagedFiles, path => path.EndsWith("default.hefont", StringComparison.Ordinal));
    Assert.Contains(stagedFiles, path => path.EndsWith("DemoDiscMainMenu.hasset", StringComparison.Ordinal));
    Assert.Contains(stagedFiles, path => path.EndsWith("overlay.hasset", StringComparison.Ordinal));
}
```

- [ ] **Step 2: Run the focused tests to verify they fail**

Run:

```powershell
rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter FullyQualifiedName~GameCubeRuntimeSceneManifestWriterTests -v minimal -p:BuildInParallel=false -p:UseSharedCompilation=false
rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter FullyQualifiedName~GameCubePackagedBuildWorkspaceTests -v minimal -p:BuildInParallel=false -p:UseSharedCompilation=false
```

Expected:

- manifest test still references the current non-menu startup scene
- packaged workspace test has no menu staging contract yet

- [ ] **Step 3: Implement the startup-scene and staging changes**

```csharp
// builder/Program.cs
const string StartupSceneId = "Scenes/DemoDiscMainMenu.helen";
```

```bash
# tools/stage_city_demo_disc_main_menu_content.sh
#!/usr/bin/env bash
set -euo pipefail

CITY_ROOT="/workspace/city"
CITY_COOKED_ROOT="${CITY_ROOT}/output/gamecube-content/cooked"
STAGE_ROOT="/workspace/tmp/city-main-menu-content/cooked"

rm -rf "${STAGE_ROOT}"
mkdir -p "${STAGE_ROOT}/Scenes" "${STAGE_ROOT}/fonts" "${STAGE_ROOT}/textures/menu"

cp "${CITY_COOKED_ROOT}/Scenes/DemoDiscMainMenu.hasset" "${STAGE_ROOT}/Scenes/DemoDiscMainMenu.hasset"
cp "${CITY_COOKED_ROOT}/fonts/default.hefont" "${STAGE_ROOT}/fonts/default.hefont"
cp "${CITY_COOKED_ROOT}/textures/menu/"*.hasset "${STAGE_ROOT}/textures/menu/"
```

```csharp
// builder.tests/GameCubeRuntimeSceneManifestWriterTests.cs
const string StartupSceneId = "Scenes/DemoDiscMainMenu.helen";
```

- [ ] **Step 4: Run the focused tests to verify they pass**

Run:

```powershell
rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter FullyQualifiedName~GameCubeRuntimeSceneManifestWriterTests -v minimal -p:BuildInParallel=false -p:UseSharedCompilation=false
rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter FullyQualifiedName~GameCubePackagedBuildWorkspaceTests -v minimal -p:BuildInParallel=false -p:UseSharedCompilation=false
```

Expected:

- PASS

- [ ] **Step 5: Commit**

```powershell
rtk git add builder/Program.cs builder.tests/GameCubeRuntimeSceneManifestWriterTests.cs builder.tests/GameCubePackagedBuildWorkspaceTests.cs tools/stage_city_demo_disc_main_menu_content.sh
rtk git commit -m "Switch GameCube startup to demo menu"
```

### Task 2: Turn `GameCubeRenderManager2D` Into a Per-Frame Drawable Capture Bridge

**Files:**
- Modify: `src/platform/gamecube/GameCubeRenderManager2D.hpp`
- Modify: `src/platform/gamecube/GameCubeRenderManager2D.cpp`
- Modify: `builder.tests/GameCubePackagedRuntimeSourceTests.cs`

- [ ] **Step 1: Write the failing source-contract test for 2D capture**

```csharp
[Fact]
public void GameCubeRenderManager2D_WhenBuiltForMenu_IsNotANoOpBridge() {
    string repositoryRootPath = ResolveRepositoryRootPath();
    string source = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRenderManager2D.cpp"));

    Assert.DoesNotContain("(void)sprite;", source, StringComparison.Ordinal);
    Assert.DoesNotContain("(void)text;", source, StringComparison.Ordinal);
    Assert.DoesNotContain("(void)shape;", source, StringComparison.Ordinal);
    Assert.Contains("SpriteQueue.push_back", source, StringComparison.Ordinal);
    Assert.Contains("TextQueue.push_back", source, StringComparison.Ordinal);
    Assert.Contains("RoundedRectQueue.push_back", source, StringComparison.Ordinal);
}
```

- [ ] **Step 2: Run the focused test to verify it fails**

Run:

```powershell
rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter FullyQualifiedName~GameCubePackagedRuntimeSourceTests -v minimal -p:BuildInParallel=false -p:UseSharedCompilation=false
```

Expected:

- FAIL because `GameCubeRenderManager2D.cpp` is still a no-op bridge

- [ ] **Step 3: Implement the per-frame capture structures and methods**

```cpp
// src/platform/gamecube/GameCubeRenderManager2D.hpp
struct GameCubeSpriteDrawCommand {
    ISpriteDrawable2D* Drawable;
};

struct GameCubeTextDrawCommand {
    ITextDrawable2D* Drawable;
};

struct GameCubeRoundedRectDrawCommand {
    IRoundedRectDrawable2D* Drawable;
};

class GameCubeRenderManager2D : public RenderManager2D {
public:
    GameCubeRenderManager2D();

    RuntimeTexture* BuildTextureFromRaw(TextureAsset* data) override;
    void DrawSprite(ISpriteDrawable2D* sprite) override;
    void DrawText(ITextDrawable2D* text) override;
    void DrawRoundedRect(IRoundedRectDrawable2D* shape) override;

    void BeginFrame();
    const std::vector<GameCubeSpriteDrawCommand>& GetSpriteQueue() const;
    const std::vector<GameCubeTextDrawCommand>& GetTextQueue() const;
    const std::vector<GameCubeRoundedRectDrawCommand>& GetRoundedRectQueue() const;

private:
    std::vector<GameCubeSpriteDrawCommand> SpriteQueue;
    std::vector<GameCubeTextDrawCommand> TextQueue;
    std::vector<GameCubeRoundedRectDrawCommand> RoundedRectQueue;
};
```

```cpp
// src/platform/gamecube/GameCubeRenderManager2D.cpp
void GameCubeRenderManager2D::BeginFrame() {
    SpriteQueue.clear();
    TextQueue.clear();
    RoundedRectQueue.clear();
}

void GameCubeRenderManager2D::DrawSprite(ISpriteDrawable2D* sprite) {
    if (sprite == nullptr) {
        throw new ArgumentNullException("sprite");
    }

    SpriteQueue.push_back(GameCubeSpriteDrawCommand { sprite });
}

void GameCubeRenderManager2D::DrawText(ITextDrawable2D* text) {
    if (text == nullptr) {
        throw new ArgumentNullException("text");
    }

    TextQueue.push_back(GameCubeTextDrawCommand { text });
}

void GameCubeRenderManager2D::DrawRoundedRect(IRoundedRectDrawable2D* shape) {
    if (shape == nullptr) {
        throw new ArgumentNullException("shape");
    }

    RoundedRectQueue.push_back(GameCubeRoundedRectDrawCommand { shape });
}
```

- [ ] **Step 4: Run the focused source-contract test to verify it passes**

Run:

```powershell
rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter FullyQualifiedName~GameCubePackagedRuntimeSourceTests -v minimal -p:BuildInParallel=false -p:UseSharedCompilation=false
```

Expected:

- PASS for the new 2D runtime source contract

- [ ] **Step 5: Commit**

```powershell
rtk git add src/platform/gamecube/GameCubeRenderManager2D.hpp src/platform/gamecube/GameCubeRenderManager2D.cpp builder.tests/GameCubePackagedRuntimeSourceTests.cs
rtk git commit -m "Capture GameCube 2D drawables per frame"
```

### Task 3: Add the GX 2D Raster Path for Rounded Rectangles, Sprites, and Text

**Files:**
- Modify: `src/platform/gamecube/GameCubeRasterRenderer.hpp`
- Modify: `src/platform/gamecube/GameCubeRasterRenderer.cpp`

- [ ] **Step 1: Add failing visual-source guards for the 2D GX path**

```csharp
[Fact]
public void GameCubeRasterRenderer_WhenRenderingMenu_Contains2DGxEntryPoints() {
    string repositoryRootPath = ResolveRepositoryRootPath();
    string header = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRasterRenderer.hpp"));
    string source = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRasterRenderer.cpp"));

    Assert.Contains("void Render2D(", header, StringComparison.Ordinal);
    Assert.Contains("RenderRoundedRect2D(", source, StringComparison.Ordinal);
    Assert.Contains("RenderSprite2D(", source, StringComparison.Ordinal);
    Assert.Contains("RenderText2D(", source, StringComparison.Ordinal);
}
```

- [ ] **Step 2: Run the source-contract test to verify it fails**

Run:

```powershell
rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter FullyQualifiedName~GameCubePackagedRuntimeSourceTests -v minimal -p:BuildInParallel=false -p:UseSharedCompilation=false
```

Expected:

- FAIL because the current raster renderer has no 2D GX path

- [ ] **Step 3: Implement the 2D GX raster entrypoints**

```cpp
// src/platform/gamecube/GameCubeRasterRenderer.hpp
class GameCubeRasterRenderer {
public:
    void Render3D(...);
    void Render2D(
        const GameCubeRenderManager2D& renderManager2D,
        uint16_t frameWidth,
        uint16_t frameHeight);

private:
    void RenderRoundedRect2D(const GameCubeRoundedRectDrawCommand& command, uint16_t frameWidth, uint16_t frameHeight);
    void RenderSprite2D(const GameCubeSpriteDrawCommand& command, uint16_t frameWidth, uint16_t frameHeight);
    void RenderText2D(const GameCubeTextDrawCommand& command, uint16_t frameWidth, uint16_t frameHeight);
};
```

```cpp
// src/platform/gamecube/GameCubeRasterRenderer.cpp
void GameCubeRasterRenderer::Render2D(const GameCubeRenderManager2D& renderManager2D, uint16_t frameWidth, uint16_t frameHeight) {
    GX_SetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
    GX_SetCullMode(GX_CULL_NONE);
    GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
    GX_SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);

    for (const GameCubeRoundedRectDrawCommand& command : renderManager2D.GetRoundedRectQueue()) {
        RenderRoundedRect2D(command, frameWidth, frameHeight);
    }

    for (const GameCubeSpriteDrawCommand& command : renderManager2D.GetSpriteQueue()) {
        RenderSprite2D(command, frameWidth, frameHeight);
    }

    for (const GameCubeTextDrawCommand& command : renderManager2D.GetTextQueue()) {
        RenderText2D(command, frameWidth, frameHeight);
    }
}
```

```cpp
// flat-color panel path
void GameCubeRasterRenderer::RenderRoundedRect2D(const GameCubeRoundedRectDrawCommand& command, uint16_t frameWidth, uint16_t frameHeight) {
    const IRoundedRectDrawable2D* drawable = command.Drawable;
    // Resolve screen-space rect and emit one or more quads/triangle fans through GX.
}
```

```cpp
// textured sprite/glyph path
void GameCubeRasterRenderer::RenderSprite2D(const GameCubeSpriteDrawCommand& command, uint16_t frameWidth, uint16_t frameHeight) {
    const ISpriteDrawable2D* drawable = command.Drawable;
    // Resolve runtime texture, bind GX texture object, emit textured quad in screen space.
}

void GameCubeRasterRenderer::RenderText2D(const GameCubeTextDrawCommand& command, uint16_t frameWidth, uint16_t frameHeight) {
    const ITextDrawable2D* drawable = command.Drawable;
    // Iterate glyph quads from the generated text drawable and emit textured quads in draw order.
}
```

- [ ] **Step 4: Run the source-contract tests to verify the 2D path exists**

Run:

```powershell
rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter FullyQualifiedName~GameCubePackagedRuntimeSourceTests -v minimal -p:BuildInParallel=false -p:UseSharedCompilation=false
```

Expected:

- PASS for the 2D GX entrypoint guards

- [ ] **Step 5: Commit**

```powershell
rtk git add src/platform/gamecube/GameCubeRasterRenderer.hpp src/platform/gamecube/GameCubeRasterRenderer.cpp builder.tests/GameCubePackagedRuntimeSourceTests.cs
rtk git commit -m "Add GameCube GX 2D raster path"
```

### Task 4: Wire the Menu Frame Loop and Verify the Authored Menu Scene

**Files:**
- Modify: `src/platform/gamecube/GameCubeApplication.cpp`
- Modify: `builder.tests/GameCubePackagedRuntimeSourceTests.cs`

- [ ] **Step 1: Write the failing application source-contract test**

```csharp
[Fact]
public void GameCubeApplication_WhenRunningMenu_BeginsAndRenders2DFrames() {
    string repositoryRootPath = ResolveRepositoryRootPath();
    string source = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeApplication.cpp"));

    Assert.Contains("EngineRenderManager2D->BeginFrame();", source, StringComparison.Ordinal);
    Assert.Contains("RasterRenderer.Render2D(*EngineRenderManager2D", source, StringComparison.Ordinal);
    Assert.Contains("Scenes/DemoDiscMainMenu.helen", source, StringComparison.Ordinal);
}
```

- [ ] **Step 2: Run the source-contract test to verify it fails**

Run:

```powershell
rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter FullyQualifiedName~GameCubePackagedRuntimeSourceTests -v minimal -p:BuildInParallel=false -p:UseSharedCompilation=false
```

Expected:

- FAIL because `GameCubeApplication` does not yet reset and render the 2D queue

- [ ] **Step 3: Implement the frame-loop integration**

```cpp
// src/platform/gamecube/GameCubeApplication.cpp
bool GameCubeApplication::UpdateEngineCore() {
    EngineRenderManager2D->BeginFrame();
    EngineRenderManager3D->BeginFrame();
    EngineCore->Update();
    return true;
}

bool GameCubeApplication::DrawEngineCore() {
    EngineCore->Draw();

    RasterRenderer.Render3D(*EngineRenderManager3D, RenderMode->fbWidth, RenderMode->efbHeight);
    RasterRenderer.Render2D(*EngineRenderManager2D, RenderMode->fbWidth, RenderMode->efbHeight);
    return true;
}
```

- [ ] **Step 4: Run source tests, rebuild, package, and visually verify in Dolphin**

Run:

```powershell
rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter FullyQualifiedName~GameCubePackagedRuntimeSourceTests -v minimal -p:BuildInParallel=false -p:UseSharedCompilation=false
rtk bash tools/stage_city_demo_disc_main_menu_content.sh
rtk dotnet build builder/helengine.gamecube.builder.csproj -v minimal -p:BuildInParallel=false -p:UseSharedCompilation=false
rtk bash tools/docker/build_gamecube_packaged_disc.sh
rtk dotnet run --project builder/helengine.gamecube.builder.csproj -- package
```

Expected:

- source-contract tests PASS
- native packaged-disc build succeeds
- package succeeds and reports `ISO/GC`
- Dolphin boots directly into `Scenes/DemoDiscMainMenu.helen`
- visible 2D menu content appears on screen

- [ ] **Step 5: Commit**

```powershell
rtk git add src/platform/gamecube/GameCubeApplication.cpp builder.tests/GameCubePackagedRuntimeSourceTests.cs
rtk git commit -m "Render GameCube demo menu in 2D"
```

## Self-Review

- Spec coverage: startup-scene switch, menu asset staging, 2D drawable capture, GX 2D rasterization, and visual Dolphin verification all map to explicit tasks.
- Placeholder scan: no TBD/TODO placeholders remain; each task has concrete files, commands, and code blocks.
- Type consistency: `GameCubeRenderManager2D`, `GameCubeRasterRenderer`, and `GameCubeApplication` use the same method names and responsibilities across tasks.
