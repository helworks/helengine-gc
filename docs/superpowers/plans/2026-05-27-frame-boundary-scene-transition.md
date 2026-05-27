# Frame-Boundary Scene Transition Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Move all destructive scene transitions to a post-render frame boundary, make renderer release methods own final destruction timing, and remove the temporary GameCube generated-core rewrite that was compensating for the old architecture.

**Architecture:** The shared engine will switch from update-time scene mutation to a request-and-commit pipeline. `SceneManager` will only queue scene changes during update, `Core` will commit them after draw/present completes, and render-manager `Release...()` methods will become the true ownership-transfer seam for runtime textures, fonts, models, and materials. Once the shared source contract is correct, the GameCube builder no longer needs to rewrite generated `SceneManager.cpp` to survive deferred release queues.

**Tech Stack:** C#, HelEngine core runtime, HelEngine editor tests, generated native ownership lowering, GameCube builder normalization layer, `dotnet test`, `dotnet build`, headless HelEngine editor export flow, Dolphin Emulator

---

## File Structure

- Modify: `C:\dev\helworks\helengine\engine\helengine.core\Core.cs`
  - Remove update-phase scene-operation flushing and add one post-draw scene-transition commit seam.
- Modify: `C:\dev\helworks\helengine\engine\helengine.core\scene\runtime\SceneManager.cs`
  - Convert pending scene work into a true frame-boundary commit pipeline and stop deleting runtime assets after ownership transfer.
- Modify: `C:\dev\helworks\helengine\engine\helengine.core\managers\rendering\RenderManager2D.cs`
  - Make runtime texture/font release semantics renderer-owned by default and document the new contract.
- Modify: `C:\dev\helworks\helengine\engine\helengine.core\managers\rendering\RenderManager3D.cs`
  - Make runtime model/material release semantics renderer-owned by default and document the new contract.
- Modify: `C:\dev\helworks\helengine\engine\helengine.directx11\DirectX11Renderer2D.cs`
  - Finish runtime texture destruction under the new ownership-transfer contract after releasing Direct3D resources.
- Modify: `C:\dev\helworks\helengine\engine\helengine.vulkan\VulkanRenderer2D.cs`
  - Finish runtime texture destruction under the new ownership-transfer contract after releasing Vulkan resources.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\testing\TestRenderManager2D.cs`
  - Record release and flush behavior in a way that matches the new frame-boundary semantics.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\testing\TestRenderManager3D.cs`
  - Record release and flush behavior for model/material verification under the new ownership-transfer contract.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\serialization\scene\SceneManagerTests.cs`
  - Replace update-time expectations with frame-boundary expectations and add coverage for release ownership.
- Modify: `C:\dev\helworks\helengine-gc\builder\GameCubeGeneratedCoreCompatibilityNormalizer.cs`
  - Remove the GameCube-only `SceneManager.cpp` ownership rewrite once the shared source contract is fixed.
- Optionally modify: `C:\dev\helworks\helengine-gc\builder.tests\GameCubePackagedBuildWorkspaceTests.cs`
  - Update only if the builder workspace assertions explicitly depend on the old generated rewrite behavior.

## Task 1: Lock Shared Tests To The New Frame-Boundary Semantics

**Files:**
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\serialization\scene\SceneManagerTests.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\testing\TestRenderManager2D.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\testing\TestRenderManager3D.cs`

- [ ] **Step 1: Read the current tests and record which assertions encode the old behavior**

Run: `rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter SceneManagerTests 2>&1`
Expected: the current suite passes before the behavior change, and the engineer can identify tests that assume update-time scene mutation.

- [ ] **Step 2: Change the update-time scene-transition test to the new expected behavior**

Update `LoadScene_whenRequestedDuringUpdate_defersSceneDisposalUntilAfterTheUpdateMethodReturns` so that after `core.Update()`:
- the requesting component was still attached during the update
- the original scene is still loaded
- the target scene is not yet active

Then add the frame-boundary half of the same test so that after `core.Draw()`:
- the original scene is no longer loaded
- the target scene is active

- [ ] **Step 3: Add explicit tests for deferred `Single` and `Additive` activation**

Add two focused tests to `SceneManagerTests.cs`:
- `LoadScene_whenModeIsSingle_defersActivationUntilAfterDrawCompletes`
- `LoadScene_whenModeIsAdditive_defersActivationUntilAfterDrawCompletes`

