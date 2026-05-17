# GameCube Platform-Cooked Textures Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Move GameCube texture and font-atlas conversion to the editor-owned cook pipeline so GC loads prepacked GX `RGB5A3` texture payloads instead of transcoding RGBA32 at runtime.

**Architecture:** Extend the shared texture asset format contract with a GameCube-specific serialized format value, expose that value through the existing per-platform editor texture settings surface, and have the editor cook both normal textures and font atlases into that format for `gamecube`. On the runtime side, keep using the shared `TextureAsset`/`FontAsset` containers, but teach the GameCube runtime texture loader to directly upload prepacked `GxRgb5A3` bytes instead of calling the RGBA conversion path.

**Tech Stack:** C#, xUnit, shared helengine editor asset pipeline, GameCube C++ runtime, libogc GX texture upload

---

## File Structure

### Shared asset and editor pipeline

- Modify: `C:\dev\helworks\helengine\engine\helengine.core\assets\raw\TextureAssetColorFormat.cs`
  - Add the new serialized GameCube texture format enum member.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\managers\asset\TextureAssetProcessor.cs`
  - Add editor-side conversion from RGBA32 to prepacked GX `RGB5A3`.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\serialization\TextureAssetImportSettingsBinarySerializer.cs`
  - Preserve the new GameCube texture format in sidecar serialization.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\managers\asset\AssetImportManager.cs`
  - Ensure cache identity and import settings hashing react to the new texture format choice.
- Modify: `C:\dev\helworks\helengine\engine\helengine.core\assets\font\FontAssetBinarySerializer.cs`
  - Keep font atlas serialization/deserialization aligned with the expanded texture format enum.

### GameCube platform metadata and runtime

- Modify: `C:\dev\helworks\helengine-gc\builder\GameCubePlatformDefinitionFactory.cs`
  - Make the editor-facing GameCube platform contract explicitly support the GC texture format choice.
- Modify: `C:\dev\helworks\helengine-gc\src\platform\gamecube\GameCubeRuntimeTexture.hpp`
  - Add direct-load support for prepacked GameCube texture payload metadata.
- Modify: `C:\dev\helworks\helengine-gc\src\platform\gamecube\GameCubeRuntimeTexture.cpp`
  - Branch between direct native upload for cooked `GxRgb5A3` and fallback RGBA conversion.
- Modify: `C:\dev\helworks\helengine-gc\src\platform\gamecube\GameCubeRenderManager2D.cpp`
  - Keep the same runtime entry point, but rely on the new cooked-texture path for menu textures and font atlases.
- Modify: `C:\dev\helworks\helengine-gc\builder\GameCubeGeneratedCoreCompatibilityNormalizer.cs`
  - Preserve any generated-source assumptions that still mention texture loading or font texture cleanup.

### Tests

- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\serialization\TextureAssetImportSettingsBinarySerializerTests.cs`
  - Add sidecar round-trip coverage for `GxRgb5A3`.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\asset\TextureAssetProcessorTests.cs`
  - Add processor coverage for GameCube RGB5A3 cooking.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\AssetImportManagerTests.cs`
  - Add cache-identity coverage for GameCube texture format changes.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\serialization\FontAssetBinarySerializerTests.cs`
  - Add font atlas serialization coverage for the new texture format.
- Modify: `C:\dev\helworks\helengine-gc\builder.tests\GameCubePackagedRuntimeSourceTests.cs`
  - Guard the runtime source contract so the old always-convert RGBA path is no longer required for cooked GameCube textures.

## Task 1: Add the shared GameCube texture format contract

**Files:**
- Modify: `C:\dev\helworks\helengine\engine\helengine.core\assets\raw\TextureAssetColorFormat.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\serialization\TextureAssetImportSettingsBinarySerializer.cs`
- Test: `C:\dev\helworks\helengine\engine\helengine.editor.tests\serialization\TextureAssetImportSettingsBinarySerializerTests.cs`

- [ ] **Step 1: Write the failing sidecar round-trip test**

```csharp
[Fact]
public void SerializeDeserialize_WhenGameCubeUsesGxRgb5A3_PreservesThatFormat() {
    TextureAssetImportSettings settings = new TextureAssetImportSettings();
    settings.Processor.Platforms["gamecube"] = new TextureAssetProcessorPlatformSettings {
        MaxResolution = 256,
        ColorFormat = TextureAssetColorFormat.GxRgb5A3,
        AlphaPrecision = TextureAssetAlphaPrecision.A8
    };

    byte[] bytes = TextureAssetImportSettingsBinarySerializer.Serialize(settings);
    TextureAssetImportSettings restored = TextureAssetImportSettingsBinarySerializer.Deserialize(bytes);
    TextureAssetProcessorPlatformSettings gamecube = restored.Processor.Platforms["gamecube"];

    Assert.Equal(256, gamecube.MaxResolution);
    Assert.Equal(TextureAssetColorFormat.GxRgb5A3, gamecube.ColorFormat);
    Assert.Equal(TextureAssetAlphaPrecision.A8, gamecube.AlphaPrecision);
}
```

- [ ] **Step 2: Run the serializer test to verify it fails**

Run:
```powershell
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~TextureAssetImportSettingsBinarySerializerTests.SerializeDeserialize_WhenGameCubeUsesGxRgb5A3_PreservesThatFormat" -v minimal -p:BuildInParallel=false -p:UseSharedCompilation=false
```

Expected: FAIL because `TextureAssetColorFormat` does not define `GxRgb5A3`.

- [ ] **Step 3: Add the new shared texture format and serializer support**

```csharp
// C:\dev\helworks\helengine\engine\helengine.core\assets\raw\TextureAssetColorFormat.cs
public enum TextureAssetColorFormat : byte {
    Rgba32 = 0,
    Rgba4444 = 1,
    Indexed4 = 2,
    Indexed8 = 3,
    GxRgb5A3 = 4
}
```

```csharp
// C:\dev\helworks\helengine\engine\helengine.editor\serialization\TextureAssetImportSettingsBinarySerializer.cs
static TextureAssetColorFormat ReadTextureAssetColorFormat(EngineBinaryReader reader) {
    byte serializedValue = reader.ReadByte();
    if (serializedValue == (byte)TextureAssetColorFormat.Rgba32) {
        return TextureAssetColorFormat.Rgba32;
    } else if (serializedValue == (byte)TextureAssetColorFormat.Rgba4444) {
        return TextureAssetColorFormat.Rgba4444;
    } else if (serializedValue == (byte)TextureAssetColorFormat.Indexed4) {
        return TextureAssetColorFormat.Indexed4;
    } else if (serializedValue == (byte)TextureAssetColorFormat.Indexed8) {
        return TextureAssetColorFormat.Indexed8;
    } else if (serializedValue == (byte)TextureAssetColorFormat.GxRgb5A3) {
        return TextureAssetColorFormat.GxRgb5A3;
    }

    throw new InvalidOperationException($"Unsupported texture color format '{serializedValue}'.");
}
```

- [ ] **Step 4: Run the serializer test to verify it passes**

Run:
```powershell
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~TextureAssetImportSettingsBinarySerializerTests.SerializeDeserialize_WhenGameCubeUsesGxRgb5A3_PreservesThatFormat" -v minimal -p:BuildInParallel=false -p:UseSharedCompilation=false
```

Expected: PASS.

- [ ] **Step 5: Commit**

```powershell
rtk git -C C:\dev\helworks\helengine add engine/helengine.core/assets/raw/TextureAssetColorFormat.cs engine/helengine.editor/serialization/TextureAssetImportSettingsBinarySerializer.cs engine/helengine.editor.tests/serialization/TextureAssetImportSettingsBinarySerializerTests.cs
rtk git -C C:\dev\helworks\helengine commit -m "Add GameCube texture format setting"
```

## Task 2: Cook GameCube textures to prepacked GX RGB5A3 in the editor

**Files:**
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\managers\asset\TextureAssetProcessor.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\managers\asset\AssetImportManager.cs`
- Test: `C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\asset\TextureAssetProcessorTests.cs`
- Test: `C:\dev\helworks\helengine\engine\helengine.editor.tests\AssetImportManagerTests.cs`

