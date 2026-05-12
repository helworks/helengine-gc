using helengine.baseplatform.Definitions;

namespace helengine.gamecube.builder;

/// <summary>
/// Creates the typed GameCube builder metadata consumed by the editor.
/// </summary>
public static class GameCubePlatformDefinitionFactory {
    /// <summary>
    /// Creates the current GameCube platform definition.
    /// </summary>
    /// <returns>The GameCube platform definition.</returns>
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
                        new PlatformSettingDefinition(
                            "texture-scale-percent",
                            "Texture Scale Percent",
                            PlatformSettingKind.Text,
                            "100",
                            true,
                            []),
                        new PlatformSettingDefinition(
                            "shader-variant-pruning",
                            "Shader Variant Pruning",
                            PlatformSettingKind.Boolean,
                            "true",
                            true,
                            [])
                    ])
            ],
            [
                new PlatformGraphicsProfileDefinition(
                    "gx",
                    "GX",
                    "Current GameCube rendering backend",
                    [
                        new PlatformSettingDefinition(
                            "default-width",
                            "Default Width",
                            PlatformSettingKind.Text,
                            "640",
                            true,
                            []),
                        new PlatformSettingDefinition(
                            "default-height",
                            "Default Height",
                            PlatformSettingKind.Text,
                            "480",
                            true,
                            []),
                        new PlatformSettingDefinition(
                            "vsync-enabled",
                            "VSync Enabled",
                            PlatformSettingKind.Boolean,
                            "true",
                            true,
                            []),
                        new PlatformSettingDefinition(
                            "fullscreen-enabled",
                            "Fullscreen Enabled",
                            PlatformSettingKind.Boolean,
                            "true",
                            true,
                            [])
                    ])
            ],
            [
                new PlatformAssetRequirementDefinition(
                    "scene",
                    "Scene",
                    true,
                    ["helen"]),
                new PlatformAssetRequirementDefinition(
                    "texture",
                    "Texture",
                    true,
                    ["png", "tga", "jpg"]),
                new PlatformAssetRequirementDefinition(
                    "font",
                    "Font",
                    false,
                    ["font.asset", "ttf", "otf"])
            ],
            [
                new PlatformComponentSupportRule(
                    "helengine.meshcomponent",
                    PlatformComponentSupportKind.Transform,
                    "Mesh components are normalized during packaging.",
                    string.Empty),
                new PlatformComponentSupportRule(
                    "helengine.cameracomponent",
                    PlatformComponentSupportKind.Transform,
                    "Camera components are normalized during packaging.",
                    string.Empty),
                new PlatformComponentSupportRule(
                    "helengine.fpscomponent",
                    PlatformComponentSupportKind.PassThrough,
                    "FPS overlay payload is canonical across platforms.",
                    string.Empty)
            ]);
    }
}
