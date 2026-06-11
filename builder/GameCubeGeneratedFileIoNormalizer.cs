namespace helengine.gamecube.builder;

/// <summary>
/// Rewrites generated native file support so packaged GameCube runtime paths are dispatched through the disc-backed file system.
/// </summary>
public sealed class GameCubeGeneratedFileIoNormalizer {
    /// <summary>
    /// Normalizes one generated <c>system/io/file.cpp</c> source file in place when it still uses host-style file access.
    /// </summary>
    /// <param name="sourcePath">Generated native file source path.</param>
    public void Normalize(string sourcePath) {
        if (string.IsNullOrWhiteSpace(sourcePath)) {
            throw new ArgumentException("Source path must not be empty.", nameof(sourcePath));
        } else if (!File.Exists(sourcePath)) {
            return;
        }

        string contents = File.ReadAllText(sourcePath);
        string normalizedContents = NormalizeSource(contents);
        if (!string.Equals(contents, normalizedContents, StringComparison.Ordinal)) {
            File.WriteAllText(sourcePath, normalizedContents);
        }
    }

    /// <summary>
    /// Rewrites generated native file support so packaged GameCube runtime paths resolve through <c>GameCubeDiscFileSystem</c>.
    /// </summary>
    /// <param name="contents">Generated native file source contents.</param>
    /// <returns>Normalized source contents.</returns>
    public static string NormalizeSource(string contents) {
        if (contents == null) {
            throw new ArgumentNullException(nameof(contents));
        } else if (contents.Contains("GameCubeDiscFileSystem", StringComparison.Ordinal)) {
            return contents;
        }

        string newline = contents.Contains("\r\n", StringComparison.Ordinal) ? "\r\n" : "\n";
        string normalizedContents = contents;
        if (!normalizedContents.Contains("#include \"platform/gamecube/GameCubeDiscFileSystem.hpp\"", StringComparison.Ordinal)) {
            normalizedContents = normalizedContents.Replace(
                "#include \"file.hpp\"",
                "#include \"file.hpp\"" + newline + "#include \"platform/gamecube/GameCubeDiscFileSystem.hpp\"",
                StringComparison.Ordinal);
        }

        normalizedContents = normalizedContents.Replace(
            "bool File::Exists(const char* fileName) {" + newline
            + "\tif (!fileName)" + newline
            + "\t{" + newline
            + "\t\treturn false;" + newline
            + "\t}" + newline + newline
            + "\tstd::ifstream file(fileName);" + newline
            + "\treturn file.good();" + newline
            + "}",
            "bool File::Exists(const char* fileName) {" + newline
            + "\tif (!fileName)" + newline
            + "\t{" + newline
            + "\t\treturn false;" + newline
            + "\t}" + newline + newline
            + "\tif (helengine::gamecube::GameCubeDiscFileSystem::CanHandlePath(fileName)) {" + newline
            + "\t\treturn helengine::gamecube::GameCubeDiscFileSystem::Exists(fileName);" + newline
            + "\t}" + newline + newline
            + "\tstd::ifstream file(fileName);" + newline
            + "\treturn file.good();" + newline
            + "}",
            StringComparison.Ordinal);

        normalizedContents = normalizedContents.Replace(
            "FileStream* File::OpenRead(const char* filePath)" + newline
            + "{" + newline
            + "\treturn new FileStream(filePath, FileMode::Open, FileAccess::Read, FileShare::Read);" + newline
            + "}",
            "FileStream* File::OpenRead(const char* filePath)" + newline
            + "{" + newline
            + "\tif (helengine::gamecube::GameCubeDiscFileSystem::CanHandlePath(filePath)) {" + newline
            + "\t\treturn helengine::gamecube::GameCubeDiscFileSystem::OpenRead(filePath);" + newline
            + "\t}" + newline + newline
            + "\treturn new FileStream(filePath, FileMode::Open, FileAccess::Read, FileShare::Read);" + newline
            + "}",
            StringComparison.Ordinal);

        return normalizedContents;
    }
}