- [ ] **Step 1: Write the failing processor and cache-identity tests**

```csharp
[Fact]
public void Apply_WhenGxRgb5A3IsRequested_ProducesPackedGameCubeTextureBytes() {
    TextureAssetProcessor processor = new TextureAssetProcessor();
    TextureAsset input = CreateOpaqueRgbaTextureAsset(4, 4);
    TextureAssetProcessorSettings settings = new TextureAssetProcessorSettings {
        MaxResolution = 0,
        ColorFormat = TextureAssetColorFormat.GxRgb5A3,
        AlphaPrecision = TextureAssetAlphaPrecision.A8
    };

    TextureAsset processed = processor.Apply(input, settings);

    Assert.Equal(TextureAssetColorFormat.GxRgb5A3, processed.ColorFormat);
    Assert.Equal(4 * 4 * 2, processed.Colors.Length);
    Assert.True(processed.PaletteColors == null || processed.PaletteColors.Length == 0);
}
```

```csharp
[Fact]
public void TryLoadTextureAsset_WhenGameCubeColorFormatChanges_ReimportsWithANewAssetId() {
    TextureAssetImportSettings settings = CreateTextureImportSettings();
    settings.Processor.Platforms["gamecube"] = new TextureAssetProcessorPlatformSettings {
        MaxResolution = 256,
        ColorFormat = TextureAssetColorFormat.Rgba32,
        AlphaPrecision = TextureAssetAlphaPrecision.A8
    };

    TextureAsset first = LoadImportedTexture(settings);
    settings.Processor.Platforms["gamecube"].ColorFormat = TextureAssetColorFormat.GxRgb5A3;
    TextureAsset second = LoadImportedTexture(settings);

    Assert.NotEqual(first.Id, second.Id);
}
```