Each test should assert:
- requests are accepted during update
- `LoadedScenes` does not mutate until draw completes
- the next frame begins with the committed scene set

- [ ] **Step 4: Add a release-ownership test for scene-owned runtime assets**

Add one focused test that loads a scene with owned textures, fonts, models, and materials, requests a `Single` replacement, and asserts:
- `SceneManager` calls the relevant renderer `Release...()` methods
- `SceneManager` does not dispose/delete those runtime objects itself immediately after release
- renderer flushes happen at the frame boundary before the next scene becomes active

- [ ] **Step 5: Extend the test render managers so the new assertions are observable**

Update:
- `TestRenderManager2D.cs`
- `TestRenderManager3D.cs`

so they expose:
- release call counts / released-instance lists
- flush call counts
- enough state to assert that frame-boundary release work happened after draw, not during update

- [ ] **Step 6: Run the focused tests and verify they fail for the right reason**

Run: `rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter SceneManagerTests 2>&1`
Expected: failures should point to update-time scene activation or immediate post-release destruction assumptions, not unrelated infrastructure.

- [ ] **Step 7: Commit the failing-test baseline**

```bash
git -C C:\dev\helworks\helengine add engine/helengine.editor.tests/serialization/scene/SceneManagerTests.cs engine/helengine.editor.tests/testing/TestRenderManager2D.cs engine/helengine.editor.tests/testing/TestRenderManager3D.cs
git -C C:\dev\helworks\helengine commit -m "Lock scene-manager tests to frame-boundary semantics"
```

## Task 2: Move Scene-Transition Commit Out Of Update And Into The Frame Boundary

**Files:**
- Modify: `C:\dev\helworks\helengine\engine\helengine.core\Core.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.core\scene\runtime\SceneManager.cs`

- [ ] **Step 1: Rename the shared responsibility in `SceneManager` from update flush to frame-boundary commit**

Replace the public update-facing `FlushPendingOperations()` behavior with a frame-boundary method whose responsibility is explicit, such as `CommitPendingOperationsAtFrameBoundary()`.

Keep the queue structure, but change the method contract so it is only called after draw/present completes.

- [ ] **Step 2: Remove the update-phase scene-operation call from `Core.AdvanceUpdate(...)`**

In `Core.cs`, remove the `SceneManager.FlushPendingOperations()` call from the update path and keep the surrounding frame instrumentation coherent.

- [ ] **Step 3: Add the new scene-transition commit call after draw completes**

In `Core.Draw()`, keep the current renderer draw order intact, then call the new `SceneManager` frame-boundary commit method only after draw/present work is complete for the frame.

The method ordering must become:
1. update current scene set
2. draw current scene set
3. commit queued scene transitions

- [ ] **Step 4: Preserve request ordering inside the frame-boundary commit**

In `SceneManager.cs`, ensure multiple scene requests issued in the same frame are applied in request order during the post-draw commit step.

- [ ] **Step 5: Run the focused scene-manager tests**

Run: `rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter SceneManagerTests 2>&1`
Expected: the new frame-boundary activation tests now pass, while ownership-related tests may still fail until the release contract is fixed.

- [ ] **Step 6: Commit the frame-order change**

```bash
git -C C:\dev\helworks\helengine add engine/helengine.core/Core.cs engine/helengine.core/scene/runtime/SceneManager.cs
git -C C:\dev\helworks\helengine commit -m "Commit scene transitions at frame boundary"
```

## Task 3: Make Render Managers Own Final Destruction Timing

**Files:**
- Modify: `C:\dev\helworks\helengine\engine\helengine.core\managers\rendering\RenderManager2D.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.core\managers\rendering\RenderManager3D.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.directx11\DirectX11Renderer2D.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.vulkan\VulkanRenderer2D.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\testing\TestRenderManager2D.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\testing\TestRenderManager3D.cs`

- [ ] **Step 1: Change the shared render-manager base classes to own release completion**

In `RenderManager2D.cs` and `RenderManager3D.cs`:
- update XML comments to state that `Release...()` transfers ownership from `SceneManager`
- give default base implementations that fully dispose/delete runtime objects for immediate-destruction backends
- keep `FlushReleasedTextures()` and `FlushReleasedAssets()` as the deferred backend safe-point seam

