# GameCube Builder Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a dynamically loaded `helengine.gamecube.builder` assembly that matches the current Windows/PS2 builder contract by exposing GameCube platform metadata and staging build payloads into the output root.

**Architecture:** The GameCube builder should follow the current platform-builder seam exactly: a thin `GameCubePlatformAssetBuilder` exposes descriptor and definition metadata and delegates `BuildAsync(...)` to a focused staging workspace. The workspace copies scene and loose-asset payloads into `request.OutputRoot`, emits `gamecube-build-manifest.json` into `request.WorkingRoot`, and returns a `PlatformBuildReport`.

**Tech Stack:** .NET 9, `helengine.baseplatform`, xUnit, dynamic platform-builder assembly pattern used by `helengine-windows` and `helengine-ps2`

---

### Task 1: Scaffold the GameCube builder projects

**Files:**
- Create: `builder/helengine.gamecube.builder.csproj`
- Create: `builder/helengine.gamecube.builder.sln`
- Create: `builder/Program.cs`
- Create: `builder.tests/helengine.gamecube.builder.tests.csproj`
- Create: `builder.tests/Builders/RecordingDiagnosticReporter.cs`
- Create: `builder.tests/Builders/RecordingProgressReporter.cs`
- Test: `builder.tests/GameCubePlatformAssetBuilderTests.cs`

- [ ] **Step 1: Create the builder project file**

```xml
<Project Sdk="Microsoft.NET.Sdk">
  <PropertyGroup>
    <TargetFramework>net9.0</TargetFramework>
    <ImplicitUsings>enable</ImplicitUsings>
    <Nullable>disable</Nullable>
    <IsPackable>false</IsPackable>
  </PropertyGroup>

  <ItemGroup>
    <ProjectReference Include="..\..\helengine\engine\helengine.baseplatform\helengine.baseplatform.csproj" />
  </ItemGroup>
</Project>
```

- [ ] **Step 2: Create the test project file**

```xml
<Project Sdk="Microsoft.NET.Sdk">
  <PropertyGroup>
    <TargetFramework>net9.0</TargetFramework>
    <IsPackable>false</IsPackable>
    <ImplicitUsings>enable</ImplicitUsings>
    <Nullable>disable</Nullable>
  </PropertyGroup>

  <ItemGroup>
    <PackageReference Include="coverlet.collector" Version="6.0.2" />
    <PackageReference Include="Microsoft.NET.Test.Sdk" Version="17.11.1" />
    <PackageReference Include="xunit" Version="2.9.0" />
    <PackageReference Include="xunit.runner.visualstudio" Version="2.8.2" />
  </ItemGroup>

  <ItemGroup>
    <Using Include="Xunit" />
  </ItemGroup>

  <ItemGroup>
    <ProjectReference Include="..\builder\helengine.gamecube.builder.csproj" SkipGetTargetFrameworkProperties="true" />
    <ProjectReference Include="..\..\helengine\engine\helengine.baseplatform\helengine.baseplatform.csproj" SkipGetTargetFrameworkProperties="true" />
  </ItemGroup>
</Project>
```

- [ ] **Step 3: Create the solution file matching the Windows builder layout**

```text
Microsoft Visual Studio Solution File, Format Version 12.00
# Visual Studio Version 18
Project("{FAE04EC0-301F-11D3-BF4B-00C04F79EFBC}") = "helengine.gamecube.builder", "helengine.gamecube.builder.csproj", "{A9C1D36B-7D4A-4F95-9A6D-8A1AF25F2A01}"
EndProject
Project("{FAE04EC0-301F-11D3-BF4B-00C04F79EFBC}") = "helengine.baseplatform", "..\..\helengine\engine\helengine.baseplatform\helengine.baseplatform.csproj", "{66F9599B-6DDF-27FE-2A0C-ABA5E2C19012}"
EndProject
Global
  GlobalSection(SolutionConfigurationPlatforms) = preSolution
    Debug|Any CPU = Debug|Any CPU
    Release|Any CPU = Release|Any CPU
  EndGlobalSection
EndGlobal
```

- [ ] **Step 4: Add the builder CLI entrypoint**

```csharp
using helengine.baseplatform.Definitions;

namespace helengine.gamecube.builder;

public static class Program {
    public static int Main(string[] args) {
        if (args.Length > 0 && string.Equals(args[0], "--describe", StringComparison.OrdinalIgnoreCase)) {
            GameCubePlatformAssetBuilder builder = new();
            Console.WriteLine(builder.Descriptor.BuilderId);
            Console.WriteLine(builder.Descriptor.TargetPlatformId);
            Console.WriteLine(builder.Definition.DisplayName);
            return 0;
        }

        if (args.Length > 0 && string.Equals(args[0], "--smoke-test", StringComparison.OrdinalIgnoreCase)) {
            Console.WriteLine("gamecube.builder smoke test entrypoint");
            return 0;
        }

        Console.WriteLine("helengine.gamecube.builder --describe | --smoke-test");
        return 0;
    }
}
```

