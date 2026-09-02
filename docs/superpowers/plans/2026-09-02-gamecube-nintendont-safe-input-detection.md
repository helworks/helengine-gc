# GameCube Nintendont Safe Input Detection Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make one unflagged GameCube disc build select Nintendont's private input transport automatically without faulting on physical GameCube hardware or Dolphin.

**Architecture:** `GameCubeInputManager` will classify the CPU with libogc's standard `mfpvr()` helper. A Gekko processor-version value (`0x00083214`) selects the normal libogc PAD path used by physical GameCube hardware and Dolphin; any other processor running this GameCube binary is the Wii-family Broadway environment used by Nintendont and selects the existing PadStub transport. This keeps the detector entirely in a CPU register and avoids both the cleared Nintendont low-memory marker and every speculative MEM2/DBAT access.

**Tech Stack:** C++17, libogc processor and PAD APIs, Nintendont PadStub ABI, xUnit source-contract tests, devkitPPC Docker build.

**Spec:** Corrected in-chat bounded design approved 2026-09-02. The superseded low-memory-marker implementation is retained in commits `60a73ee` and `2d6b938` only as history; it must not be built because Nintendont's multidol `RAMInit` clears `0x80003138` before `Apploader_Run` enters the title.

## Root-Cause Evidence

- Nintendont's second-stage `multidol/main.c` calls `RAMInit()` immediately before `Apploader_Run()`.
- Nintendont's second-stage `multidol/global.c` clears `0x80003000..0x812cffff`, which includes `0x80003138`, and does not restore that marker.
- libogc's maintained `dvd.c` already uses `mfpvr() == 0x00083214` as its GameCube hardware classifier.
- libogc defines `mfpvr()` as a Processor Version Register read, so classification requires no memory mapping probe and cannot produce the invalid `0x93000000` read seen on GameCube.

## Global Constraints

- Produce one GameCube artifact with automatic behavior; do not add build flags, runtime flags, or separate GameCube/Nintendont variants.
- Preserve Nintendont's existing PadStub transport at `0x93000000`, `SIInited` word at `0x93003060`, and virtual-pad buffer at `0x93003100`.
- Never probe `0x93xxxxxx` memory until the processor classifier has selected the Nintendont path.
- Preserve the libogc `PAD_Init` and `PAD_ScanPads` path for GameCube hardware and Dolphin.
- Do not read DBAT registers, install a DSI exception handler, or inspect a loader-owned low-memory marker.
- Do not modify generated sources or unrelated GameCube rendering code.
- Do not take Dolphin screenshots without explicit user authorization.

---

### Task 1: Replace the cleared loader marker with the PVR classifier

**Files:**
- Modify: `builder.tests/GameCubePackagedRuntimeSourceTests.cs`
- Modify: `src/platform/gamecube/GameCubeInputManager.cpp`
- Test: `builder.tests/GameCubePackagedRuntimeSourceTests.cs`

**Interfaces:**
- Consumes: libogc `mfpvr()` and the Gekko PVR value `0x00083214`.
- Produces: `bool IsNintendontEnvironment()` with no memory access; existing initialization and capture interfaces remain unchanged.

- [ ] **Step 1: Rewrite the source-contract test to require processor-only detection**

Rename the test to `GameCubeInputManagerSource_SelectsNintendontTransportFromProcessorVersion`. State in its XML summary that a Gekko PVR selects libogc PAD while a Wii-family PVR selects Nintendont before any private MEM2 address is used.

Replace the low-memory-marker assertions and the blanket prohibition on PVR reads with these assertions:

```csharp
Assert.Contains("#include <ogc/machine/processor.h>", inputSource, StringComparison.Ordinal);
Assert.Contains("constexpr uint32_t GameCubeProcessorVersion = 0x00083214U;", inputSource, StringComparison.Ordinal);
Assert.Contains("return mfpvr() != GameCubeProcessorVersion;", inputSource, StringComparison.Ordinal);
Assert.DoesNotContain("NintendontHollywoodVersionAddress", inputSource, StringComparison.Ordinal);
Assert.DoesNotContain("0x80003138", inputSource, StringComparison.Ordinal);
Assert.DoesNotContain("#include <tuxedo/ppc/exception.h>", inputSource, StringComparison.Ordinal);
Assert.DoesNotContain("PPCExcptCurPanicFn", inputSource, StringComparison.Ordinal);
Assert.DoesNotContain("PPC_EXCPT_DSI", inputSource, StringComparison.Ordinal);
Assert.DoesNotContain("NintendontProbeFaulted", inputSource, StringComparison.Ordinal);
Assert.DoesNotContain("PowerPcHighDataBat", inputSource, StringComparison.Ordinal);
Assert.DoesNotContain("ReadDataBat", inputSource, StringComparison.Ordinal);
Assert.DoesNotContain("mfspr %0, 536", inputSource, StringComparison.Ordinal);
Assert.DoesNotContain("mfspr %0, 537", inputSource, StringComparison.Ordinal);
Assert.DoesNotContain("asm volatile(\"lwz", inputSource, StringComparison.Ordinal);
```

