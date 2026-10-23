using System.Diagnostics;
using System.Reflection;

namespace helengine.gamecube.builder.tests;

/// <summary>
/// Verifies GameCube Docker native-build invocation maps generated-core roots correctly for editor-owned builds.
/// </summary>
public sealed class GameCubeDockerNativeBuildExecutorTests {





    /// <summary>
    /// Ensures external generated-core roots are mounted separately into Docker and exported through one absolute container path.
    /// </summary>
    [Fact]
    public void CreateStartInfo_WhenGeneratedCoreRootIsOutsideRepositoryRoot_MountsGeneratedCoreSeparately() {
        string repositoryRootPath = ResolveRepositoryRootPath();
        string generatedCoreRootPath = Path.Combine(Path.GetTempPath(), "native executor generated core");
        Directory.CreateDirectory(generatedCoreRootPath);

        try {
            GameCubeBuilderPaths paths = new(
                repositoryRootPath,
                generatedCoreRootPath,
                Path.Combine(repositoryRootPath, "tmp", "staged-content"),
                Path.Combine(repositoryRootPath, "tmp", "disc"),
                Path.Combine(repositoryRootPath, "tmp", "game.gcm"),
                Path.Combine(repositoryRootPath, "tmp", "native", "helengine_gc.dol"),
                Path.Combine(repositoryRootPath, "tmp", "native", "apploader.img"),
                Path.Combine(repositoryRootPath, "tmp", "native", "gbi.hdr"));
            MethodInfo createStartInfoMethod = typeof(GameCubeDockerNativeBuildExecutor).GetMethod(
                "CreateStartInfo",
                BindingFlags.Static | BindingFlags.NonPublic);

            Assert.NotNull(createStartInfoMethod);

            ProcessStartInfo startInfo = (ProcessStartInfo)createStartInfoMethod.Invoke(null, [paths]);

            Assert.Equal("docker", startInfo.FileName);
            Assert.Equal("run", startInfo.ArgumentList[0]);
            Assert.Equal("--platform", startInfo.ArgumentList[1]);
            Assert.Equal("linux/amd64", startInfo.ArgumentList[2]);
            Assert.DoesNotContain("docker", startInfo.ArgumentList);
            Assert.Contains(repositoryRootPath + ":/workspace", startInfo.ArgumentList);
            Assert.Contains(generatedCoreRootPath + ":/helengine-generated-core", startInfo.ArgumentList);
            Assert.Contains("HELENGINE_CORE_CPP_ROOT=/helengine-generated-core", startInfo.ArgumentList);
            Assert.False(startInfo.UseShellExecute);
            Assert.True(startInfo.RedirectStandardOutput);
            Assert.True(startInfo.RedirectStandardError);
        } finally {
            if (Directory.Exists(generatedCoreRootPath)) {
                Directory.Delete(generatedCoreRootPath, true);
            }
        }
    }

    /// <summary>Retains the real failing child's exit code and both redirected streams without a logging wrapper.</summary>
    [Fact]
    public void ProcessOutputStreamer_WhenChildFails_PreservesExitCodeAndDiagnostics() {
        ProcessStartInfo startInfo = new() {
            FileName = OperatingSystem.IsWindows() ? Environment.GetEnvironmentVariable("ComSpec") : "/bin/sh",
            UseShellExecute = false,
            CreateNoWindow = true,
            RedirectStandardOutput = true,
            RedirectStandardError = true
        };
        if (OperatingSystem.IsWindows()) {
            startInfo.ArgumentList.Add("/d");
            startInfo.ArgumentList.Add("/c");
            startInfo.ArgumentList.Add("echo native-stdout & echo native-stderr 1>&2 & exit /b 37");
        } else {
            startInfo.ArgumentList.Add("-c");
            startInfo.ArgumentList.Add("printf 'native-stdout\\n'; printf 'native-stderr\\n' >&2; exit 37");
        }

        GameCubeProcessRunResult result = new GameCubeProcessOutputStreamer().Run(startInfo, CancellationToken.None);

        Assert.Equal(37, result.ExitCode);
        Assert.Contains("native-stdout", result.StandardOutput, StringComparison.Ordinal);
        Assert.Contains("native-stderr", result.StandardError, StringComparison.Ordinal);
    }

    /// <summary>Locates the real checkout independently of an external managed artifacts directory.</summary>
    /// <param name="sourcePath">Compiler-supplied test source path in the actual checkout.</param>
    /// <returns>The repository containing this test and its builder sources.</returns>
    static string ResolveRepositoryRootPath([System.Runtime.CompilerServices.CallerFilePath] string sourcePath = "") {
        return Path.GetFullPath(Path.Combine(Path.GetDirectoryName(sourcePath), ".."));
    }
}