- [ ] **Step 2: Run the new tests to verify they fail**

Run:
```powershell
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~TextureAssetProcessorTests.Apply_WhenGxRgb5A3IsRequested_ProducesPackedGameCubeTextureBytes|FullyQualifiedName~AssetImportManagerTests.TryLoadTextureAsset_WhenGameCubeColorFormatChanges_ReimportsWithANewAssetId" -v minimal -p:BuildInParallel=false -p:UseSharedCompilation=false
```

Expected: FAIL because the processor does not support `GxRgb5A3` and cache identity does not yet exercise that path.

- [ ] **Step 3: Implement GameCube RGB5A3 cooking and cache identity**

```csharp
// C:\dev\helworks\helengine\engine\helengine.editor\managers\asset\TextureAssetProcessor.cs
TextureAsset ConvertColorFormat(TextureAsset asset, TextureAssetColorFormat targetFormat, TextureAssetAlphaPrecision alphaPrecision) {
    if (targetFormat == TextureAssetColorFormat.Rgba32) {
        return ApplyAlphaPrecision(asset, alphaPrecision);
    } else if (targetFormat == TextureAssetColorFormat.Rgba4444) {
        return ConvertToRgba4444(asset, alphaPrecision);
    } else if (targetFormat == TextureAssetColorFormat.Indexed4) {
        return ConvertToIndexed(asset, 16, TextureAssetColorFormat.Indexed4, alphaPrecision);
    } else if (targetFormat == TextureAssetColorFormat.Indexed8) {
        return ConvertToIndexed(asset, 256, TextureAssetColorFormat.Indexed8, alphaPrecision);
    } else if (targetFormat == TextureAssetColorFormat.GxRgb5A3) {
        return ConvertToGxRgb5A3(asset, alphaPrecision);
    }

    throw new InvalidOperationException($"Unsupported texture color format '{targetFormat}'.");
}
```

