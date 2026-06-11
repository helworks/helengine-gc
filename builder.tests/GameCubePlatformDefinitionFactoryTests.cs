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
        Assert.Equal("default", codegenProfile.Id);
        Assert.Contains(codegenProfile.Settings, setting => setting.Id == "generated-math-convention" && setting.DefaultValue == "native-column-vector");
        Assert.Contains(codegenProfile.Settings, setting => setting.Id == "pointer-size-bytes" && setting.DefaultValue == "4");
        Assert.Contains(codegenProfile.Settings, setting => setting.Id == "type-remaps" && setting.DefaultValue.Contains("System.Numerics.Vector3=helengine.float3", StringComparison.Ordinal));
        Assert.Contains(codegenProfile.Settings, setting => setting.Id == "load-native-runtime-metadata" && setting.DefaultValue == "true");
        Assert.Contains(codegenProfile.Settings, setting => setting.Id == "include-project-defined-preprocessor-symbols" && setting.DefaultValue == "false");
    }
}
