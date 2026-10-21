using helengine.baseplatform.Builders;
using helengine.baseplatform.Definitions;

namespace helengine.gamecube.builder.tests;

/// <summary>Checks that build-selected video modes provide deterministic cooked viewport targets.</summary>
public sealed class GameCubeViewportCookTests {
    /// <summary>Cook and native video selection must agree for both supported regions.</summary>
    [Theory]
    [InlineData(GameCubeVideoModes.DefaultModeId, 480, 1d)]
    [InlineData(GameCubeVideoModes.PalModeId, 528, 1.1d)]
    public void SelectedVideoMode_DeterminesCookTarget(string id, int height, double aspect) {
        GameCubePlatformAssetBuilder builder = new GameCubePlatformAssetBuilder();
        Dictionary<string, string> options = new Dictionary<string, string> { [GameCubeVideoModes.SettingId] = id };
        PlatformLayoutTarget target = builder.ResolveLayoutTarget(new PlatformLayoutTargetRequest("release", "gx", options));
        PlatformVideoMode mode = GameCubeVideoModes.Resolve(options);
        Assert.True(builder.Definition.RuntimeGenerationContract.RequiresCookedViewportLayout);
        Assert.Equal(640, target.Width);
        Assert.Equal(height, target.Height);
        Assert.Equal(aspect, target.PixelAspect, 10);
        Assert.Equal(mode.Width, target.Width);
        Assert.Equal(mode.Height, target.Height);
        Assert.NotNull(builder.Definition.VideoModeCapability);
    }
    /// <summary>Existing projects receive the approved NTSC default while an invalid explicit mode is rejected.</summary>
    [Fact]
    public void VideoMode_DefaultsToNtscAndRejectsUnknownOverrides() {
        Assert.Equal(GameCubeVideoModes.DefaultModeId, GameCubeVideoModes.Resolve(new Dictionary<string, string>()).Id);
        Assert.Throws<InvalidOperationException>(() => GameCubeVideoModes.Resolve(new Dictionary<string, string> { [GameCubeVideoModes.SettingId] = "invalid" }));
    }
}
