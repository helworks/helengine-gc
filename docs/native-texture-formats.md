# GameCube GX texture formats

Texture and font-atlas platform settings publish 20 native selections:

| Encoding | Identifier | GX sampler | Native tile |
| --- | --- | --- | --- |
| Intensity | `Gx.I4` | 0 | 8x8, 32 bytes |
| Intensity | `Gx.I8` | 1 | 8x4, 32 bytes |
| Intensity and alpha | `Gx.IA4` | 2 | 8x4, 32 bytes |
| Intensity and alpha | `Gx.IA8` | 3 | 4x4, 32 bytes |
| Color | `Gx.RGB565` | 4 | 4x4, 32 bytes |
| Color and alpha | `Gx.RGB5A3` | 5 | 4x4, 32 bytes |
| Color and alpha | `Gx.RGBA8` | 6 | 4x4, 64 bytes |
| Indexed | `Gx.CI4.TLUT.*` | 8 | 8x8, 32 bytes |
| Indexed | `Gx.CI8.TLUT.*` | 9 | 8x4, 32 bytes |
| Indexed | `Gx.CI14X2.TLUT.*` | 10 | 4x4, 32 bytes |
| Compressed color | `Gx.CMPR` | 14 | 8x8, 32 bytes |
| Raw depth view | `Gx.Z8` | I8 (1) | 8x4, 32 bytes |
| Raw depth view | `Gx.Z16` | IA8 (3) | 4x4, 32 bytes |
| Raw depth view | `Gx.Z24X8` | RGBA8 (6) | 4x4, 64 bytes |

Each indexed encoding has `IA8`, `RGB565`, and `RGB5A3` TLUT selections.
The existing `GxRgb5A3` setting and serialized tag 4 retain their byte layout,
rounding, alpha behavior and 512-pixel/A8 defaults. New selections use
`TextureAssetColorFormat.GxNative` (17); existing format values are unchanged.

Native RGB5A3 selections expose Opaque, Binary, and A3 alpha. A3 has stable
enum value 5 and denotes the actual eight alpha levels stored by RGB5A3.
Generic texture and atlas readers reject A3 outside GxNative storage.
Intensity formats couple sampled alpha to intensity; I4 uses A4 and I8 uses
A8 without claiming an independent alpha plane. CMPR supports opaque or binary
coverage and uses GX interpolation, big-endian endpoints and GX selector order.

## Cooking, storage and palettes

Cooking first decodes into private RGBA storage, then applies the authored
maximum resolution. Zero disables resizing; resulting dimensions must fit
within 1..1024 on each axis. Logical dimensions remain unchanged by native
tile padding. Edge tiles repeat the final source texels, and GX receives the
logical dimensions for normalized sampling.

Indexed cooking uses the quantizer selected in platform settings. CI4 cooks
up to 16 colors; the current shared quantizer cooks up to 256 colors for CI8
and CI14X2. This is a valid subset of CI14X2. The engine's explicit
`GxNativeTextureCodec.EncodeIndexed` API accepts ushort indices and RGBA
palettes up to 16384 entries, preserving indices above 255. Runtime loading
supports the complete fourteen-bit range, including index 16383.

Native palettes are contiguous big-endian 16-bit entries, padded to a multiple
of sixteen entries. CI14X2 words are big endian with their upper two bits
clear. The parser validates every stored index, including padded edge tiles.
The editor's memory estimate rounds complete tiles and budgets the maximum
palette for the selected format, excluding the CPU descriptor. Actual runtime
accounting uses the stored palette size.

GXN1 stores twelve little-endian uint32 words in `TextureAsset.Colors` before
the tiled GPU bytes:

| Byte offset | Value |
| --- | --- |
| 0 | `GXN1` magic, 0x314E5847 |
| 4 | Version 1 |
| 8 | Native format code, including depth aliases 17/19/22 |
| 12 / 16 | Authored width / height |
| 20 | Texel payload byte count, excluding the descriptor |
| 24 | TLUT format 0/1/2, or 0xFFFFFFFF for direct storage |
| 28 / 32 | Palette entry count / byte count |
| 36 | Alpha policy matching the asset metadata |
| 40 / 44 | Flags / reserved, both zero |

`PaletteColors` stores the separate native TLUT. Outer HELE texture or font
metadata may be little or big endian; neither changes GXN1 or GPU byte order.
Invalid formats, lengths, alpha combinations, indices and reserved bits fail
before upload.

## Runtime and resource contract

The texture owner uploads aligned native tiles without RGBA expansion.
Indexed binds load the native TLUT before their GX texture object. CI14X2 uses
large TLUT regions and preserves the eleven-bit load count required for a
complete 16384-entry palette instead of relying on libogc's narrower region
initializer. GPU resource disposal follows renderer synchronization.

RGBA region updates preserve legacy RGB5A3 behavior and update supported
uncompressed native color storage in place. Indexed, compressed and raw depth
views reject RGBA region updates before mutation.

Depth views encode raw source R, R/G, or R/G/B data into Z8, Z16, or Z24X8
storage. Sampling uses the corresponding I8, IA8, or RGBA8 interpretation;
it does not enable GX Z-texture depth writes, create render targets or add
shadow-comparison sampling. EFB copy-only modes are separate operations and
are not advertised as additional texture samplers.

## References and validation

Hardware constants and SDK contracts:
[libogc GX API](https://github.com/devkitPro/libogc/blob/master/gc/ogc/gx.h)
and [libogc GX implementation](https://github.com/devkitPro/libogc/blob/master/libogc/gx.c).
Tile, palette, CMPR and copy/sampler semantics are checked against the local
Dolphin reference under `C:/dev/helworks/reference/gc/dolphin`, especially
[TextureDecoder.h](https://github.com/dolphin-emu/dolphin/blob/master/Source/Core/VideoCommon/TextureDecoder.h)
and [TextureDecoder_Generic.cpp](https://github.com/dolphin-emu/dolphin/blob/master/Source/Core/VideoCommon/TextureDecoder_Generic.cpp).

Artifacts remain in `C:/dev/helworks/builds/helengine-gc/texture-formats`.
Managed tests, native SDK-boundary tests, serialized payload compatibility,
C++ generation and SDK cross compilation do not establish visual correctness
on hardware or in Dolphin. This change does not include emulator execution.
