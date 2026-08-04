# GameCube Multi-Camera Rendering Design

## Goal

Render every enabled runtime camera on GameCube in the engine's ordered camera list. Each camera must own its viewport, clear settings, 3D render queue, and 2D render queue.

## Root cause

`GameCubeSceneRenderBridge` builds a plan only for the first enabled camera. `GameCubeRenderManager2D` already visits every active camera, but its captured commands do not retain their source camera. Consequently the GameCube renderer drops higher-order overlays such as the DemoDisc splash and cannot safely iterate more than one plan without replaying all 2D commands.

## Design

The scene bridge will create an ordered `GameCubeFramePlan` for each enabled `CameraComponent`, using the existing extraction service separately for each camera. The ordered `ObjectManager.Cameras` list remains the sole camera-order authority.

The 2D manager will attach the current queue camera to every captured sprite, text, and rounded-rectangle command. The raster renderer will render only commands belonging to the frame-plan camera.

The GX path will process plans sequentially. Before each plan, it will apply that camera's viewport-scoped colour and depth clear in the EFB, then render 3D and 2D for that same camera. A camera with clear disabled leaves earlier camera output intact, enabling overlay cameras. The post-display EFB clear remains a stable neutral clear and is not overloaded with an individual camera's intent.

## Validation

Source-contract tests will prove that the bridge returns frame plans rather than resolving a single active camera, that 2D commands retain their camera owner, and that the render manager iterates all plans. The GameCube package build and Dolphin launch remain the executable validation.
