# GameCube Bad Triangle Capture Simulation Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a temporary local-only GameCube renderer patch that simulates the old broken two-triangle behavior for video capture on the latest stable build.

**Architecture:** Keep the simulation entirely inside `GameCubeRasterRenderer` so scene boot, menu flow, content loading, and 2D rendering remain untouched. The renderer will clamp to two white triangles, route both through intentionally bad transform math, and apply deterministic frame-based disappearance and distortion so the footage is easy to reproduce.

**Tech Stack:** C++, libogc GX, GameCube packaged build pipeline, Dolphin

---

### Task 1: Add Temporary 3D Capture Simulation Path

**Files:**
- Modify: `src/platform/gamecube/GameCubeRasterRenderer.hpp`
- Modify: `src/platform/gamecube/GameCubeRasterRenderer.cpp`

- [ ] **Step 1: Add the temporary renderer declarations**

Update `src/platform/gamecube/GameCubeRasterRenderer.hpp` to declare the temporary helpers needed to keep the bad capture logic localized:

```cpp
        /// Builds one intentionally wrong model-view matrix used only by the temporary capture simulation.
        void BuildBadCaptureModelViewMatrix(GameCubeFramePlan* framePlan, Entity* entity, float4x4& modelViewMatrix);

        /// Returns whether one of the two simulated capture triangles should be emitted this frame.
        bool ShouldDrawCaptureTriangle(uint32_t frameIndex, int32_t triangleIndex) const;

        /// Returns one per-frame scale factor used to simulate the old matrix explosions.
        float GetCaptureDistortionScale(uint32_t frameIndex) const;

        /// Emits one temporary capture triangle through the intentionally bad transform path.
        void DrawCaptureTriangle(GameCubeFramePlan* framePlan, Entity* entity, const float3& localA, const float3& localB, const float3& localC, uint32_t frameIndex, int32_t triangleIndex);
```

- [ ] **Step 2: Replace the normal first-submesh path with two simulated triangles**

In `src/platform/gamecube/GameCubeRasterRenderer.cpp`, update `DrawFrame(...)` so it no longer submits the full runtime mesh during capture. Instead:

- take the first drawable submission only
- resolve its entity
- call `DrawCaptureTriangle(...)` twice with two different local-space triangles
- return immediately after those two draws

Use these local triangles for the capture:

```cpp
        const float3 leftTriangleA(-0.8f, -0.8f, -0.5f);
        const float3 leftTriangleB(0.0f, 0.8f, -0.5f);
        const float3 leftTriangleC(0.8f, -0.8f, -0.5f);

        const float3 rightTriangleA(-0.6f, -0.6f, -0.5f);
        const float3 rightTriangleB(0.2f, 0.9f, -0.5f);
        const float3 rightTriangleC(0.9f, -0.4f, -0.5f);
```

Drive the frame counter with a local static:

```cpp
        static uint32_t captureFrameIndex = 0U;
        captureFrameIndex++;
```

- [ ] **Step 3: Implement the intentionally broken matrix path**

In `src/platform/gamecube/GameCubeRasterRenderer.cpp`, implement `BuildBadCaptureModelViewMatrix(...)` so it keeps using the current entity world transform but composes it incorrectly on purpose:

```cpp
    void GameCubeRasterRenderer::BuildBadCaptureModelViewMatrix(GameCubeFramePlan* framePlan, Entity* entity, float4x4& modelViewMatrix) {
        if (framePlan == nullptr) {
            throw new ArgumentNullException("framePlan");
        } else if (entity == nullptr) {
            throw new ArgumentNullException("entity");
        }

        float4x4 worldMatrix;
        BuildWorldMatrix(entity, worldMatrix);
        float4x4::Multiply(framePlan->View, worldMatrix, modelViewMatrix);
    }
```

This is intentionally the wrong order. Do not “fix” it in this temporary patch.

- [ ] **Step 4: Implement the disappearance and distortion pattern**

In `src/platform/gamecube/GameCubeRasterRenderer.cpp`, implement:

```cpp
    bool GameCubeRasterRenderer::ShouldDrawCaptureTriangle(uint32_t frameIndex, int32_t triangleIndex) const {
        const uint32_t phase = frameIndex % 180U;
        if (phase >= 40U && phase < 52U) {
            return triangleIndex == 0;
        }

        if (phase >= 100U && phase < 112U) {
            return triangleIndex == 1;
        }

        if (phase >= 150U && phase < 162U) {
            return false;
        }

        return true;
    }

    float GameCubeRasterRenderer::GetCaptureDistortionScale(uint32_t frameIndex) const {
        const uint32_t phase = frameIndex % 180U;
        if (phase >= 20U && phase < 28U) {
            return 2.8f;
        }

        if (phase >= 80U && phase < 88U) {
            return 0.35f;
        }

        if (phase >= 120U && phase < 128U) {
            return 4.5f;
        }

        return 1.0f;
    }
```

These numbers are deliberately theatrical rather than historically exact. The goal is clear footage, not perfect bug archaeology.

- [ ] **Step 5: Emit both white triangles through the bad path**

Implement `DrawCaptureTriangle(...)` in `src/platform/gamecube/GameCubeRasterRenderer.cpp`:

