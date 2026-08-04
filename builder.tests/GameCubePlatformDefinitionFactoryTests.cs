using helengine.baseplatform.Definitions;

namespace helengine.gamecube.builder.tests;

/// <summary>
/// Verifies the GameCube editor-facing platform definition publishes the generic codegen shape settings required by the shared C++ generator.
/// </summary>
public sealed class GameCubePlatformDefinitionFactoryTests {
    /// <summary>
    /// Ensures the default GameCube codegen profile declares the same generic platform-shape settings the shared code generator expects from custom native targets.
    /// </summary>
    [Fact]
    public void Create_WhenDefaultCodegenProfileIsResolved_PublishesGenericPlatformShapeSettings() {
        PlatformDefinition definition = GameCubePlatformDefinitionFactory.Create();

        PlatformCodegenProfileDefinition codegenProfile = Assert.Single(definition.CodegenProfiles);
        Assert.Equal("default", codegenProfile.ProfileId);
        Assert.Contains(codegenProfile.Settings, setting => setting.SettingId == "generated-math-convention" && setting.DefaultValue == "native-column-vector");
        Assert.Contains(codegenProfile.Settings, setting => setting.SettingId == "pointer-size-bytes" && setting.DefaultValue == "4");
        Assert.Contains(codegenProfile.Settings, setting => setting.SettingId == "type-remaps" && setting.DefaultValue.Contains("System.Numerics.Vector3=helengine.float3", StringComparison.Ordinal));
        Assert.Contains(codegenProfile.Settings, setting => setting.SettingId == "load-native-runtime-metadata" && setting.DefaultValue == "true");
        Assert.Contains(codegenProfile.Settings, setting => setting.SettingId == "include-project-defined-preprocessor-symbols" && setting.DefaultValue == "false");
        Assert.Contains(codegenProfile.Settings, setting => setting.SettingId == "native-file-system-header" && setting.DefaultValue == "\"platform/gamecube/GameCubeDiscFileSystem.hpp\"");
        Assert.Contains(codegenProfile.Settings, setting => setting.SettingId == "native-file-system-type" && setting.DefaultValue == "helengine::gamecube::GameCubeDiscFileSystem");
        Assert.Contains(PortableInputPreprocessorSymbolCatalog.MatrixAbiGxGameCubeWiiSymbol, definition.RuntimeGenerationContract.PortableInputPreprocessorSymbols);
    }

    /// <summary>
    /// Ensures the GameCube build profile exposes the opt-in memory-card diagnostic journal setting without enabling it for normal retail builds.
    /// </summary>
    [Fact]
    public void Create_WhenDefaultBuildProfileIsResolved_PublishesDisabledMemoryCardDiagnosticJournalSetting() {
        PlatformDefinition definition = GameCubePlatformDefinitionFactory.Create();

        PlatformBuildProfileDefinition buildProfile = Assert.Single(definition.BuildProfiles);
        Assert.Contains(
            buildProfile.Settings,
            setting => setting.SettingId == "enable-memory-card-diagnostic-journal"
                && setting.SettingKind == PlatformSettingKind.Boolean
                && setting.DefaultValue == "false");
    }

    /// <summary>
    /// Ensures the GameCube build profile exposes the opt-in Nintendont handoff diagnostic without changing normal retail builds.
    /// </summary>
    [Fact]
    public void Create_WhenDefaultBuildProfileIsResolved_PublishesDisabledNintendontHandoffDiagnosticSetting() {
        PlatformDefinition definition = GameCubePlatformDefinitionFactory.Create();

        PlatformBuildProfileDefinition buildProfile = Assert.Single(definition.BuildProfiles);
        Assert.Contains(
            buildProfile.Settings,
            setting => setting.SettingId == "enable-nintendont-handoff-diagnostic"
                && setting.SettingKind == PlatformSettingKind.Boolean
                && setting.DefaultValue == "false");
    }

    /// <summary>
    /// Ensures the GameCube build profile exposes the opt-in first-frame trace without enabling it for retail builds.
    /// </summary>
    [Fact]
    public void Create_WhenDefaultBuildProfileIsResolved_PublishesDisabledFirstFrameTraceDiagnosticSetting() {
        PlatformDefinition definition = GameCubePlatformDefinitionFactory.Create();

        PlatformBuildProfileDefinition buildProfile = Assert.Single(definition.BuildProfiles);
        Assert.Contains(
            buildProfile.Settings,
            setting => setting.SettingId == "enable-first-frame-trace-diagnostic"
                && setting.SettingKind == PlatformSettingKind.Boolean
                && setting.DefaultValue == "false");
    }

    /// <summary>
    /// Ensures the GameCube build profile exposes the disabled exception-screen diagnostic independently from retail builds.
    /// </summary>
    [Fact]
    public void Create_WhenDefaultBuildProfileIsResolved_PublishesDisabledExceptionScreenDiagnosticSetting() {
        PlatformDefinition definition = GameCubePlatformDefinitionFactory.Create();

        PlatformBuildProfileDefinition buildProfile = Assert.Single(definition.BuildProfiles);
        Assert.Contains(
            buildProfile.Settings,
            setting => setting.SettingId == "enable-exception-screen-diagnostic"
                && setting.SettingKind == PlatformSettingKind.Boolean
                && setting.DefaultValue == "false");
    }
}
