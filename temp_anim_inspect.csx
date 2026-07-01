using System;
using System.IO;
using helengine;

var path = @"C:\dev\helprojs\city\assets\Animations\DemoDiscLogoIdle.hanim";
using var stream = new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.Read);
var asset = (AnimationClipAsset)AssetSerializer.Deserialize(stream);
Console.WriteLine($"Duration={asset.Duration}");
Console.WriteLine($"Position={asset.PositionTracks.Length} PositionOffset={asset.PositionOffsetTracks.Length} Scale={asset.ScaleTracks.Length} Rotation={asset.RotationTracks.Length}");
if (asset.ScaleTracks.Length > 0) {
    Console.WriteLine($"ScaleKeyframes={asset.ScaleTracks[0].Keyframes.Length}");
    foreach (var keyframe in asset.ScaleTracks[0].Keyframes) {
        Console.WriteLine($"S {keyframe.Time} {keyframe.Value.X},{keyframe.Value.Y},{keyframe.Value.Z} {keyframe.Interpolation}");
    }
}
if (asset.RotationTracks.Length > 0) {
    Console.WriteLine($"RotationKeyframes={asset.RotationTracks[0].Keyframes.Length}");
    foreach (var keyframe in asset.RotationTracks[0].Keyframes) {
        Console.WriteLine($"R {keyframe.Time} {keyframe.Value.X},{keyframe.Value.Y},{keyframe.Value.Z},{keyframe.Value.W} {keyframe.Interpolation}");
    }
}
if (asset.PositionOffsetTracks.Length > 0) {
    Console.WriteLine($"OffsetKeyframes={asset.PositionOffsetTracks[0].Keyframes.Length}");
    foreach (var keyframe in asset.PositionOffsetTracks[0].Keyframes) {
        Console.WriteLine($"P {keyframe.Time} {keyframe.Value.X},{keyframe.Value.Y},{keyframe.Value.Z} {keyframe.Interpolation}");
    }
}