Keep the existing assertions for the three Nintendont private addresses, PadStub invocation, `SIInited` write and flush, libogc fallback branch, and rejection of DBAT/high-memory probing. Remove only assertions that reject every processor-version read or the `GameCubeProcessorVersion` identifier.

- [ ] **Step 2: Run the targeted test and verify RED**

Run:

```powershell
rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --no-restore --filter "FullyQualifiedName~GameCubeInputManagerSource_SelectsNintendontTransportFromProcessorVersion" --verbosity minimal
```

Expected: FAIL because `GameCubeInputManager.cpp` still checks `NintendontHollywoodVersionAddress` and does not call `mfpvr()`.

- [ ] **Step 3: Commit the failing contract**

```powershell
rtk git add -- builder.tests/GameCubePackagedRuntimeSourceTests.cs
rtk git commit -m "Test GameCube processor input detection"
```

- [ ] **Step 4: Implement the minimal processor-only detector**

Add the maintained libogc processor header after `gccore.h`:

```cpp
#include <ogc/machine/processor.h>
```

Replace the low-memory marker constant with:

```cpp
constexpr uint32_t GameCubeProcessorVersion = 0x00083214U;
```

Replace `IsNintendontEnvironment()` with:

```cpp
/// Determines whether this GameCube binary is running on Nintendont's Wii-family processor before accessing private MEM2 addresses.
bool IsNintendontEnvironment() {
    return mfpvr() != GameCubeProcessorVersion;
}
```

Do not change `InitializeNintendontInputTransport`, `CaptureNintendontPadStatus`, or the libogc fallback branch.

- [ ] **Step 5: Run the targeted test and verify GREEN**

Run the same filtered `dotnet test` command from Step 2.

Expected: PASS with one test executed and zero failures.

- [ ] **Step 6: Run the complete GameCube builder test suite**

Run:

```powershell
rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --no-restore --verbosity minimal
```

Expected: all GameCube builder tests pass with zero failures.

- [ ] **Step 7: Check the scoped diff and commit the implementation**

```powershell
rtk git diff --check -- src/platform/gamecube/GameCubeInputManager.cpp builder.tests/GameCubePackagedRuntimeSourceTests.cs
rtk git add -- src/platform/gamecube/GameCubeInputManager.cpp
rtk git commit -m "Classify GameCube input transport by processor"
```

### Task 2: Validate the single artifact across runtime boundaries

**Files:**
- Modify: none
- Verify: `build/helengine_gc.elf`
- Verify: `build/helengine_gc.dol`
- Verify: DemoDisc direct-launch `.gcm`

**Interfaces:**
- Consumes: the repaired `GameCubeInputManager.cpp` and the existing generated DemoDisc core cache.
- Produces: one direct-launch GameCube disc image suitable for Dolphin, physical GameCube, and Nintendont.

- [ ] **Step 1: Rebuild the native GameCube runtime in Docker**

Use the existing GameCube builder worktree, generated-core cache, and `HELENGINE_GAMECUBE_BOOT_MODE=packaged-disc`. Run `make all packaged-disc-assets` in the `helengine-gc` Docker image.

Expected: `GameCubeInputManager.cpp` recompiles and the ELF/DOL relink successfully. During the build, monitor for `codegen.exe` windows and `WerFault.exe`; abort immediately if either appears.

- [ ] **Step 2: Verify processor-only classification in the linked runtime**

Disassemble `IsNintendontEnvironment()` in `build/helengine_gc.elf` with `powerpc-eabi-objdump`.

Expected: the detector reads the Processor Version Register and compares it with `0x00083214`; it performs no data-memory load, no DBAT read, and no access to `0x93000000` before the branch to the Nintendont initialization path.

- [ ] **Step 3: Repack the existing direct-launch DemoDisc image**

Temporarily install the preserved direct manifest whose startup scene is `software_path_tracer`, rebuild the native runtime so the manifest is compiled into the DOL, and restore the generated-core cache's original manifest immediately afterward. Copy the rebuilt DOL into the dedicated direct-launch output's `native/helengine_gc.dol` and `disc/sys/main.dol`, then use the existing validation repack helper to create `software-path-tracer.gcm`.

Expected: the GCM exists, has a non-zero length, contains the rebuilt DOL, and the generated-core cache is byte-for-byte restored to its pre-build manifest.

- [ ] **Step 4: Launch the one GCM in Dolphin**

Launch `software-path-tracer.gcm` with the repository's `scripts/launch_in_emulator.ps1`. Do not request or take a screenshot.

Expected: Dolphin no longer reports an invalid read at `0x93000000`, the orange exception screen does not appear, and the DemoDisc software path-tracer scene begins rendering.

- [ ] **Step 5: Preserve the same artifact for hardware validation**

Record the final GCM path, size, and SHA-256. The exact same file is the candidate for both physical GameCube and Nintendont testing; do not rebuild it with different options.
