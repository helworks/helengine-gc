namespace helengine.gamecube.builder.tests;

/// <summary>
/// Verifies the deterministic physical path rules for extracted GameCube disc content.
/// </summary>
public sealed class GameCubeDiscPathResolverTests {
    /// <summary>
    /// Ensures cooked scene payloads are staged under the extracted <c>files/</c> tree.
    /// </summary>
    [Fact]
    public void ResolvePhysicalRelativePath_WhenGivenCookedScenePath_PlacesFileUnderFilesTree() {
        string resolved = GameCubeDiscPathResolver.ResolvePhysicalRelativePath("cooked/scenes/rendering/cube_test.hasset");

        Assert.Equal("files/cooked/scenes/rendering/cube_test.hasset", resolved);
    }

    /// <summary>
    /// Ensures Windows-style separators are normalized before the extracted-disc path is returned.
    /// </summary>
    [Fact]
    public void ResolvePhysicalRelativePath_WhenGivenBackslashPath_NormalizesToForwardSlashTree() {
        string resolved = GameCubeDiscPathResolver.ResolvePhysicalRelativePath(@"cooked\engine\models\cube.hasset");

        Assert.Equal("files/cooked/engine/models/cube.hasset", resolved);
    }

    /// <summary>
    /// Ensures blank logical paths are rejected instead of producing an invalid extracted-disc entry.
    /// </summary>
    [Fact]
    public void ResolvePhysicalRelativePath_WhenGivenBlankPath_ThrowsArgumentException() {
        Assert.Throws<ArgumentException>(() => GameCubeDiscPathResolver.ResolvePhysicalRelativePath("  "));
    }
}