- [ ] **Step 5: Add the test reporters used by builder tests**

```csharp
using helengine.baseplatform.Builders;
using helengine.baseplatform.Reporting;

namespace helengine.gamecube.builder.tests.Builders;

public sealed class RecordingProgressReporter : IPlatformBuildProgressReporter {
    public List<PlatformBuildProgressUpdate> Updates { get; } = [];

    public void Report(PlatformBuildProgressUpdate update) {
        Updates.Add(update);
    }
}
```

```csharp
using helengine.baseplatform.Builders;
using helengine.baseplatform.Reporting;

namespace helengine.gamecube.builder.tests.Builders;

public sealed class RecordingDiagnosticReporter : IPlatformBuildDiagnosticReporter {
    public List<PlatformBuildDiagnostic> Diagnostics { get; } = [];

    public void Report(PlatformBuildDiagnostic diagnostic) {
        Diagnostics.Add(diagnostic);
    }
}
```

- [ ] **Step 6: Add the initial failing metadata test**

```csharp
using helengine.gamecube.builder;

namespace helengine.gamecube.builder.tests;

public class GameCubePlatformAssetBuilderTests {
    [Fact]
    public void Descriptor_and_definition_expose_gamecube_metadata() {
        GameCubePlatformAssetBuilder builder = new();

        Assert.Equal("helengine.gamecube.builder", builder.Descriptor.BuilderId);
        Assert.Equal("gamecube", builder.Descriptor.TargetPlatformId);
        Assert.Equal("gamecube", builder.Definition.PlatformId);
    }
}
```

- [ ] **Step 7: Run the metadata test to verify the scaffold is incomplete**

Run: `rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter Descriptor_and_definition_expose_gamecube_metadata -v minimal`

Expected: FAIL because `GameCubePlatformAssetBuilder` does not exist yet.

- [ ] **Step 8: Commit the scaffold**

```bash
git add builder/helengine.gamecube.builder.csproj builder/helengine.gamecube.builder.sln builder/Program.cs builder.tests/helengine.gamecube.builder.tests.csproj builder.tests/Builders/RecordingDiagnosticReporter.cs builder.tests/Builders/RecordingProgressReporter.cs builder.tests/GameCubePlatformAssetBuilderTests.cs
git commit -m "Add GameCube builder project scaffold"
```

### Task 2: Implement the GameCube builder metadata layer

**Files:**
- Create: `builder/GameCubePlatformAssetBuilder.cs`
- Create: `builder/GameCubePlatformDefinitionFactory.cs`
- Modify: `builder.tests/GameCubePlatformAssetBuilderTests.cs`
- Test: `builder.tests/GameCubePlatformAssetBuilderTests.cs`

- [ ] **Step 1: Extend the metadata test to assert the full definition shape**

```csharp
[Fact]
public void Descriptor_and_definition_expose_gamecube_metadata() {
    GameCubePlatformAssetBuilder builder = new();

    Assert.Equal("helengine.gamecube.builder", builder.Descriptor.BuilderId);
    Assert.Equal("gamecube", builder.Descriptor.TargetPlatformId);
    Assert.Contains("gamecube", builder.Descriptor.SupportedRuntimeBackendIds);
    Assert.Equal("gamecube", builder.Definition.PlatformId);
    Assert.Contains(builder.Definition.BuildProfiles, profile => profile.ProfileId == "gamecube-default");
    Assert.Contains(builder.Definition.GraphicsProfiles, profile => profile.ProfileId == "gx");
}
```

- [ ] **Step 2: Run the metadata test to verify it fails**

Run: `rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter Descriptor_and_definition_expose_gamecube_metadata -v minimal`

Expected: FAIL because the builder metadata layer is not implemented.

- [ ] **Step 3: Implement the platform definition factory**

