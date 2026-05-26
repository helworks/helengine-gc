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
        Assert.Contains("GENERATED_CORE_TRANSLATION_UNIT := helengine_core_amalgamated.cpp", makefileSource, StringComparison.Ordinal);
        Assert.Contains("GENERATED_CORE_TRANSLATION_UNIT := helengine_core_unity.cpp", makefileSource, StringComparison.Ordinal);
        Assert.Contains("does not contain helengine_core_amalgamated.cpp or helengine_core_unity.cpp", makefileSource, StringComparison.Ordinal);
        Assert.Contains("$(BUILD_DIR)/generated/$(GENERATED_CORE_TRANSLATION_UNIT:.cpp=.o): $(GENERATED_CORE_SOURCE)", makefileSource, StringComparison.Ordinal);
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
        string renderManagerSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRenderManager3D.cpp"));
        string renderManager2DSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRenderManager2D.cpp"));
        string rasterRendererSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRasterRenderer.cpp"));
        string sceneManagerSource = File.ReadAllText(Path.Combine(repositoryRootPath, "tmp", "generated-core-gamecube", "SceneManager.cpp"));
        string contentManagerSource = File.ReadAllText(Path.Combine(repositoryRootPath, "tmp", "generated-core-gamecube", "ContentManager.cpp"));
        string platformMenuSceneResolverSource = File.ReadAllText(Path.Combine(repositoryRootPath, "tmp", "generated-core-gamecube", "PlatformMenuSceneResolver.cpp"));
        string fontAssetBinarySerializerSource = File.ReadAllText(Path.Combine(repositoryRootPath, "tmp", "generated-core-gamecube", "FontAssetBinarySerializer.cpp"));
        string fileSource = File.ReadAllText(Path.Combine(repositoryRootPath, "tmp", "generated-input-gamecube", "system", "io", "file.cpp"));
        string generatedCoreNormalizerSource = File.ReadAllText(Path.Combine(repositoryRootPath, "builder", "GameCubeGeneratedCoreCompatibilityNormalizer.cs"));

        Assert.Contains("[GC] Packaged content root:", applicationSource, StringComparison.Ordinal);
        Assert.Contains("[GC] Packaged startup scene id:", applicationSource, StringComparison.Ordinal);
        Assert.Contains("[GC] Runtime build stamp:", applicationSource, StringComparison.Ordinal);
        Assert.Contains("[GC] First update begin.", applicationSource, StringComparison.Ordinal);
        Assert.Contains("[GC] First update completed.", applicationSource, StringComparison.Ordinal);
        Assert.Contains("[GC] Engine update threw Exception*:", applicationSource, StringComparison.Ordinal);
        Assert.Contains("[GC] Engine update threw std::exception:", applicationSource, StringComparison.Ordinal);
        Assert.Contains("[GC] First draw begin.", applicationSource, StringComparison.Ordinal);
        Assert.Contains("[GC] First draw completed.", applicationSource, StringComparison.Ordinal);
        Assert.Contains("[GC] Scene load to first draw scene=%s elapsedMs=%.3f", applicationSource, StringComparison.Ordinal);
        Assert.Contains("!EngineRenderManager3D->HasRenderedScene()", applicationSource, StringComparison.Ordinal);
        Assert.Contains("EngineRenderManager3D != nullptr && EngineRenderManager3D->HasRenderedScene()", applicationSource, StringComparison.Ordinal);
        Assert.Contains("GXColor { 0xFF, 0x80, 0x00, 0xFF }", applicationSource, StringComparison.Ordinal);
        Assert.Contains("GXColor { 0x00, 0x60, 0x00, 0xFF }", applicationSource, StringComparison.Ordinal);
        Assert.Contains("GXColor { 0xFF, 0x00, 0xFF, 0xFF }", applicationSource, StringComparison.Ordinal);
        Assert.Contains("GXColor { 0x80, 0x00, 0x80, 0xFF }", applicationSource, StringComparison.Ordinal);
        Assert.Contains(
            "FrameBufferIndex ^= 1U;\n        GX_CopyDisp(FrameBuffers[FrameBufferIndex], GX_TRUE);\n        GX_DrawDone();",
            applicationSource,
            StringComparison.Ordinal);
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
        Assert.Contains("GX_SetAlphaUpdate(GX_FALSE);", rasterRendererSource, StringComparison.Ordinal);
        Assert.DoesNotContain("[GC] Packaged startup probe begin.", applicationSource, StringComparison.Ordinal);
        Assert.DoesNotContain("[GC] Packaged startup probe completed.", applicationSource, StringComparison.Ordinal);
        Assert.Contains("[GC] Packaged manifest entry count:", bootstrapSource, StringComparison.Ordinal);
        Assert.Contains("[GC] Packaged manifest entry[%u] scene=%s path=%s", bootstrapSource, StringComparison.Ordinal);
        Assert.Contains("GXColor { 0x64, 0x95, 0xED, 0xFF }", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("guPerspective(", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("GX_LoadProjectionMtx(projectionMatrix, GX_PERSPECTIVE);", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("EvaluateLitVertexColor(", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("submission->get_Material()", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("GX_SetCullMode(GX_CULL_FRONT);", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("ResolveGxCullMode(", rasterRendererSource, StringComparison.Ordinal);
        Assert.DoesNotContain("[GC] 2D queue snapshot", applicationSource, StringComparison.Ordinal);
        Assert.DoesNotContain("[GC] Frame %u extracted.", renderManagerSource, StringComparison.Ordinal);
        Assert.Contains("RecordTraceState(\"LoadSceneImmediateBeforeContentLoad\"", sceneManagerSource, StringComparison.Ordinal);
        Assert.Contains("RecordTraceState(\"LoadSceneImmediateBeforeSceneLoadServiceLoad\"", sceneManagerSource, StringComparison.Ordinal);
        Assert.Contains("GameCubeRecordSceneLoadRequest(sceneId.c_str());", sceneManagerSource, StringComparison.Ordinal);
        Assert.Contains("[GC] ContentManager opening asset:", contentManagerSource, StringComparison.Ordinal);
        Assert.Contains("std::string PlatformMenuSceneResolver::DesktopMainMenuSceneId = \"Scenes/DemoDiscMainMenu.helen\";", platformMenuSceneResolverSource, StringComparison.Ordinal);
        Assert.Contains("uint8_t FontAssetBinarySerializer::CurrentVersion = 5;", fontAssetBinarySerializerSource, StringComparison.Ordinal);
        Assert.Contains("uint8_t FontAssetBinarySerializer::ExternalCookedAtlasPathVersion = 5;", fontAssetBinarySerializerSource, StringComparison.Ordinal);
        Assert.Contains("FontAssetBinarySerializer::set_LastDeserializeStage(\"ReadCookedAtlasTexturePath\");", fontAssetBinarySerializerSource, StringComparison.Ordinal);
        Assert.Contains("if (sourceTexture->Width > 0 && sourceTexture->Height > 0 && sourceTexture->Colors != nullptr && sourceTexture->Colors->get_Length() > 0)", fontAssetBinarySerializerSource, StringComparison.Ordinal);
        Assert.Contains("[GC] File::Exists path=", fileSource, StringComparison.Ordinal);
        Assert.Contains("[GC] File::OpenRead path=", fileSource, StringComparison.Ordinal);
        Assert.Contains("delete sourceTextureAsset;", renderManager2DSource, StringComparison.Ordinal);
        Assert.Contains("delete static_cast<GameCubeRuntimeTexture*>(texture);", renderManager2DSource, StringComparison.Ordinal);
        Assert.Contains("delete static_cast<GameCubeRuntimeMaterial*>(material);", renderManagerSource, StringComparison.Ordinal);
        Assert.Contains("delete runtimeModel;", renderManagerSource, StringComparison.Ordinal);
        Assert.Contains("delete textureAsset->Colors;", generatedCoreNormalizerSource, StringComparison.Ordinal);
        Assert.Contains("NormalizeGeneratedCoreFile(generatedCoreRootPath, \"SceneManager.cpp\");", generatedCoreNormalizerSource, StringComparison.Ordinal);
        Assert.Contains("NormalizeGeneratedCoreFile(generatedCoreRootPath, \"PlatformMenuSceneResolver.cpp\");", generatedCoreNormalizerSource, StringComparison.Ordinal);
        Assert.Contains("NormalizePlatformMenuSceneResolverSource", generatedCoreNormalizerSource, StringComparison.Ordinal);
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
        string applicationSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeApplication.cpp"));
        string fileSource = File.ReadAllText(Path.Combine(repositoryRootPath, "tmp", "generated-input-gamecube", "system", "io", "file.cpp"));
        string fileStreamHeaderSource = File.ReadAllText(Path.Combine(repositoryRootPath, "tmp", "generated-core-gamecube", "system", "io", "file-stream.hpp"));
        string discFileSystemSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeDiscFileSystem.cpp"));

        Assert.Contains("GameCubeDiscFileSystem.cpp", makefileSource, StringComparison.Ordinal);
        Assert.Contains("#include \"platform/gamecube/GameCubeDiscFileSystem.hpp\"", fileSource, StringComparison.Ordinal);
        Assert.Contains("GameCubeDiscFileSystem::CanHandlePath(fileName)", fileSource, StringComparison.Ordinal);
        Assert.Contains("GameCubeDiscFileSystem::Exists(fileName)", fileSource, StringComparison.Ordinal);
        Assert.Contains("GameCubeDiscFileSystem::CanHandlePath(filePath)", fileSource, StringComparison.Ordinal);
        Assert.Contains("GameCubeDiscFileSystem::OpenRead(filePath)", fileSource, StringComparison.Ordinal);
        Assert.Contains("FileStream(const uint8_t* data, size_t length);", fileStreamHeaderSource, StringComparison.Ordinal);
        Assert.Contains("bool ReadDiscRange(void* destination, std::size_t offset, std::size_t length)", discFileSystemSource, StringComparison.Ordinal);
        Assert.Contains("__io_gcdvd.readSectors(static_cast<sec_t>(sectorIndex), 1, sectorBuffer)", discFileSystemSource, StringComparison.Ordinal);
        Assert.Contains("ReadDiscRange(buffer, discOffset, fileSize)", discFileSystemSource, StringComparison.Ordinal);
        Assert.Contains("const uint32_t fstOffset = ReadBigEndianU32(discHeader + 0x424);", discFileSystemSource, StringComparison.Ordinal);
        Assert.Contains("const uint32_t fstSize = ReadBigEndianU32(discHeader + 0x428);", discFileSystemSource, StringComparison.Ordinal);
        Assert.Contains("const char expectedAssetName[] = \"DemoDiscMainMenu.hasset\";", applicationSource, StringComparison.Ordinal);
        Assert.Contains("const std::string expectedPath = \"dvd:/cooked/scenes/DemoDiscMainMenu.hasset\";", applicationSource, StringComparison.Ordinal);
        Assert.Contains("foundDemoDiscMainMenu", applicationSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures the menu bring-up path has an explicit city-content staging helper for the authored menu scene and required fonts.
    /// </summary>
    [Fact]
    public void PackagedDiscBootSource_HasExplicitMenuStagingScript() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string stagingScriptPath = Path.Combine(repositoryRootPath, "tools", "stage_city_demo_disc_main_menu_content.sh");

        Assert.True(File.Exists(stagingScriptPath));

        string stagingScriptSource = File.ReadAllText(stagingScriptPath);
        Assert.Contains("DemoDiscMainMenu.hasset", stagingScriptSource, StringComparison.Ordinal);
        Assert.Contains("cp \"${CITY_COOKED_ROOT}/scenes/rendering/\"*.hasset", stagingScriptSource, StringComparison.Ordinal);
        Assert.Contains("DemoDiscBody.hefont", stagingScriptSource, StringComparison.Ordinal);
        Assert.Contains("default.hefont", stagingScriptSource, StringComparison.Ordinal);
        Assert.Contains("cp -R \"${CITY_COOKED_ROOT}/materials/.\"", stagingScriptSource, StringComparison.Ordinal);
        Assert.Contains("cp -R \"${CITY_COOKED_ROOT}/imported/.\"", stagingScriptSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures the retail-check harness packages the authored demo-disc target scene catalog instead of a one-scene startup-only manifest.
    /// </summary>
    [Fact]
    public void PackagedDiscBootSource_RetailHarnessIncludesPlayableDemoDiscSceneCatalog() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string retailHarnessSource = File.ReadAllText(Path.Combine(repositoryRootPath, "tmp", "builder-retail-check", "Program.cs"));

        Assert.Contains("const string CubeTestSceneId = \"cube_test\";", retailHarnessSource, StringComparison.Ordinal);
        Assert.Contains("const string StartupSceneAliasId = \"DemoDiscMainMenu\";", retailHarnessSource, StringComparison.Ordinal);
        Assert.Contains("const string ColoredCubeGridSceneId = \"colored_cube_grid\";", retailHarnessSource, StringComparison.Ordinal);
        Assert.Contains("const string TexturedCubeGridSceneId = \"textured_cube_grid\";", retailHarnessSource, StringComparison.Ordinal);
        Assert.Contains("const string AxisTestSceneId = \"axis_test\";", retailHarnessSource, StringComparison.Ordinal);
        Assert.Contains("const string AxisTest2SceneId = \"axis_test2\";", retailHarnessSource, StringComparison.Ordinal);
        Assert.Contains("const string DirectionalShadowPlazaSceneId = \"directional_shadow_plaza\";", retailHarnessSource, StringComparison.Ordinal);
        Assert.Contains("const string SpotlightStreetSliceSceneId = \"spotlight_street_slice\";", retailHarnessSource, StringComparison.Ordinal);
        Assert.Contains("new PlatformBuildScene(", retailHarnessSource, StringComparison.Ordinal);
        Assert.Contains("Demo Disc Main Menu Alias", retailHarnessSource, StringComparison.Ordinal);
        Assert.Contains("List<string> cookedSceneAssetPaths = new(manifest.Scenes.Length);", retailHarnessSource, StringComparison.Ordinal);
        Assert.Contains("DiscoverAutomaticRuntimeComponentTypesFromCookedScenes(cookedSceneAssetPaths, scriptTypeResolver)", retailHarnessSource, StringComparison.Ordinal);
        Assert.DoesNotContain("IReadOnlyList<string> cookedSceneAssetPaths = [ cookedSceneAssetPath ];", retailHarnessSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures the retail-check harness rewrites staged menu-scene material payloads into the GameCube cooked material contract before packaging.
    /// </summary>
    [Fact]
    public void PackagedDiscBootSource_RetailHarnessRecooksGeneratedAndAuthoredMaterialsForGameCube() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string retailHarnessSource = File.ReadAllText(Path.Combine(repositoryRootPath, "tmp", "builder-retail-check", "Program.cs"));

        Assert.Contains("RecookPackagedMaterialAssets(repositoryRootPath);", retailHarnessSource, StringComparison.Ordinal);
        Assert.Contains("static void RecookGeneratedStandardMaterialAsset(", retailHarnessSource, StringComparison.Ordinal);
        Assert.Contains("engine/materials/standard.hasset", retailHarnessSource, StringComparison.Ordinal);
        Assert.Contains("static void RecookStagedAuthoredMaterialAssets(", retailHarnessSource, StringComparison.Ordinal);
        Assert.Contains("Directory.GetFiles(stagedMaterialsRootPath, \"*.hasset\", SearchOption.AllDirectories);", retailHarnessSource, StringComparison.Ordinal);
        Assert.Contains("settingsService.LoadMaterialAsset(sourceMaterialAssetPath, \"gamecube\")", retailHarnessSource, StringComparison.Ordinal);
        Assert.Contains("settingsService.TryLoadPlatformSettings(sourceMaterialAssetPath, \"gamecube\", out MaterialAssetProcessorSettings platformSettings)", retailHarnessSource, StringComparison.Ordinal);
        Assert.Contains("fieldValues[GameCubeMaterialSchemaIds.TextureRelativePathFieldId] = \"cooked/imported/\" + materialAsset.DiffuseTextureAssetId;", retailHarnessSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures the GameCube input backend maps native pad buttons onto the shared gamepad-state contract used by menu navigation.
    /// </summary>
    [Fact]
    public void GameCubeInputManager_WhenBuiltForMenu_MapsNativePadButtonsToSharedGamepadState() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string source = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeInputManager.cpp"));

        Assert.Contains("PAD_ScanPads();", source, StringComparison.Ordinal);
        Assert.Contains("frame.set_GamepadCount(1);", source, StringComparison.Ordinal);
        Assert.Contains("Array<InputGamepadState>* gamepads = new Array<InputGamepadState>(1);", source, StringComparison.Ordinal);
        Assert.Contains("gamepadState.set_Connected(true);", source, StringComparison.Ordinal);
        Assert.Contains("gamepadState.SetButtonDown(InputGamepadButton::DPadUp", source, StringComparison.Ordinal);
        Assert.Contains("gamepadState.SetButtonDown(InputGamepadButton::DPadDown", source, StringComparison.Ordinal);
        Assert.Contains("gamepadState.SetButtonDown(InputGamepadButton::DPadLeft", source, StringComparison.Ordinal);
        Assert.Contains("gamepadState.SetButtonDown(InputGamepadButton::DPadRight", source, StringComparison.Ordinal);
        Assert.Contains("gamepadState.SetButtonDown(InputGamepadButton::South", source, StringComparison.Ordinal);
        Assert.Contains("gamepadState.SetButtonDown(InputGamepadButton::East", source, StringComparison.Ordinal);
        Assert.Contains("gamepadState.SetButtonDown(InputGamepadButton::Start", source, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures the GameCube frame plan preserves extracted light submissions and authored normals for the lit mesh path.
    /// </summary>
    [Fact]
    public void PackagedDiscBootSource_PreservesExtractedLightsAndNormalsForLitRendering() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string framePlanSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeFramePlan.hpp"));
        string sceneRenderBridgeSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeSceneRenderBridge.cpp"));
        string runtimeModelSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRuntimeModel.hpp"));
        string renderManagerSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRenderManager3D.cpp"));
        string renderManagerHeaderSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRenderManager3D.hpp"));
        string rasterRendererSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRasterRenderer.cpp"));

        Assert.Contains("List<RenderFrameLightSubmission*>* LightSubmissions;", framePlanSource, StringComparison.Ordinal);
        Assert.Contains("frame->get_LightSubmissions()", sceneRenderBridgeSource, StringComparison.Ordinal);
        Assert.Contains("Array<float3>* Normals;", runtimeModelSource, StringComparison.Ordinal);
        Assert.Contains("ModelAsset* OwnedSourceModelAsset;", runtimeModelSource, StringComparison.Ordinal);
        Assert.Contains("RuntimeModel* BuildModelFromCooked(std::string cookedAssetPath) override;", renderManagerHeaderSource, StringComparison.Ordinal);
        Assert.Contains("GameCubeRuntimeModel* runtimeModel = static_cast<GameCubeRuntimeModel*>(BuildModelFromRaw(cookedModelAsset));", renderManagerSource, StringComparison.Ordinal);
        Assert.Contains("runtimeModel->OwnedSourceModelAsset = cookedModelAsset;", renderManagerSource, StringComparison.Ordinal);
        Assert.Contains("ReleaseOwnedSourceModelAsset(runtimeModel);", renderManagerSource, StringComparison.Ordinal);
        Assert.Contains("GameCubeRuntimeModel* runtimeModel = MeshCache->Resolve(submission->get_Drawable()->get_Model());", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("DrawSubmesh(framePlan, submission, runtimeModel, (*submeshes)[submeshIndex], entity);", rasterRendererSource, StringComparison.Ordinal);
        Assert.DoesNotContain("DrawCaptureTriangle(framePlan, entity", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("runtimeModel->Normals = data->Normals;", renderManagerSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures the GameCube raster renderer uses the shared lighting-model contract for the first white ambient-plus-directional diffuse path.
    /// </summary>
    [Fact]
    public void PackagedDiscBootSource_UsesSharedLightingModelForWhiteDirectionalDiffuseRendering() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string renderManagerHeaderSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRenderManager3D.hpp"));
        string renderManagerSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRenderManager3D.cpp"));
        string runtimeMaterialHeaderSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRuntimeMaterial.hpp"));
        string runtimeMaterialSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRuntimeMaterial.cpp"));
        string rasterRendererHeaderSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRasterRenderer.hpp"));
        string rasterRendererSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRasterRenderer.cpp"));

        Assert.Contains("class GameCubeRuntimeMaterial;", renderManagerHeaderSource, StringComparison.Ordinal);
        Assert.Contains("GameCubeRuntimeMaterial* runtimeMaterial = new GameCubeRuntimeMaterial();", renderManagerSource, StringComparison.Ordinal);
        Assert.Contains("runtimeMaterial->SetBaseColor(float3(", renderManagerSource, StringComparison.Ordinal);
        Assert.Contains("class GameCubeRuntimeMaterial final : public RuntimeMaterial", runtimeMaterialHeaderSource, StringComparison.Ordinal);
        Assert.Contains("float3 GetBaseColor() const;", runtimeMaterialHeaderSource, StringComparison.Ordinal);
        Assert.Contains("void SetBaseColor(float3 value);", runtimeMaterialHeaderSource, StringComparison.Ordinal);
        Assert.Contains("GameCubeRuntimeMaterial::GameCubeRuntimeMaterial()", runtimeMaterialSource, StringComparison.Ordinal);
        Assert.Contains("RuntimeMaterialLightingModel", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("EvaluateLitVertexColor(GameCubeFramePlan* framePlan, Entity* entity, GameCubeRuntimeMaterial* material, float3 normal);", rasterRendererHeaderSource, StringComparison.Ordinal);
        Assert.Contains("AccumulateAmbientAndDirectionalLight(", rasterRendererHeaderSource, StringComparison.Ordinal);
        Assert.Contains("submission->get_Material()", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("GameCubeRuntimeMaterial* gameCubeRuntimeMaterial = static_cast<GameCubeRuntimeMaterial*>(material);", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("const float3 baseColor = material->GetBaseColor();", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("lighting.X * baseColor.X", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("lighting.Y * baseColor.Y", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("lighting.Z * baseColor.Z", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("RuntimeMaterialLightingModel::Unlit", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("RuntimeMaterialLightingModel::MetalRoughPbr", rasterRendererSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures the GameCube cooked-material seam preserves authored texture paths and binds resolved runtime textures through the GX lit branch.
    /// </summary>
    [Fact]
    public void PackagedDiscBootSource_PreservesCookedTexturePathsAndUsesTexturedGxBranch() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string renderManagerSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRenderManager3D.cpp"));
        string runtimeMaterialHeaderSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRuntimeMaterial.hpp"));
        string runtimeMaterialSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRuntimeMaterial.cpp"));
        string runtimeTextureHeaderSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRuntimeTexture.hpp"));
        string runtimeTextureSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRuntimeTexture.cpp"));
        string renderManager2DSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRenderManager2D.cpp"));
        string rasterRendererSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRasterRenderer.cpp"));
        string normalizerSource = File.ReadAllText(Path.Combine(repositoryRootPath, "builder", "GameCubeGeneratedCoreCompatibilityNormalizer.cs"));
        string makefileSource = File.ReadAllText(Path.Combine(repositoryRootPath, "Makefile"));

        Assert.Contains("const std::string& GetTextureRelativePath() const;", runtimeMaterialHeaderSource, StringComparison.Ordinal);
        Assert.Contains("void SetTextureRelativePath(std::string value);", runtimeMaterialHeaderSource, StringComparison.Ordinal);
        Assert.Contains("TextureRelativePathValue", runtimeMaterialSource, StringComparison.Ordinal);
        Assert.Contains("runtimeMaterial->SetTextureRelativePath(materialAsset->TextureRelativePath);", renderManagerSource, StringComparison.Ordinal);
        Assert.Contains("class GameCubeRuntimeTexture final : public RuntimeTexture", runtimeTextureHeaderSource, StringComparison.Ordinal);
        Assert.Contains("void LoadFromRaw(TextureAsset* data);", runtimeTextureHeaderSource, StringComparison.Ordinal);
        Assert.Contains("GX_InitTexObj(&NativeTextureObject", runtimeTextureSource, StringComparison.Ordinal);
        Assert.Contains("GX_InitTexObjFilterMode(&NativeTextureObject, GX_LINEAR, GX_LINEAR);", runtimeTextureSource, StringComparison.Ordinal);
        Assert.Contains("GameCubeRuntimeTexture* runtimeTexture = new GameCubeRuntimeTexture();", renderManager2DSource, StringComparison.Ordinal);
        Assert.Contains("materialAsset->TextureRelativePath", normalizerSource, StringComparison.Ordinal);
        Assert.Contains("BuildTextureFromRaw(textureAsset)", normalizerSource, StringComparison.Ordinal);
        Assert.Contains("SetTexture(StandardMaterialTextureBindingDefaults::DiffuseTextureBindingName, runtimeTexture);", normalizerSource, StringComparison.Ordinal);
        Assert.Contains("GameCubeRuntimeTexture* boundTexture = expectsTexture", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("? ResolveBoundTexture(gameCubeRuntimeMaterial)", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("GameCubeRuntimeTexture* gameCubeRuntimeTexture = static_cast<GameCubeRuntimeTexture*>(runtimeTexture);", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("const bool expectsTexture = !gameCubeRuntimeMaterial->GetTextureRelativePath().empty();", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("GameCube textured material requires one resolved runtime texture.", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("GX_SetNumTexGens(useTexturedBranch ? 1 : 0);", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("GX_SetTevOp(GX_TEVSTAGE0, useTexturedBranch ? GX_MODULATE : GX_PASSCLR);", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("GX_LoadTexObj(boundTexture->GetNativeTextureObject(), GX_TEXMAP0);", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("GX_TexCoord2f32(textureCoordinate.X, textureCoordinate.Y);", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("GameCubeRuntimeTexture.cpp", makefileSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures the GameCube runtime texture loader can accept already cooked GX RGB5A3 payloads without forcing an RGBA32 transcode path.
    /// </summary>
    [Fact]
    public void PackagedRuntimeTextureLoader_WhenTextureIsAlreadyGxRgb5A3_DoesNotRequireRgba32Transcode() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string runtimeTextureSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRuntimeTexture.cpp"));

        Assert.Contains("TextureAssetColorFormat::GxRgb5A3", runtimeTextureSource, StringComparison.Ordinal);
        Assert.DoesNotContain("GameCube runtime textures currently require RGBA32 texture assets.", runtimeTextureSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures the first GameCube lit branch does not inject an artificial minimum brightness when the shared light result is zero.
    /// </summary>
    [Fact]
    public void PackagedDiscBootSource_DoesNotForceMinimumLightFloorWhenAccumulatedLightingIsZero() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string rasterRendererSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRasterRenderer.cpp"));

        Assert.DoesNotContain("return float3(0.15f, 0.15f, 0.15f);", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("return accumulated;", rasterRendererSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures the GameCube renderer preserves the shared cull-mode contract while reversing it for GX face interpretation.
    /// </summary>
    [Fact]
    public void PackagedDiscBootSource_ReversesSharedCullModesForGxFaceCulling() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string rasterRendererHeaderSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRasterRenderer.hpp"));
        string rasterRendererSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRasterRenderer.cpp"));

        Assert.Contains("u8 ResolveGxCullMode(MaterialCullMode cullMode);", rasterRendererHeaderSource, StringComparison.Ordinal);
        Assert.Contains("switch (cullMode)", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("case MaterialCullMode::None:", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("return GX_CULL_NONE;", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("case MaterialCullMode::Back:", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("return GX_CULL_FRONT;", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("case MaterialCullMode::Front:", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("return GX_CULL_BACK;", rasterRendererSource, StringComparison.Ordinal);
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

        Assert.Contains("EngineCore->Initialize(EngineRenderManager3D, EngineRenderManager2D, EngineInputManager, EnginePlatformInfo, options);", applicationSource, StringComparison.Ordinal);
        Assert.Contains("EngineCore->get_SceneManager()->LoadScene(packagedStartupSceneId, SceneLoadMode::Single);", applicationSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures the GameCube 2D render bridge records per-frame draw requests instead of remaining a no-op stub.
    /// </summary>
    [Fact]
    public void GameCubeRenderManager2D_WhenBuiltForMenu_CapturesPerFrameDrawRequests() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRenderManager2D.hpp"));
        string source = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRenderManager2D.cpp"));

        Assert.Contains("struct GameCubeSpriteDrawCommand", headerSource, StringComparison.Ordinal);
        Assert.Contains("struct GameCubeTextDrawCommand", headerSource, StringComparison.Ordinal);
        Assert.Contains("struct GameCubeRoundedRectDrawCommand", headerSource, StringComparison.Ordinal);
        Assert.Contains("void BeginFrame();", headerSource, StringComparison.Ordinal);
        Assert.Contains("bool HasCapturedDrawables() const;", headerSource, StringComparison.Ordinal);
        Assert.Contains("const std::vector<GameCubeSpriteDrawCommand>& GetSpriteQueue() const;", headerSource, StringComparison.Ordinal);
        Assert.Contains("const std::vector<GameCubeTextDrawCommand>& GetTextQueue() const;", headerSource, StringComparison.Ordinal);
        Assert.Contains("const std::vector<GameCubeRoundedRectDrawCommand>& GetRoundedRectQueue() const;", headerSource, StringComparison.Ordinal);
        Assert.DoesNotContain("(void)sprite;", source, StringComparison.Ordinal);
        Assert.DoesNotContain("(void)text;", source, StringComparison.Ordinal);
        Assert.DoesNotContain("(void)shape;", source, StringComparison.Ordinal);
        Assert.Contains("SpriteQueue.clear();", source, StringComparison.Ordinal);
        Assert.Contains("TextQueue.clear();", source, StringComparison.Ordinal);
        Assert.Contains("RoundedRectQueue.clear();", source, StringComparison.Ordinal);
        Assert.Contains("SpriteQueue.push_back(GameCubeSpriteDrawCommand { sprite });", source, StringComparison.Ordinal);
        Assert.Contains("TextQueue.push_back(GameCubeTextDrawCommand { text });", source, StringComparison.Ordinal);
        Assert.Contains("RoundedRectQueue.push_back(GameCubeRoundedRectDrawCommand { shape });", source, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures the GameCube raster renderer exposes a GX-backed 2D pass for menu drawables.
    /// </summary>
    [Fact]
    public void GameCubeRasterRenderer_WhenRenderingMenu_Contains2DGxEntryPoints() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRasterRenderer.hpp"));
        string source = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRasterRenderer.cpp"));

        Assert.Contains("void Render2D(", headerSource, StringComparison.Ordinal);
        Assert.Contains("RenderRoundedRect2D(", headerSource, StringComparison.Ordinal);
        Assert.Contains("RenderSprite2D(", headerSource, StringComparison.Ordinal);
        Assert.Contains("RenderText2D(", headerSource, StringComparison.Ordinal);
        Assert.Contains("Render2D(const GameCubeRenderManager2D& renderManager2D, uint16_t frameWidth, uint16_t frameHeight)", source, StringComparison.Ordinal);
        Assert.Contains("RenderRoundedRect2D(command, frameWidth, frameHeight);", source, StringComparison.Ordinal);
        Assert.Contains("RenderSprite2D(command, frameWidth, frameHeight);", source, StringComparison.Ordinal);
        Assert.Contains("RenderText2D(command, frameWidth, frameHeight);", source, StringComparison.Ordinal);
        Assert.Contains("GX_LoadProjectionMtx(projectionMatrix, GX_ORTHOGRAPHIC);", source, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures the GameCube application resets and renders the 2D queue during the packaged menu frame loop.
    /// </summary>
    [Fact]
    public void GameCubeApplication_WhenRunningMenu_BeginsAndRenders2DFrames() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string applicationSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeApplication.cpp"));
        string bootstrapSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeSceneBootstrap.cpp"));

        Assert.Contains("EngineRenderManager2D->BeginFrame();", applicationSource, StringComparison.Ordinal);
        Assert.Contains("EngineRenderManager3D->Draw2D(EngineRenderManager2D, RenderMode->fbWidth, RenderMode->efbHeight);", applicationSource, StringComparison.Ordinal);
        Assert.Contains("Scenes/DemoDiscMainMenu.helen", bootstrapSource, StringComparison.Ordinal);
        Assert.Contains("const std::string startupSceneAliasId = \"DemoDiscMainMenu\";", bootstrapSource, StringComparison.Ordinal);
        Assert.Contains("bool startupSceneSourceExists = false;", bootstrapSource, StringComparison.Ordinal);
        Assert.Contains("const bool shouldAddStartupSceneAlias = !startupSceneAliasExists && startupSceneSourceExists;", bootstrapSource, StringComparison.Ordinal);
        Assert.Contains("const std::size_t runtimeEntryCount = shouldAddStartupSceneAlias ? entryCount + 1U : entryCount;", bootstrapSource, StringComparison.Ordinal);
        Assert.Contains("if (shouldAddStartupSceneAlias && StartupSceneId == entries[index].SceneId) {", bootstrapSource, StringComparison.Ordinal);
        Assert.Contains("new RuntimeSceneCatalogEntry(startupSceneAliasId, entries[index].CookedRelativePath)", bootstrapSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures the runtime model and mesh cache expose the cached mesh ownership contract required by the GX-native mesh path.
    /// </summary>
    [Fact]
    public void CachedMeshOwnershipSource_DefinesRuntimeOwnershipContract() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string runtimeModelSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRuntimeModel.hpp"));
        string meshCacheHeaderSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeMeshCache.hpp"));
        string meshCacheSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeMeshCache.cpp"));
        string cachedMeshHeaderPath = Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeCachedMeshData.hpp");

        Assert.True(File.Exists(cachedMeshHeaderPath), "Expected GameCubeCachedMeshData.hpp to exist.");
        string cachedMeshHeaderSource = File.ReadAllText(cachedMeshHeaderPath);

        Assert.Contains("class GameCubeCachedMeshData", cachedMeshHeaderSource, StringComparison.Ordinal);
        Assert.Contains("struct GameCubePackedNormal3", cachedMeshHeaderSource, StringComparison.Ordinal);
        Assert.Contains("Array<GameCubePackedNormal3>* PackedNormals;", cachedMeshHeaderSource, StringComparison.Ordinal);
        Assert.Contains("#include \"platform/gamecube/GameCubeCachedMeshData.hpp\"", runtimeModelSource, StringComparison.Ordinal);
        Assert.Contains("GameCubeCachedMeshData* CachedMeshData;", runtimeModelSource, StringComparison.Ordinal);
        Assert.Contains("GameCubeCachedMeshData", meshCacheHeaderSource, StringComparison.Ordinal);
        Assert.Contains("GameCubeCachedMeshData* GameCubeMeshCache::BuildCachedMeshData(GameCubeRuntimeModel* runtimeModel)", meshCacheSource, StringComparison.Ordinal);
        Assert.Contains("typedRuntimeModel->CachedMeshData = BuildCachedMeshData(typedRuntimeModel);", meshCacheSource, StringComparison.Ordinal);
        Assert.Contains("cachedMeshData->PackedNormals", meshCacheSource, StringComparison.Ordinal);
        Assert.Contains("DCFlushRange(&(*cachedMeshData->PackedNormals)[0]", meshCacheSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures the unlit and textured GameCube 3D path uses cached indexed GX arrays instead of walking authored geometry arrays directly.
    /// </summary>
    [Fact]
    public void CachedMeshRenderSource_UsesIndexedGeometryForUnlitAndTexturedSubmeshes() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string rasterRendererHeaderSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRasterRenderer.hpp"));
        string rasterRendererSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRasterRenderer.cpp"));

        Assert.Contains("BindCachedMeshArrays", rasterRendererHeaderSource, StringComparison.Ordinal);
        Assert.Contains("DrawCachedSubmesh", rasterRendererHeaderSource, StringComparison.Ordinal);
        Assert.Contains("runtimeModel->CachedMeshData", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("GX_SetArray(GX_VA_POS", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("GX_Position1x16(", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("GX_TexCoord1x16(", rasterRendererSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures the lit GameCube mesh path keeps cached indexed geometry and limits dynamic work to per-vertex color evaluation.
    /// </summary>
    [Fact]
    public void CachedMeshRenderSource_UsesCachedNormalsForLitGeometry() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string rasterRendererHeaderSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRasterRenderer.hpp"));
        string rasterRendererSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRasterRenderer.cpp"));

        Assert.Contains("void ConfigureLitPipeline(bool useTexturedBranch, bool useIndexedGeometry);", rasterRendererHeaderSource, StringComparison.Ordinal);
        Assert.Contains("void ConfigureDirectionalLight(", rasterRendererHeaderSource, StringComparison.Ordinal);
        Assert.Contains("DrawCachedLitSubmesh", rasterRendererHeaderSource, StringComparison.Ordinal);
        Assert.Contains("cachedMeshData->PackedNormals", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("GX_SetArray(GX_VA_NRM", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("GX_Normal1x16(", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("GX_SetVtxDesc(GX_VA_NRM, GX_INDEX16);", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("GX_SetChanCtrl(GX_COLOR0A0", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("GX_InitLightDir(", rasterRendererSource, StringComparison.Ordinal);
    }
}
