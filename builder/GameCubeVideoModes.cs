using helengine.baseplatform.Definitions;

namespace helengine.gamecube.builder;

/// <summary>Declares the build-selected VI mode used by both layout cooking and native startup.</summary>
public static class GameCubeVideoModes {
    /// <summary>Graphics option selecting the fixed native display mode.</summary>
    public const string SettingId = "video-mode";
    /// <summary>Default mode selected for projects without an explicit region override.</summary>
    public const string DefaultModeId = "ntsc-640x480i";
    /// <summary>PAL mode corresponding to libogc's TVPal528IntDf.</summary>
    public const string PalModeId = "pal-640x528i";
    /// <summary>Creates the exact VI framebuffer modes accepted by the native application.</summary>
    public static PlatformVideoModeCapability CreateCapability() {
        return new PlatformVideoModeCapability(SettingId, [
            new PlatformVideoMode(DefaultModeId, 640, 480, false, 59.94d, 60d, 4, 3),
            new PlatformVideoMode(PalModeId, 640, 528, false, 50d, 50d, 4, 3)
        ]);
    }
    /// <summary>Exposes the declared NTSC default and PAL choice to project graphics settings.</summary>
    public static PlatformSettingDefinition CreateGraphicsSetting() {
        return new PlatformSettingDefinition(SettingId, "Video Mode", PlatformSettingKind.Choice,
            DefaultModeId, true, CreateCapability().GetModeIds());
    }
    /// <summary>Resolves the selected mode, applying only the explicitly declared NTSC default when no override exists.</summary>
    public static PlatformVideoMode Resolve(IReadOnlyDictionary<string, string> graphicsOptions) {
        ArgumentNullException.ThrowIfNull(graphicsOptions);
        string id = graphicsOptions.TryGetValue(SettingId, out string selected) ? selected : DefaultModeId;
        foreach (PlatformVideoMode mode in CreateCapability().Modes) {
            if (mode.Id == id) { return mode; }
        }
        throw new InvalidOperationException($"Unknown GameCube video mode '{id}'. Select '{DefaultModeId}' or '{PalModeId}'.");
    }
    /// <summary>Writes a generated native input from the same resolved mode that the cooker uses.</summary>
    public static void WriteNativeHeader(string generatedCoreRoot, IReadOnlyDictionary<string, string> graphicsOptions) {
        PlatformVideoMode mode = Resolve(graphicsOptions);
        Directory.CreateDirectory(generatedCoreRoot);
        File.WriteAllText(Path.Combine(generatedCoreRoot, "GameCubeVideoMode.hpp"),
            "#pragma once\n#define HELENGINE_GAMECUBE_VIDEO_MODE_PAL " + (mode.Id == PalModeId ? "1\n" : "0\n"));
    }
}
