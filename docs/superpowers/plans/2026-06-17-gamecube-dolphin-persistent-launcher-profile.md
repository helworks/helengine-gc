# GameCube Dolphin Persistent Launcher Profile Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Keep the GameCube Dolphin launcher profile persistent across launches so controller mappings remain stable while still forcing logging configuration on every run.

**Architecture:** Change the launcher script so `tmp\dolphin-launcher-user` is seeded only on first run and then reused on later runs. Keep the current log-window enforcement by rewriting `Config\Logger.ini` and `Config\Qt.ini` from the global Dolphin profile every launch, and update the repo contract tests and README to match.

**Tech Stack:** PowerShell launcher script, xUnit source-contract tests, Markdown documentation

---

### Task 1: Lock The New Launcher Contract In Tests

**Files:**
- Modify: `builder.tests/GameCubeDolphinLauncherScriptTests.cs`
- Test: `builder.tests/GameCubeDolphinLauncherScriptTests.cs`

- [ ] **Step 1: Write the failing test**

Add assertions that require the launcher to preserve the dedicated profile directory instead of deleting it every run, while still rewriting `Logger.ini` and `Qt.ini`.

- [ ] **Step 2: Run test to verify it fails**

Run: `dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter FullyQualifiedName~GameCubeDolphinLauncherScriptTests`
Expected: FAIL because the script still contains `Remove-Item -LiteralPath $userDir -Recurse -Force` and the README still says the profile is recreated.

- [ ] **Step 3: Write minimal implementation**

Update the launcher script and README so the dedicated launcher profile is created and seeded only when missing, then reused on subsequent runs.

- [ ] **Step 4: Run test to verify it passes**

Run: `dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter FullyQualifiedName~GameCubeDolphinLauncherScriptTests`
Expected: PASS

### Task 2: Verify Real Launcher Behavior

**Files:**
- Modify: `scripts/launch_gamecube_image_in_dolphin.ps1`
- Modify: `README.md`

- [ ] **Step 1: Launch with the updated script**

Run the launcher against the current built image so Dolphin starts with the persistent dedicated profile path.

- [ ] **Step 2: Confirm behavior**

Check that the script output still reports:

- `USER_DIR=C:\dev\helworks\helengine-gc\tmp\dolphin-launcher-user`
- `LOGGER_CONFIG=C:\dev\helworks\helengine-gc\tmp\dolphin-launcher-user\Config\Logger.ini`
- `PROCESS_ID=<pid>`

and verify the launcher no longer wipes the profile before startup.
