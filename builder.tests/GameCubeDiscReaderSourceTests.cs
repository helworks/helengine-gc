namespace helengine.gamecube.builder.tests;

/// <summary>
/// Verifies the source-level contract of the GameCube DI reader used by packaged-disc builds.
/// </summary>
public sealed class GameCubeDiscReaderSourceTests {
    /// <summary>
    /// Ensures synchronous DI reads can observe enabled terminal status bits when a compatible loader cannot route a synthetic interrupt through libogc.
    /// </summary>
    [Fact]
    public void DiscReaderSource_PollsEnabledTerminalStatusDuringSynchronousRead() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string source = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeDiscReader.cpp"));

        Assert.Contains("bool TryCompleteRead(volatile uint32_t* discInterface)", source, StringComparison.Ordinal);
        Assert.Contains("const uint32_t terminalInterrupts = (status & DiscCompletionInterruptMask) & enabledInterrupts;", source, StringComparison.Ordinal);
        Assert.Contains("TryCompleteRead(discInterface);", source, StringComparison.Ordinal);
        Assert.Contains("return WaitForReadCompletion(discInterface);", source, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures the direct-frame diagnostic build turns a non-completing DI request into a persistent reader-owned checkpoint.
    /// </summary>
    [Fact]
    public void DiscReaderSource_LatchesDiagnosticCheckpointWhenPollingTimesOut() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string source = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeDiscReader.cpp"));

        Assert.Contains("DiscReadPollingIterationLimit", source, StringComparison.Ordinal);
        Assert.Contains("ReportDirectFrameDiagnosticCode(0xD001U);", source, StringComparison.Ordinal);
    }
}
