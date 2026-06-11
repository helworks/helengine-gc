# Helengine GameCube Packaged Triangle Probe Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace the current packaged-runtime mesh submission with one hardcoded GX triangle so Dolphin shows visible geometry through the working retail `.gcm` path.

**Architecture:** Keep the retail `.gcm` boot chain, DVD mount, generated-core scene startup, frame extraction, and presentation flow unchanged. Isolate the next rendering checkpoint entirely inside `GameCubeRasterRenderer` by bypassing authored mesh submission and emitting one direct-color triangle in screen space.

**Tech Stack:** C++17, libogc GX, Docker devkitPPC build via `rtk`, .NET builder harness, Dolphin manual verification

---

## File Map

- Modify: `src/platform/gamecube/GameCubeRasterRenderer.hpp`
  - Add one temporary private helper that draws the triangle probe.
- Modify: `src/platform/gamecube/GameCubeRasterRenderer.cpp`
  - Route `DrawFrame` into the triangle probe and keep existing extraction diagnostics intact.
- Reuse: `tmp/builder-retail-check/builder-retail-check.csproj`
  - Repackage the updated `build/helengine_gc.dol` into the retail `.gcm`.

### Task 1: Replace Authored Mesh Submission With One Hardcoded Triangle

**Files:**
- Modify: `src/platform/gamecube/GameCubeRasterRenderer.hpp`
- Modify: `src/platform/gamecube/GameCubeRasterRenderer.cpp`

- [ ] **Step 1: Add the triangle-probe helper declaration**

Update `src/platform/gamecube/GameCubeRasterRenderer.hpp` so the renderer exposes one focused helper for the temporary triangle path:

```cpp
    private:
        /// Draws one temporary hardcoded triangle directly into the active GX frame.
        void DrawProbeTriangle(GameCubeFramePlan* framePlan);

        /// Draws one authored runtime submesh through immediate GX triangle submission.
        void DrawSubmesh(GameCubeRuntimeModel* runtimeModel, RuntimeSubmesh* runtimeSubmesh);
```

- [ ] **Step 2: Route `DrawFrame` to the triangle probe**

In `src/platform/gamecube/GameCubeRasterRenderer.cpp`, keep viewport setup, frame diagnostics, and camera/view initialization, but stop before the authored submesh loop and call the probe helper instead:

```cpp
bool GameCubeRasterRenderer::DrawFrame(GameCubeFramePlan* framePlan) {
    if (framePlan == nullptr) {
        throw new ArgumentNullException("framePlan");
    }

    ConfigurePipeline();
    GX_SetViewport(framePlan->Viewport.X, framePlan->Viewport.Y, framePlan->Viewport.Z, framePlan->Viewport.W, 0.0f, 1.0f);
    GX_SetScissor(
        static_cast<u32>(framePlan->Viewport.X),
        static_cast<u32>(framePlan->Viewport.Y),
        static_cast<u32>(framePlan->Viewport.Z),
        static_cast<u32>(framePlan->Viewport.W));

    Mtx44 projection;
    BuildProjectionMatrix(framePlan, projection);
    GX_LoadProjectionMtx(projection, GX_PERSPECTIVE);

    Mtx viewMatrix;
    BuildViewMatrix(framePlan->Camera, viewMatrix);
    RasterizedFrameCount++;

    if ((RasterizedFrameCount <= 5U || (RasterizedFrameCount % 60U) == 0U)
        && framePlan->DrawableSubmissions->get_Count() > 0) {
        RenderFrameDrawableSubmission* submission = (*framePlan->DrawableSubmissions)[0];
        GameCubeRuntimeModel* runtimeModel = MeshCache->Resolve(submission->get_Drawable()->get_Model());
        RuntimeSubmesh* runtimeSubmesh = (*runtimeModel->get_Submeshes())[submission->get_SubmeshIndex()];
        const int32_t firstIndex = runtimeSubmesh->get_IndexStart();
        const uint32_t firstPositionIndex = runtimeModel->Uses32BitIndices
            ? (*runtimeModel->Indices32)[firstIndex]
            : (*runtimeModel->Indices16)[firstIndex];
        const float3 firstPosition = (*runtimeModel->Positions)[firstPositionIndex];
        const float3 entityPosition = submission->get_Drawable()->get_Parent()->get_Position();
        SYS_Report(
            "[GC] Frame %u raster. indexCount=%d firstVertex=(%.3f, %.3f, %.3f) entity=(%.3f, %.3f, %.3f)\n",
            RasterizedFrameCount,
            runtimeSubmesh->get_IndexCount(),
            firstPosition.X,
            firstPosition.Y,
            firstPosition.Z,
            entityPosition.X,
            entityPosition.Y,
            entityPosition.Z);
    }

    DrawProbeTriangle(framePlan);
    return true;
}
```

- [ ] **Step 3: Implement the hardcoded triangle probe**

Add the helper in `src/platform/gamecube/GameCubeRasterRenderer.cpp`. Use an orthographic projection and identity model-view matrix so the triangle is visible without depending on authored transforms or winding:

