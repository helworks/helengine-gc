# GameCube Text Effects Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Render authored GameCube text shadows and four-direction outlines before primary glyphs.

**Architecture:** Keep effect policy in `GameCubeRasterRenderer::RenderText2D` and reuse a new no-allocation glyph-pass helper for every pass. The helper retains the current wrapped-content cache, clipping, font atlas, logical-to-physical transform, and GX textured-quad submission. The GameCube implementation mirrors the engine's `TextRenderEffectPassBuilder` order without calling its generated `List`-allocating API each frame.

**Tech Stack:** C++20, libogc GX, generated HelEngine text/font runtime, xUnit source-contract tests.

---

### Task 1: Lock the GameCube text-effect contract in a failing test

**Files:**
- Modify: `builder.tests/GameCubePackagedRuntimeSourceTests.cs`
- Modify: `src/platform/gamecube/GameCubeRasterRenderer.cpp`
- Modify: `src/platform/gamecube/GameCubeRasterRenderer.hpp`

- [ ] **Step 1: Write the failing source-contract test**

Add this fact beside the existing GameCube renderer source-contract tests:

```csharp
/// <summary>
/// Ensures GameCube text replays cached glyph layout for shadow, outline, and primary passes.
/// </summary>
[Fact]
public void GameCubeRasterRendererSource_RendersAuthoredTextShadowAndOutlinePasses() {
    string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
    string source = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRasterRenderer.cpp"));
    string header = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRasterRenderer.hpp"));

    Assert.Contains("get_ShadowOffset()", source, StringComparison.Ordinal);
    Assert.Contains("get_OutlineScale()", source, StringComparison.Ordinal);
    Assert.Contains("DrawTextGlyphPass2D", source, StringComparison.Ordinal);
    Assert.Contains("void DrawTextGlyphPass2D", header, StringComparison.Ordinal);
}
```

- [ ] **Step 2: Run the focused test and verify it fails**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine-gc\builder.tests\helengine.gamecube.builder.tests.csproj --no-restore --filter "FullyQualifiedName~GameCubeRasterRendererSource_RendersAuthoredTextShadowAndOutlinePasses" --nologo
```

Expected: the test fails because `DrawTextGlyphPass2D` and authored effect properties are absent from the GameCube renderer.

- [ ] **Step 3: Add the focused glyph-pass helper declaration**

Add this private member beside `RenderText2D` in `GameCubeRasterRenderer.hpp`:

```cpp
/// Draws one cached text layout at one authored effect offset and color without rebuilding the layout.
void DrawTextGlyphPass2D(
    GameCubeFramePlan* framePlan,
    FontAsset* font,
    GameCubeRuntimeTexture* texture,
    const std::string& content,
    double fontScale,
    double baseX,
    double baseY,
    const float2& passOffset,
    GXColor glyphColor);
```

- [ ] **Step 4: Implement the helper in `GameCubeRasterRenderer.cpp`**

Move the existing character loop from `RenderText2D` into `DrawTextGlyphPass2D`. Preserve newline handling, space advance, missing-glyph skip, `TransformLogicalRectToPhysicalViewport`, `DrawTexturedQuad2D`, and advance-width fallback. Add `passOffset.X` and `passOffset.Y` to each glyph's logical position before the viewport transform.

- [ ] **Step 5: Render passes in authored order**

After resolving `content`, `glyphColor`, `baseX`, and `baseY` in `RenderText2D`, submit these passes:

```cpp
const float2 shadowOffset = drawable->get_ShadowOffset();
if (shadowOffset.X != 0.0f || shadowOffset.Y != 0.0f) {
    DrawTextGlyphPass2D(framePlan, font, texture, content, fontScale, baseX, baseY, shadowOffset, ConvertByteColorToGx(drawable->get_ShadowColor()));
}

const float outlineScale = drawable->get_OutlineScale();
if (outlineScale > 0.0f) {
    const GXColor outlineColor = ConvertByteColorToGx(drawable->get_OutlineColor());
    DrawTextGlyphPass2D(framePlan, font, texture, content, fontScale, baseX, baseY, float2(-outlineScale, 0.0f), outlineColor);
    DrawTextGlyphPass2D(framePlan, font, texture, content, fontScale, baseX, baseY, float2(outlineScale, 0.0f), outlineColor);
    DrawTextGlyphPass2D(framePlan, font, texture, content, fontScale, baseX, baseY, float2(0.0f, -outlineScale), outlineColor);
    DrawTextGlyphPass2D(framePlan, font, texture, content, fontScale, baseX, baseY, float2(0.0f, outlineScale), outlineColor);
}

DrawTextGlyphPass2D(framePlan, font, texture, content, fontScale, baseX, baseY, float2(0.0f, 0.0f), glyphColor);
```

- [ ] **Step 6: Run the focused test and verify it passes**

Run the command from Step 2.

Expected: PASS.

### Task 2: Compile the packaged native renderer

**Files:**
- Modify: `src/platform/gamecube/GameCubeRasterRenderer.cpp`
- Modify: `src/platform/gamecube/GameCubeRasterRenderer.hpp`
- Test: `builder.tests/GameCubePackagedRuntimeSourceTests.cs`

- [ ] **Step 1: Run the source-contract test after the implementation**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine-gc\builder.tests\helengine.gamecube.builder.tests.csproj --no-restore --filter "FullyQualifiedName~GameCubeRasterRendererSource_RendersAuthoredTextShadowAndOutlinePasses" --nologo
```

Expected: PASS with no test failures.

- [ ] **Step 2: Build one packaged GameCube image through the editor CLI**

Run:

```powershell
rtk dotnet C:\dev\helworks\helengine\helengine.ui\helengine.editor.app\bin\Debug\net9.0-windows\helengine.editor.app.dll --project C:\dev\helprojs\demodisc\project.heproj --build gamecube --output C:\dev\helprojs\demodisc\output\gamecube-text-effects
```

Expected: `game.gcm` exists and `gamecube-build-phase.txt` ends with `packaged outputs verified`.

- [ ] **Step 3: Verify the focused diff has no whitespace errors**

Run:

```powershell
rtk git diff --check -- src/platform/gamecube/GameCubeRasterRenderer.cpp src/platform/gamecube/GameCubeRasterRenderer.hpp builder.tests/GameCubePackagedRuntimeSourceTests.cs
```

Expected: no whitespace errors.
