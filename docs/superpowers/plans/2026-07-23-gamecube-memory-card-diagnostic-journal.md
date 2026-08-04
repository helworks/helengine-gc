# GameCube Memory-Card Diagnostic Journal Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add an opt-in GameCube build setting that persists startup checkpoints to a normal Slot-A memory-card save, enabling diagnosis on stock Nintendont and retail hardware.

**Architecture:** The builder carries one Boolean build option into Docker as a native preprocessor definition. When enabled, `GameCubeMemoryCardDiagnosticJournal` owns CARD initialization, one fixed-size `HELDBG` file, and bounded checkpoint writes. `GameCubeApplication` and `GameCubeInputManager` publish checkpoints but remain independent of CARD details.

**Tech Stack:** C#/.NET 9 builder tests; C++20; libogc `ogc/card.h`; Dockerized devkitPPC build.

---

### Task 1: Expose and propagate the opt-in build setting

**Files:**
- Modify: `builder/GameCubePlatformDefinitionFactory.cs`
- Modify: `builder/GameCubeBuilderPaths.cs`
- Modify: `builder/GameCubeBuildWorkspace.cs`
- Modify: `builder/GameCubeDockerNativeBuildExecutor.cs`
- Modify: `builder.tests/GameCubePlatformDefinitionFactoryTests.cs`
- Modify: `builder.tests/GameCubeDockerNativeBuildExecutorTests.cs`

- [ ] **Step 1: Write failing builder tests**

Add a factory test that requires the `enable-memory-card-diagnostic-journal` Boolean build setting to default to `false`. Add a native-executor source/argument test that requires `HELENGINE_GAMECUBE_MEMORY_CARD_DIAGNOSTIC_JOURNAL=1` when the selected build option is true and `=0` otherwise.

- [ ] **Step 2: Run the focused builder tests and verify they fail**

Run:
```powershell
rtk dotnet test builder.tests\helengine.gamecube.builder.tests.csproj -c Debug --no-restore --filter "FullyQualifiedName~GameCubePlatformDefinitionFactoryTests|FullyQualifiedName~GameCubeDockerNativeBuildExecutorTests" --verbosity minimal
```

Expected: the new assertions fail because the setting and Docker environment value do not exist.

- [ ] **Step 3: Implement the option transport**

Add the default-false build setting to `GameCubePlatformDefinitionFactory`. Add a `MemoryCardDiagnosticJournalEnabled` Boolean to `GameCubeBuilderPaths`, resolve it from `request.SelectedBuildOptionValues`, and forward it through `GameCubeBuildWorkspace` into `GameCubeDockerNativeBuildExecutor`. Add this Docker argument:
```csharp
startInfo.ArgumentList.Add("-e");
startInfo.ArgumentList.Add(
    "HELENGINE_GAMECUBE_MEMORY_CARD_DIAGNOSTIC_JOURNAL="
    + (paths.MemoryCardDiagnosticJournalEnabled ? "1" : "0"));
```

- [ ] **Step 4: Run the focused builder tests and verify they pass**

Run the command from Step 2. Expected: all selected tests pass.

### Task 2: Define the native compile-time contract and journal class

**Files:**
- Modify: `Makefile`
- Create: `src/platform/gamecube/GameCubeMemoryCardDiagnosticJournal.hpp`
- Create: `src/platform/gamecube/GameCubeMemoryCardDiagnosticJournal.cpp`
- Modify: `builder.tests/GameCubePackagedRuntimeSourceTests.cs`

- [ ] **Step 1: Write failing native source-contract tests**

Add tests that require `HELENGINE_GAMECUBE_MEMORY_CARD_DIAGNOSTIC_JOURNAL ?= 0` in `Makefile`, a `GameCubeMemoryCardDiagnosticJournal` type, `Initialize`, `Record`, and `Disable` methods, use of `CARD_Init`, `CARD_Mount`, `CARD_Open`, `CARD_Create`, and `CARD_Write`, and a `HELDBG` file name.

- [ ] **Step 2: Run the source-contract tests and verify they fail**

Run:
```powershell
rtk dotnet test builder.tests\helengine.gamecube.builder.tests.csproj -c Debug --no-restore --filter "FullyQualifiedName~GameCubePackagedRuntimeSourceTests" --verbosity minimal
```

Expected: new assertions fail because the compile-time setting and journal implementation are absent.