```cpp
    void GameCubeRasterRenderer::DrawCaptureTriangle(GameCubeFramePlan* framePlan, Entity* entity, const float3& localA, const float3& localB, const float3& localC, uint32_t frameIndex, int32_t triangleIndex) {
        if (framePlan == nullptr) {
            throw new ArgumentNullException("framePlan");
        } else if (entity == nullptr) {
            throw new ArgumentNullException("entity");
        }

        if (!ShouldDrawCaptureTriangle(frameIndex, triangleIndex)) {
            return;
        }

        const float distortionScale = GetCaptureDistortionScale(frameIndex);
        const float offsetX = triangleIndex == 0 ? -2.0f : 2.0f;
        const float offsetY = (frameIndex % 60U) < 30U ? 0.0f : 0.7f;

        float4x4 modelViewMatrix;
        BuildBadCaptureModelViewMatrix(framePlan, entity, modelViewMatrix);
        modelViewMatrix.M41 += offsetX;
        modelViewMatrix.M42 += offsetY;
        modelViewMatrix.M11 *= distortionScale;
        modelViewMatrix.M22 *= distortionScale;
        modelViewMatrix.M33 *= distortionScale;

        Mtx nativeModelViewMatrix;
        CopyAffineMatrixToGx(modelViewMatrix, nativeModelViewMatrix);
        GX_LoadPosMtxImm(nativeModelViewMatrix, GX_PNMTX0);
        GX_SetCurrentMtx(GX_PNMTX0);

        ConfigurePipeline(false);
        GX_SetCullMode(GX_CULL_NONE);

        GX_Begin(GX_TRIANGLES, GX_VTXFMT0, 3);
        GX_Position3f32(localA.X, localA.Y, localA.Z);
        GX_Color4u8(255, 255, 255, 255);
        GX_Position3f32(localB.X, localB.Y, localB.Z);
        GX_Color4u8(255, 255, 255, 255);
        GX_Position3f32(localC.X, localC.Y, localC.Z);
        GX_Color4u8(255, 255, 255, 255);
        GX_End();
    }
```

- [ ] **Step 6: Review the patch for containment**

Confirm only these two files changed:

```powershell
git diff -- src/platform/gamecube/GameCubeRasterRenderer.hpp src/platform/gamecube/GameCubeRasterRenderer.cpp
```

Expected: only temporary capture logic in the GameCube renderer, no shared engine or asset changes.

- [ ] **Step 7: Commit the temporary capture patch**

```powershell
git add src/platform/gamecube/GameCubeRasterRenderer.hpp src/platform/gamecube/GameCubeRasterRenderer.cpp
git commit -m "Add temporary GameCube bad triangle capture mode"
```

### Task 2: Build and Verify the Capture Footage State

**Files:**
- Modify: none

- [ ] **Step 1: Build the latest GameCube image**

Run:

```powershell
rtk powershell -NoProfile -ExecutionPolicy Bypass -File "C:\dev\helworks\helengine-gc\tools\build_and_launch_editor_gamecube_probe.ps1"
```

Expected:
- editor-owned GameCube build succeeds
- fresh `game.gcm` is copied to `C:\dev\helworks\emus\gc-probe\game.gcm`
- Dolphin launches automatically

- [ ] **Step 2: Verify the visual capture result**

In Dolphin, verify:
- two white triangles are visible
- one or both disappear periodically
- scale/position jumps happen repeatedly
- the effect loops predictably enough for recording

Expected: obviously broken pre-matrix-fix-looking footage, but on the latest stable boot path.

- [ ] **Step 3: If the visual effect is too subtle, tune only the three control points**

If needed, adjust only:
- `ShouldDrawCaptureTriangle(...)`
- `GetCaptureDistortionScale(...)`
- the `offsetX` / `offsetY` values in `DrawCaptureTriangle(...)`

Do not expand the scope into app boot, scene loading, or shared engine code.

- [ ] **Step 4: Commit the final tuned capture patch**

```powershell
git add src/platform/gamecube/GameCubeRasterRenderer.hpp src/platform/gamecube/GameCubeRasterRenderer.cpp
git commit -m "Tune temporary GameCube capture distortion"
```

### Task 3: Revert After Recording

**Files:**
- Modify: `src/platform/gamecube/GameCubeRasterRenderer.hpp`
- Modify: `src/platform/gamecube/GameCubeRasterRenderer.cpp`

- [ ] **Step 1: Revert the temporary capture commits after footage is recorded**

Run:

```powershell
git log --oneline -n 5
```

Identify the temporary capture commits, then revert them:

```powershell
git revert <capture-commit-sha>
git revert <tuning-commit-sha>
```

Expected: the latest stable renderer behavior is restored without losing history.

- [ ] **Step 2: Rebuild the normal GameCube image**

Run:

```powershell
rtk powershell -NoProfile -ExecutionPolicy Bypass -File "C:\dev\helworks\helengine-gc\tools\build_and_launch_editor_gamecube_probe.ps1"
```

Expected: normal menu / cube flow is back, with no simulated broken triangles.

- [ ] **Step 3: Commit the revert if needed**

If the revert commits are not already created by `git revert`, ensure the repo is clean:

```powershell
git status --short
```

Expected: no leftover temporary capture edits.
