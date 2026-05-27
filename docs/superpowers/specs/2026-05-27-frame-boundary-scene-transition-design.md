# Frame-Boundary Scene Transition Design

## Goal

Move all destructive scene transition work to a post-render frame boundary so the active scene set remains stable for the entire frame and render-owned assets are never destroyed before the renderer has finished with them.

This design applies to all scene transition modes:

- `Single`
- `Additive`

It also establishes the shared ownership contract that generated code and platform backends must follow.

## Current State

The shared engine currently applies pending scene operations during update, before draw and present complete.

Relevant source seams:

- `C:\dev\helworks\helengine\engine\helengine.core\Core.cs`
- `C:\dev\helworks\helengine\engine\helengine.core\scene\runtime\SceneManager.cs`
- `C:\dev\helworks\helengine\engine\helengine.core\managers\rendering\RenderManager2D.cs`
- `C:\dev\helworks\helengine\engine\helengine.core\managers\rendering\RenderManager3D.cs`

The current behavior has two architectural problems:

1. scene membership mutates during the update phase instead of at a frame boundary
2. `SceneManager` assumes `Release...()` means immediate destruction and then disposes/deletes the same runtime asset itself

That assumption is invalid for backends that defer destruction until a safe render boundary. GameCube exposed the problem first because it uses queued release paths, but the ordering issue is shared-engine behavior and should not be solved with generated-code rewrites.

## Design

Use a request-and-commit scene transition pipeline.

### Request Phase

Calls such as `LoadScene(...)` and other scene-changing requests only enqueue intent during update.

During the request phase:

- no loaded scene is destructively unloaded
- no new scene becomes active immediately
- no render-owned runtime asset is disposed or deleted

The current frame continues using the scene set that was active at the start of the frame.

### Commit Phase

After draw and present complete, the engine runs a dedicated scene-transition commit step.

That commit step performs destructive work in a deterministic order:

1. resolve the queued scene operation list for the frame
2. unload scenes that are leaving the active set
3. release scene-owned runtime assets through the render managers
4. allow render managers to finalize any deferred release work at the frame boundary
5. load and attach the scenes that should become active next
6. publish the new active scene set for the next frame

This means the active scene set is immutable from frame-update start until present completes.

### Frame Lifecycle

The shared frame shape becomes:

1. `Update`
2. `Draw`
3. `Present / end-of-frame renderer work`
4. `Commit queued scene transitions`
5. `Begin next frame with the committed scene set`

The commit step is the only place where scene membership and scene-owned runtime asset lifetime can change destructively.

### Ownership Contract

After `SceneManager` calls any of the following methods:

- `RenderManager2D.ReleaseTexture(...)`
- `RenderManager2D.ReleaseFont(...)`
- `RenderManager3D.ReleaseModel(...)`
- `RenderManager3D.ReleaseMaterial(...)`

`SceneManager` no longer owns the runtime object.

From that point forward:

- the renderer owns final destruction timing
- immediate-destruction backends may destroy the object synchronously
- deferred-destruction backends may queue and flush at frame end

`SceneManager` must not call `Dispose()` or `delete`-equivalent ownership helpers on those released runtime assets after transfer.

This contract belongs in the shared C# source. Generated code should follow naturally from the source contract. The solution must not depend on generated-code post-processing or GameCube-specific rewrites.

### Single And Additive Semantics

`Single` and `Additive` both defer destructively to the same frame-boundary commit step.

`Single`:

- the current active scene set remains visible and valid for the rest of the current frame
- replacement happens only during the post-render commit

`Additive`:

- the currently active scene set also remains stable for the rest of the current frame
- additional scenes are attached only during the post-render commit

This gives one consistent rule: no scene request mutates the active scene set mid-frame.

### Ordering

Requests issued during a frame are committed in request order at that frame boundary.

If multiple scene operations are issued in the same frame, the engine should preserve their authored call order when producing the committed next-frame scene set.

## Required Source Changes

### Core Frame Ordering

Update the shared core frame flow so scene-operation flushing is removed from the update phase and replaced by a post-render commit call.

This is expected to change `Core.cs`, not generated output.

### SceneManager Responsibilities

Update `SceneManager.cs` so that:

- pending operations remain queued during update
- destructive unload/load work runs only from the new frame-boundary commit method
- released runtime textures, fonts, models, and materials are transferred to render-manager ownership instead of being disposed/deleted immediately by `SceneManager`

### Render Manager Contract

Update the shared render-manager base classes so their release methods are documented and implemented around ownership transfer.

The shared contract should make it explicit that:

- `Release...()` takes over final destruction responsibility
- `FlushReleased...()` is the safe-point hook for deferred backends
- immediate backends are still free to complete destruction synchronously

## Rejected Approaches

### Generated-Code Rewrites

Do not solve this by rewriting generated `SceneManager.cpp`.

That is the wrong abstraction boundary. The source contract in shared C# is wrong today. Code generation should emit the corrected behavior instead of being patched afterward.

### GameCube-Only Ownership Rules

Do not keep a GameCube-specific rule where only the GameCube build avoids deleting released runtime assets.

The ordering problem is architectural, not platform-local.

### Keep Update-Phase Scene Mutation And Only Patch Ownership

Do not keep scene transition mutation in update and only stop `SceneManager` from deleting runtime assets.

That would reduce one class of crash, but it would still leave the engine mutating the active scene set at the wrong time in the frame.

## Verification

Verification should cover both shared semantics and GameCube regression safety.

Shared-engine verification:

- update or add tests that prove scene requests do not mutate the active scene set until the frame-boundary commit
- update or add tests that prove released runtime assets are not disposed by `SceneManager` after ownership transfer
- verify request ordering for multiple scene operations in the same frame

GameCube verification:

- boot the demo-disc main menu
- enter a demo scene such as `cube_test`
- return to menu repeatedly
- confirm there are no use-after-free crashes on either scene entry or scene exit

## Scope

In scope:

- shared frame-order changes in `Core.cs`
- shared scene-transition ownership and timing changes in `SceneManager.cs`
- shared render-manager release contract clarification
- removal of GameCube-specific generated `SceneManager` rewrite logic after the shared source fix is in place

Out of scope:

- redesigning the entire object-manager update model
- introducing a full double-buffered world architecture
- changing authored scene content or project startup selection