```csharp
TextureAsset ConvertToGxRgb5A3(TextureAsset asset, TextureAssetAlphaPrecision alphaPrecision) {
    byte[] encoded = new byte[asset.Width * asset.Height * 2];
    for (int pixelIndex = 0; pixelIndex < asset.Width * asset.Height; pixelIndex++) {
        int sourceOffset = pixelIndex * 4;
        byte red = asset.Colors[sourceOffset + 0];
        byte green = asset.Colors[sourceOffset + 1];
        byte blue = asset.Colors[sourceOffset + 2];
        byte alpha = QuantizeAlpha(asset.Colors[sourceOffset + 3], alphaPrecision);
        ushort packed = PackGxRgb5A3(red, green, blue, alpha);
        encoded[pixelIndex * 2 + 0] = (byte)(packed & 0xFF);
        encoded[pixelIndex * 2 + 1] = (byte)(packed >> 8);
    }

    return new TextureAsset {
        Id = asset.Id,
        RuntimeAssetId = asset.RuntimeAssetId,
        Width = asset.Width,
        Height = asset.Height,
        ColorFormat = TextureAssetColorFormat.GxRgb5A3,
        AlphaPrecision = alphaPrecision,
        Colors = encoded,
        PaletteColors = Array.Empty<byte>()
    };
}
```

```csharp
// C:\dev\helworks\helengine\engine\helengine.editor\managers\asset\AssetImportManager.cs
builder.Append(textureProcessorSettings.MaxResolution.ToString(CultureInfo.InvariantCulture)).Append('\n');
builder.Append((int)textureProcessorSettings.ColorFormat).Append('\n');
builder.Append((int)textureProcessorSettings.AlphaPrecision).Append('\n');
```

- [ ] **Step 4: Run the processor and cache tests to verify they pass**

Run:
```powershell
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~TextureAssetProcessorTests.Apply_WhenGxRgb5A3IsRequested_ProducesPackedGameCubeTextureBytes|FullyQualifiedName~AssetImportManagerTests.TryLoadTextureAsset_WhenGameCubeColorFormatChanges_ReimportsWithANewAssetId" -v minimal -p:BuildInParallel=false -p:UseSharedCompilation=false
```

Expected: PASS.

- [ ] **Step 5: Commit**

```powershell
rtk git -C C:\dev\helworks\helengine add engine/helengine.editor/managers/asset/TextureAssetProcessor.cs engine/helengine.editor/managers/asset/AssetImportManager.cs engine/helengine.editor.tests/managers/asset/TextureAssetProcessorTests.cs engine/helengine.editor.tests/AssetImportManagerTests.cs
rtk git -C C:\dev\helworks\helengine commit -m "Cook GameCube textures to RGB5A3"
```

## Task 3: Keep GameCube font atlases on the cooked texture path

**Files:**
- Modify: `C:\dev\helworks\helengine\engine\helengine.core\assets\font\FontAssetBinarySerializer.cs`
- Test: `C:\dev\helworks\helengine\engine\helengine.editor.tests\serialization\FontAssetBinarySerializerTests.cs`

- [ ] **Step 1: Write the failing font atlas format test**

```csharp
[Fact]
public void SerializeDeserialize_WhenSourceTextureUsesGxRgb5A3_PreservesThatFormatAndPayload() {
    FontAsset asset = CreateFontAssetWithTextureFormat(TextureAssetColorFormat.GxRgb5A3, TextureAssetAlphaPrecision.A8);

    byte[] bytes = FontAssetBinarySerializer.Serialize(asset);
    using MemoryStream stream = new MemoryStream(bytes, false);
    FontAsset restored = FontAssetBinarySerializer.Deserialize(stream);

    Assert.Equal(TextureAssetColorFormat.GxRgb5A3, restored.SourceTextureAsset.ColorFormat);
    Assert.Equal(asset.SourceTextureAsset.Colors.Length, restored.SourceTextureAsset.Colors.Length);
}
```

