using System.Diagnostics;

namespace helengine.gamecube.builder.tests;

/// <summary>
/// Verifies that external GameCube build and packaging processes expose their output while they run.
/// </summary>
public sealed class GameCubeProcessOutputStreamingTests {
    /// <summary>
    /// Ensures the Dockerized native build uses the same live output path as external packaging tools.
    /// </summary>
    [Fact]
    public void NativeBuildExecutor_UsesLiveProcessOutputStreamer() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string source = File.ReadAllText(Path.Combine(repositoryRootPath, "builder", "GameCubeDockerNativeBuildExecutor.cs"));

        Assert.Contains("new GameCubeProcessOutputStreamer().Run(startInfo, cancellationToken)", source, StringComparison.Ordinal);
        Assert.DoesNotContain("ReadToEndAsync", source, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures a real process runner forwards both redirected streams to the host console and retains them in its result.
    /// </summary>
    [Fact]
    public void Run_WhenProcessWritesOutput_ForwardsOutputToConsoleAndRetainsCapturedResult() {
        if (!OperatingSystem.IsWindows()) {
            return;
        }

        ProcessStartInfo startInfo = new() {
            FileName = "cmd.exe",
            UseShellExecute = false
        };
        startInfo.ArgumentList.Add("/c");
        startInfo.ArgumentList.Add("echo gamecube-streamed-output && echo gamecube-streamed-error 1>&2");

        TextWriter originalOutput = Console.Out;
        TextWriter originalError = Console.Error;
        using StringWriter output = new();
        using StringWriter error = new();

        try {
            Console.SetOut(output);
            Console.SetError(error);

            GameCubeProcessRunResult result = new GameCubeProcessRunner().Run(startInfo, CancellationToken.None);

            Assert.Equal(0, result.ExitCode);
            Assert.Contains("gamecube-streamed-output", result.StandardOutput, StringComparison.Ordinal);
            Assert.Contains("gamecube-streamed-error", result.StandardError, StringComparison.Ordinal);
            Assert.Contains("gamecube-streamed-output", output.ToString(), StringComparison.Ordinal);
            Assert.Contains("gamecube-streamed-error", error.ToString(), StringComparison.Ordinal);
        } finally {
            Console.SetOut(originalOutput);
            Console.SetError(originalError);
        }
    }
}
