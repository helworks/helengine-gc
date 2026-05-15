using helengine.baseplatform.Definitions;

namespace helengine.gamecube.builder;

/// <summary>
/// Provides a small command-line entrypoint for the GameCube builder assembly.
/// </summary>
public static class Program {
    /// <summary>
    /// Runs the builder smoke mode or prints the builder identity.
    /// </summary>
    /// <param name="args">Command-line arguments.</param>
    /// <returns>Zero on success.</returns>
    public static int Main(string[] args) {
        if (args.Length > 0 && string.Equals(args[0], "--describe", StringComparison.OrdinalIgnoreCase)) {
            GameCubePlatformAssetBuilder builder = new();
            Console.WriteLine(builder.Descriptor.BuilderId);
            Console.WriteLine(builder.Descriptor.TargetPlatformId);
            Console.WriteLine(builder.Definition.DisplayName);
            return 0;
        }

        if (args.Length > 0 && string.Equals(args[0], "--smoke-test", StringComparison.OrdinalIgnoreCase)) {
            string apploaderPath = Environment.GetEnvironmentVariable("HELENGINE_GAMECUBE_APPLOADER_PATH");
            Console.WriteLine(
                "gamecube.builder smoke test entrypoint "
                + "(packager: direct-raw-gcm, "
                + $"apploader: {(string.IsNullOrWhiteSpace(apploaderPath) ? "missing" : "configured")})");
            return 0;
        }

        Console.WriteLine("helengine.gamecube.builder --describe | --smoke-test");
        return 0;
    }
}