- [ ] **Step 2: Update the 2D base font-release path to use the new ownership rule consistently**

Ensure `ReleaseFont(FontAsset font)` transfers the attached runtime texture through `ReleaseTexture(...)` and does not leave `SceneManager` responsible for final destruction.

- [ ] **Step 3: Update DirectX11 texture release to complete runtime-object destruction**

In `DirectX11Renderer2D.cs`, after releasing native `ShaderResourceView` and `Texture2D` resources, complete runtime ownership using the shared native-ownership helper so the runtime texture object is not leaked under the new contract.

- [ ] **Step 4: Update Vulkan texture release to complete runtime-object destruction**

In `VulkanRenderer2D.cs`, after destroying the Vulkan texture resource, complete runtime ownership using the shared native-ownership helper for all releasable runtime textures except the permanent white texture sentinel.

- [ ] **Step 5: Make the test render managers reflect the new contract**

Update the test managers so they still record released instances and flush calls, but do not rely on `SceneManager` to destroy those runtime objects after release.

- [ ] **Step 6: Run the focused tests again**

Run: `rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter SceneManagerTests 2>&1`
Expected: ownership-transfer tests now pass and no test still assumes `SceneManager` performs post-release deletion.

- [ ] **Step 7: Commit the renderer ownership contract**

```bash
git -C C:\dev\helworks\helengine add engine/helengine.core/managers/rendering/RenderManager2D.cs engine/helengine.core/managers/rendering/RenderManager3D.cs engine/helengine.directx11/DirectX11Renderer2D.cs engine/helengine.vulkan/VulkanRenderer2D.cs engine/helengine.editor.tests/testing/TestRenderManager2D.cs engine/helengine.editor.tests/testing/TestRenderManager3D.cs
git -C C:\dev\helworks\helengine commit -m "Transfer released runtime asset ownership to render managers"
```

## Task 4: Remove Immediate Post-Release Deletion From `SceneManager`

**Files:**
- Modify: `C:\dev\helworks\helengine\engine\helengine.core\scene\runtime\SceneManager.cs`

- [ ] **Step 1: Remove immediate delete/dispose from runtime texture release**

Update `ReleaseOwnedAsset(RuntimeTexture ownedAsset)` so it:
- validates the runtime texture
- calls `Core.Instance.RenderManager2D.ReleaseTexture(ownedAsset)`
- does not call `NativeOwnership.DisposeAndDelete(...)` afterward

- [ ] **Step 2: Remove immediate delete/dispose from runtime font release**

Keep font release aligned with the renderer-owned contract so `SceneManager` transfers the font to `RenderManager2D.ReleaseFont(...)` and does not delete it afterward.

- [ ] **Step 3: Remove immediate delete/dispose from runtime model release**

Update `ReleaseOwnedModel(RuntimeModel ownedAsset)` so it transfers the runtime model to `RenderManager3D.ReleaseModel(...)` and stops deleting it in `SceneManager`.

- [ ] **Step 4: Remove immediate delete/dispose from runtime material release**

Update `ReleaseOwnedMaterial(RuntimeMaterial ownedAsset)` so it transfers the runtime material to `RenderManager3D.ReleaseMaterial(...)` and stops deleting it in `SceneManager`.

- [ ] **Step 5: Keep the frame-boundary flush ordering explicit**

In the scene-transition commit path, keep renderer flushes at the safe point after releases from the old scene set and before the next scene is published as active.

- [ ] **Step 6: Run the focused tests**

Run: `rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter SceneManagerTests 2>&1`
Expected: the full focused scene-manager suite passes under the new ownership model.

- [ ] **Step 7: Commit the `SceneManager` ownership-transfer changes**

```bash
git -C C:\dev\helworks\helengine add engine/helengine.core/scene/runtime/SceneManager.cs
git -C C:\dev\helworks\helengine commit -m "Stop scene manager from deleting released runtime assets"
```

## Task 5: Remove The GameCube `SceneManager` Rewrite Seam

**Files:**
- Modify: `C:\dev\helworks\helengine-gc\builder\GameCubeGeneratedCoreCompatibilityNormalizer.cs`
- Optionally modify: `C:\dev\helworks\helengine-gc\builder.tests\GameCubePackagedBuildWorkspaceTests.cs`

