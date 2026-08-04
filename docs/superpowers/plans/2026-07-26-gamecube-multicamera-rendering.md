# GameCube Multi-Camera Rendering Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make the GameCube renderer execute all enabled cameras in draw order without cross-camera 2D replay.

**Architecture:** The scene bridge constructs one temporary frame plan per enabled camera. The 3D manager owns and executes those plans in order. The 2D manager records the camera that submitted each command, while the raster renderer uses that owner to render only the active plan's overlay content and clear region.

**Tech Stack:** Generated C++ runtime, libogc GX, xUnit source-contract tests, GameCube editor CLI package build, Dolphin.

---

### Task 1: Specify the multi-camera renderer contract

**Files:**
- Modify: `builder.tests/GameCubePackagedRuntimeSourceTests.cs`
- Modify: `src/platform/gamecube/GameCubeSceneRenderBridge.hpp`
- Modify: `src/platform/gamecube/GameCubeSceneRenderBridge.cpp`
- Modify: `src/platform/gamecube/GameCubeRenderManager3D.cpp`

- [ ] **Step 1: Write the failing source-contract test**

Assert that the bridge exposes `BuildFramePlans`, no longer exposes `ResolveActiveCamera`, and the 3D manager iterates `framePlans`.

- [ ] **Step 2: Run the focused test and observe failure**

Run: `dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --configuration Release --no-restore --filter FullyQualifiedName~GameCubeRenderer_WhenMultipleCamerasAreActive_ExecutesEachCameraPlan --nologo`

Expected: FAIL because the single-camera bridge remains.

- [ ] **Step 3: Implement ordered plan construction and execution**

Build one `GameCubeFramePlan` for each enabled `CameraComponent` in `ObjectManager.Cameras`, then execute and release every plan in that order.

- [ ] **Step 4: Run the focused test and observe success**

Run the command from Step 2.

### Task 2: Preserve 2D camera ownership and GX clears

**Files:**
- Modify: `builder.tests/GameCubePackagedRuntimeSourceTests.cs`
- Modify: `src/platform/gamecube/GameCubeRenderManager2D.hpp`
- Modify: `src/platform/gamecube/GameCubeRenderManager2D.cpp`
- Modify: `src/platform/gamecube/GameCubeRasterRenderer.hpp`
- Modify: `src/platform/gamecube/GameCubeRasterRenderer.cpp`

- [ ] **Step 1: Extend the failing source-contract test**

Assert that every 2D command includes `CameraComponent* Camera`, capture records the active camera, and the raster path checks command ownership before drawing.

- [ ] **Step 2: Run the focused test and observe failure**

Run the command from Task 1.

Expected: FAIL because captured commands do not have a camera owner.

- [ ] **Step 3: Implement camera-owned 2D capture and viewport scoped clearing**

Tag commands during per-camera queue traversal, filter them during the corresponding plan, and clear colour/depth only inside the active camera viewport before that plan's 3D and 2D work.

- [ ] **Step 4: Run the focused test and observe success**

Run the command from Task 1.

### Task 3: Verify the packaged runtime

**Files:**
- No source changes required.

- [ ] **Step 1: Run the focused source-contract test**

Run the command from Task 1.

- [ ] **Step 2: Build the GameCube package through the editor CLI**

Use the repository GameCube packaging command and wait for code generation and native compilation to complete.

- [ ] **Step 3: Launch the package through the repository Dolphin launcher**

Run: `scripts/launch_in_emulator.ps1`

- [ ] **Step 4: Verify the splash overlays the menu camera in Dolphin**

The splash remains visible for its authored duration and normal menu rendering resumes after it unloads.
