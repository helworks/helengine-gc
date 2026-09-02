# GameCube Nintendont Safe Input Detection Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make one unflagged GameCube disc build select Nintendont's private input transport automatically without faulting on GameCube hardware or Dolphin.

**Architecture:** `GameCubeInputManager` will detect Nintendont using the loader-provided Hollywood revision marker at mapped low-memory address `0x80003138`, which Nintendont initializes to `0x11` before launching a GameCube title. Only a successful marker comparison permits access to Nintendont's `0x93000000` PadStub and shared controller buffer; all other environments continue through libogc `PAD_Init` and `PAD_ScanPads`.

**Tech Stack:** C++17, libogc cache and PAD APIs, Nintendont loader ABI, xUnit source-contract tests, devkitPPC Docker build.

**Spec:** In-chat bounded design approved 2026-09-02; historical transport context is documented in `docs/superpowers/specs/2026-07-22-gamecube-nintendont-input-transport-design.md`.

## Global Constraints

- Produce one GameCube artifact with automatic behavior; do not add build flags, runtime flags, or separate GameCube/Nintendont variants.
- Preserve Nintendont's existing PadStub transport at `0x93000000`, `SIInited` word at `0x93003060`, and virtual-pad buffer at `0x93003100`.
- Never probe `0x93xxxxxx` memory until the safe low-memory Nintendont marker has matched.
- Preserve the libogc `PAD_Init` and `PAD_ScanPads` path for GameCube hardware and Dolphin.
- Remove the DSI exception handler and speculative-load probe rather than masking or recovering from its fault.
- Do not modify generated sources or unrelated GameCube rendering code.

---

### Task 1: Replace the faulting Nintendont probe

**Files:**
- Modify: `builder.tests/GameCubePackagedRuntimeSourceTests.cs:1366`
- Modify: `src/platform/gamecube/GameCubeInputManager.cpp:6`
- Test: `builder.tests/GameCubePackagedRuntimeSourceTests.cs`

**Interfaces:**
- Consumes: Nintendont's low-memory Hollywood revision marker at `0x80003138`, value `0x11`.
- Produces: `bool IsNintendontEnvironment()` with no speculative high-memory access; the existing initialization and capture interfaces remain unchanged.

- [ ] **Step 1: Rewrite the source-contract test to describe safe automatic detection**

Rename the test to `GameCubeInputManagerSource_SelectsNintendontTransportFromSafeLowMemoryMarker`. Change its XML summary to state that Nintendont is selected from its mapped low-memory marker before any private MEM2 address is used.

Replace the assertions requiring `<tuxedo/ppc/exception.h>`, `PPCExcptCurPanicFn`, `PPC_EXCPT_DSI`, the program-counter adjustment, and inline `lwz` with these assertions:

```csharp
Assert.Contains("constexpr std::uintptr_t NintendontHollywoodVersionAddress = 0x80003138U;", inputSource, StringComparison.Ordinal);
Assert.Contains("DCInvalidateRange(reinterpret_cast<void*>(NintendontHollywoodVersionAddress), sizeof(uint32_t));", inputSource, StringComparison.Ordinal);
Assert.Contains("return *reinterpret_cast<volatile const uint32_t*>(NintendontHollywoodVersionAddress) == 0x11U;", inputSource, StringComparison.Ordinal);
Assert.DoesNotContain("#include <tuxedo/ppc/exception.h>", inputSource, StringComparison.Ordinal);
Assert.DoesNotContain("PPCExcptCurPanicFn", inputSource, StringComparison.Ordinal);
Assert.DoesNotContain("PPC_EXCPT_DSI", inputSource, StringComparison.Ordinal);
Assert.DoesNotContain("NintendontProbeFaulted", inputSource, StringComparison.Ordinal);
Assert.DoesNotContain("asm volatile(\"lwz", inputSource, StringComparison.Ordinal);
```

Keep the existing assertions for the three Nintendont private addresses, PadStub invocation, `SIInited` write and flush, libogc fallback branch, and rejection of the obsolete PVR/DBAT detector.

- [ ] **Step 2: Run the targeted test and verify RED**

Run:

```powershell
rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --no-restore --filter "FullyQualifiedName~GameCubeInputManagerSource_SelectsNintendontTransportFromSafeLowMemoryMarker" --verbosity minimal
```