```cpp
void GameCubeRasterRenderer::DrawProbeTriangle(GameCubeFramePlan* framePlan) {
    if (framePlan == nullptr) {
        throw new ArgumentNullException("framePlan");
    }

    Mtx44 projection;
    guOrtho(projection, 0.0f, framePlan->Viewport.W, 0.0f, framePlan->Viewport.Z, -1.0f, 1.0f);
    GX_LoadProjectionMtx(projection, GX_ORTHOGRAPHIC);

    Mtx modelView;
    guMtxIdentity(modelView);
    GX_LoadPosMtxImm(modelView, GX_PNMTX0);
    GX_SetCurrentMtx(GX_PNMTX0);
    GX_SetCullMode(GX_CULL_NONE);
    GX_SetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);

    const f32 viewportWidth = framePlan->Viewport.Z;
    const f32 viewportHeight = framePlan->Viewport.W;
    const f32 topY = viewportHeight * 0.75f;
    const f32 bottomY = viewportHeight * 0.25f;
    const f32 leftX = viewportWidth * 0.25f;
    const f32 rightX = viewportWidth * 0.75f;
    const f32 centerX = viewportWidth * 0.5f;

    GX_Begin(GX_TRIANGLES, GX_VTXFMT0, 3);
    GX_Position3f32(centerX, topY, 0.0f);
    GX_Color4u8(0xFF, 0x30, 0x30, 0xFF);
    GX_Position3f32(leftX, bottomY, 0.0f);
    GX_Color4u8(0x30, 0xFF, 0x30, 0xFF);
    GX_Position3f32(rightX, bottomY, 0.0f);
    GX_Color4u8(0x30, 0x80, 0xFF, 0xFF);
    GX_End();
}
```

- [ ] **Step 4: Rebuild the GameCube binary**

Run:

```powershell
rtk docker run --rm -v C:/dev/helworks/helengine-gc:/workspace -w /workspace -e HELENGINE_CORE_CPP_ROOT=/workspace/tmp/generated-core-gamecube -e HELENGINE_GAMECUBE_BOOT_MODE=packaged-disc helengine-gc make all
```

Expected:
- `powerpc-eabi-g++` recompiles `GameCubeRasterRenderer.cpp`
- `elf2dol` regenerates `build/helengine_gc.dol`
- No build errors

- [ ] **Step 5: Repackage the retail `.gcm`**

Run:

```powershell
rtk dotnet run --project tmp\builder-retail-check\builder-retail-check.csproj -v minimal
```

Expected:
- Builder completes successfully
- Output includes `ISO/GC`
- Final image path is `C:\dev\helworks\helengine-gc\tmp\session-2026-05-15-gcm-demo-check\game.gcm`

- [ ] **Step 6: Copy to the stable Dolphin launch path**

Run:

```powershell
Copy-Item 'C:\dev\helworks\helengine-gc\tmp\session-2026-05-15-gcm-demo-check\game.gcm' 'C:\dev\helworks\emus\gc-probe\game.gcm' -Force
Get-Item 'C:\dev\helworks\emus\gc-probe\game.gcm' | Select-Object FullName,Length,LastWriteTime
```

Expected:
- `C:\dev\helworks\emus\gc-probe\game.gcm` exists
- Timestamp matches the fresh build

- [ ] **Step 7: Launch Dolphin against the stable copied image**

Run:

```powershell
Get-Process -Name 'Dolphin*' -ErrorAction SilentlyContinue | Stop-Process -Force -ErrorAction SilentlyContinue
Start-Sleep -Milliseconds 500
Start-Process -FilePath 'C:\dev\helworks\emus\dolphin-2603a-x64\Dolphin-x64\Dolphin.exe' -WorkingDirectory 'C:\dev\helworks\emus\dolphin-2603a-x64\Dolphin-x64' -ArgumentList @('-e','C:\dev\helworks\emus\gc-probe\game.gcm')
```

Expected:
- Dolphin opens the packaged image without “file does not exist”
- Packaged runtime still reaches first update and first draw
- Final visible output is one colored triangle

- [ ] **Step 8: Manual verification checkpoint**

Record the result:

```text
If the triangle appears:
- packaged boot path is still good
- presentation path is still good
- raw GX immediate geometry is visible
- next step is to reintroduce authored geometry in very small slices

If the triangle does not appear:
- stay inside GameCubeRasterRenderer only
- inspect projection mode, vertex format, depth state, and presentation ownership
```

- [ ] **Step 9: Commit the triangle probe**

Run:

```bash
git add src/platform/gamecube/GameCubeRasterRenderer.hpp src/platform/gamecube/GameCubeRasterRenderer.cpp
git commit -m "Add GC packaged triangle probe"
```

Expected:
- One focused commit containing only the triangle-probe renderer change

## Self-Review

- Spec coverage: the plan keeps packaged boot, scene startup, and frame extraction intact while narrowing the visible output to one hardcoded triangle.
- Placeholder scan: no TODO/TBD markers remain; each step includes exact file paths and commands.
- Type consistency: the plan introduces one new helper named `DrawProbeTriangle(GameCubeFramePlan* framePlan)` and uses that exact signature consistently.
