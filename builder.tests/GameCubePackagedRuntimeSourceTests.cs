namespace helengine.gamecube.builder.tests;

/// <summary>
/// Guards the packaged-disc native boot source contract in the GameCube host and build files.
/// </summary>
public sealed class GameCubePackagedRuntimeSourceTests {
    /// <summary>
    /// Ensures the native runtime contains the opt-in, bounded memory-card diagnostic journal required for retail-compatible hardware traces.
    /// </summary>
    [Fact]
    public void MemoryCardDiagnosticJournalSource_UsesOptInCardJournalContract() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string makefileSource = File.ReadAllText(Path.Combine(repositoryRootPath, "Makefile"));
        string journalHeaderPath = Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeMemoryCardDiagnosticJournal.hpp");
        string journalSourcePath = Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeMemoryCardDiagnosticJournal.cpp");

        Assert.Contains("HELENGINE_GAMECUBE_MEMORY_CARD_DIAGNOSTIC_JOURNAL ?= 0", makefileSource, StringComparison.Ordinal);
        Assert.True(File.Exists(journalHeaderPath));
        Assert.True(File.Exists(journalSourcePath));
        string journalHeaderSource = File.ReadAllText(journalHeaderPath);
        string journalSource = File.ReadAllText(journalSourcePath);
        Assert.Contains("GameCubeMemoryCardDiagnosticJournal", journalHeaderSource, StringComparison.Ordinal);
        Assert.Contains("bool Initialize();", journalHeaderSource, StringComparison.Ordinal);
        Assert.Contains("void Record(", journalHeaderSource, StringComparison.Ordinal);
        Assert.Contains("void Disable();", journalHeaderSource, StringComparison.Ordinal);
        Assert.Contains("CARD_Init", journalSource, StringComparison.Ordinal);
        Assert.Contains("CARD_Mount", journalSource, StringComparison.Ordinal);
        Assert.Contains("CARD_Open", journalSource, StringComparison.Ordinal);
        Assert.Contains("CARD_Create", journalSource, StringComparison.Ordinal);
        Assert.Contains("CARD_Write", journalSource, StringComparison.Ordinal);
        Assert.Contains("HELDBG", journalHeaderSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures the optional journal records the first input, generated update, and generated draw boundaries without becoming a per-frame write path.
    /// </summary>
    [Fact]
    public void MemoryCardDiagnosticJournalSource_RecordsFirstRuntimeBoundaries() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string applicationSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeApplication.cpp"));
        string inputSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeInputManager.cpp"));

        Assert.Contains("if (!InitializeMemoryCardDiagnosticJournal())", applicationSource, StringComparison.Ordinal);
        Assert.Contains("GameCubeMemoryCardDiagnosticStage::CoreUpdateBegin", applicationSource, StringComparison.Ordinal);
        Assert.Contains("GameCubeMemoryCardDiagnosticStage::CoreUpdateComplete", applicationSource, StringComparison.Ordinal);
        Assert.Contains("GameCubeMemoryCardDiagnosticStage::CoreDrawBegin", applicationSource, StringComparison.Ordinal);
        Assert.Contains("GameCubeMemoryCardDiagnosticStage::CoreDrawComplete", applicationSource, StringComparison.Ordinal);
        Assert.Contains("GameCubeMemoryCardDiagnosticStage::NintendontPadReadBegin", inputSource, StringComparison.Ordinal);
        Assert.Contains("GameCubeMemoryCardDiagnosticStage::NintendontPadReadComplete", inputSource, StringComparison.Ordinal);
        Assert.Contains("HasRecordedNintendontPadRead", inputSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures an enabled journal build stops on a durable color that identifies the exact CARD initialization operation that failed.
    /// </summary>
    [Fact]
    public void MemoryCardDiagnosticJournalSource_PresentsPersistentFailureColorBeforeEngineStartup() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string applicationSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeApplication.cpp"));
        string journalHeaderSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeMemoryCardDiagnosticJournal.hpp"));

        Assert.Contains("if (!InitializeMemoryCardDiagnosticJournal())", applicationSource, StringComparison.Ordinal);
        Assert.Contains("GetMemoryCardDiagnosticFailureColor", applicationSource, StringComparison.Ordinal);
        Assert.Contains("GetInitializationResultCode", applicationSource, StringComparison.Ordinal);
        Assert.Contains("while (true) {\n                PresentBootFrame();\n            }", applicationSource, StringComparison.Ordinal);
        Assert.Contains("GameCubeMemoryCardDiagnosticFailure", journalHeaderSource, StringComparison.Ordinal);
        Assert.Contains("s32 GetInitializationResultCode() const;", journalHeaderSource, StringComparison.Ordinal);
        Assert.Contains("CardInit", journalHeaderSource, StringComparison.Ordinal);
        Assert.Contains("CardWrite", journalHeaderSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures the opt-in Nintendont handoff diagnostic evaluates the private trampoline completion word before engine startup.
    /// </summary>
    [Fact]
    public void NintendontHandoffDiagnosticSource_StopsBeforeEngineStartupWithTrampolineResult() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string applicationSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeApplication.cpp"));
        string applicationHeaderSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeApplication.hpp"));
        string makefileSource = File.ReadAllText(Path.Combine(repositoryRootPath, "Makefile"));

        Assert.Contains("HELENGINE_GAMECUBE_NINTENDONT_HANDOFF_DIAGNOSTIC ?= 0", makefileSource, StringComparison.Ordinal);
        Assert.Contains("if (!VerifyNintendontHandoff())", applicationSource, StringComparison.Ordinal);
        Assert.Contains("NintendontHandoffStatusAddress", applicationSource, StringComparison.Ordinal);
        Assert.Contains("NintendontTrampolinePendingStatus", applicationSource, StringComparison.Ordinal);
        Assert.Contains("VerifyNintendontHandoff", applicationHeaderSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures the opt-in first-frame trace presents every runtime boundary and retains its final success frame.
    /// </summary>
    [Fact]
    public void FirstFrameTraceDiagnosticSource_PresentsEveryFirstFrameBoundaryAndStopsAfterSuccess() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string applicationSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeApplication.cpp"));
        string applicationHeaderSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeApplication.hpp"));
        string makefileSource = File.ReadAllText(Path.Combine(repositoryRootPath, "Makefile"));

        Assert.Contains("HELENGINE_GAMECUBE_FIRST_FRAME_TRACE_DIAGNOSTIC ?= 0", makefileSource, StringComparison.Ordinal);
        Assert.Contains("PresentFirstFrameTraceCheckpoint", applicationSource, StringComparison.Ordinal);
        Assert.Contains("FirstFrameTraceCompleted", applicationSource, StringComparison.Ordinal);
        Assert.Contains("PresentFirstFrameTraceCheckpoint", applicationHeaderSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures the optional first-frame trace separates the queued startup-scene commit from the subsequent core draw call.
    /// </summary>
    [Fact]
    public void FirstFrameTraceDiagnosticSource_SeparatesSceneCommitFromRendererDraw() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string applicationSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeApplication.cpp"));

        Assert.Contains("options->CommitPendingSceneOperationsDuringDraw = false;", applicationSource, StringComparison.Ordinal);
        Assert.Contains("EngineCore->CompleteFrameBoundary();", applicationSource, StringComparison.Ordinal);
        Assert.Contains("EngineCore->CompleteFrameBoundary();\n            PresentFirstFrameTraceCheckpoint(GXColor { 0x20, 0x60, 0xFF, 0xFF });", applicationSource, StringComparison.Ordinal);
        Assert.Contains("EngineCore->Draw();", applicationSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures the first-frame trace can distinguish vector capacity allocation from recording the loaded scene.
    /// </summary>
    [Fact]
    public void FirstFrameTraceDiagnosticSource_SeparatesSceneRecordListReserveFromAdd() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string applicationSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeApplication.cpp"));

        Assert.Contains("PresentFirstFrameTraceCheckpoint(GXColor { 0xFF, 0x00, 0xC0, 0xFF });\n            auto* loadedScenes = EngineCore->get_SceneManager()->get_LoadedScenes();\n            loadedScenes->set_Capacity(loadedScenes->get_Count() + 1);", applicationSource, StringComparison.Ordinal);
        Assert.Contains("PresentFirstFrameTraceCheckpoint(GXColor { 0x00, 0xFF, 0xFF, 0xFF });\n            EngineCore->CompleteFrameBoundary();", applicationSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures the opt-in exception diagnostic captures processor context and the scene-list state through the native framebuffer.
    /// </summary>
    [Fact]
    public void ExceptionScreenDiagnosticSource_InstallsPpcPanicCaptureAndDisplaysSceneListState() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string applicationSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeApplication.cpp"));
        string diagnosticsSourcePath = Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeExceptionDiagnostics.cpp");
        string makefileSource = File.ReadAllText(Path.Combine(repositoryRootPath, "Makefile"));

        Assert.Contains("HELENGINE_GAMECUBE_EXCEPTION_SCREEN_DIAGNOSTIC ?= 0", makefileSource, StringComparison.Ordinal);
        Assert.Contains("ifeq ($(strip $(HELENGINE_GAMECUBE_FIRST_FRAME_TRACE_DIAGNOSTIC)),1)\nHELENGINE_GAMECUBE_EXCEPTION_SCREEN_DIAGNOSTIC := 1", makefileSource, StringComparison.Ordinal);
        Assert.Contains("GameCubeExceptionDiagnostics::Install(this);", applicationSource, StringComparison.Ordinal);
        Assert.Contains("GameCubeExceptionDiagnostics::CaptureSceneListState", applicationSource, StringComparison.Ordinal);
        Assert.True(File.Exists(diagnosticsSourcePath));

        string diagnosticsSource = File.ReadAllText(diagnosticsSourcePath);
        Assert.Contains("PPCExcptCurPanicFn", diagnosticsSource, StringComparison.Ordinal);
        Assert.Contains("PPCContext", diagnosticsSource, StringComparison.Ordinal);
        Assert.Contains("PPCMfspr(DAR)", diagnosticsSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures the first-frame diagnostic presents the generated scene-manager transition stages that bracket the initial disc load and runtime materialization.
    /// </summary>
    [Fact]
    public void FirstFrameTraceDiagnosticSource_PresentsInitialSceneCommitStages() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string applicationSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeApplication.cpp"));
        string diagnosticsSourcePath = Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeSceneTransitionTraceDiagnostics.cpp");

        Assert.Contains("GameCubeSceneTransitionTraceDiagnostics", applicationSource, StringComparison.Ordinal);
        Assert.Contains("options->set_RuntimeDiagnosticsProvider(SceneTransitionTraceDiagnostics);", applicationSource, StringComparison.Ordinal);
        Assert.True(File.Exists(diagnosticsSourcePath));

        string diagnosticsSource = File.ReadAllText(diagnosticsSourcePath);
        Assert.Contains("LoadSceneImmediateBeforeResolveSceneContentPath", diagnosticsSource, StringComparison.Ordinal);
        Assert.Contains("LoadSceneImmediateAfterResolveSceneContentPath", diagnosticsSource, StringComparison.Ordinal);
        Assert.Contains("LoadSceneImmediateBeforeLoadedSceneRecordLookup", diagnosticsSource, StringComparison.Ordinal);
        Assert.Contains("LoadSceneImmediateAfterLoadedSceneRecordLookup", diagnosticsSource, StringComparison.Ordinal);
        Assert.Contains("code = 0xE013U;", diagnosticsSource, StringComparison.Ordinal);
        Assert.Contains("code = 0xE016U;", diagnosticsSource, StringComparison.Ordinal);
        Assert.Contains("LoadSceneImmediateBeforeContentLoad", diagnosticsSource, StringComparison.Ordinal);
        Assert.Contains("LoadSceneImmediateBeforeSceneLoadServiceLoad", diagnosticsSource, StringComparison.Ordinal);
        Assert.Contains("LoadSceneImmediateAfterSceneLoadServiceLoad", diagnosticsSource, StringComparison.Ordinal);
        Assert.Contains("LoadSceneImmediateAfterLoadedSceneRecordListAdd", diagnosticsSource, StringComparison.Ordinal);
        Assert.Contains("LoadSceneImmediateAfterLoadedSceneRecordDictionaryAdd", diagnosticsSource, StringComparison.Ordinal);
        Assert.Contains("LoadSceneImmediateBeforeRegisterOwnedTextures", diagnosticsSource, StringComparison.Ordinal);
        Assert.Contains("LoadSceneImmediateBeforeRegisterOwnedFonts", diagnosticsSource, StringComparison.Ordinal);
        Assert.Contains("LoadSceneImmediateBeforeRegisterOwnedAudio", diagnosticsSource, StringComparison.Ordinal);
        Assert.Contains("LoadSceneImmediateBeforeRegisterOwnedModels", diagnosticsSource, StringComparison.Ordinal);
        Assert.Contains("LoadSceneImmediateBeforeRegisterOwnedMaterials", diagnosticsSource, StringComparison.Ordinal);
        Assert.Contains("LoadSceneImmediateAfterRegisterOwnedAssets", diagnosticsSource, StringComparison.Ordinal);
        Assert.Contains("LoadSceneImmediateBeforeSceneLoadedEvent", diagnosticsSource, StringComparison.Ordinal);
        Assert.Contains("LoadSceneImmediateAfterSceneLoadedEvent", diagnosticsSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures a journal-disabled native build does not retain direct journal method references in the Nintendont input transport.
    /// </summary>
    [Fact]
    public void NintendontInputSource_CompilesJournalCallbacksOnlyWhenTheJournalIsEnabled() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string inputSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeInputManager.cpp"));

        Assert.Contains("#if HELENGINE_GAMECUBE_MEMORY_CARD_DIAGNOSTIC_JOURNAL\n#include \"platform/gamecube/GameCubeMemoryCardDiagnosticJournal.hpp\"\n#endif", inputSource, StringComparison.Ordinal);
        Assert.Contains("#if HELENGINE_GAMECUBE_MEMORY_CARD_DIAGNOSTIC_JOURNAL\n        if (diagnosticJournal != nullptr)", inputSource, StringComparison.Ordinal);
        Assert.Contains("#if HELENGINE_GAMECUBE_MEMORY_CARD_DIAGNOSTIC_JOURNAL\n            if (!HasRecordedNintendontPadRead && DiagnosticJournal != nullptr)", inputSource, StringComparison.Ordinal);
    }

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
        string discReaderSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeDiscReader.cpp"));
        string manifestWriterSource = File.ReadAllText(Path.Combine(repositoryRootPath, "builder", "GameCubeRuntimeSceneManifestWriter.cs"));

        Assert.Contains("HELENGINE_GAMECUBE_BOOT_MODE", makefileSource, StringComparison.Ordinal);
        Assert.Contains("HELENGINE_GAMECUBE_PACKAGED_DISC_BOOT", makefileSource, StringComparison.Ordinal);
        Assert.Contains("HELENGINE_GAMECUBE_BATCH_VERIFY_FRAME_LIMIT", makefileSource, StringComparison.Ordinal);
        Assert.Contains("HELENGINE_GAMECUBE_MINIMAL_SAMPLE ?= 0", makefileSource, StringComparison.Ordinal);
        Assert.Contains("GameCubeMinimalSampleMain.cpp", makefileSource, StringComparison.Ordinal);
        Assert.Contains("GENERATED_CORE_TRANSLATION_UNIT := helengine_core_amalgamated.cpp", makefileSource, StringComparison.Ordinal);
        Assert.Contains("GENERATED_CORE_TRANSLATION_UNIT := helengine_core_unity.cpp", makefileSource, StringComparison.Ordinal);
        Assert.Contains("GENERATED_CORE_TRANSLATION_UNIT := generated_unity.cpp", makefileSource, StringComparison.Ordinal);
        Assert.Contains("does not contain helengine_core_amalgamated.cpp, helengine_core_unity.cpp, or generated_unity.cpp", makefileSource, StringComparison.Ordinal);
        Assert.Contains("$(BUILD_DIR)/generated/$(GENERATED_CORE_TRANSLATION_UNIT:.cpp=.o): $(GENERATED_CORE_SOURCE)", makefileSource, StringComparison.Ordinal);
        Assert.Contains("-MMD", makefileSource, StringComparison.Ordinal);
        Assert.Contains("-MP", makefileSource, StringComparison.Ordinal);
        Assert.Contains("DEPFILES := $(OBJECTS:.o=.d)", makefileSource, StringComparison.Ordinal);
        Assert.Contains("-include $(DEPFILES)", makefileSource, StringComparison.Ordinal);
        Assert.Contains("packaged-disc-assets", makefileSource, StringComparison.Ordinal);
        Assert.Contains("APPLOADER_SOURCE_ROOT := $(THIRD_PARTY_DIR)/cubeboot-tools", makefileSource, StringComparison.Ordinal);
        Assert.Contains("gamecube_runtime_scene_manifest.inl", manifestWriterSource, StringComparison.Ordinal);
        Assert.Contains("string.Equals(manifest.Scenes[index].SceneId, \"HelenOfCodeSplash\", StringComparison.Ordinal)", manifestWriterSource, StringComparison.Ordinal);
        Assert.Contains("static bool InitializePackagedDisc();", bootstrapHeaderSource, StringComparison.Ordinal);
        Assert.Contains("static bool InitializePackagedDiscInterface();", bootstrapHeaderSource, StringComparison.Ordinal);
        Assert.Contains("static bool VerifyPackagedDiscReadiness();", bootstrapHeaderSource, StringComparison.Ordinal);
        Assert.DoesNotContain("PresentBootDiagnostic", applicationHeaderSource, StringComparison.Ordinal);
        Assert.DoesNotContain("PresentBootDiagnostic", applicationSource, StringComparison.Ordinal);
        Assert.Contains("if (!GameCubeSceneBootstrap::InitializePackagedDiscInterface()) {", applicationSource, StringComparison.Ordinal);
        Assert.Contains("if (!GameCubeSceneBootstrap::VerifyPackagedDiscReadiness()) {", applicationSource, StringComparison.Ordinal);
        Assert.Contains("GameCubeDiscReader::Initialize();", bootstrapSource, StringComparison.Ordinal);
        Assert.Contains("GameCubeDiscReader::ReadBytes(discHeader, 0U, sizeof(discHeader))", bootstrapSource, StringComparison.Ordinal);
        Assert.Contains("GameCubeDiscInterruptTrampoline", discReaderSource, StringComparison.Ordinal);
        Assert.Contains("IRQ_Request(IRQ_PI_DI, GameCubeDiscInterruptTrampoline, nullptr);", discReaderSource, StringComparison.Ordinal);
        Assert.Contains("GameCubeSceneBootstrap::GetPackagedContentRootPath()", applicationSource, StringComparison.Ordinal);
        Assert.Contains("GameCubeSceneBootstrap::CreatePackagedSceneCatalog()", applicationSource, StringComparison.Ordinal);
        Assert.Contains("GameCubeSceneBootstrap::GetPackagedStartupSceneId()", applicationSource, StringComparison.Ordinal);
        Assert.Contains("GX_SetPixelFmt(GX_PF_RGBA6_Z24, GX_ZC_LINEAR);", applicationSource, StringComparison.Ordinal);
        Assert.Contains("SetBootPhase(GameCubeBootPhase::NativeVideo, GXColor { 0xFF, 0x00, 0x00, 0xFF });", applicationSource, StringComparison.Ordinal);
        Assert.Contains("SetBootPhase(GameCubeBootPhase::NativeGraphics, GXColor { 0x00, 0x00, 0xFF, 0xFF });", applicationSource, StringComparison.Ordinal);
        Assert.Contains("VIDEO_ClearFrameBuffer(RenderMode, FrameBuffers[0], COLOR_BLACK);", applicationSource, StringComparison.Ordinal);
        Assert.Contains("VIDEO_ClearFrameBuffer(RenderMode, FrameBuffers[1], COLOR_BLACK);", applicationSource, StringComparison.Ordinal);
        Assert.Contains("GX_CopyDisp(FrameBuffers[0], GX_TRUE);", applicationSource, StringComparison.Ordinal);
        Assert.Contains("void PresentBootFrame();", applicationHeaderSource, StringComparison.Ordinal);
        Assert.Contains("static uint32_t ConvertToVideoClearColor(const GXColor& color);", applicationHeaderSource, StringComparison.Ordinal);
        Assert.Contains("PresentBootFrame();\n        if (!InitializeGraphics())", applicationSource, StringComparison.Ordinal);
        Assert.Contains("const uint32_t videoClearColor = ConvertToVideoClearColor(ClearColor);", applicationSource, StringComparison.Ordinal);
        Assert.Contains("VIDEO_ClearFrameBuffer(RenderMode, FrameBuffers[0], videoClearColor);", applicationSource, StringComparison.Ordinal);
        Assert.Contains("GX_SetCopyClear(ClearColor, 0x00FFFFFF);", applicationSource, StringComparison.Ordinal);
        Assert.Contains("EngineCore = new Core();", applicationSource, StringComparison.Ordinal);
        Assert.Contains("EngineCore->Initialize(", applicationSource, StringComparison.Ordinal);
        Assert.Contains("static std::string GetRuntimeTestSceneOverride();", applicationHeaderSource, StringComparison.Ordinal);
        Assert.Contains("HELENGINE_GAMECUBE_RUNTIME_TEST_SCENE", applicationSource, StringComparison.Ordinal);
        Assert.Contains("GameCubeCubeTestSceneInstaller::InstallSlopeScene();", applicationSource, StringComparison.Ordinal);
        Assert.DoesNotContain("RunPackagedStartupProbe()", applicationSource, StringComparison.Ordinal);
        Assert.DoesNotContain("int RunPackagedStartupProbe();", applicationHeaderSource, StringComparison.Ordinal);
        Assert.Contains("HasSatisfiedVerificationExitCondition()", applicationSource, StringComparison.Ordinal);
        Assert.DoesNotContain("PresentFailureLoop();", applicationSource, StringComparison.Ordinal);
        Assert.DoesNotContain("void PresentFailureLoop();", applicationHeaderSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures the packaged runtime manifest writer canonicalizes cooked scene asset paths during build generation instead of relying on runtime rejection.
    /// </summary>
    [Fact]
    public void PackagedDiscBootSource_CanonicalizesRuntimeSceneManifestPathsDuringBuild() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string manifestWriterSource = File.ReadAllText(Path.Combine(repositoryRootPath, "builder", "GameCubeRuntimeSceneManifestWriter.cs"));

        Assert.Contains("CanonicalPackagedAssetPath.Normalize(metadataEntry.Value)", manifestWriterSource, StringComparison.Ordinal);
        Assert.DoesNotContain("metadataEntry.Value.Replace('\\\\', '/')", manifestWriterSource, StringComparison.Ordinal);
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

        Assert.Contains("[GC] Packaged content root:", applicationSource, StringComparison.Ordinal);
        Assert.Contains("#include \"RuntimeSceneLoadService.hpp\"", applicationSource, StringComparison.Ordinal);
        Assert.Contains("#include \"SceneManager.hpp\"", applicationSource, StringComparison.Ordinal);
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
        Assert.Contains("ConfigureDirectionalLight(", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("submission->get_Material()", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("ResolveGxCullMode(", rasterRendererSource, StringComparison.Ordinal);
        Assert.DoesNotContain("[GC] 2D queue snapshot", applicationSource, StringComparison.Ordinal);
        Assert.DoesNotContain("[GC] Frame %u extracted.", renderManagerSource, StringComparison.Ordinal);
        Assert.Contains("RecordTraceState(\"LoadSceneImmediateBeforeContentLoad\"", sceneManagerSource, StringComparison.Ordinal);
        Assert.Contains("RecordTraceState(\"LoadSceneImmediateBeforeSceneLoadServiceLoad\"", sceneManagerSource, StringComparison.Ordinal);
        Assert.DoesNotContain("GameCubeRecordSceneLoadRequest(sceneId.c_str());", sceneManagerSource, StringComparison.Ordinal);
        Assert.Contains("[GC] ContentManager opening asset:", contentManagerSource, StringComparison.Ordinal);
        Assert.Contains("std::string PlatformMenuSceneResolver::DesktopMainMenuSceneId = \"Scenes/DemoDiscMainMenu.helen\";", platformMenuSceneResolverSource, StringComparison.Ordinal);
        Assert.Contains("uint8_t FontAssetBinarySerializer::CurrentVersion = 5;", fontAssetBinarySerializerSource, StringComparison.Ordinal);
        Assert.Contains("uint8_t FontAssetBinarySerializer::ExternalCookedAtlasPathVersion = 5;", fontAssetBinarySerializerSource, StringComparison.Ordinal);
        Assert.Contains("FontAssetBinarySerializer::set_LastDeserializeStage(\"ReadCookedAtlasTexturePath\");", fontAssetBinarySerializerSource, StringComparison.Ordinal);
        Assert.Contains("if (sourceTexture->Width > 0 && sourceTexture->Height > 0 && sourceTexture->Colors != nullptr && sourceTexture->Colors->get_Length() > 0)", fontAssetBinarySerializerSource, StringComparison.Ordinal);
        Assert.Contains("[GC] File::Exists path=", fileSource, StringComparison.Ordinal);
        Assert.Contains("[GC] File::OpenRead path=", fileSource, StringComparison.Ordinal);
        Assert.Contains("delete sourceTextureAsset;", renderManager2DSource, StringComparison.Ordinal);
        Assert.Contains("#include \"PackagedAssetBinarySerializer.hpp\"", renderManager2DSource, StringComparison.Ordinal);
        Assert.Contains("::PackagedAssetBinarySerializer::Deserialize(textureStream)", renderManager2DSource, StringComparison.Ordinal);
        Assert.DoesNotContain("EditorAssetBinarySerializer", renderManager2DSource, StringComparison.Ordinal);
        Assert.Contains("delete static_cast<GameCubeRuntimeTexture*>(texture);", renderManager2DSource, StringComparison.Ordinal);
        Assert.Contains("#include \"PackagedAssetBinarySerializer.hpp\"", renderManagerSource, StringComparison.Ordinal);
        Assert.Contains("::PackagedAssetBinarySerializer::Deserialize(stream)", renderManagerSource, StringComparison.Ordinal);
        Assert.DoesNotContain("EditorAssetBinarySerializer", renderManagerSource, StringComparison.Ordinal);
        Assert.Contains("normalizedMaterialAssetPath.find(\"cooked/\")", renderManagerSource, StringComparison.Ordinal);
        Assert.Contains("contentRootPath = normalizedMaterialAssetPath.substr(0, cookedMarkerIndex)", renderManagerSource, StringComparison.Ordinal);
        Assert.Contains("delete static_cast<GameCubeRuntimeMaterial*>(material);", renderManagerSource, StringComparison.Ordinal);
        Assert.Contains("delete runtimeModel;", renderManagerSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures the GameCube 2D raster path reuses persistent text-layout and rounded-rectangle scratch storage instead of allocating fresh UI buffers every frame.
    /// </summary>
    [Fact]
    public void PackagedDiscBootSource_ReusesUiScratchBuffersDuringRasterOverlayDraws() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string rasterRendererSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRasterRenderer.cpp"));
        string rasterRendererHeaderSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRasterRenderer.hpp"));

        Assert.Contains("struct CachedTextLayoutEntry {", rasterRendererHeaderSource, StringComparison.Ordinal);
        Assert.Contains("std::vector<CachedTextLayoutEntry> CachedTextLayouts;", rasterRendererHeaderSource, StringComparison.Ordinal);
        Assert.Contains("std::vector<float2> RoundedRectOutlineScratch;", rasterRendererHeaderSource, StringComparison.Ordinal);
        Assert.Contains("uint32_t ActiveTextLayoutFrameId;", rasterRendererHeaderSource, StringComparison.Ordinal);
        Assert.Contains("const std::string& ResolveTextLayoutContent(ITextDrawable2D* drawable, FontAsset* font, double fontScale);", rasterRendererHeaderSource, StringComparison.Ordinal);
        Assert.Contains("void PruneTextLayoutCache();", rasterRendererHeaderSource, StringComparison.Ordinal);
        Assert.Contains("const std::string& content = ResolveTextLayoutContent(drawable, font, fontScale);", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("CachedTextLayouts.erase(", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("BuildRoundedRectOutline(x, y, width, height, radius, corners, RoundedRectOutlineScratch);", rasterRendererSource, StringComparison.Ordinal);
        Assert.DoesNotContain("std::string content = drawable->get_Text();", rasterRendererSource, StringComparison.Ordinal);
        Assert.DoesNotContain("std::vector<float2> outline;", rasterRendererSource, StringComparison.Ordinal);
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
        Assert.Contains("HasRenderedSceneValue = RasterRenderer->DrawFrame(framePlan) || HasRenderedSceneValue;", renderManagerSource, StringComparison.Ordinal);
        Assert.DoesNotContain("HasRenderedSceneValue = true;", renderManagerSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures the GameCube runtime source no longer emits per-frame snapshot and render queue spam through OSReport.
    /// </summary>
    [Fact]
    public void PackagedDiscBootSource_DoesNotEmitPerFrameRenderSpamDiagnostics() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sceneRenderBridgeSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeSceneRenderBridge.cpp"));
        string renderManagerSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRenderManager3D.cpp"));

        Assert.DoesNotContain("[GC] snap=", sceneRenderBridgeSource, StringComparison.Ordinal);
        Assert.DoesNotContain("[GC] extract=", sceneRenderBridgeSource, StringComparison.Ordinal);
        Assert.DoesNotContain("[GC] has3D=", renderManagerSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures packaged disc reads expose the DI completion bridge that Nintendont patches for GameCube applications.
    /// </summary>
    [Fact]
    public void PackagedDiscReaderSource_UsesNintendontCompatibleInterruptBridge() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string discReaderSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeDiscReader.cpp"));
        string discReaderHeaderSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeDiscReader.hpp"));
        string bootstrapSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeSceneBootstrap.cpp"));

        Assert.Contains("static bool Initialize();", discReaderHeaderSource, StringComparison.Ordinal);
        Assert.Contains("GameCubeDiscReader::Initialize", bootstrapSource, StringComparison.Ordinal);
        Assert.Contains("GameCubeDiscInterruptTrampoline", discReaderSource, StringComparison.Ordinal);
        Assert.Contains("lis 3, 0xCC00", discReaderSource, StringComparison.Ordinal);
        Assert.Contains("addi 3, 3, 0x6000", discReaderSource, StringComparison.Ordinal);
        Assert.Contains("li 5, 0x002A", discReaderSource, StringComparison.Ordinal);
        Assert.Contains("li 6, 0x0054", discReaderSource, StringComparison.Ordinal);
        Assert.Contains("TryCompleteRead(discInterface)", discReaderSource, StringComparison.Ordinal);
        Assert.DoesNotContain("__io_gcdvd.readSectors", discReaderSource, StringComparison.Ordinal);
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
        string discReaderSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeDiscReader.cpp"));

        Assert.Contains("GameCubeDiscFileSystem.cpp", makefileSource, StringComparison.Ordinal);
        Assert.Contains("GameCubeDiscReader.cpp", makefileSource, StringComparison.Ordinal);
        Assert.Contains("#include \"platform/gamecube/GameCubeDiscFileSystem.hpp\"", fileSource, StringComparison.Ordinal);
        Assert.Contains("GameCubeDiscFileSystem::CanHandlePath(fileName)", fileSource, StringComparison.Ordinal);
        Assert.Contains("GameCubeDiscFileSystem::Exists(fileName)", fileSource, StringComparison.Ordinal);
        Assert.Contains("GameCubeDiscFileSystem::CanHandlePath(filePath)", fileSource, StringComparison.Ordinal);
        Assert.Contains("GameCubeDiscFileSystem::OpenRead(filePath)", fileSource, StringComparison.Ordinal);
        Assert.Contains("FileStream(const uint8_t* data, size_t length);", fileStreamHeaderSource, StringComparison.Ordinal);
        Assert.Contains("bool ReadDiscRange(void* destination, std::size_t offset, std::size_t length)", discFileSystemSource, StringComparison.Ordinal);
        Assert.Contains("GameCubeDiscReader::ReadBytes(scratchBuffer, sectorIndex * DiscSectorSize, sectorsToRead * DiscSectorSize)", discFileSystemSource, StringComparison.Ordinal);
        Assert.DoesNotContain("__io_gcdvd.readSectors", discFileSystemSource, StringComparison.Ordinal);
        Assert.Contains("#include <ogc/dvd.h>", discReaderSource, StringComparison.Ordinal);
        Assert.Contains("TryCompleteRead(discInterface)", discReaderSource, StringComparison.Ordinal);
        Assert.Contains("WaitForReadCompletion(discInterface)", discReaderSource, StringComparison.Ordinal);
        Assert.DoesNotContain("__io_gcdvd.readSectors", discReaderSource, StringComparison.Ordinal);
        Assert.Contains("ReadDiscRange(buffer, discOffset, fileSize)", discFileSystemSource, StringComparison.Ordinal);
        Assert.Contains("FileStream* stream = nullptr;", discFileSystemSource, StringComparison.Ordinal);
        Assert.Contains("// FileStream copies memory-backed input", discFileSystemSource, StringComparison.Ordinal);
        Assert.Contains("const uint32_t fstOffset = ReadBigEndianU32(discHeader + 0x424);", discFileSystemSource, StringComparison.Ordinal);
        Assert.Contains("const uint32_t fstSize = ReadBigEndianU32(discHeader + 0x428);", discFileSystemSource, StringComparison.Ordinal);
        Assert.Contains("const char expectedAssetName[] = \"DemoDiscMainMenu.hasset\";", applicationSource, StringComparison.Ordinal);
        Assert.Contains("const std::string expectedPath = \"dvd:/cooked/scenes/DemoDiscMainMenu.hasset\";", applicationSource, StringComparison.Ordinal);
        Assert.Contains("foundDemoDiscMainMenu", applicationSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures packaged DVD byte-range reads use a bounded scratch buffer instead of allocating one sector span per file.
    /// </summary>
    [Fact]
    public void PackagedDiscBootSource_BoundsDvdReadScratchAllocation() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string applicationSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeApplication.cpp"));
        string discFileSystemSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeDiscFileSystem.cpp"));

        Assert.Contains("const std::size_t firstSectorIndex = offset / DiscSectorSize;", discFileSystemSource, StringComparison.Ordinal);
        Assert.Contains("const std::size_t sectorCount = lastSectorIndex - firstSectorIndex;", discFileSystemSource, StringComparison.Ordinal);
        Assert.Contains("constexpr std::size_t MaximumSectorsPerRead = 32;", discFileSystemSource, StringComparison.Ordinal);
        Assert.Contains("const std::size_t scratchBufferLength = MaximumSectorsPerRead * DiscSectorSize;", discFileSystemSource, StringComparison.Ordinal);
        Assert.Contains("GameCubeDiscReader::ReadBytes(scratchBuffer, sectorIndex * DiscSectorSize, sectorsToRead * DiscSectorSize)", discFileSystemSource, StringComparison.Ordinal);
        Assert.DoesNotContain("const std::size_t sectorBufferLength = sectorCount * DiscSectorSize;", discFileSystemSource, StringComparison.Ordinal);
        Assert.Contains("const std::size_t firstSectorIndex = offset / DiscSectorSize;", applicationSource, StringComparison.Ordinal);
        Assert.Contains("const std::size_t sectorCount = lastSectorIndex - firstSectorIndex;", applicationSource, StringComparison.Ordinal);
        Assert.Contains("GameCubeDiscReader::ReadBytes(sectorBuffer, firstSectorIndex * DiscSectorSize, sectorCount * DiscSectorSize)", applicationSource, StringComparison.Ordinal);
        Assert.DoesNotContain("__io_gcdvd.readSectors(static_cast<sec_t>(sectorIndex), 1, sectorBuffer)", applicationSource, StringComparison.Ordinal);
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
        Assert.Contains("const bool hasActivePort0State", source, StringComparison.Ordinal);
        Assert.Contains("const bool port0Connected", source, StringComparison.Ordinal);
        Assert.Contains("gamepadState.set_Connected(padStatus.err == PAD_ERR_NONE);", source, StringComparison.Ordinal);
        Assert.Contains("gamepadState.SetButtonDown(InputGamepadButton::DPadUp", source, StringComparison.Ordinal);
        Assert.Contains("gamepadState.SetButtonDown(InputGamepadButton::DPadDown", source, StringComparison.Ordinal);
        Assert.Contains("gamepadState.SetButtonDown(InputGamepadButton::DPadLeft", source, StringComparison.Ordinal);
        Assert.Contains("gamepadState.SetButtonDown(InputGamepadButton::DPadRight", source, StringComparison.Ordinal);
        Assert.Contains("gamepadState.SetButtonDown(InputGamepadButton::South", source, StringComparison.Ordinal);
        Assert.Contains("gamepadState.SetButtonDown(InputGamepadButton::East", source, StringComparison.Ordinal);
        Assert.Contains("gamepadState.SetButtonDown(InputGamepadButton::Start", source, StringComparison.Ordinal);
        Assert.Contains("gamepadState.set_LeftStickY(static_cast<int16_t>(-padStatus.stickY * 256));", source, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures the retail input branch preserves libogc controller initialization and polling when the Nintendont probe fails.
    /// </summary>
    [Fact]
    public void GameCubeInputManager_RetailInputBranchPreservesLibogcTransport() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string source = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeInputManager.cpp"));

        Assert.Contains("PAD_Init();", source, StringComparison.Ordinal);
        Assert.Contains("PAD_ScanPads();", source, StringComparison.Ordinal);
        Assert.Contains("} else {\n            PAD_Init();\n        }", source, StringComparison.Ordinal);
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
        Assert.Contains("void ConfigureLitPipeline(bool useTexturedBranch, bool useIndexedGeometry);", rasterRendererHeaderSource, StringComparison.Ordinal);
        Assert.Contains("void ConfigureDirectionalLight(GameCubeFramePlan* framePlan, GXLightObj& lightObject, GXColor& ambientColor, bool& hasDirectionalLight);", rasterRendererHeaderSource, StringComparison.Ordinal);
        Assert.Contains("submission->get_Material()", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("GameCubeRuntimeMaterial* gameCubeRuntimeMaterial = static_cast<GameCubeRuntimeMaterial*>(material);", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("const float3 baseColor = material->GetBaseColor();", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("GX_SetChanMatColor(GX_COLOR0A0, ConvertLightingColorToGx(baseColor));", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("GX_Normal1x16(", rasterRendererSource, StringComparison.Ordinal);
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
        string platformDefinitionSource = File.ReadAllText(Path.Combine(repositoryRootPath, "builder", "GameCubePlatformDefinitionFactory.cs"));
        string generatedResolverSource = File.ReadAllText(Path.Combine(repositoryRootPath, "tmp", "generated-core-gamecube", "RuntimeSceneAssetReferenceResolver.cpp"));
        string makefileSource = File.ReadAllText(Path.Combine(repositoryRootPath, "Makefile"));

        Assert.Contains("const std::string& GetTextureRelativePath() const;", runtimeMaterialHeaderSource, StringComparison.Ordinal);
        Assert.Contains("void SetTextureRelativePath(std::string value);", runtimeMaterialHeaderSource, StringComparison.Ordinal);
        Assert.Contains("TextureRelativePathValue", runtimeMaterialSource, StringComparison.Ordinal);
        Assert.Contains("#include \"MaterialPropertyBlock.hpp\"", renderManagerSource, StringComparison.Ordinal);
        Assert.Contains("#include \"TextureAsset.hpp\"", renderManagerSource, StringComparison.Ordinal);
        Assert.Contains("runtimeMaterial->SetTextureRelativePath(materialAsset->TextureRelativePath);", renderManagerSource, StringComparison.Ordinal);
        Assert.Contains("class GameCubeRuntimeTexture final : public RuntimeTexture", runtimeTextureHeaderSource, StringComparison.Ordinal);
        Assert.Contains("void LoadFromRaw(TextureAsset* data);", runtimeTextureHeaderSource, StringComparison.Ordinal);
        Assert.Contains("GX_InitTexObj(&NativeTextureObject", runtimeTextureSource, StringComparison.Ordinal);
        Assert.Contains("GX_InitTexObjFilterMode(&NativeTextureObject, GX_LINEAR, GX_LINEAR);", runtimeTextureSource, StringComparison.Ordinal);
        Assert.Contains("#include \"Entity.hpp\"", renderManager2DSource, StringComparison.Ordinal);
        Assert.Contains("#include \"IRenderQueue2D.hpp\"", renderManager2DSource, StringComparison.Ordinal);
        Assert.Contains("#include \"system/io/file.hpp\"", renderManager2DSource, StringComparison.Ordinal);
        Assert.Contains("GameCubeRuntimeTexture* runtimeTexture = new GameCubeRuntimeTexture();", renderManager2DSource, StringComparison.Ordinal);
        Assert.Contains("RuntimeMaterialResolutionMode.CookedPlatformOwned", platformDefinitionSource, StringComparison.Ordinal);
        Assert.Contains("PackagedPathPolicy.ContentRelativeOnly", platformDefinitionSource, StringComparison.Ordinal);
        Assert.Contains("BuildMaterialFromCooked(generatedFullPath)", generatedResolverSource, StringComparison.Ordinal);
        Assert.Contains("BuildMaterialFromCooked(fullPath)", generatedResolverSource, StringComparison.Ordinal);
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
    /// Ensures the GameCube host advances the generated engine with libogc's monotonic timebase instead of relying on the generic stopwatch runtime.
    /// </summary>
    [Fact]
    public void GameCubeApplication_WhenAdvancingGeneratedEngine_UsesLibogcFrameDelta() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string applicationSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeApplication.cpp"));
        string applicationHeaderSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeApplication.hpp"));

        Assert.Contains("const u64 currentFrameTicks = gettime();", applicationSource, StringComparison.Ordinal);
        Assert.Contains("ticks_to_millisecs(currentFrameTicks - PreviousFrameTicks)", applicationSource, StringComparison.Ordinal);
        Assert.Contains("EngineCore->Update(elapsedSeconds);", applicationSource, StringComparison.Ordinal);
        Assert.Contains("u64 PreviousFrameTicks;", applicationHeaderSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures direct frame diagnostics report the host clock and loaded scene count without changing retail builds.
    /// </summary>
    [Fact]
    public void GameCubeApplication_WhenDirectFrameDiagnosticsAreEnabled_ReportsRuntimeTelemetry() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string applicationSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeApplication.cpp"));
        string applicationHeaderSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeApplication.hpp"));

        Assert.Contains("ReportRuntimeFrameTelemetry(LastElapsedFrameSeconds);", applicationSource, StringComparison.Ordinal);
        Assert.Contains("[GC] Runtime telemetry", applicationSource, StringComparison.Ordinal);
        Assert.Contains("[GC] EFB pixels", applicationSource, StringComparison.Ordinal);
        Assert.Contains("GX_PeekARGB(ProbeCenterSampleX, ProbeCenterSampleY, &centerColor);", applicationSource, StringComparison.Ordinal);
        Assert.Contains("scene0=%s scene1=%s", applicationSource, StringComparison.Ordinal);
        Assert.Contains("[GC] 2D rectangle", applicationSource, StringComparison.Ordinal);
        Assert.Contains("[GC] 2D queue counts", applicationSource, StringComparison.Ordinal);
        Assert.Contains("PresentedFrameCount % 60U", applicationSource, StringComparison.Ordinal);
        Assert.True(
            applicationSource.IndexOf("ReportRuntimeFrameTelemetry(LastElapsedFrameSeconds);", StringComparison.Ordinal)
                > applicationSource.IndexOf("EngineCore->Draw();", StringComparison.Ordinal));
        Assert.Contains("void ReportRuntimeFrameTelemetry(double elapsedSeconds);", applicationHeaderSource, StringComparison.Ordinal);
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
    /// Ensures the GameCube host only uses generated-core lifecycle APIs that exist in the checked-in generated output.
    /// </summary>
    [Fact]
    public void PackagedDiscBootSource_DoesNotDependOnMissingGeneratedCoreFrameBoundaryApis() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string applicationSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeApplication.cpp"));
        string applicationHeaderSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeApplication.hpp"));

        Assert.DoesNotContain("CommitPendingSceneOperationsDuringDraw", applicationSource, StringComparison.Ordinal);
        Assert.DoesNotContain("CompleteFrameBoundary", applicationSource, StringComparison.Ordinal);
        Assert.DoesNotContain("FinalizePresentedFrame", applicationSource, StringComparison.Ordinal);
        Assert.DoesNotContain("FinalizePresentedFrame", applicationHeaderSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures the GameCube host registers generated runtime modules only when the generated core exported the generic runtime module registration header.
    /// </summary>
    [Fact]
    public void PackagedDiscBootSource_RegistersGeneratedRuntimeModulesConditionallyBeforeStartupSceneLoad() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string makefileSource = File.ReadAllText(Path.Combine(repositoryRootPath, "Makefile"));
        string applicationSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeApplication.cpp"));

        Assert.Contains("HELENGINE_GAMECUBE_HAS_GENERATED_RUNTIME_MODULE_REGISTRATION", makefileSource, StringComparison.Ordinal);
        Assert.Contains("#if HELENGINE_GAMECUBE_HAS_GENERATED_RUNTIME_MODULE_REGISTRATION", applicationSource, StringComparison.Ordinal);
        Assert.Contains("#include \"GeneratedRuntimeModuleRegistration.hpp\"", applicationSource, StringComparison.Ordinal);
        Assert.Contains("RegisterGeneratedRuntimeModules(EngineCore);", applicationSource, StringComparison.Ordinal);
        Assert.Contains("EngineCore->get_SceneManager()->LoadScene(packagedStartupSceneId, SceneLoadMode::Single);", applicationSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures the generated runtime-module registration can be disabled for one diagnostic build without changing the default retail behavior.
    /// </summary>
    [Fact]
    public void PackagedDiscBootSource_CanDisableGeneratedRuntimeModuleRegistrationForDiagnostics() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string applicationSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeApplication.cpp"));
        string makefileSource = File.ReadAllText(Path.Combine(repositoryRootPath, "Makefile"));
        string builderPathsSource = File.ReadAllText(Path.Combine(repositoryRootPath, "builder", "GameCubeBuilderPaths.cs"));
        string nativeBuildExecutorSource = File.ReadAllText(Path.Combine(repositoryRootPath, "builder", "GameCubeDockerNativeBuildExecutor.cs"));

        Assert.Contains("HELENGINE_GAMECUBE_GENERATED_RUNTIME_MODULE_REGISTRATION_ENABLED ?= 1", makefileSource, StringComparison.Ordinal);
        Assert.Contains("-DHELENGINE_GAMECUBE_GENERATED_RUNTIME_MODULE_REGISTRATION_ENABLED=$(HELENGINE_GAMECUBE_GENERATED_RUNTIME_MODULE_REGISTRATION_ENABLED)", makefileSource, StringComparison.Ordinal);
        Assert.Contains("#if HELENGINE_GAMECUBE_HAS_GENERATED_RUNTIME_MODULE_REGISTRATION && HELENGINE_GAMECUBE_GENERATED_RUNTIME_MODULE_REGISTRATION_ENABLED", applicationSource, StringComparison.Ordinal);
        Assert.Contains("enable-generated-runtime-module-registration", builderPathsSource, StringComparison.Ordinal);
        Assert.Contains("GeneratedRuntimeModuleRegistrationEnabled", builderPathsSource, StringComparison.Ordinal);
        Assert.Contains("HELENGINE_GAMECUBE_GENERATED_RUNTIME_MODULE_REGISTRATION_ENABLED=", nativeBuildExecutorSource, StringComparison.Ordinal);
        Assert.Contains("paths.GeneratedRuntimeModuleRegistrationEnabled", nativeBuildExecutorSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures system reports can be disabled through a link-time wrapper when an EXI-safe diagnostic build is required.
    /// </summary>
    [Fact]
    public void PackagedDiscBootSource_CanDisableSystemReportForDiagnostics() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string makefileSource = File.ReadAllText(Path.Combine(repositoryRootPath, "Makefile"));
        string builderPathsSource = File.ReadAllText(Path.Combine(repositoryRootPath, "builder", "GameCubeBuilderPaths.cs"));
        string nativeBuildExecutorSource = File.ReadAllText(Path.Combine(repositoryRootPath, "builder", "GameCubeDockerNativeBuildExecutor.cs"));
        string systemReportWrapperPath = Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeSystemReport.cpp");

        Assert.Contains("HELENGINE_GAMECUBE_SYSTEM_REPORT_ENABLED ?= 1", makefileSource, StringComparison.Ordinal);
        Assert.Contains("-DHELENGINE_GAMECUBE_SYSTEM_REPORT_ENABLED=$(HELENGINE_GAMECUBE_SYSTEM_REPORT_ENABLED)", makefileSource, StringComparison.Ordinal);
        Assert.Contains("-Wl,--wrap=SYS_Report", makefileSource, StringComparison.Ordinal);
        Assert.Contains("GameCubeSystemReport.cpp", makefileSource, StringComparison.Ordinal);
        Assert.Contains("enable-system-report", builderPathsSource, StringComparison.Ordinal);
        Assert.Contains("SystemReportEnabled", builderPathsSource, StringComparison.Ordinal);
        Assert.Contains("HELENGINE_GAMECUBE_SYSTEM_REPORT_ENABLED=", nativeBuildExecutorSource, StringComparison.Ordinal);
        Assert.Contains("paths.SystemReportEnabled", nativeBuildExecutorSource, StringComparison.Ordinal);
        Assert.True(File.Exists(systemReportWrapperPath));
    }

    /// <summary>
    /// Ensures an opt-in direct-XFB diagnostic can render numeric breadcrumbs without using engine text or libogc system reports.
    /// </summary>
    [Fact]
    public void PackagedDiscBootSource_CanEnableDirectFrameDiagnostic() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string makefileSource = File.ReadAllText(Path.Combine(repositoryRootPath, "Makefile"));
        string builderPathsSource = File.ReadAllText(Path.Combine(repositoryRootPath, "builder", "GameCubeBuilderPaths.cs"));
        string nativeBuildExecutorSource = File.ReadAllText(Path.Combine(repositoryRootPath, "builder", "GameCubeDockerNativeBuildExecutor.cs"));
        string applicationSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeApplication.cpp"));
        string applicationHeaderSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeApplication.hpp"));

        Assert.Contains("HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC ?= 0", makefileSource, StringComparison.Ordinal);
        Assert.Contains("-DHELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC=$(HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC)", makefileSource, StringComparison.Ordinal);
        Assert.Contains("enable-direct-frame-diagnostic", builderPathsSource, StringComparison.Ordinal);
        Assert.Contains("DirectFrameDiagnosticEnabled", builderPathsSource, StringComparison.Ordinal);
        Assert.Contains("HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC=", nativeBuildExecutorSource, StringComparison.Ordinal);
        Assert.Contains("paths.DirectFrameDiagnosticEnabled", nativeBuildExecutorSource, StringComparison.Ordinal);
        Assert.Contains("DisplayDirectFrameDiagnosticCode", applicationSource, StringComparison.Ordinal);
        Assert.Contains("VIDEO_SetPostRetraceCallback", applicationSource, StringComparison.Ordinal);
        Assert.Contains("DisplayDirectFrameDiagnosticCode", applicationHeaderSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures the bounded logo-animation diagnostic can be enabled without enabling the continuous direct-frame overlay.
    /// </summary>
    [Fact]
    public void PackagedDiscBootSource_CanEnableLatchedLogoAnimationDiagnostic() {
        string repositoryRootPath = Environment.GetEnvironmentVariable("HELENGINE_GAMECUBE_REPOSITORY_ROOT");
        if (string.IsNullOrWhiteSpace(repositoryRootPath)) {
            repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        }

        string makefileSource = File.ReadAllText(Path.Combine(repositoryRootPath, "Makefile"));
        string builderPathsSource = File.ReadAllText(Path.Combine(repositoryRootPath, "builder", "GameCubeBuilderPaths.cs"));
        string nativeBuildExecutorSource = File.ReadAllText(Path.Combine(repositoryRootPath, "builder", "GameCubeDockerNativeBuildExecutor.cs"));
        string applicationSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeApplication.cpp"));
        string rasterRendererSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRasterRenderer.cpp"));

        Assert.Contains("HELENGINE_GAMECUBE_LOGO_ANIMATION_DIAGNOSTIC ?= 0", makefileSource, StringComparison.Ordinal);
        Assert.Contains("-DHELENGINE_GAMECUBE_LOGO_ANIMATION_DIAGNOSTIC=$(HELENGINE_GAMECUBE_LOGO_ANIMATION_DIAGNOSTIC)", makefileSource, StringComparison.Ordinal);
        Assert.Contains("enable-logo-animation-diagnostic", builderPathsSource, StringComparison.Ordinal);
        Assert.Contains("LogoAnimationDiagnosticEnabled", builderPathsSource, StringComparison.Ordinal);
        Assert.Contains("HELENGINE_GAMECUBE_LOGO_ANIMATION_DIAGNOSTIC=", nativeBuildExecutorSource, StringComparison.Ordinal);
        Assert.Contains("paths.LogoAnimationDiagnosticEnabled", nativeBuildExecutorSource, StringComparison.Ordinal);
        Assert.Contains("LatchLogoAnimationDiagnosticCode", applicationSource, StringComparison.Ordinal);
        Assert.Contains("LatchLogoAnimationDiagnosticCode", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("0xA000U", rasterRendererSource, StringComparison.Ordinal);
        Assert.DoesNotContain("minimumLogoDimension", rasterRendererSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures controller tracing is opt-in and reports only the first nonzero port-zero state.
    /// </summary>
    [Fact]
    public void PackagedDiscBootSource_CanEnableInputTraceDiagnostic() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string makefileSource = File.ReadAllText(Path.Combine(repositoryRootPath, "Makefile"));
        string builderPathsSource = File.ReadAllText(Path.Combine(repositoryRootPath, "builder", "GameCubeBuilderPaths.cs"));
        string nativeBuildExecutorSource = File.ReadAllText(Path.Combine(repositoryRootPath, "builder", "GameCubeDockerNativeBuildExecutor.cs"));
        string inputManagerSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeInputManager.cpp"));

        Assert.Contains("HELENGINE_GAMECUBE_INPUT_TRACE_DIAGNOSTIC ?= 0", makefileSource, StringComparison.Ordinal);
        Assert.Contains("-DHELENGINE_GAMECUBE_INPUT_TRACE_DIAGNOSTIC=$(HELENGINE_GAMECUBE_INPUT_TRACE_DIAGNOSTIC)", makefileSource, StringComparison.Ordinal);
        Assert.Contains("enable-input-trace-diagnostic", builderPathsSource, StringComparison.Ordinal);
        Assert.Contains("InputTraceDiagnosticEnabled", builderPathsSource, StringComparison.Ordinal);
        Assert.Contains("HELENGINE_GAMECUBE_INPUT_TRACE_DIAGNOSTIC=", nativeBuildExecutorSource, StringComparison.Ordinal);
        Assert.Contains("paths.InputTraceDiagnosticEnabled", nativeBuildExecutorSource, StringComparison.Ordinal);
        Assert.Contains("Input trace port0", inputManagerSource, StringComparison.Ordinal);
        Assert.Contains("hasReportedInput", inputManagerSource, StringComparison.Ordinal);
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
        Assert.Contains("bool HasCapturedDrawablesForCamera(CameraComponent* camera) const;", headerSource, StringComparison.Ordinal);
        Assert.Contains("const std::vector<GameCubeSpriteDrawCommand>& GetSpriteQueue() const;", headerSource, StringComparison.Ordinal);
        Assert.Contains("const std::vector<GameCubeTextDrawCommand>& GetTextQueue() const;", headerSource, StringComparison.Ordinal);
        Assert.Contains("const std::vector<GameCubeRoundedRectDrawCommand>& GetRoundedRectQueue() const;", headerSource, StringComparison.Ordinal);
        Assert.DoesNotContain("(void)sprite;", source, StringComparison.Ordinal);
        Assert.DoesNotContain("(void)text;", source, StringComparison.Ordinal);
        Assert.DoesNotContain("(void)shape;", source, StringComparison.Ordinal);
        Assert.Contains("SpriteQueue.clear();", source, StringComparison.Ordinal);
        Assert.Contains("TextQueue.clear();", source, StringComparison.Ordinal);
        Assert.Contains("RoundedRectQueue.clear();", source, StringComparison.Ordinal);
        Assert.Contains("SpriteQueue.push_back(GameCubeSpriteDrawCommand { ActiveCaptureCamera, sprite });", source, StringComparison.Ordinal);
        Assert.Contains("TextQueue.push_back(GameCubeTextDrawCommand { ActiveCaptureCamera, text });", source, StringComparison.Ordinal);
        Assert.Contains("RoundedRectQueue.push_back(GameCubeRoundedRectDrawCommand { ActiveCaptureCamera, shape });", source, StringComparison.Ordinal);
        Assert.Contains("ActiveCaptureCamera = camera;", source, StringComparison.Ordinal);
        Assert.Contains("for (int32_t cameraIndex = 0; cameraIndex < cameras->get_Count(); cameraIndex++)", source, StringComparison.Ordinal);
        Assert.DoesNotContain("camera->get_RenderQueue2D()->VisitOrdered(this);\n            return;", source, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures the GameCube raster renderer exposes a GX-backed 2D pass for menu drawables.
    /// </summary>
    [Fact]
    public void GameCubeRasterRenderer_WhenRenderingMenu_Contains2DGxEntryPoints() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRasterRenderer.hpp"));
        string source = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRasterRenderer.cpp"));
        int render2DStartIndex = source.IndexOf("void GameCubeRasterRenderer::Render2D", StringComparison.Ordinal);
        int render2DEndIndex = source.IndexOf("void GameCubeRasterRenderer::ConfigurePipeline", render2DStartIndex, StringComparison.Ordinal);
        string render2DSource = source.Substring(render2DStartIndex, render2DEndIndex - render2DStartIndex);

        Assert.Contains("void Render2D(", headerSource, StringComparison.Ordinal);
        Assert.Contains("void TransformLogicalRectToPhysicalViewport(GameCubeFramePlan* framePlan, float& x, float& y, float& width, float& height) const;", headerSource, StringComparison.Ordinal);
        Assert.Contains("RenderRoundedRect2D(", headerSource, StringComparison.Ordinal);
        Assert.Contains("RenderSprite2D(", headerSource, StringComparison.Ordinal);
        Assert.Contains("RenderText2D(", headerSource, StringComparison.Ordinal);
        Assert.Contains("Render2D(GameCubeFramePlan* framePlan, const GameCubeRenderManager2D& renderManager2D)", source, StringComparison.Ordinal);
        Assert.Contains("RenderRoundedRect2D(framePlan, command);", source, StringComparison.Ordinal);
        Assert.Contains("RenderSprite2D(framePlan, command);", source, StringComparison.Ordinal);
        Assert.Contains("RenderText2D(framePlan, command);", source, StringComparison.Ordinal);
        Assert.Contains("Configure2DProjection(framePlan);", source, StringComparison.Ordinal);
        Assert.DoesNotContain("ConfigureCameraViewport2D(framePlan);\n#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC", source, StringComparison.Ordinal);
        Assert.Contains("if (framePlan->DrawableSubmissions->get_Count() <= 0)", render2DSource, StringComparison.Ordinal);
        Assert.Contains("DrawSolidQuad2D(x, y, width, height, ResolveClearColor(clearSettings));", render2DSource, StringComparison.Ordinal);
        Assert.Contains("GX_LoadProjectionMtx(projectionMatrix, GX_ORTHOGRAPHIC);", source, StringComparison.Ordinal);
        Assert.Contains("[GC] Overlay GX probe", source, StringComparison.Ordinal);
        Assert.Contains("[GC] Overlay first rectangle", source, StringComparison.Ordinal);
        Assert.Contains("[GC] Overlay queue summary", source, StringComparison.Ordinal);
        Assert.Contains("[GC] Overlay EFB after camera", source, StringComparison.Ordinal);
        Assert.Contains("[GC] Overlay EFB after probe", source, StringComparison.Ordinal);
        Assert.Contains("[GC] Overlay first rectangle", source, StringComparison.Ordinal);
        Assert.Contains("DrawSolidQuad2D(0.0f, 0.0f, 64.0f, 64.0f, GXColor { 0xFF, 0x00, 0x00, 0xFF });", source, StringComparison.Ordinal);
        Assert.Contains("if (borderColor.a != 0U && borderThickness > 0.0f)", source, StringComparison.Ordinal);
        Assert.Contains("if (fillColor.a != 0U && innerWidth > 0.0f && innerHeight > 0.0f)", source, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures the GameCube renderer constructs and executes one frame plan per enabled camera without replaying one camera's 2D commands through another camera.
    /// </summary>
    [Fact]
    public void GameCubeRenderer_WhenMultipleCamerasAreActive_ExecutesEachCameraPlan() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sceneBridgeHeaderSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeSceneRenderBridge.hpp"));
        string sceneBridgeSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeSceneRenderBridge.cpp"));
        string renderManagerSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRenderManager3D.cpp"));
        string renderManager2DHeaderSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRenderManager2D.hpp"));
        string rasterRendererSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRasterRenderer.cpp"));

        Assert.Contains("List<GameCubeFramePlan*>* BuildFramePlans(", sceneBridgeHeaderSource, StringComparison.Ordinal);
        Assert.DoesNotContain("ResolveActiveCamera", sceneBridgeHeaderSource, StringComparison.Ordinal);
        Assert.Contains("framePlans->Add(BuildFramePlanForCamera(", sceneBridgeSource, StringComparison.Ordinal);
        Assert.Contains("SortFramePlansByCameraDrawOrder(framePlans);", sceneBridgeSource, StringComparison.Ordinal);
        Assert.Contains("get_CameraDrawOrder() > framePlan->Camera->get_CameraDrawOrder()", sceneBridgeSource, StringComparison.Ordinal);
        Assert.Contains("BuildFramePlans(CapabilityProfile", renderManagerSource, StringComparison.Ordinal);
        Assert.Contains("for (int32_t framePlanIndex = 0; framePlanIndex < framePlans->get_Count(); framePlanIndex++)", renderManagerSource, StringComparison.Ordinal);
        Assert.Contains("CameraComponent* Camera;", renderManager2DHeaderSource, StringComparison.Ordinal);
        Assert.Contains("command.Camera != framePlan->Camera", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("if (framePlan->DrawableSubmissions->get_Count() <= 0)", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("DrawSolidQuad2D(x, y, width, height, ResolveClearColor(clearSettings));", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("[GC] Camera plan", rasterRendererSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures the GameCube text renderer submits the authored shadow and outline passes before the primary glyph pass.
    /// </summary>
    [Fact]
    public void GameCubeRasterRenderer_WhenRenderingText_RendersAuthoredShadowAndOutlinePasses() {
        string repositoryRootPath = Environment.GetEnvironmentVariable("HELENGINE_GAMECUBE_REPOSITORY_ROOT");
        if (string.IsNullOrWhiteSpace(repositoryRootPath)) {
            repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        }

        string headerSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRasterRenderer.hpp"));
        string source = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRasterRenderer.cpp"));

        Assert.Contains("void DrawTextGlyphPass2D(", headerSource, StringComparison.Ordinal);
        Assert.Contains("drawable->get_ShadowOffset()", source, StringComparison.Ordinal);
        Assert.Contains("drawable->get_ShadowColor()", source, StringComparison.Ordinal);
        Assert.Contains("drawable->get_OutlineScale()", source, StringComparison.Ordinal);
        Assert.Contains("drawable->get_OutlineColor()", source, StringComparison.Ordinal);
        Assert.Contains("DrawTextGlyphPass2D(framePlan, font, texture, content, fontScale, baseX, baseY, shadowOffset, shadowColor);", source, StringComparison.Ordinal);
        Assert.Contains("DrawTextGlyphPass2D(framePlan, font, texture, content, fontScale, baseX, baseY, float2(-outlineScale, 0.0f), outlineColor);", source, StringComparison.Ordinal);
        Assert.Contains("DrawTextGlyphPass2D(framePlan, font, texture, content, fontScale, baseX, baseY, float2(0.0f, outlineScale), outlineColor);", source, StringComparison.Ordinal);
        Assert.Contains("DrawTextGlyphPass2D(framePlan, font, texture, content, fontScale, baseX, baseY, float2(0.0f, 0.0f), glyphColor);", source, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures the GameCube sprite overlay path applies entity scale and centered 2D orientation instead of rendering every sprite as an axis-aligned quad.
    /// </summary>
    [Fact]
    public void GameCubeRasterRenderer_WhenRenderingSprites_AppliesEntityScaleAndOrientation() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRasterRenderer.hpp"));
        string source = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRasterRenderer.cpp"));

        Assert.Contains("void DrawTransformedTexturedQuad2D(float centerX, float centerY, float width, float height, float rotation, const float4& sourceRect, GXColor color, GameCubeRuntimeTexture* texture);", headerSource, StringComparison.Ordinal);
        Assert.Contains("float3 scale = drawable->get_Parent()->get_Scale();", source, StringComparison.Ordinal);
        Assert.Contains("width *= scale.X;", source, StringComparison.Ordinal);
        Assert.Contains("height *= scale.Y;", source, StringComparison.Ordinal);
        Assert.Contains("float4 orientation = drawable->get_Parent()->get_Orientation();", source, StringComparison.Ordinal);
        Assert.Contains("float3 rotatedRight = float4::RotateVector(float3(1.0f, 0.0f, 0.0f), orientation);", source, StringComparison.Ordinal);
        Assert.Contains("const float rotation = std::atan2(rotatedRight.Y, rotatedRight.X);", source, StringComparison.Ordinal);
        Assert.Contains("DrawTransformedTexturedQuad2D(centerX, centerY, width, height, rotation, drawable->get_SourceRect(), ConvertByteColorToGx(drawable->get_Color()), texture);", source, StringComparison.Ordinal);
        Assert.DoesNotContain("DrawTexturedQuad2D(x, y, width, height, drawable->get_SourceRect(), ConvertByteColorToGx(drawable->get_Color()), texture);", source, StringComparison.Ordinal);
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
        Assert.Contains("EngineRenderManager3D->SetOverlayRenderManager2D(EngineRenderManager2D);", applicationSource, StringComparison.Ordinal);
        Assert.Contains("Scenes/DemoDiscMainMenu.helen", bootstrapSource, StringComparison.Ordinal);
        Assert.Contains("const std::string startupSceneAliasId = \"DemoDiscMainMenu\";", bootstrapSource, StringComparison.Ordinal);
        Assert.Contains("bool startupSceneSourceExists = false;", bootstrapSource, StringComparison.Ordinal);
        Assert.Contains("const bool shouldAddStartupSceneAlias = !startupSceneAliasExists && startupSceneSourceExists;", bootstrapSource, StringComparison.Ordinal);
        Assert.Contains("const std::size_t runtimeEntryCount = shouldAddStartupSceneAlias ? entryCount + 1U : entryCount;", bootstrapSource, StringComparison.Ordinal);
        Assert.Contains("if (shouldAddStartupSceneAlias && StartupSceneId == entries[index].SceneId) {", bootstrapSource, StringComparison.Ordinal);
        Assert.Contains("new RuntimeSceneCatalogEntry(startupSceneAliasId, entries[index].CookedRelativePath)", bootstrapSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures the GameCube host completes the 2D overlay pass inside the render-manager draw call before shared scene commits run.
    /// </summary>
    [Fact]
    public void GameCubeFrameBoundarySource_Executes2DOverlayInsideRenderManagerDraw() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string applicationSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeApplication.cpp"));
        string renderManagerHeaderSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRenderManager3D.hpp"));
        string renderManagerSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRenderManager3D.cpp"));
        string rasterRendererHeaderSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRasterRenderer.hpp"));
        string rasterRendererSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRasterRenderer.cpp"));

        Assert.Contains("void SetOverlayRenderManager2D(GameCubeRenderManager2D* renderManager2D);", renderManagerHeaderSource, StringComparison.Ordinal);
        Assert.Contains("GameCubeRenderManager2D* OverlayRenderManager2D;", renderManagerHeaderSource, StringComparison.Ordinal);
        Assert.Contains("void Render2D(GameCubeFramePlan* framePlan, const GameCubeRenderManager2D& renderManager2D);", rasterRendererHeaderSource, StringComparison.Ordinal);
        Assert.Contains("void PrepareOverlayViewport(GameCubeFramePlan* framePlan);", rasterRendererHeaderSource, StringComparison.Ordinal);
        Assert.Contains("OverlayRenderManager2D->Draw();", renderManagerSource, StringComparison.Ordinal);
        Assert.Contains("RasterRenderer->Render2D(framePlan, *OverlayRenderManager2D);", renderManagerSource, StringComparison.Ordinal);
        Assert.Contains("RasterRenderer->PrepareOverlayViewport(framePlan);", renderManagerSource, StringComparison.Ordinal);
        Assert.Contains("EngineRenderManager3D->SetOverlayRenderManager2D(EngineRenderManager2D);", applicationSource, StringComparison.Ordinal);
        Assert.DoesNotContain("EngineRenderManager3D->Draw2D(EngineRenderManager2D, RenderMode->fbWidth, RenderMode->efbHeight);", applicationSource, StringComparison.Ordinal);
        Assert.Contains("if (framePlan->DrawableSubmissions->get_Count() <= 0)", rasterRendererSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures the GameCube camera frame path matches the Wii logical-versus-physical viewport contract so GX presentation does not widen the captured scene.
    /// </summary>
    [Fact]
    public void GameCubeViewportSource_SeparatesLogicalAndPhysicalViewportState() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string applicationSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeApplication.cpp"));
        string renderManagerHeaderSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRenderManager3D.hpp"));
        string renderManagerSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRenderManager3D.cpp"));
        string framePlanHeaderSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeFramePlan.hpp"));
        string sceneRenderBridgeHeaderSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeSceneRenderBridge.hpp"));
        string sceneRenderBridgeSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeSceneRenderBridge.cpp"));
        string rasterRendererHeaderSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRasterRenderer.hpp"));
        string rasterRendererSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRasterRenderer.cpp"));

        Assert.Contains("void SetPresentedFrameSize(uint16_t width, uint16_t height);", renderManagerHeaderSource, StringComparison.Ordinal);
        Assert.Contains("uint16_t PresentedFrameWidth;", renderManagerHeaderSource, StringComparison.Ordinal);
        Assert.Contains("uint16_t PresentedFrameHeight;", renderManagerHeaderSource, StringComparison.Ordinal);
        Assert.Contains("EngineRenderManager3D->SetPresentedFrameSize(static_cast<uint16_t>(RenderMode->fbWidth), static_cast<uint16_t>(RenderMode->efbHeight));", applicationSource, StringComparison.Ordinal);
        Assert.Contains("float4 LogicalViewport;", framePlanHeaderSource, StringComparison.Ordinal);
        Assert.Contains("float4 PhysicalViewport;", framePlanHeaderSource, StringComparison.Ordinal);
        Assert.Contains("List<GameCubeFramePlan*>* BuildFramePlans(RendererBackendCapabilityProfile* capabilities, int32_t logicalWidth, int32_t logicalHeight, int32_t physicalWidth, int32_t physicalHeight);", sceneRenderBridgeHeaderSource, StringComparison.Ordinal);
        Assert.Contains("float4 physicalViewport = ResolvePhysicalViewport(logicalViewport, logicalWidth, logicalHeight, physicalWidth, physicalHeight);", sceneRenderBridgeSource, StringComparison.Ordinal);
        Assert.Contains("float4 ResolvePhysicalViewport(const float4& logicalViewport, int32_t logicalWidth, int32_t logicalHeight, int32_t physicalWidth, int32_t physicalHeight);", sceneRenderBridgeHeaderSource, StringComparison.Ordinal);
        Assert.Contains("float4x4 projection = BuildProjectionMatrix(camera, logicalViewport.Z / logicalViewport.W);", sceneRenderBridgeSource, StringComparison.Ordinal);
        Assert.Contains("List<GameCubeFramePlan*>* framePlans = SceneRenderBridge->BuildFramePlans(CapabilityProfile, MainWindowSize.X, MainWindowSize.Y, PresentedFrameWidth, PresentedFrameHeight);", renderManagerSource, StringComparison.Ordinal);
        Assert.Contains("GX_SetViewport(framePlan->PhysicalViewport.X", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("GX_SetScissor(", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("static_cast<u32>(framePlan->PhysicalViewport.X)", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("void CopyProjectionMatrixToGx(const float4x4& source, Mtx44& destination);", rasterRendererHeaderSource, StringComparison.Ordinal);
        Assert.Contains("CopyProjectionMatrixToGx(framePlan->Projection, projectionMatrix);", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("TransformLogicalRectToPhysicalViewport(framePlan, x, y, width, height);", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("x *= horizontalScale;", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("y *= verticalScale;", rasterRendererSource, StringComparison.Ordinal);
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
        Assert.Contains("const GameCubePackedTexCoord2 packedTextureCoordinate = (*cachedMeshData->PackedTexCoords)[cachedIndex];", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("GX_TexCoord2f32(textureCoordinate.X, textureCoordinate.Y);", rasterRendererSource, StringComparison.Ordinal);
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
        Assert.Contains("GX_SetVtxDesc(GX_VA_NRM, useIndexedGeometry ? GX_INDEX16 : GX_DIRECT);", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("GX_SetChanCtrl(GX_COLOR0A0", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("GX_InitLightPos(", rasterRendererSource, StringComparison.Ordinal);
        Assert.DoesNotContain("GX_InitLightDir(", rasterRendererSource, StringComparison.Ordinal);
        Assert.DoesNotContain("EvaluateLitVertexColor(framePlan, entity, material", rasterRendererSource, StringComparison.Ordinal);
        Assert.DoesNotContain("GX_Color4u8(litColor.r, litColor.g, litColor.b, litColor.a);", rasterRendererSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures the GameCube 3D mesh pipelines do not overwrite the material-resolved cull mode with a hardcoded front-face cull.
    /// </summary>
    [Fact]
    public void CachedMeshRenderSource_PreservesMaterialResolvedCullMode() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string rasterRendererSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRasterRenderer.cpp"));

        Assert.Contains("GX_SetCullMode(ResolveGxCullMode(material->get_RenderState()->get_CullMode()));", rasterRendererSource, StringComparison.Ordinal);
        Assert.DoesNotContain("GX_SetCullMode(GX_CULL_FRONT);", rasterRendererSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures the GameCube renderer preserves outward-facing authored triangles by mapping shared-engine cull modes onto GX's reversed face-culling convention.
    /// </summary>
    [Fact]
    public void CachedMeshRenderSource_UsesReversedGxCullMappings() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string rasterRendererSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRasterRenderer.cpp"));

        Assert.Contains("case MaterialCullMode::Back:\n                return GX_CULL_FRONT;", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("case MaterialCullMode::Front:\n                return GX_CULL_BACK;", rasterRendererSource, StringComparison.Ordinal);
        Assert.DoesNotContain("case MaterialCullMode::Back:\n                return GX_CULL_BACK;", rasterRendererSource, StringComparison.Ordinal);
        Assert.DoesNotContain("case MaterialCullMode::Front:\n                return GX_CULL_FRONT;", rasterRendererSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures the GameCube directional-light path emulates an authored directional ray by placing a non-attenuated GX light far away in view space instead of using the spotlight-only GX_InitLightDir helper.
    /// </summary>
    [Fact]
    public void CachedMeshRenderSource_EmulatesDirectionalLightUsingFarViewSpacePosition() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string rasterRendererSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRasterRenderer.cpp"));
        string directXRendererSource = File.ReadAllText(Path.Combine(repositoryRootPath, "..", "helengine", "engine", "helengine.directx11", "DirectX11Renderer3D.cs"));
        string forwardShaderSource = File.ReadAllText(Path.Combine(repositoryRootPath, "..", "helengine", "engine", "helengine.editor", "shaders", "builtin", "ForwardStandardShader.hlsl"));

        Assert.Contains("float3 lightDirection = float3.Normalize(LightDirectionUtility.GetEntityForwardDirection(directionalLight.Parent)) * -1f;", directXRendererSource, StringComparison.Ordinal);
        Assert.Contains("lightDirection = normalize(-directionAndShadow.xyz);", forwardShaderSource, StringComparison.Ordinal);
        Assert.Contains("directionalDirection = float4::RotateVector(float3(0.0f, 0.0f, -1.0f), lightOrientation);", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("directionalPosition = float3::Normalize(directionalDirection) * -1024.0f;", rasterRendererSource, StringComparison.Ordinal);
        Assert.Contains("GX_InitLightPos(&lightObject, directionalPosition.X, directionalPosition.Y, directionalPosition.Z);", rasterRendererSource, StringComparison.Ordinal);
        Assert.DoesNotContain("GX_InitLightDir(", rasterRendererSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures the GameCube 3D runtime emits explicit frame-plan drawable diagnostics so empty-scene fallback clears can be traced to the correct extraction stage.
    /// </summary>
    [Fact]
    public void GameCubeFramePlanSource_EmitsDrawableCountDiagnostics() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string renderManagerSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeRenderManager3D.cpp"));
        string sceneRenderBridgeSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeSceneRenderBridge.cpp"));

        Assert.Contains("[GC] Frame snapshot visibleDrawables=", sceneRenderBridgeSource, StringComparison.Ordinal);
        Assert.Contains("[GC] Frame extraction drawableSubmissions=", sceneRenderBridgeSource, StringComparison.Ordinal);
        Assert.Contains("[GC] Render frame has3D=", renderManagerSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures the builder no longer depends on the Windows GDI font importer project and no longer forces a Windows-only target framework.
    /// </summary>
    [Fact]
    public void BuilderProjectSource_DoesNotDependOnGdiImporterOrWindowsHost() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string builderProjectSource = File.ReadAllText(Path.Combine(repositoryRootPath, "builder", "helengine.gamecube.builder.csproj"));
        string builderTestsProjectSource = File.ReadAllText(Path.Combine(repositoryRootPath, "builder.tests", "helengine.gamecube.builder.tests.csproj"));

        Assert.Contains("<TargetFramework>net9.0</TargetFramework>", builderProjectSource, StringComparison.Ordinal);
        Assert.Contains("<TargetFramework>net9.0</TargetFramework>", builderTestsProjectSource, StringComparison.Ordinal);
        Assert.DoesNotContain("net9.0-windows", builderProjectSource, StringComparison.Ordinal);
        Assert.DoesNotContain("net9.0-windows", builderTestsProjectSource, StringComparison.Ordinal);
        Assert.DoesNotContain("helengine.editor.windows.gdiimporter", builderProjectSource, StringComparison.Ordinal);
        Assert.DoesNotContain("helengine.editor.windows\\helengine.editor.windows.csproj", builderProjectSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures the retail-style apploader accepts Nintendont's temporary low-memory entry trampoline while preserving its normal DOL text-entry validation.
    /// </summary>
    [Fact]
    public void RetailApploaderSource_AcceptsNintendontTemporaryEntrypoint() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string apploaderSource = File.ReadAllText(Path.Combine(repositoryRootPath, "third_party", "cubeboot-tools", "ppc", "apploader", "retail_apploader.c"));

        Assert.Contains("#define NINTENDONT_TEMPORARY_ENTRY_MIN 0x80001000", apploaderSource, StringComparison.Ordinal);
        Assert.Contains("#define NINTENDONT_TEMPORARY_ENTRY_MAX 0x80003000", apploaderSource, StringComparison.Ordinal);
        Assert.Contains("static int al_is_nintendont_temporary_entry_point(uint32_t entry_point)", apploaderSource, StringComparison.Ordinal);
        Assert.Contains("if (al_is_nintendont_temporary_entry_point(h->entry_point))", apploaderSource, StringComparison.Ordinal);
        Assert.Contains("valid = 1;", apploaderSource, StringComparison.Ordinal);
        Assert.Contains("panic(\"entry point out of text segment\\n\");", apploaderSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures native boot diagnostics distinguish completion of the generated core initialization-options getter from the scene-bootstrap work that follows it.
    /// </summary>
    [Fact]
    public void PackagedDiscBootSource_PresentsMarkerAfterCoreOptionsGetter() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string applicationSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeApplication.cpp"));

        Assert.Contains(
            "CoreInitializationOptions* options = EngineCore->get_InitializationOptions();\n            initializationStage = \"ReadInitializationOptionsCompleted\";\n            SetBootPhase(GameCubeBootPhase::CoreOptions, GXColor { 0x80, 0xFF, 0x00, 0xFF });\n            PresentBootFrame();",
            applicationSource,
            StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures native boot diagnostics present a boundary immediately before the packaged DVD bootstrap begins.
    /// </summary>
    [Fact]
    public void PackagedDiscBootSource_PresentsMarkerBeforePackagedDiscBootstrap() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string applicationSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeApplication.cpp"));
        string applicationHeaderSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeApplication.hpp"));

        Assert.Contains(
            "initializationStage = \"ConfigureSceneBootstrap\";\n            SetBootPhase(GameCubeBootPhase::SceneBootstrap, GXColor { 0x00, 0x40, 0x80, 0xFF });\n            PresentVideoBootFrame();\n#if HELENGINE_GAMECUBE_PACKAGED_DISC_BOOT\n            initializationStage = \"InitializePackagedDisc\";\n            SetBootPhase(GameCubeBootPhase::SceneBootstrap, GXColor { 0xFF, 0xFF, 0xFF, 0xFF });\n            PresentVideoBootFrame();\n            GameCubeSceneBootstrap::InitializePackagedDiscInterface();",
            applicationSource,
            StringComparison.Ordinal);
        Assert.Contains("void PresentVideoBootFrame();", applicationHeaderSource, StringComparison.Ordinal);
        Assert.Contains("void GameCubeApplication::PresentVideoBootFrame()", applicationSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures boot diagnostics copy each GX frame to the inactive external framebuffer before presenting it through VI.
    /// </summary>
    [Fact]
    public void PackagedDiscBootSource_AlternatesFramebuffersBeforeGxBootPresentation() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string applicationSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeApplication.cpp"));

        Assert.Contains(
            "        FrameBufferIndex ^= 1U;\n        GX_SetCopyClear(ClearColor, 0x00FFFFFF);\n        GX_CopyDisp(FrameBuffers[FrameBufferIndex], GX_TRUE);",
            applicationSource,
            StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures packaged-disc diagnostics distinguish DVD interface initialization from the direct-read validation that replaces a drive remount.
    /// </summary>
    [Fact]
    public void PackagedDiscBootSource_PresentsMarkersAroundDvdInitializationAndReadinessProbe() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string applicationSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeApplication.cpp"));
        string bootstrapHeaderSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeSceneBootstrap.hpp"));

        Assert.Contains(
            "GameCubeSceneBootstrap::InitializePackagedDiscInterface();\n            initializationStage = \"VerifyPackagedDiscReadiness\";\n            SetBootPhase(GameCubeBootPhase::SceneBootstrap, GXColor { 0xFF, 0xFF, 0x00, 0xFF });\n            PresentVideoBootFrame();\n            if (!GameCubeSceneBootstrap::VerifyPackagedDiscReadiness())",
            applicationSource,
            StringComparison.Ordinal);
        Assert.Contains(
            "initializationStage = \"PackagedDiscReadinessVerified\";\n            SetBootPhase(GameCubeBootPhase::SceneBootstrap, GXColor { 0x00, 0xFF, 0xFF, 0xFF });\n            PresentVideoBootFrame();",
            applicationSource,
            StringComparison.Ordinal);
        Assert.Contains("static void InitializePackagedDiscInterface();", bootstrapHeaderSource, StringComparison.Ordinal);
        Assert.Contains("static bool VerifyPackagedDiscReadiness();", bootstrapHeaderSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures bridge construction, window setup, and generated-core initialization have distinct VI diagnostic markers.
    /// </summary>
    [Fact]
    public void PackagedDiscBootSource_PresentsMarkersAroundGeneratedCoreInitialization() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string applicationSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeApplication.cpp"));

        Assert.DoesNotContain("DetectNintendontInputTransport", applicationSource, StringComparison.Ordinal);
        Assert.DoesNotContain("DetectHighDataBatSupport", applicationSource, StringComparison.Ordinal);
        Assert.Contains("initializationStage = \"InitializePlatformInput\";", applicationSource, StringComparison.Ordinal);
        Assert.Contains("GameCubeInputManager::InitializePlatformInput(\n#if HELENGINE_GAMECUBE_MEMORY_CARD_DIAGNOSTIC_JOURNAL\n                MemoryCardDiagnosticJournal\n#else\n                nullptr\n#endif\n            );", applicationSource, StringComparison.Ordinal);
        Assert.Contains("EngineInputManager = new GameCubeInputManager(\n#if HELENGINE_GAMECUBE_MEMORY_CARD_DIAGNOSTIC_JOURNAL\n                MemoryCardDiagnosticJournal\n#else\n                nullptr\n#endif\n            );", applicationSource, StringComparison.Ordinal);
        Assert.Contains("EnginePlatformInfo = new PlatformInfo(\"gamecube\", \"1.0.0\");", applicationSource, StringComparison.Ordinal);
        Assert.DoesNotContain(
            "initializationStage = \"ConstructRenderManager3D\";\n            SetBootPhase(GameCubeBootPhase::BridgeConstruction, GXColor { 0x00, 0x00, 0xFF, 0xFF });\n            PresentVideoBootFrame();\n            EngineRenderManager3D = new GameCubeRenderManager3D();\n\n            initializationStage = \"ConstructRenderManager2D\";\n            SetBootPhase(GameCubeBootPhase::BridgeConstruction, GXColor { 0xFF, 0x00, 0x00, 0xFF });\n            PresentVideoBootFrame();\n            EngineRenderManager2D = new GameCubeRenderManager2D();\n\n            initializationStage = \"ConnectRenderManagers\";\n            SetBootPhase(GameCubeBootPhase::BridgeConstruction, GXColor { 0xFF, 0xFF, 0x00, 0xFF });\n            PresentVideoBootFrame();\n            EngineRenderManager3D->SetOverlayRenderManager2D(EngineRenderManager2D);\n\n            initializationStage = \"InitializePlatformInput\";\n            SetBootPhase(GameCubeBootPhase::BridgeConstruction, GXColor { 0x00, 0xFF, 0x00, 0xFF });\n            PresentVideoBootFrame();\n            GameCubeInputManager::InitializePlatformInput();\n\n            initializationStage = \"ConstructInputManager\";\n            SetBootPhase(GameCubeBootPhase::BridgeConstruction, GXColor { 0x80, 0x00, 0xFF, 0xFF });\n            PresentVideoBootFrame();\n            EngineInputManager = new GameCubeInputManager();\n\n            initializationStage = \"ConstructAudioBackend\";\n            SetBootPhase(GameCubeBootPhase::BridgeConstruction, GXColor { 0xFF, 0x00, 0xFF, 0xFF });\n            PresentVideoBootFrame();\n            EngineAudioBackend = new GameCubeAudioBackend();\n\n            initializationStage = \"ConstructPlatformInfo\";\n            SetBootPhase(GameCubeBootPhase::BridgeConstruction, GXColor { 0xFF, 0xFF, 0xFF, 0xFF });\n            PresentVideoBootFrame();\n            EnginePlatformInfo = new PlatformInfo(\"gamecube\", \"gc-headless\");",
            applicationSource,
            StringComparison.Ordinal);
        Assert.Contains(
            "initializationStage = \"AddPrimaryWindow\";\n            SetBootPhase(GameCubeBootPhase::CoreInitialization, GXColor { 0x00, 0x40, 0xFF, 0xFF });\n            PresentVideoBootFrame();\n            EngineRenderManager3D->AddWindow(0, RenderMode->fbWidth, RenderMode->efbHeight);\n\n            initializationStage = \"SetPresentedFrameSize\";\n            SetBootPhase(GameCubeBootPhase::CoreInitialization, GXColor { 0xFF, 0x80, 0x00, 0xFF });\n            PresentVideoBootFrame();\n            EngineRenderManager3D->SetPresentedFrameSize(static_cast<uint16_t>(RenderMode->fbWidth), static_cast<uint16_t>(RenderMode->efbHeight));\n\n            initializationStage = \"InitializeCore\";\n            SetBootPhase(GameCubeBootPhase::CoreInitialization, GXColor { 0xFF, 0x00, 0x00, 0xFF });\n            PresentVideoBootFrame();\n            EngineCore->Initialize(EngineRenderManager3D, EngineRenderManager2D, EngineInputManager, EnginePlatformInfo, options);\n            EngineCore->SetAudioBackend(EngineAudioBackend);\n            initializationStage = \"InitializeCoreCompleted\";\n            SetBootPhase(GameCubeBootPhase::CoreInitialization, GXColor { 0xFF, 0x00, 0xFF, 0xFF });\n            PresentVideoBootFrame();",
            applicationSource,
            StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures corrupted disc FST names are rejected before the native runtime constructs an unbounded C string from their offset.
    /// </summary>
    [Fact]
    public void PackagedDiscFileSystemSource_ValidatesFstNameBoundsBeforeStringConstruction() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string discFileSystemSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeDiscFileSystem.cpp"));

        Assert.Contains("bool GameCubeDiscFileSystem::IndexDirectory", discFileSystemSource, StringComparison.Ordinal);
        Assert.Contains("bool GameCubeDiscFileSystem::TryReadEntryName", discFileSystemSource, StringComparison.Ordinal);
        Assert.Contains("stringTableOffset >= FstBytes.size()", discFileSystemSource, StringComparison.Ordinal);
        Assert.Contains("nameOffset >= FstBytes.size() - stringTableOffset", discFileSystemSource, StringComparison.Ordinal);
        Assert.Contains("std::memchr", discFileSystemSource, StringComparison.Ordinal);
        Assert.DoesNotContain("return std::string(reinterpret_cast<const char*>(FstBytes.data() + stringTableOffset + nameOffset));", discFileSystemSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Ensures Nintendont input is selected only after a fault-contained virtual-pad probe succeeds without privileged processor-register reads.
    /// </summary>
    [Fact]
    public void GameCubeInputManagerSource_UsesNintendontVirtualPadTransport() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string inputSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeInputManager.cpp"));
        string inputHeaderSource = File.ReadAllText(Path.Combine(repositoryRootPath, "src", "platform", "gamecube", "GameCubeInputManager.hpp"));

        Assert.DoesNotContain("DetectNintendontInputTransport", inputHeaderSource, StringComparison.Ordinal);
        Assert.DoesNotContain("DetectHighDataBatSupport", inputHeaderSource, StringComparison.Ordinal);
        Assert.Contains("#include <tuxedo/ppc/exception.h>", inputSource, StringComparison.Ordinal);
        Assert.Contains("PPCExcptCurPanicFn", inputSource, StringComparison.Ordinal);
        Assert.Contains("PPC_EXCPT_DSI", inputSource, StringComparison.Ordinal);
        Assert.Contains("context->pc += sizeof(uint32_t);", inputSource, StringComparison.Ordinal);
        Assert.Contains("asm volatile(\"lwz %0, 0(%1)\"", inputSource, StringComparison.Ordinal);
        Assert.Contains("\"+r\"(nintendontPadStubInstruction)", inputSource, StringComparison.Ordinal);
        Assert.Contains("return !NintendontProbeFaulted && nintendontPadStubInstruction != 0U && nintendontPadStubInstruction != UINT32_MAX;", inputSource, StringComparison.Ordinal);
        Assert.Contains("constexpr std::uintptr_t NintendontVirtualPadBufferAddress = 0x93003100U;", inputSource, StringComparison.Ordinal);
        Assert.Contains("constexpr std::uintptr_t NintendontSiInitializedAddress = 0x93003060U;", inputSource, StringComparison.Ordinal);
        Assert.Contains("constexpr std::uintptr_t NintendontPadReadStubAddress = 0x93000000U;", inputSource, StringComparison.Ordinal);
        Assert.DoesNotContain("WiiMem2Physical", inputSource, StringComparison.Ordinal);
        Assert.DoesNotContain("PowerPcGekko", inputSource, StringComparison.Ordinal);
        Assert.DoesNotContain("PowerPcHighDataBat", inputSource, StringComparison.Ordinal);
        Assert.DoesNotContain("ReadDataBat", inputSource, StringComparison.Ordinal);
        Assert.DoesNotContain("ReadProcessorVersion", inputSource, StringComparison.Ordinal);
        Assert.DoesNotContain("mfspr %0", inputSource, StringComparison.Ordinal);
        Assert.Contains("*nintendontSiInitialized = 1U;", inputSource, StringComparison.Ordinal);
        Assert.Contains("DCFlushRange(const_cast<uint32_t*>(nintendontSiInitialized), sizeof(uint32_t));", inputSource, StringComparison.Ordinal);
        Assert.Contains("const NintendontPadReadFunction nintendontPadRead = reinterpret_cast<NintendontPadReadFunction>(NintendontPadReadStubAddress);", inputSource, StringComparison.Ordinal);
        Assert.Contains("nintendontPadRead(1U);", inputSource, StringComparison.Ordinal);
        Assert.DoesNotContain("mfspr %0, 536", inputSource, StringComparison.Ordinal);
        Assert.DoesNotContain("mfspr %0, 537", inputSource, StringComparison.Ordinal);
        Assert.DoesNotContain("return *nintendontSiInitialized != 0U;", inputSource, StringComparison.Ordinal);
        Assert.DoesNotContain("DCInvalidateRange(nintendontPadStatuses, sizeof(PADStatus) * 4U);", inputSource, StringComparison.Ordinal);
        Assert.DoesNotContain("NintendontWiiUHardwareAddress", inputSource, StringComparison.Ordinal);
        Assert.Contains("if (IsNintendontEnvironment()) {\n            InitializeNintendontInputTransport();\n        } else {\n            PAD_Init();\n        }", inputSource, StringComparison.Ordinal);
    }
}
