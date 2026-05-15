namespace helengine.gamecube.builder.tests;

/// <summary>
/// Verifies the extracted GameCube disc layout produced for packaged builds.
/// </summary>
public sealed class GameCubeDiscLayoutWriterTests {
    /// <summary>
    /// Ensures the layout writer stages <c>sys/main.dol</c> and cooked payloads into the extracted-disc tree.
    /// </summary>
    [Fact]
    public void Write_CopiesMainDolAndCookedPayloadsIntoDiscTree() {
        string rootPath = Path.Combine(Path.GetTempPath(), "gamecube-disc-layout-tests", Guid.NewGuid().ToString("N"));
        string stagingRootPath = Path.Combine(rootPath, "staging");
        string discRootPath = Path.Combine(rootPath, "disc");
        string nativeExecutablePath = Path.Combine(rootPath, "build", "helengine_gc.dol");
        string apploaderImagePath = Path.Combine(rootPath, "tooling", "apploader.img");
        string scenePath = Path.Combine(stagingRootPath, "cooked", "scenes", "rendering", "cube_test.hasset");
        string modelPath = Path.Combine(stagingRootPath, "cooked", "engine", "models", "cube.hasset");

        Directory.CreateDirectory(Path.GetDirectoryName(scenePath) ?? throw new InvalidOperationException("Scene directory path could not be resolved."));
        Directory.CreateDirectory(Path.GetDirectoryName(modelPath) ?? throw new InvalidOperationException("Model directory path could not be resolved."));
        Directory.CreateDirectory(Path.GetDirectoryName(nativeExecutablePath) ?? throw new InvalidOperationException("Executable directory path could not be resolved."));
        Directory.CreateDirectory(Path.GetDirectoryName(apploaderImagePath) ?? throw new InvalidOperationException("Apploader directory path could not be resolved."));
        File.WriteAllText(scenePath, "scene");
        File.WriteAllText(modelPath, "model");
        File.WriteAllText(nativeExecutablePath, "dol");
        File.WriteAllText(apploaderImagePath, "apploader");

        try {
            GameCubeDiscLayoutWriter writer = new();
            GameCubeDiscLayoutResult result = writer.Write(
                stagingRootPath,
                nativeExecutablePath,
                discRootPath,
                new GameCubeDiscSystemAreaOptions(apploaderImagePath, "CITYHE", "City"));

            Assert.True(File.Exists(Path.Combine(discRootPath, "sys", "main.dol")));
            Assert.True(File.Exists(Path.Combine(discRootPath, "sys", "boot.bin")));
            Assert.True(File.Exists(Path.Combine(discRootPath, "sys", "bi2.bin")));
            Assert.True(File.Exists(Path.Combine(discRootPath, "sys", "apploader.img")));
            Assert.True(File.Exists(Path.Combine(discRootPath, "sys", "fst.bin")));
            Assert.True(File.Exists(Path.Combine(discRootPath, "setup.txt")));
            Assert.True(File.Exists(Path.Combine(discRootPath, "files", "cooked", "scenes", "rendering", "cube_test.hasset")));
            Assert.True(File.Exists(Path.Combine(discRootPath, "files", "cooked", "engine", "models", "cube.hasset")));
            Assert.Equal("files/cooked/scenes/rendering/cube_test.hasset", result.LogicalToPhysicalPaths["cooked/scenes/rendering/cube_test.hasset"]);
            Assert.Equal(Path.Combine(discRootPath, "sys", "main.dol"), result.DiscExecutablePath);

            byte[] bootBinBytes = File.ReadAllBytes(Path.Combine(discRootPath, "sys", "boot.bin"));
            byte[] fstBytes = File.ReadAllBytes(Path.Combine(discRootPath, "sys", "fst.bin"));
            Assert.Equal(GameCubeDiscSystemAreaWriter.BootBinSize, bootBinBytes.Length);
            Assert.Equal(0xC2339F3D, System.Buffers.Binary.BinaryPrimitives.ReadUInt32BigEndian(bootBinBytes.AsSpan(0x1C, sizeof(uint))));
            uint expectedMainDolOffset = (GameCubeDiscSystemAreaWriter.ApploaderOffset + (uint)File.ReadAllBytes(apploaderImagePath).Length + 31U) & ~31U;
            uint expectedFstOffset = (expectedMainDolOffset + (uint)File.ReadAllBytes(nativeExecutablePath).Length + 31U) & ~31U;
            Assert.Equal(expectedMainDolOffset >> 2, System.Buffers.Binary.BinaryPrimitives.ReadUInt32BigEndian(bootBinBytes.AsSpan(GameCubeDiscSystemAreaWriter.BootBinDolOffsetFieldOffset, sizeof(uint))));
            Assert.Equal(expectedFstOffset >> 2, System.Buffers.Binary.BinaryPrimitives.ReadUInt32BigEndian(bootBinBytes.AsSpan(GameCubeDiscSystemAreaWriter.BootBinFstOffsetFieldOffset, sizeof(uint))));
            Assert.Equal((uint)fstBytes.Length >> 2, System.Buffers.Binary.BinaryPrimitives.ReadUInt32BigEndian(bootBinBytes.AsSpan(GameCubeDiscSystemAreaWriter.BootBinFstSizeFieldOffset, sizeof(uint))));
            Assert.Equal((uint)fstBytes.Length >> 2, System.Buffers.Binary.BinaryPrimitives.ReadUInt32BigEndian(bootBinBytes.AsSpan(GameCubeDiscSystemAreaWriter.BootBinFstMaxSizeFieldOffset, sizeof(uint))));
        } finally {
            if (Directory.Exists(rootPath)) {
                Directory.Delete(rootPath, recursive: true);
            }
        }
    }
}
