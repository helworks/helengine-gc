namespace helengine.gamecube.builder.tests;

/// <summary>
/// Guards the packaged-disc native boot source contract in the GameCube host and build files.
/// </summary>
public sealed class GameCubePackagedRuntimeSourceTests {
    /// <summary>
    /// Ensures the Makefile exposes packaged-disc boot and batch-verification defines and the native host switches to packaged scene bootstrap helpers when that define is enabled.
    /// </summary>
    [Fact]
    public void PackagedDiscBootSource_UsesPackagedDefineAndBootstrapHelpers() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string makefileSource = File.ReadAllText(Path.Combine(repositoryRootPath, "Makefile"));
        string applicationSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeApplication.cpp"));
        string applicationHeaderSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeApplication.hpp"));
        string bootstrapHeaderSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeSceneBootstrap.hpp"));
        string bootstrapSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeSceneBootstrap.cpp"));
        string manifestWriterSource = File.ReadAllText(Path.Combine(repositoryRootPath, "builder", "GameCubeRuntimeSceneManifestWriter.cs"));

        Assert.Contains("HELENGINE_GAMECUBE_BOOT_MODE", makefileSource, StringComparison.Ordinal);
        Assert.Contains("HELENGINE_GAMECUBE_PACKAGED_DISC_BOOT", makefileSource, StringComparison.Ordinal);
        Assert.Contains("HELENGINE_GAMECUBE_BATCH_VERIFY_FRAME_LIMIT", makefileSource, StringComparison.Ordinal);
        Assert.Contains("HELENGINE_GAMECUBE_MINIMAL_SAMPLE ?= 0", makefileSource, StringComparison.Ordinal);
        Assert.Contains("GameCubeMinimalSampleMain.cpp", makefileSource, StringComparison.Ordinal);
        Assert.Contains("packaged-disc-assets", makefileSource, StringComparison.Ordinal);
        Assert.Contains("APPLOADER_SOURCE_ROOT := $(THIRD_PARTY_DIR)/cubeboot-tools", makefileSource, StringComparison.Ordinal);
        Assert.Contains("gamecube_runtime_scene_manifest.inl", manifestWriterSource, StringComparison.Ordinal);
        Assert.Contains("static bool InitializePackagedDisc();", bootstrapHeaderSource, StringComparison.Ordinal);
        Assert.Contains("if (!GameCubeSceneBootstrap::InitializePackagedDisc()) {", applicationSource, StringComparison.Ordinal);
        Assert.Contains("DVD_Init();", bootstrapSource, StringComparison.Ordinal);
        Assert.Contains("const s32 mountResult = DVD_Mount();", bootstrapSource, StringComparison.Ordinal);
        Assert.Contains("GameCubeSceneBootstrap::GetPackagedContentRootPath()", applicationSource, StringComparison.Ordinal);
        Assert.Contains("GameCubeSceneBootstrap::CreatePackagedSceneCatalog()", applicationSource, StringComparison.Ordinal);
        Assert.Contains("GameCubeSceneBootstrap::GetPackagedStartupSceneId()", applicationSource, StringComparison.Ordinal);
        Assert.Contains("GX_SetPixelFmt(GX_PF_RGBA6_Z24, GX_ZC_LINEAR);", applicationSource, StringComparison.Ordinal);
        Assert.Contains("SetBootPhase(GameCubeBootPhase::NativeVideo, GXColor { 0xFF, 0x00, 0x00, 0xFF });", applicationSource, StringComparison.Ordinal);
        Assert.Contains("SetBootPhase(GameCubeBootPhase::NativeGraphics, GXColor { 0x00, 0x00, 0xFF, 0xFF });", applicationSource, StringComparison.Ordinal);
        Assert.Contains("VIDEO_ClearFrameBuffer(RenderMode, FrameBuffers[0], COLOR_BLACK);", applicationSource, StringComparison.Ordinal);
        Assert.Contains("VIDEO_ClearFrameBuffer(RenderMode, FrameBuffers[1], COLOR_BLACK);", applicationSource, StringComparison.Ordinal);
        Assert.Contains("GX_CopyDisp(FrameBuffers[0], GX_TRUE);", applicationSource, StringComparison.Ordinal);
        Assert.DoesNotContain("RunPackagedStartupProbe()", applicationSource, StringComparison.Ordinal);
        Assert.DoesNotContain("int RunPackagedStartupProbe();", applicationHeaderSource, StringComparison.Ordinal);
        Assert.Contains("HasSatisfiedVerificationExitCondition()", applicationSource, StringComparison.Ordinal);
        Assert.DoesNotContain("PresentFailureLoop();", applicationSource, StringComparison.Ordinal);
        Assert.DoesNotContain("void PresentFailureLoop();", applicationHeaderSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures the packaged-disc runtime source keeps narrow diagnostics around startup-scene queueing, first-frame execution, and DVD-backed content reads.
    /// </summary>
    [Fact]
    public void PackagedDiscBootSource_EmitsStartupAndFirstFrameDiagnostics() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string applicationSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeApplication.cpp"));
        string bootstrapSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeSceneBootstrap.cpp"));
        string rasterRendererSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRasterRenderer.cpp"));
        string sceneManagerSource = File.ReadAllText(Path.Combine(repositoryRootPath, "tmp", "generated-core-gamecube", "SceneManager.cpp"));
        string contentManagerSource = File.ReadAllText(Path.Combine(repositoryRootPath, "tmp", "generated-core-gamecube", "ContentManager.cpp"));
        string fileSource = File.ReadAllText(Path.Combine(repositoryRootPath, "tmp", "generated-input-gamecube", "system", "io", "file.cpp"));

        Assert.Contains("[GC] Packaged content root:", applicationSource, StringComparison.Ordinal);
        Assert.Contains("[GC] Packaged startup scene id:", applicationSource, StringComparison.Ordinal);
        Assert.Contains("[GC] First update begin.", applicationSource, StringComparison.Ordinal);
        Assert.Contains("[GC] First update completed.", applicationSource, StringComparison.Ordinal);
        Assert.Contains("[GC] First draw begin.", applicationSource, StringComparison.Ordinal);
        Assert.Contains("[GC] First draw completed.", applicationSource, StringComparison.Ordinal);
        Assert.Contains("!EngineRenderManager3D->HasRenderedScene()", applicationSource, StringComparison.Ordinal);
        Assert.Contains("EngineRenderManager3D != nullptr && EngineRenderManager3D->HasRenderedScene()", applicationSource, StringComparison.Ordinal);
        Assert.Contains("GXColor { 0xFF, 0x80, 0x00, 0xFF }", applicationSource, StringComparison.Ordinal);
        Assert.Contains("GXColor { 0x80, 0x40, 0x00, 0xFF }", applicationSource, StringComparison.Ordinal);
        Assert.Contains("GXColor { 0xFF, 0x00, 0xFF, 0xFF }", applicationSource, StringComparison.Ordinal);
        Assert.Contains("GXColor { 0x80, 0x00, 0x80, 0xFF }", applicationSource, StringComparison.Ordinal);
        Assert.Contains("(PresentedFrameCount & 1U) == 0U", applicationSource, StringComparison.Ordinal);
        Assert.True(
            applicationSource.IndexOf("FrameBufferIndex ^= 1U;", StringComparison.Ordinal) < applicationSource.IndexOf("GX_CopyDisp(FrameBuffers[FrameBufferIndex], GX_TRUE);", StringComparison.Ordinal),
            "PresentFrame should flip the XFB index before copying the EFB into the next framebuffer.");
        Assert.True(
            applicationSource.IndexOf("GX_CopyDisp(FrameBuffers[FrameBufferIndex], GX_TRUE);", StringComparison.Ordinal) < applicationSource.IndexOf("GX_DrawDone();", StringComparison.Ordinal),
            "PresentFrame should match the working Octave copy-before-drawdone sequence.");
        Assert.Contains("FrameBuffers[0] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(RenderMode));", applicationSource, StringComparison.Ordinal);
        Assert.Contains("FrameBuffers[1] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(RenderMode));", applicationSource, StringComparison.Ordinal);
        Assert.Contains("GX_SetCopyClear(visibleColor, 0x00FFFFFF);", applicationSource, StringComparison.Ordinal);
        Assert.Contains("PresentedFrameCount++;", applicationSource, StringComparison.Ordinal);
        Assert.Contains("GX_CopyDisp(FrameBuffers[FrameBufferIndex], GX_TRUE);", applicationSource, StringComparison.Ordinal);
        Assert.Contains("GX_CopyDisp(FrameBuffers[FrameBufferIndex], GX_TRUE);\n        GX_DrawDone();", applicationSource, StringComparison.Ordinal);
        Assert.Contains("GX_CopyDisp(FrameBuffers[FrameBufferIndex], GX_TRUE);", applicationSource, StringComparison.Ordinal);
        Assert.Contains("VIDEO_SetNextFramebuffer(FrameBuffers[FrameBufferIndex]);", applicationSource, StringComparison.Ordinal);
        Assert.Contains("FrameBufferIndex ^= 1U;", applicationSource, StringComparison.Ordinal);
        Assert.Contains("GX_SetColorUpdate(GX_TRUE);", applicationSource, StringComparison.Ordinal);
        Assert.Contains("GX_SetAlphaUpdate(GX_TRUE);", applicationSource, StringComparison.Ordinal);
        Assert.DoesNotContain("[GC] Packaged startup probe begin.", applicationSource, StringComparison.Ordinal);
        Assert.DoesNotContain("[GC] Packaged startup probe completed.", applicationSource, StringComparison.Ordinal);
        Assert.Contains("[GC] Packaged manifest entry count:", bootstrapSource, StringComparison.Ordinal);
        Assert.Contains("[GC] Packaged manifest entry[%u] scene=%s path=%s", bootstrapSource, StringComparison.Ordinal);
        Assert.Contains("GXColor { 0x64, 0x95, 0xED, 0xFF }", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("guOrtho(", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("GX_LoadProjectionMtx(projection, GX_ORTHOGRAPHIC);", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("GX_Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("GX_Position3f32(0.0f, 0.0f, 0.0f);", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("GX_Color4u8(0x64, 0x95, 0xED, 0xFF);", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("GX_SetCullMode(GX_CULL_NONE);", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("GX_SetCullMode(GX_CULL_FRONT);", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("RasterizedFrameCount++;\n        DrawProbeFullscreenClear(framePlan);\n        return true;", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("[GC] SceneManager loading scene id=", sceneManagerSource, StringComparison.Ordinal);
        Assert.Contains("[GC] SceneManager content loaded for scene id=", sceneManagerSource, StringComparison.Ordinal);
        Assert.Contains("[GC] ContentManager opening asset:", contentManagerSource, StringComparison.Ordinal);
        Assert.Contains("[GC] File::Exists path=", fileSource, StringComparison.Ordinal);
        Assert.Contains("[GC] File::OpenRead path=", fileSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures the present-ownership diagnostic is driven by the current raster frame result instead of a sticky lifetime flag.
    /// </summary>
    [Fact]
    public void PackagedDiscBootSource_TracksRenderedScenePerFrame() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string renderManagerSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRenderManager3D.cpp"));
        string rasterRendererHeaderSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRasterRenderer.hpp"));

        Assert.Contains("bool DrawFrame(GameCubeFramePlan* framePlan);", rasterRendererHeaderSource, StringComparison.Ordinal);
        Assert.Contains("void DrawProbeFullscreenClear(GameCubeFramePlan* framePlan);", rasterRendererHeaderSource, StringComparison.Ordinal);
        Assert.Contains("HasRenderedSceneValue = RasterRenderer->DrawFrame(framePlan);", renderManagerSource, StringComparison.Ordinal);
        Assert.DoesNotContain("HasRenderedSceneValue = true;", renderManagerSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures packaged DVD-backed content reads no longer rely on raw host file APIs for <c>dvd:/</c> paths.
    /// </summary>
    [Fact]
    public void PackagedDiscBootSource_UsesGameCubeDiscReadBridgeForDvdPaths() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string makefileSource = File.ReadAllText(Path.Combine(repositoryRootPath, "Makefile"));
        string fileSource = File.ReadAllText(Path.Combine(repositoryRootPath, "tmp", "generated-input-gamecube", "system", "io", "file.cpp"));
        string fileStreamHeaderSource = File.ReadAllText(Path.Combine(repositoryRootPath, "tmp", "generated-core-gamecube", "system", "io", "file-stream.hpp"));
        string discFileSystemSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeDiscFileSystem.cpp"));

        Assert.Contains("GameCubeDiscFileSystem.cpp", makefileSource, StringComparison.Ordinal);
        Assert.Contains("#include \"platform/gamecube/GameCubeDiscFileSystem.hpp\"", fileSource, StringComparison.Ordinal);
        Assert.Contains("GameCubeDiscFileSystem::CanHandlePath(fileName)", fileSource, StringComparison.Ordinal);
        Assert.Contains("GameCubeDiscFileSystem::Exists(fileName)", fileSource, StringComparison.Ordinal);
        Assert.Contains("GameCubeDiscFileSystem::CanHandlePath(filePath)", fileSource, StringComparison.Ordinal);
        Assert.Contains("GameCubeDiscFileSystem::OpenRead(filePath)", fileSource, StringComparison.Ordinal);
        Assert.Contains("FileStream(uint8_t* memoryBuffer, size_t length);", fileStreamHeaderSource, StringComparison.Ordinal);
        Assert.Contains("bool ReadDiscRange(void* destination, std::size_t offset, std::size_t length)", discFileSystemSource, StringComparison.Ordinal);
        Assert.Contains("__io_gcdvd.readSectors(static_cast<sec_t>(sectorIndex), 1, sectorBuffer)", discFileSystemSource, StringComparison.Ordinal);
        Assert.Contains("ReadDiscRange(buffer, discOffset, fileSize)", discFileSystemSource, StringComparison.Ordinal);
        Assert.Contains("const uint32_t fstOffset = ReadBigEndianU32(discHeader + 0x424) << 2;", discFileSystemSource, StringComparison.Ordinal);
        Assert.Contains("const uint32_t fstSize = ReadBigEndianU32(discHeader + 0x428) << 2;", discFileSystemSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures packaged builds run the generated engine update and draw loop instead of staying in the temporary host-only liveness probe.
    /// </summary>
    [Fact]
    public void PackagedDiscBootSource_RunsGeneratedEngineLoop() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string applicationSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeApplication.cpp"));

        Assert.Contains("while (true) {", applicationSource, StringComparison.Ordinal);
        Assert.Contains("if (!UpdateEngineCore()) {", applicationSource, StringComparison.Ordinal);
        Assert.Contains("if (!DrawEngineCore()) {", applicationSource, StringComparison.Ordinal);
        Assert.Contains("PresentFrame();", applicationSource, StringComparison.Ordinal);
        Assert.Contains("if (HasSatisfiedVerificationExitCondition()) {", applicationSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures packaged verification builds return success once the configured rendered-frame requirement has been satisfied.
    /// </summary>
    [Fact]
    public void PackagedDiscBootSource_ReturnsSuccessWhenVerificationCompletes() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string applicationSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeApplication.cpp"));

        Assert.Contains("if (HasSatisfiedVerificationExitCondition()) {", applicationSource, StringComparison.Ordinal);
        Assert.Contains("return 0;", applicationSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures the generated core still initializes and queues the packaged startup scene before the steady-state runtime loop begins.
    /// </summary>
    [Fact]
    public void PackagedDiscBootSource_InitializesCoreAndQueuesStartupSceneBeforeLoop() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string applicationSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeApplication.cpp"));

        Assert.Contains("EngineCore->Initialize(EngineRenderManager3D, EngineRenderManager2D, EngineInputManager, options);", applicationSource, StringComparison.Ordinal);
        Assert.Contains("EngineCore->get_SceneManager()->LoadScene(packagedStartupSceneId, SceneLoadMode::Single);", applicationSource, StringComparison.Ordinal);
    }
}
