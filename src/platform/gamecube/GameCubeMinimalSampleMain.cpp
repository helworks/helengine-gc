#include <gccore.h>
#include <ogc/system.h>

namespace {
    constexpr u32 DefaultFifoSize = 256 * 1024;
    constexpr GXColor StableClearColor = GXColor { 0xC0, 0x10, 0x10, 0xFF };

    alignas(32) u8 FifoBuffer[DefaultFifoSize];
}

int main() {
    VIDEO_Init();

    GXRModeObj* renderMode = VIDEO_GetPreferredMode(nullptr);
    if (renderMode == nullptr) {
        return 1;
    }

    void* frameBuffers[2] = {
        MEM_K0_TO_K1(SYS_AllocateFramebuffer(renderMode)),
        MEM_K0_TO_K1(SYS_AllocateFramebuffer(renderMode))
    };
    if (frameBuffers[0] == nullptr || frameBuffers[1] == nullptr) {
        return 1;
    }

    VIDEO_ClearFrameBuffer(renderMode, frameBuffers[0], COLOR_BLACK);
    VIDEO_ClearFrameBuffer(renderMode, frameBuffers[1], COLOR_BLACK);
    VIDEO_Configure(renderMode);
    VIDEO_SetNextFramebuffer(frameBuffers[0]);
    VIDEO_SetBlack(FALSE);
    VIDEO_Flush();
    VIDEO_WaitVSync();
    if (renderMode->viTVMode & VI_NON_INTERLACE) {
        VIDEO_WaitVSync();
    }

    GX_Init(FifoBuffer, DefaultFifoSize);

    const f32 yScale = GX_GetYScaleFactor(renderMode->efbHeight, renderMode->xfbHeight);
    const u16 xfbHeight = GX_SetDispCopyYScale(yScale);
    GX_SetViewport(0.0F, 0.0F, static_cast<f32>(renderMode->fbWidth), static_cast<f32>(renderMode->efbHeight), 0.0F, 1.0F);
    GX_SetScissor(0, 0, renderMode->fbWidth, renderMode->efbHeight);
    GX_SetDispCopySrc(0, 0, renderMode->fbWidth, renderMode->efbHeight);
    GX_SetDispCopyDst(renderMode->fbWidth, xfbHeight);
    GX_SetCopyFilter(renderMode->aa, renderMode->sample_pattern, GX_TRUE, renderMode->vfilter);
    GX_SetFieldMode(renderMode->field_rendering, ((renderMode->viHeight == (renderMode->xfbHeight * 2)) ? GX_ENABLE : GX_DISABLE));
    GX_SetPixelFmt(GX_PF_RGBA6_Z24, GX_ZC_LINEAR);
    GX_SetCullMode(GX_CULL_NONE);
    GX_SetDispCopyGamma(GX_GM_1_0);
    GX_SetNumChans(1);
    GX_SetNumTexGens(0);
    GX_SetZMode(GX_FALSE, GX_LEQUAL, GX_FALSE);
    GX_SetColorUpdate(GX_TRUE);
    GX_SetAlphaUpdate(GX_TRUE);
    GX_InvVtxCache();
    GX_InvalidateTexAll();
    GX_SetCopyClear(StableClearColor, 0x00FFFFFF);
    GX_CopyDisp(frameBuffers[0], GX_TRUE);
    GX_CopyDisp(frameBuffers[0], GX_TRUE);

    u32 frameBufferIndex = 0U;
    while (true) {
        GX_SetCopyClear(StableClearColor, 0x00FFFFFF);
        frameBufferIndex ^= 1U;
        GX_CopyDisp(frameBuffers[frameBufferIndex], GX_TRUE);
        GX_DrawDone();
        VIDEO_SetNextFramebuffer(frameBuffers[frameBufferIndex]);
        VIDEO_Flush();
        VIDEO_WaitVSync();
    }
}