- [ ] **Step 3: Implement the bounded card journal**

Add the compile definition to the Makefile. Implement one class that:

```cpp
bool Initialize();
void Record(GameCubeMemoryCardDiagnosticStage stage, int32_t resultCode);
void Disable();
```

It must allocate a 32-byte-aligned `CARD_WORKAREA_SIZE` mount buffer, call `CARD_Init` using the GCM identifiers, mount `CARD_SLOTA`, determine the sector size with `CARD_GetSectorSize`, and open or create `HELDBG` with exactly one sector. Each record write must be sector-aligned and use a fixed binary header plus fixed checkpoint records. On any CARD error it must close/unmount as applicable, disable itself, and return control to the game without throwing.

- [ ] **Step 4: Run the source-contract tests and verify they pass**

Run the command from Step 2. Expected: all selected tests pass.

### Task 3: Record the boot, input, update, and draw boundaries

**Files:**
- Modify: `src/platform/gamecube/GameCubeApplication.hpp`
- Modify: `src/platform/gamecube/GameCubeApplication.cpp`
- Modify: `src/platform/gamecube/GameCubeInputManager.hpp`
- Modify: `src/platform/gamecube/GameCubeInputManager.cpp`
- Modify: `builder.tests/GameCubePackagedRuntimeSourceTests.cs`

- [ ] **Step 1: Write failing checkpoint-placement tests**

Require source-level checkpoints for journal initialization, Nintendont transport initialization, the start and completion of `UpdateNintendontPadBuffer`, the start and completion of `EngineCore->Update()`, and the start and completion of `EngineCore->Draw()`.

- [ ] **Step 2: Run the checkpoint-placement tests and verify they fail**

Run:
```powershell
rtk dotnet test builder.tests\helengine.gamecube.builder.tests.csproj -c Debug --no-restore --filter "FullyQualifiedName~GameCubePackagedRuntimeSourceTests" --verbosity minimal
```

Expected: the new checkpoint assertions fail.

- [ ] **Step 3: Wire checkpoints without adding frame-loop writes**

Store one journal instance in `GameCubeApplication` only when the compile setting is enabled. Initialize it after native video/graphics setup. Record each named boundary exactly once using Boolean latches for first input capture, first core update, and first core draw. Pass a journal callback or pointer into `GameCubeInputManager` so it records immediately before and after the pad-stub call. Do not write after every subsequent frame.

- [ ] **Step 4: Run the checkpoint-placement tests and verify they pass**

Run the command from Step 2. Expected: all selected tests pass.

### Task 4: Build and verify the opt-in GCM

**Files:**
- Modify: `docs/superpowers/specs/2026-07-23-gamecube-memory-card-diagnostic-journal-design.md`

- [ ] **Step 1: Run the focused builder regression suite**

Run:
```powershell
rtk dotnet test builder.tests\helengine.gamecube.builder.tests.csproj -c Debug --no-restore --filter "FullyQualifiedName~GameCubePlatformDefinitionFactoryTests|FullyQualifiedName~GameCubeDockerNativeBuildExecutorTests|FullyQualifiedName~GameCubePackagedRuntimeSourceTests" --verbosity minimal
```

Expected: all selected tests pass.

- [ ] **Step 2: Build through the rebuilt editor CLI with the setting enabled**

Set the GameCube build option `enable-memory-card-diagnostic-journal` to `true` in the project’s GameCube CLI/profile settings, publish the editor CLI if needed, then package a new GCM. Verify the build log contains `native build completed`, `disc image packaged`, and `packaged outputs verified`.

- [ ] **Step 3: Verify the output image and hand off retrieval steps**

Run `Get-FileHash -Algorithm SHA256` on the new `game.gcm`. Tell the user to boot it once with Nintendont defaults, then provide the expected `I:\saves\<GameID>.raw` location and Dolphin Memory Card Manager extraction steps for `HELDBG`.

## Plan self-review

- Spec coverage: Tasks 1-3 cover the opt-in setting, one-file card storage, bounded records, no-op failure behavior, and boot checkpoints; Task 4 covers package and hardware handoff.
- Placeholder scan: no deferred implementation placeholders remain.
- Type consistency: the plan consistently uses `GameCubeMemoryCardDiagnosticJournal`, `enable-memory-card-diagnostic-journal`, and `HELENGINE_GAMECUBE_MEMORY_CARD_DIAGNOSTIC_JOURNAL`.