- [ ] **Step 2: Run the font serializer test to verify it fails**

Run:
```powershell
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~FontAssetBinarySerializerTests.SerializeDeserialize_WhenSourceTextureUsesGxRgb5A3_PreservesThatFormatAndPayload" -v minimal -p:BuildInParallel=false -p:UseSharedCompilation=false
```

Expected: FAIL because font texture format readers do not yet accept `GxRgb5A3`.

- [ ] **Step 3: Extend font texture format serialization/deserialization**

```csharp
// C:\dev\helworks\helengine\engine\helengine.core\assets\font\FontAssetBinarySerializer.cs
static TextureAssetColorFormat ReadTextureAssetColorFormat(EngineBinaryReader reader) {
    byte serializedValue = reader.ReadByte();
    if (serializedValue == (byte)TextureAssetColorFormat.Rgba32) {
        return TextureAssetColorFormat.Rgba32;
    } else if (serializedValue == (byte)TextureAssetColorFormat.Rgba4444) {
        return TextureAssetColorFormat.Rgba4444;
    } else if (serializedValue == (byte)TextureAssetColorFormat.Indexed4) {
        return TextureAssetColorFormat.Indexed4;
    } else if (serializedValue == (byte)TextureAssetColorFormat.Indexed8) {
        return TextureAssetColorFormat.Indexed8;
    } else if (serializedValue == (byte)TextureAssetColorFormat.GxRgb5A3) {
        return TextureAssetColorFormat.GxRgb5A3;
    }

    throw new InvalidOperationException($"Unsupported texture color format '{serializedValue}'.");
}
```

- [ ] **Step 4: Run the font serializer test to verify it passes**

Run:
```powershell
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~FontAssetBinarySerializerTests.SerializeDeserialize_WhenSourceTextureUsesGxRgb5A3_PreservesThatFormatAndPayload" -v minimal -p:BuildInParallel=false -p:UseSharedCompilation=false
```

Expected: PASS.

- [ ] **Step 5: Commit**

```powershell
rtk git -C C:\dev\helworks\helengine add engine/helengine.core/assets/font/FontAssetBinarySerializer.cs engine/helengine.editor.tests/serialization/FontAssetBinarySerializerTests.cs
rtk git -C C:\dev\helworks\helengine commit -m "Preserve GameCube font atlas texture format"
```

## Task 4: Load cooked GameCube textures directly at runtime

**Files:**
- Modify: `C:\dev\helworks\helengine-gc\src\platform\gamecube\GameCubeRuntimeTexture.hpp`
- Modify: `C:\dev\helworks\helengine-gc\src\platform\gamecube\GameCubeRuntimeTexture.cpp`
- Modify: `C:\dev\helworks\helengine-gc\src\platform\gamecube\GameCubeRenderManager2D.cpp`
- Test: `C:\dev\helworks\helengine-gc\builder.tests\GameCubePackagedRuntimeSourceTests.cs`

- [ ] **Step 1: Write the failing source-contract test**

```csharp
[Fact]
public void PackagedRuntimeTextureLoader_WhenTextureIsAlreadyGxRgb5A3_DoesNotRequireRgba32Transcode() {
    string source = File.ReadAllText(GetRuntimeSourcePath("src/platform/gamecube/GameCubeRuntimeTexture.cpp"));

    Assert.Contains("TextureAssetColorFormat::GxRgb5A3", source, StringComparison.Ordinal);
    Assert.DoesNotContain("GameCube runtime textures currently require RGBA32 texture assets.", source, StringComparison.Ordinal);
}
```

- [ ] **Step 2: Run the GameCube source-contract test to verify it fails**