- [ ] **Step 1: Remove the GameCube-only `SceneManager.cpp` ownership rewrite**

Delete the normalizer logic that rewrites generated `SceneManager.cpp` to suppress immediate destruction of released runtime textures, materials, and models.

Leave unrelated GameCube diagnostics or scene-load timing hooks intact unless they directly depend on the removed rewrite block.

- [ ] **Step 2: Update any builder assertions that still expect the old rewrite**

If `GameCubePackagedBuildWorkspaceTests.cs` or other builder tests assert the presence of the removed rewrite text, update them so they verify the shared-source fix indirectly instead of the old patch.

- [ ] **Step 3: Rebuild the GameCube builder**

Run: `rtk dotnet build C:\dev\helworks\helengine-gc\builder\helengine.gamecube.builder.csproj 2>&1`
Expected: the GameCube builder compiles cleanly without the `SceneManager.cpp` ownership rewrite.

- [ ] **Step 4: Commit the GameCube cleanup**

```bash
git -C C:\dev\helworks\helengine-gc add builder/GameCubeGeneratedCoreCompatibilityNormalizer.cs builder.tests/GameCubePackagedBuildWorkspaceTests.cs
git -C C:\dev\helworks\helengine-gc commit -m "Remove GameCube scene-manager ownership rewrite"
```

## Task 6: Run End-To-End Verification On The Real GameCube Flow

**Files:**
- Inspect: `C:\dev\helprojs\output\gamecube-mainmenu-smoke`
- Output: `C:\dev\helprojs\output\gamecube-mainmenu-smoke\game.gcm`

- [ ] **Step 1: Re-run the focused shared-engine tests**

Run: `rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter SceneManagerTests 2>&1`
Expected: the focused scene-manager suite passes.

- [ ] **Step 2: Rebuild the GameCube builder**

Run: `rtk dotnet build C:\dev\helworks\helengine-gc\builder\helengine.gamecube.builder.csproj 2>&1`
Expected: build succeeds with no dependency on the removed generated rewrite.

- [ ] **Step 3: Rebuild the main-menu GameCube smoke export**

Run: `rtk dotnet run --project C:\dev\helworks\helengine.editor.app\helengine.editor.app.csproj -- export-gamecube --project C:\dev\helprojs\city\project.heproj --output C:\dev\helprojs\output\gamecube-mainmenu-smoke`
Expected: export completes successfully and produces `C:\dev\helprojs\output\gamecube-mainmenu-smoke\game.gcm`.

- [ ] **Step 4: Launch Dolphin on the rebuilt ISO**

Run: `rtk "C:\dev\tools\dolphin\2603a\Dolphin-x64\Dolphin.exe" -e C:\dev\helprojs\output\gamecube-mainmenu-smoke\game.gcm`
Expected: Dolphin opens the rebuilt ISO.

- [ ] **Step 5: Manually validate both transition directions**

In Dolphin:
- enter `cube_test` from the main menu
- return to the menu with `B`
- repeat the transition several times

Expected:
- no crash on scene enter
- no crash on scene return
- no GameCube-only ownership workaround is required in generated code

- [ ] **Step 6: Commit only if verification forced additional real source changes**

Expected: no new commit is needed if the previous tasks already captured all source changes.

## Verification Summary

- `SceneManager` no longer mutates the active scene set during update.
- `SceneManager` no longer disposes/deletes runtime textures, fonts, models, or materials after handing them to a render manager.
- `Core` commits queued scene transitions only after draw/present completes.
- Shared scene-manager tests pass with the new frame-boundary expectations.
- The GameCube builder no longer rewrites generated `SceneManager.cpp` to survive ownership-transfer bugs.
- The real GameCube main-menu to `cube_test` to main-menu flow survives repeated transitions in Dolphin.

## Commit Strategy

1. Commit the failing-test baseline in `helengine`.
2. Commit the shared frame-boundary ordering change in `helengine`.
3. Commit the shared renderer ownership contract in `helengine`.
4. Commit the final `SceneManager` ownership-transfer cleanup in `helengine`.
5. Commit the GameCube normalizer cleanup in `helengine-gc`.
6. Do not squash these during implementation; each commit should leave the codebase in a logically reviewable state.