```csharp
using helengine.baseplatform.Definitions;

namespace helengine.gamecube.builder;

public static class GameCubePlatformDefinitionFactory {
    public static PlatformDefinition Create() {
        return new PlatformDefinition(
            "gamecube",
            "Nintendo GameCube",
            [
                new PlatformBuildProfileDefinition(
                    "gamecube-default",
                    "GameCube Default",
                    "Standard GameCube player build",
                    "gx",
                    [
                        new PlatformSettingDefinition("texture-scale-percent", "Texture Scale Percent", PlatformSettingKind.Text, "100", true, []),
                        new PlatformSettingDefinition("shader-variant-pruning", "Shader Variant Pruning", PlatformSettingKind.Boolean, "true", true, [])
                    ])
            ],
            [
                new PlatformGraphicsProfileDefinition(
                    "gx",
                    "GX",
                    "Current GameCube rendering backend",
                    [
                        new PlatformSettingDefinition("default-width", "Default Width", PlatformSettingKind.Text, "640", true, []),
                        new PlatformSettingDefinition("default-height", "Default Height", PlatformSettingKind.Text, "480", true, []),
                        new PlatformSettingDefinition("vsync-enabled", "VSync Enabled", PlatformSettingKind.Boolean, "true", true, []),
                        new PlatformSettingDefinition("fullscreen-enabled", "Fullscreen Enabled", PlatformSettingKind.Boolean, "true", true, [])
                    ])
            ],
            [
                new PlatformAssetRequirementDefinition("scene", "Scene", true, ["helen"]),
                new PlatformAssetRequirementDefinition("texture", "Texture", true, ["png", "tga", "jpg"]),
                new PlatformAssetRequirementDefinition("font", "Font", false, ["font.asset", "ttf", "otf"])
            ]);
    }
}
```

- [ ] **Step 4: Implement the builder shell that exposes descriptor and definition**

```csharp
using helengine.baseplatform.Builders;
using helengine.baseplatform.Definitions;
using helengine.baseplatform.Descriptors;
using helengine.baseplatform.Reporting;
using helengine.baseplatform.Requests;

namespace helengine.gamecube.builder;

public sealed class GameCubePlatformAssetBuilder : IPlatformAssetBuilder {
    public GameCubePlatformAssetBuilder() {
        Descriptor = new PlatformBuilderDescriptor(
            "helengine.gamecube.builder",
            "1.0.0",
            "gamecube",
            new EngineCompatibilityRange("1.0.0", "999.0.0"),
            new ManifestCompatibilityRange(1, 1),
            ["gamecube"],
            ["gamecube-default"]);
        Definition = GameCubePlatformDefinitionFactory.Create();
    }

    public PlatformBuilderDescriptor Descriptor { get; }

    public PlatformDefinition Definition { get; }

    public Task<PlatformBuildReport> BuildAsync(
        PlatformBuildRequest request,
        IPlatformBuildProgressReporter progressReporter,
        IPlatformBuildDiagnosticReporter diagnosticReporter,
        CancellationToken cancellationToken) {
        return GameCubeBuildWorkspace.BuildAsync(request, progressReporter, diagnosticReporter, cancellationToken);
    }
}
```

- [ ] **Step 5: Run the metadata test to verify it passes**

Run: `rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter Descriptor_and_definition_expose_gamecube_metadata -v minimal`

Expected: PASS

- [ ] **Step 6: Commit the metadata layer**

```bash
git add builder/GameCubePlatformAssetBuilder.cs builder/GameCubePlatformDefinitionFactory.cs builder.tests/GameCubePlatformAssetBuilderTests.cs
git commit -m "Add GameCube builder metadata"
```

### Task 3: Implement payload staging and GameCube build manifest output

**Files:**
- Create: `builder/GameCubeBuildWorkspace.cs`
- Modify: `builder/GameCubePlatformAssetBuilder.cs`
- Modify: `builder.tests/GameCubePlatformAssetBuilderTests.cs`
- Test: `builder.tests/GameCubePlatformAssetBuilderTests.cs`

- [ ] **Step 1: Add the failing staging test**

