# GameCube Text Outline and Shadow Design

## Goal

Render authored text shadows and outlines on GameCube with the same pass order and property semantics used by the engine's shared text-effect builder.

## Rendering Behavior

For each captured GameCube text draw command, the renderer will reuse the command's existing cached glyph layout for every pass. It will mirror the pass contract of `TextRenderEffectPassBuilder` directly in native code, because invoking the generated builder would allocate a managed list for every text element every frame.

The renderer will submit passes in this order:

1. The optional shadow at `ShadowOffset` using `ShadowColor`.
2. The optional outline at negative and positive horizontal and vertical `OutlineScale` offsets using `OutlineColor`.
3. The primary glyph pass at zero offset using the text color.

The shadow is omitted only when its offset is zero. The outline is omitted only when `OutlineScale` is not positive. The primary pass is always submitted, matching the shared builder even when an authored effect color is transparent.

## Integration

`GameCubeRasterRenderer::RenderText2D` remains responsible for clipping, font-atlas binding, alignment, and cached layout resolution. It will add one focused helper that draws an already-resolved layout at one offset and color. The helper will not allocate memory and will preserve the existing GX textured 2D pipeline.

`TextRenderEffectPassBuilder` remains the reference definition of pass ordering and authored effect decisions. GameCube mirrors that compact fixed pass sequence locally to keep the native frame path allocation-free; its source-contract test locks the two contracts together.

## Validation

A source-contract test will require the GameCube renderer to consume the authored shadow and outline properties, render effect passes before the primary pass, and preserve cached text-layout rendering. The targeted GameCube builder test must fail before the renderer change and pass afterward. A packaged GameCube build will verify the native C++ path compiles.