Run:
```powershell
rtk dotnet test C:\dev\helworks\helengine-gc\builder.tests\helengine.gamecube.builder.tests.csproj --filter "FullyQualifiedName~GameCubePackagedRuntimeSourceTests.PackagedRuntimeTextureLoader_WhenTextureIsAlreadyGxRgb5A3_DoesNotRequireRgba32Transcode" -v minimal -p:BuildInParallel=false -p:UseSharedCompilation=false
```

Expected: FAIL because the runtime still insists on RGBA32 for cooked textures.

- [ ] **Step 3: Implement direct load for cooked GxRgb5A3 textures**

```cpp
// C:\dev\helworks\helengine-gc\src\platform\gamecube\GameCubeRuntimeTexture.cpp
void GameCubeRuntimeTexture::LoadFromRaw(TextureAsset* data) {
    if (data == nullptr) {
        throw new ArgumentNullException("data");
    }

    ResetNativeTextureData();

    if (data->ColorFormat == TextureAssetColorFormat::GxRgb5A3) {
        LoadPrepackedRgb5A3(data);
        return;
    } else if (data->ColorFormat == TextureAssetColorFormat::Rgba32) {
        EncodeRgba32ToRgb5A3(data);
        return;
    }

    throw new InvalidOperationException("GameCube runtime textures require either GxRgb5A3 or RGBA32 texture assets.");
}
```

```cpp
void GameCubeRuntimeTexture::LoadPrepackedRgb5A3(TextureAsset* data) {
    if (data->Width == 0 || data->Height == 0) {
        throw new InvalidOperationException("GameCube runtime textures require nonzero dimensions.");
    } else if (data->Colors == nullptr) {
        throw new InvalidOperationException("GameCube runtime textures require prepacked color bytes.");
    }

    const std::size_t nativeByteCount = static_cast<std::size_t>(data->Width) * static_cast<std::size_t>(data->Height) * 2U;
    if (data->Colors->Length != static_cast<int32_t>(nativeByteCount)) {
        throw new InvalidOperationException("GameCube prepacked textures must contain tightly packed RGB5A3 bytes.");
    }

    NativeTextureDataSize = nativeByteCount;
    NativeTextureData = memalign(32, NativeTextureDataSize);
    if (NativeTextureData == nullptr) {
        throw new InvalidOperationException("Could not allocate GameCube texture memory.");
    }

    std::memcpy(NativeTextureData, &(*data->Colors)[0], NativeTextureDataSize);
    DCFlushRange(NativeTextureData, NativeTextureDataSize);
    GX_InitTexObj(&NativeTextureObject, NativeTextureData, data->Width, data->Height, GX_TF_RGB5A3, GX_REPEAT, GX_REPEAT, GX_FALSE);
    GX_InitTexObjFilterMode(&NativeTextureObject, GX_LINEAR, GX_LINEAR);
    NativeTextureObjectInitialized = true;
    this->set_Width(static_cast<int32_t>(data->Width));
    this->set_Height(static_cast<int32_t>(data->Height));
}
```

- [ ] **Step 4: Run the source-contract test to verify it passes**

Run:
```powershell
rtk dotnet test C:\dev\helworks\helengine-gc\builder.tests\helengine.gamecube.builder.tests.csproj --filter "FullyQualifiedName~GameCubePackagedRuntimeSourceTests.PackagedRuntimeTextureLoader_WhenTextureIsAlreadyGxRgb5A3_DoesNotRequireRgba32Transcode" -v minimal -p:BuildInParallel=false -p:UseSharedCompilation=false
```

Expected: PASS.

- [ ] **Step 5: Commit**

```powershell
rtk git -C C:\dev\helworks\helengine-gc add src/platform/gamecube/GameCubeRuntimeTexture.hpp src/platform/gamecube/GameCubeRuntimeTexture.cpp src/platform/gamecube/GameCubeRenderManager2D.cpp builder.tests/GameCubePackagedRuntimeSourceTests.cs
rtk git -C C:\dev\helworks\helengine-gc commit -m "Load cooked GameCube textures directly"
```