```csharp
[Fact]
public async Task BuildAsync_copies_payloads_into_the_output_root() {
    string workingRoot = Path.Combine(Path.GetTempPath(), Guid.NewGuid().ToString("N"));
    string outputRoot = Path.Combine(workingRoot, "out");
    string sourceRoot = Path.Combine(workingRoot, "project");
    string sceneSourcePath = Path.Combine(sourceRoot, "scenes", "startup.helen");
    string textureSourcePath = Path.Combine(sourceRoot, "assets", "textures", "checker.png");

    Directory.CreateDirectory(Path.GetDirectoryName(sceneSourcePath)!);
    Directory.CreateDirectory(Path.GetDirectoryName(textureSourcePath)!);
    File.WriteAllText(sceneSourcePath, "scene payload");
    File.WriteAllText(textureSourcePath, "texture payload");

    string previousDirectory = Directory.GetCurrentDirectory();
    try {
        Directory.SetCurrentDirectory(sourceRoot);

        PlatformBuildManifest manifest = new(
            1,
            "project",
            "1.0.0",
            "1.0.0",
            [
                new PlatformBuildScene("startup", "Startup", "scenes/startup.helen", [], [])
            ],
            [
                new PlatformBuildAsset("checker", "Checker Texture", "assets/textures/checker.png", new PlatformBuildPayloadReference("checker-payload", "assets/textures/checker.png"), [])
            ]);

        PlatformBuildRequest request = new(
            manifest,
            [new PlatformBuildTargetVariant("gamecube-default", "gamecube", "gamecube", "gamecube-default")],
            [new PlatformCookProfile("gamecube-default", "GameCube Default", new PlatformCookProfileCapabilities("gamecube", "raw", "rgba", "gamecube-scene-v1", PlatformSerializationEndianness.BigEndian))],
            outputRoot,
            Path.Combine(workingRoot, "tmp"));

        GameCubePlatformAssetBuilder builder = new();
        RecordingProgressReporter progressReporter = new();
        RecordingDiagnosticReporter diagnosticReporter = new();

        PlatformBuildReport report = await builder.BuildAsync(request, progressReporter, diagnosticReporter, CancellationToken.None);

        Assert.True(report.Succeeded);
        Assert.Empty(diagnosticReporter.Diagnostics);
        Assert.Equal(2, progressReporter.Updates.Count);
        Assert.True(File.Exists(Path.Combine(outputRoot, "scenes", "startup.helen")));
        Assert.True(File.Exists(Path.Combine(outputRoot, "assets", "textures", "checker.png")));
        Assert.True(File.Exists(Path.Combine(workingRoot, "tmp", "gamecube-build-manifest.json")));
    } finally {
        Directory.SetCurrentDirectory(previousDirectory);
        if (Directory.Exists(workingRoot)) {
            Directory.Delete(workingRoot, recursive: true);
        }
    }
}
```

- [ ] **Step 2: Run the staging test to verify it fails**

Run: `rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter BuildAsync_copies_payloads_into_the_output_root -v minimal`

Expected: FAIL because `GameCubeBuildWorkspace` does not exist yet.

- [ ] **Step 3: Implement the staging workspace**

```csharp
using System.Text.Json;
using helengine.baseplatform.Builders;
using helengine.baseplatform.Manifest;
using helengine.baseplatform.Reporting;
using helengine.baseplatform.Requests;

namespace helengine.gamecube.builder;

public static class GameCubeBuildWorkspace {
    static readonly JsonSerializerOptions JsonOptions = new() {
        WriteIndented = true
    };

    public static Task<PlatformBuildReport> BuildAsync(
        PlatformBuildRequest request,
        IPlatformBuildProgressReporter progressReporter,
        IPlatformBuildDiagnosticReporter diagnosticReporter,
        CancellationToken cancellationToken) {
        if (request == null) {
            throw new ArgumentNullException(nameof(request));
        } else if (progressReporter == null) {
            throw new ArgumentNullException(nameof(progressReporter));
        } else if (diagnosticReporter == null) {
            throw new ArgumentNullException(nameof(diagnosticReporter));
        }

        Directory.CreateDirectory(request.OutputRoot);
        Directory.CreateDirectory(request.WorkingRoot);

        List<PlatformBuildDiagnostic> diagnostics = [];
        List<PlatformBuildItemOutcome> sceneOutcomes = [];
        List<PlatformBuildItemOutcome> looseAssetOutcomes = [];
        List<GameCubeBuildManifestEntry> sceneEntries = [];
        List<GameCubeBuildManifestEntry> looseAssetEntries = [];

        int totalItems = request.Manifest.Scenes.Length + request.Manifest.LooseAssets.Length;
        int completedItems = 0;

        CopyScenePayloads(request, progressReporter, diagnosticReporter, cancellationToken, diagnostics, sceneOutcomes, sceneEntries, totalItems, ref completedItems);
        CopyLooseAssetPayloads(request, progressReporter, diagnosticReporter, cancellationToken, diagnostics, looseAssetOutcomes, looseAssetEntries, totalItems, ref completedItems);
        WriteBuildManifest(request, sceneEntries, looseAssetEntries);

        bool succeeded = diagnostics.Count == 0
            && sceneOutcomes.TrueForAll(outcome => outcome.OutcomeKind == PlatformBuildItemOutcomeKind.Succeeded)
            && looseAssetOutcomes.TrueForAll(outcome => outcome.OutcomeKind == PlatformBuildItemOutcomeKind.Succeeded);

        return Task.FromResult(new PlatformBuildReport(succeeded, [.. diagnostics], [.. sceneOutcomes], [.. looseAssetOutcomes]));
    }
}
```