Expected: FAIL because `GameCubeInputManager.cpp` still contains the exception-based high-memory probe and does not contain `NintendontHollywoodVersionAddress`.

- [ ] **Step 3: Commit the failing contract**

```powershell
rtk git add -- builder.tests/GameCubePackagedRuntimeSourceTests.cs
rtk git commit -m "Test safe GameCube Nintendont input detection"
```

- [ ] **Step 4: Implement the minimal safe detector**

In `src/platform/gamecube/GameCubeInputManager.cpp`, remove the tuxedo exception include, `NintendontProbeFaulted`, `NintendontProbePreviousPanicFunction`, and `HandleNintendontProbeException`.

Add the safe marker address beside the existing Nintendont ABI addresses:

```cpp
constexpr std::uintptr_t NintendontHollywoodVersionAddress = 0x80003138U;
```

Replace `IsNintendontEnvironment()` with:

```cpp
/// Determines whether Nintendont initialized its mapped low-memory Hollywood revision marker before accessing private MEM2 addresses.
bool IsNintendontEnvironment() {
    DCInvalidateRange(reinterpret_cast<void*>(NintendontHollywoodVersionAddress), sizeof(uint32_t));
    return *reinterpret_cast<volatile const uint32_t*>(NintendontHollywoodVersionAddress) == 0x11U;
}
```

Do not change `InitializeNintendontInputTransport`, `CaptureNintendontPadStatus`, or the libogc fallback branch.

- [ ] **Step 5: Run the targeted test and verify GREEN**

Run the same filtered `dotnet test` command from Step 2.

Expected: PASS.

- [ ] **Step 6: Run the complete GameCube builder test suite**

Run:

```powershell
rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --no-restore --verbosity minimal
```

Expected: all tests pass with zero failures.

- [ ] **Step 7: Check the scoped diff and commit the implementation**

```powershell
rtk git diff --check -- src/platform/gamecube/GameCubeInputManager.cpp builder.tests/GameCubePackagedRuntimeSourceTests.cs
rtk git add -- src/platform/gamecube/GameCubeInputManager.cpp
rtk git commit -m "Detect Nintendont without faulting GameCube"
```

### Task 2: Validate the single artifact across runtime boundaries

**Files:**
- Modify: none
- Verify: `build/helengine_gc.elf`
- Verify: `build/helengine_gc.dol`
- Verify: DemoDisc direct-launch `.gcm`

**Interfaces:**
- Consumes: the repaired `GameCubeInputManager.cpp` and the existing generated DemoDisc core cache.
- Produces: one direct-launch GameCube disc image suitable for Dolphin, GameCube, and Nintendont.

- [ ] **Step 1: Rebuild the native GameCube runtime in Docker**

Use the existing GameCube builder worktree, generated-core cache, and `HELENGINE_GAMECUBE_BOOT_MODE=packaged-disc`. Run `make all packaged-disc-assets` in the `helengine-gc` Docker image.

Expected: `GameCubeInputManager.cpp` recompiles and the ELF/DOL relink successfully.

- [ ] **Step 2: Verify the faulting probe is absent from the linked runtime**

Disassemble `IsNintendontEnvironment()` in `build/helengine_gc.elf` with `powerpc-eabi-objdump`. Confirm its first data read targets `0x80003138`, its comparison is against `0x11`, and no instruction in that detector loads from `0x93000000`.

- [ ] **Step 3: Repack the existing direct-launch DemoDisc image**

Copy the rebuilt DOL into the dedicated direct-launch output's `native/helengine_gc.dol` and `disc/sys/main.dol`, preserve the direct manifest whose startup scene is `software_path_tracer`, and use the existing validation repack helper to create `software-path-tracer.gcm`.

Expected: the GCM exists, has a non-zero length, and contains the rebuilt DOL.

- [ ] **Step 4: Launch the one GCM in Dolphin**

Launch `software-path-tracer.gcm` with the repository's `scripts/launch_in_emulator.ps1`. Do not request or take a screenshot.

Expected: Dolphin no longer reports an invalid read at `0x93000000`, the orange exception screen does not appear, and the DemoDisc software path-tracer scene begins rendering.

- [ ] **Step 5: Preserve the same artifact for hardware validation**

Record the final GCM path, size, and SHA-256. The exact same file is the candidate for both physical GameCube and Nintendont testing; do not rebuild it with different options.