## Task 5: Verify the editor-owned GameCube menu build uses the cooked path

**Files:**
- Modify: `C:\dev\helworks\helengine-gc\tools\build_and_launch_editor_gamecube_probe.ps1`
- Modify: `C:\dev\helworks\helengine-gc\builder\GameCubeGeneratedCoreCompatibilityNormalizer.cs`
- Test: `C:\dev\helworks\helengine-gc\builder.tests\GameCubePackagedRuntimeSourceTests.cs`

- [ ] **Step 1: Add one focused runtime/source assertion for the cooked-texture path**

```csharp
[Fact]
public void PackagedRuntimeResolver_WhenMaterialTexturePathIsLoaded_DirectBuildPathStillExists() {
    string source = File.ReadAllText(GetGeneratedCompatibilityNormalizerPath());

    Assert.Contains("BuildTextureFromRaw(textureAsset)", source, StringComparison.Ordinal);
    Assert.Contains("TextureRelativePath", source, StringComparison.Ordinal);
}
```

- [ ] **Step 2: Run the focused GameCube source tests**

Run:
```powershell
rtk dotnet test C:\dev\helworks\helengine-gc\builder.tests\helengine.gamecube.builder.tests.csproj --filter "FullyQualifiedName~GameCubePackagedRuntimeSourceTests" -v minimal -p:BuildInParallel=false -p:UseSharedCompilation=false
```

Expected: PASS after the runtime source contract is updated.

- [ ] **Step 3: Rebuild and launch the editor-owned GameCube probe**

Run:
```powershell
rtk powershell -NoProfile -ExecutionPolicy Bypass -File "C:\dev\helworks\helengine-gc\tools\build_and_launch_editor_gamecube_probe.ps1"
```

Expected:
- editor-owned GameCube build succeeds
- fresh `C:\dev\helprojs\output\gamecube-editor-smoke\game.gcm` is produced
- fresh `C:\dev\helworks\emus\gc-probe\game.gcm` is copied
- Dolphin launches from that exact image

- [ ] **Step 4: Verify the menu load timings improve**

Run in Dolphin and capture:
- `[GC] ContentManager opening asset: dvd:/cooked/scenes/DemoDiscMainMenu.hasset`
- `[GC] ContentManager opening asset: dvd:/cooked/Fonts/DemoDiscBody.hefont`
- `[GC] ContentManager opening asset: dvd:/cooked/imported/...`
- `[GC] Scene load to first draw scene=DemoDiscMainMenu elapsedMs=...`

Expected:
- menu still renders correctly
- the cooked imported logo payload is smaller than the old RGBA32-derived payload
- menu first-draw timing is materially below the current `647 ms` baseline

- [ ] **Step 5: Commit**

```powershell
rtk git -C C:\dev\helworks\helengine-gc add tools/build_and_launch_editor_gamecube_probe.ps1 builder/GameCubeGeneratedCoreCompatibilityNormalizer.cs builder.tests/GameCubePackagedRuntimeSourceTests.cs
rtk git -C C:\dev\helworks\helengine-gc commit -m "Verify cooked GameCube texture pipeline"
```

## Self-Review

- Spec coverage:
  - editor-exposed per-platform GameCube texture format: covered in Tasks 1-2
  - GameCube texture cooking in editor: covered in Task 2
  - GameCube font-atlas cooking path: covered in Task 3
  - runtime direct consumption without RGBA transcode: covered in Task 4
  - end-to-end editor-owned build verification: covered in Task 5
- Placeholder scan:
  - no `TODO`, `TBD`, or “implement later” placeholders remain
- Type consistency:
  - shared enum name is consistently `TextureAssetColorFormat.GxRgb5A3`
  - runtime path consistently uses `LoadPrepackedRgb5A3(...)`
  - verification consistently uses the editor-owned probe launcher