- [ ] **Step 4: Fill in the copy helpers and manifest writer**

```csharp
static void WriteBuildManifest(
    PlatformBuildRequest request,
    IReadOnlyList<GameCubeBuildManifestEntry> sceneEntries,
    IReadOnlyList<GameCubeBuildManifestEntry> looseAssetEntries) {
    string manifestPath = Path.Combine(request.WorkingRoot, "gamecube-build-manifest.json");
    object manifest = new {
        request.Manifest.ProjectId,
        request.Manifest.ProjectVersion,
        request.Manifest.RequiredEngineVersion,
        request.OutputRoot,
        Scenes = sceneEntries,
        LooseAssets = looseAssetEntries
    };

    File.WriteAllText(manifestPath, JsonSerializer.Serialize(manifest, JsonOptions));
}

sealed record GameCubeBuildManifestEntry(string ItemId, string SourceIdentity, string OutputPath);
```

```csharp
static string ResolveSourcePath(string sourceIdentity) {
    string normalizedSourceIdentity = sourceIdentity.Replace('\\', Path.DirectorySeparatorChar).Replace('/', Path.DirectorySeparatorChar);
    if (Path.IsPathRooted(normalizedSourceIdentity)) {
        return Path.GetFullPath(normalizedSourceIdentity);
    }

    return Path.GetFullPath(Path.Combine(Directory.GetCurrentDirectory(), normalizedSourceIdentity));
}

static string ResolveOutputPath(string outputRoot, string sourceIdentity) {
    string normalizedSourceIdentity = sourceIdentity.Replace('\\', Path.DirectorySeparatorChar).Replace('/', Path.DirectorySeparatorChar);
    if (Path.IsPathRooted(normalizedSourceIdentity)) {
        normalizedSourceIdentity = Path.GetFileName(normalizedSourceIdentity);
    }

    return Path.GetFullPath(Path.Combine(outputRoot, normalizedSourceIdentity));
}
```

- [ ] **Step 5: Run the staging test to verify it passes**

Run: `rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj --filter BuildAsync_copies_payloads_into_the_output_root -v minimal`

Expected: PASS

- [ ] **Step 6: Run the full GameCube builder test project**

Run: `rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj -v minimal`

Expected: PASS with the metadata and staging tests green.

- [ ] **Step 7: Commit the staging implementation**

```bash
git add builder/GameCubeBuildWorkspace.cs builder/GameCubePlatformAssetBuilder.cs builder.tests/GameCubePlatformAssetBuilderTests.cs
git commit -m "Add GameCube builder staging workspace"
```

### Task 4: Document and verify the dynamic builder slice

**Files:**
- Modify: `README.md`
- Modify: `builder/Program.cs`
- Test: `builder.tests/GameCubePlatformAssetBuilderTests.cs`

- [ ] **Step 1: Add a builder section to the README**

```md
## Builder Output

The GameCube builder assembly lives under `builder/` and is intended to be loaded dynamically by the editor through `user_settings/platforms.json`.

The current builder slice exposes GameCube platform metadata and stages payloads into the output root. It does not yet invoke the native GameCube player build or emit the final `.dol`.
```

- [ ] **Step 2: Verify the CLI description path matches the dynamic loader expectations**

```csharp
if (args.Length > 0 && string.Equals(args[0], "--describe", StringComparison.OrdinalIgnoreCase)) {
    GameCubePlatformAssetBuilder builder = new();
    Console.WriteLine(builder.Descriptor.BuilderId);
    Console.WriteLine(builder.Descriptor.TargetPlatformId);
    Console.WriteLine(builder.Definition.DisplayName);
    return 0;
}
```

- [ ] **Step 3: Run the builder CLI describe mode**

Run: `rtk dotnet run --project builder/helengine.gamecube.builder.csproj -- --describe`

Expected:

```text
helengine.gamecube.builder
gamecube
Nintendo GameCube
```

- [ ] **Step 4: Run the builder CLI smoke-test mode**

Run: `rtk dotnet run --project builder/helengine.gamecube.builder.csproj -- --smoke-test`

Expected:

```text
gamecube.builder smoke test entrypoint
```

- [ ] **Step 5: Run the final verification sweep**

Run: `rtk dotnet test builder.tests/helengine.gamecube.builder.tests.csproj -v minimal`

Expected: PASS

Run: `rtk dotnet build builder/helengine.gamecube.builder.sln -v minimal`

Expected: PASS

- [ ] **Step 6: Commit the documented builder slice**

```bash
git add README.md builder/Program.cs
git commit -m "Document GameCube builder bootstrap"
```
