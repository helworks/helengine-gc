# GameCube Diagnostic Entity Ownership Plan

## Failure

The current generated `Entity` type requires an owning `Core*`. The optional native cube and slope diagnostic installer still constructs four entities with the removed parameterless constructor, so the real DemoDisc build now fails in devkitPPC after the texture-region repair compiles successfully.

## Repair

1. Add source-contract coverage that requires both diagnostic entry points and every private entity factory to accept an explicit `Core*`.
2. Thread `GameCubeApplication::EngineCore` through the cube and slope diagnostic calls.
3. Construct each camera, light, and cube entity with `new Entity(ownerCore)`. Reject a null owner at the diagnostic boundary so no synthetic entity can enter the runtime without ownership.
4. Run the focused source tests, the complete GameCube builder suite, and the Docker/native build against the generated core preserved by the failed DemoDisc build.
5. Rerun the authoritative DemoDisc GameCube build and continue from the next concrete native or packaging result.

## Acceptance

- No `new Entity()` remains in maintained GameCube native code.
- All optional diagnostic entities use the active `GameCubeApplication` core.
- The GameCube builder suite remains green.
- devkitPPC compiles past `GameCubeCubeTestSceneInstaller.cpp`.
- User-owned changes in `C:\dev\helworks\helengine-gc` remain untouched.
