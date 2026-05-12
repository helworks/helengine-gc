#ifdef DrawText
#undef DrawText
#endif
#include "Core.hpp"
#include "InputSystem.hpp"
#include "runtime/native_exceptions.hpp"
#include "CoreInitializationOptions.hpp"
#include "ContentManager.hpp"
#include "RuntimeContentManagerConfiguration.hpp"
#include "RuntimeComponentRegistry.hpp"
#include "runtime/native_string.hpp"
#include "system/number.hpp"
#include "system/io/path.hpp"
#include "PhysicsFixedStepScheduler.hpp"
#include "FPSComponent.hpp"
#include "ObjectManager.hpp"
#include "SceneManager.hpp"
#include "PointerInteractionSystem.hpp"
#include "RenderManager3D.hpp"
#include "RenderManager2D.hpp"
#include "RuntimeSceneAssetReferenceResolver.hpp"
#include "ShaderCompileTarget.hpp"
#include "RuntimeSceneLoadService.hpp"
#include "NullTextClipboardService.hpp"
#include "TextBoxShortcutRegistry.hpp"
#include "runtime/native_dictionary.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"
#include "system/io/path.hpp"
#include "system/string_comparer.hpp"

::Core* Core::Instance;

::Core* Core::get_Instance()
{
return Core::Instance;
}

void Core::set_Instance(::Core* value)
{
Core::Instance = value;
}

::ContentManager* Core::get_ContentManager()
{
return this->GetContentManager();
}

::FontAsset* Core::get_DefaultFontAsset()
{
return this->DefaultFontAssetValue;}

void Core::set_DefaultFontAsset(::FontAsset* value)
{
    if (value == nullptr)
    {
throw new ArgumentNullException("value");
    }
this->DefaultFontAssetValue = value;
}

double Core::get_FrameDeltaSeconds()
{
return this->FrameDeltaSeconds;
}

void Core::set_FrameDeltaSeconds(double value)
{
this->FrameDeltaSeconds = value;
}

::CoreInitializationOptions* Core::get_InitializationOptions()
{
return this->InitializationOptions;
}

void Core::set_InitializationOptions(::CoreInitializationOptions* value)
{
this->InitializationOptions = value;
}

::InputSystem* Core::get_Input()
{
return this->Input;
}

void Core::set_Input(::InputSystem* value)
{
this->Input = value;
}

::InputSystem* Core::get_InputSystem()
{
return this->Input;
}

::ObjectManager* Core::get_ObjectManager()
{
return this->ObjectManager;
}

void Core::set_ObjectManager(::ObjectManager* value)
{
this->ObjectManager = value;
}

::IPhysicsRuntime* Core::get_PhysicsRuntime()
{
return this->PhysicsRuntimeValue;
}

::PhysicsFixedStepScheduler* Core::get_PhysicsScheduler()
{
return this->PhysicsSchedulerValue;
}

::PointerInteractionSystem* Core::get_PointerInteractionSystem()
{
return this->PointerInteractionSystem;
}

void Core::set_PointerInteractionSystem(::PointerInteractionSystem* value)
{
this->PointerInteractionSystem = value;
}

::RenderManager2D* Core::get_RenderManager2D()
{
return this->RenderManager2D;
}

void Core::set_RenderManager2D(::RenderManager2D* value)
{
this->RenderManager2D = value;
}

::RenderManager3D* Core::get_RenderManager3D()
{
return this->RenderManager3D;
}

void Core::set_RenderManager3D(::RenderManager3D* value)
{
this->RenderManager3D = value;
}

::RuntimeSceneAssetReferenceResolver* Core::get_SceneAssetReferenceResolver()
{
return this->SceneAssetReferenceResolver;
}

void Core::set_SceneAssetReferenceResolver(::RuntimeSceneAssetReferenceResolver* value)
{
this->SceneAssetReferenceResolver = value;
}

::RuntimeSceneLoadService* Core::get_SceneLoadService()
{
return this->SceneLoadService;
}

void Core::set_SceneLoadService(::RuntimeSceneLoadService* value)
{
this->SceneLoadService = value;
}

::SceneManager* Core::get_SceneManager()
{
return this->SceneManager;
}

void Core::set_SceneManager(::SceneManager* value)
{
this->SceneManager = value;
}

::RuntimeComponentRegistry* Core::get_SceneRuntimeComponentRegistry()
{
return this->SceneRuntimeComponentRegistry;
}

void Core::set_SceneRuntimeComponentRegistry(::RuntimeComponentRegistry* value)
{
this->SceneRuntimeComponentRegistry = value;
}

::TextBoxShortcutRegistry* Core::get_TextBoxShortcutRegistry()
{
return this->TextBoxShortcutRegistryValue;
}

::ITextClipboardService* Core::get_TextClipboardService()
{
return this->TextClipboardServiceValue;
}

double Core::get_TotalElapsedSeconds()
{
return this->TotalElapsedSeconds;
}

void Core::set_TotalElapsedSeconds(double value)
{
this->TotalElapsedSeconds = value;
}

void Core::AttachPhysicsRuntime(::IPhysicsRuntime* runtime)
{
    if (runtime == nullptr)
    {
throw new ArgumentNullException("runtime");
    }
this->PhysicsRuntimeValue = runtime;
this->PhysicsSchedulerValue->Reset();
}

void Core::DetachPhysicsRuntime()
{
this->PhysicsRuntimeValue = nullptr;
this->PhysicsSchedulerValue->Reset();
}

void Core::Dispose()
{
this->RenderManager3D->Dispose();
this->RenderManager2D->Dispose();
}

void Core::Draw()
{
this->RenderManager3D->Draw();
FPSComponent::RecordRenderFrame();
}

::ContentManager* Core::GetContentManager()
{
return this->GetContentManager(this->InitializationOptions->get_ContentRootPath());}

::ContentManager* Core::GetContentManager(std::string rootDirectory)
{
    if (String::IsNullOrWhiteSpace(rootDirectory))
    {
throw ([&]() {
auto __ctor_arg_0000020D = "Root directory must be provided.";
auto __ctor_arg_0000020E = "rootDirectory";
return new ArgumentException(__ctor_arg_0000020D, __ctor_arg_0000020E);
})();
    }
const std::string normalizedRootDirectory = Path::GetFullPath(rootDirectory);
// Lock omitted in TypeScript
::ContentManager* contentManager;
    if (this->ContentManagersByRootPath->TryGetValue(normalizedRootDirectory, contentManager))
    {
return contentManager;    }
contentManager = new ::ContentManager(normalizedRootDirectory);
this->ContentManagersByRootPath->Add(normalizedRootDirectory, contentManager);
return contentManager;}

void Core::Initialize(::RenderManager3D* render3D, ::RenderManager2D* render2D, ::IInputBackend* input)
{
this->Initialize(render3D, render2D, input, this->InitializationOptions);
}

void Core::Initialize(::RenderManager3D* render3D, ::RenderManager2D* render2D, ::IInputBackend* input, ::CoreInitializationOptions* options)
{
this->set_RenderManager3D(render3D);
this->set_RenderManager2D(render2D);
this->Input->SetBackend(input);
    if (options == nullptr)
    {
throw new ArgumentNullException("options");
    }
options->Normalize();
this->set_InitializationOptions(options);
this->PhysicsSchedulerValue = this->CreatePhysicsScheduler(options);
this->set_ObjectManager(new ::ObjectManager(options));
::ContentManager *contentManager = this->GetContentManager();
RuntimeContentManagerConfiguration::ConfigureSharedAssetContentManager(contentManager);
this->set_SceneAssetReferenceResolver(new ::RuntimeSceneAssetReferenceResolver(contentManager, this->InitializationOptions->get_ContentRootPath(), ShaderCompileTarget::DirectX11));
this->set_SceneRuntimeComponentRegistry(RuntimeComponentRegistry::CreateDefault());
this->set_SceneLoadService(new ::RuntimeSceneLoadService(this->SceneAssetReferenceResolver, this->SceneRuntimeComponentRegistry));
this->set_SceneManager(this->CreateSceneManager(contentManager, this->InitializationOptions->get_SceneCatalog()));
}

Core::Core() : Core(new CoreInitializationOptions())
{
}

Core::Core(::CoreInitializationOptions* options) : FrameDeltaSeconds(0), InitializationOptions(), Input(), ObjectManager(), PointerInteractionSystem(), RenderManager2D(), RenderManager3D(), SceneAssetReferenceResolver(), SceneLoadService(), SceneManager(), SceneRuntimeComponentRegistry(), TotalElapsedSeconds(0), ContentManagerLock(), ContentManagersByRootPath(), DefaultFontAssetValue(), PhysicsRuntimeValue(), PhysicsSchedulerValue(), TextBoxShortcutRegistryValue(), TextClipboardServiceValue()
{
    if (options == nullptr)
    {
throw new ArgumentNullException("options");
    }
this->ContentManagersByRootPath = new Dictionary<std::string, ::ContentManager*>(StringComparer::OrdinalIgnoreCase);
this->ContentManagerLock = new char[1];
Core::set_Instance(this);
this->set_InitializationOptions(options);
this->InitializationOptions->Normalize();
this->PhysicsSchedulerValue = this->CreatePhysicsScheduler(this->InitializationOptions);
this->set_Input(new ::InputSystem());
this->set_PointerInteractionSystem(new ::PointerInteractionSystem(this, this->Input));
this->TextClipboardServiceValue = new ::NullTextClipboardService();
this->TextBoxShortcutRegistryValue = new ::TextBoxShortcutRegistry();
}

void Core::RegisterRuntimeComponentDeserializer(::IRuntimeComponentDeserializer* deserializer)
{
    if (deserializer == nullptr)
    {
throw new ArgumentNullException("deserializer");
    }
    if (this->SceneRuntimeComponentRegistry == nullptr)
    {
throw new InvalidOperationException("Core must be initialized before runtime component deserializers can be registered.");
    }
this->SceneRuntimeComponentRegistry->Register(deserializer);
}

void Core::SetTextClipboardService(::ITextClipboardService* clipboardService)
{
    if (clipboardService == nullptr)
    {
throw new ArgumentNullException("clipboardService");
    }
this->TextClipboardServiceValue = clipboardService;
}

void Core::Update()
{
this->Update(this->InitializationOptions->get_DefaultUpdateDeltaSeconds());
}

void Core::Update(double elapsedSeconds)
{
this->ValidateElapsedSeconds(elapsedSeconds);
this->set_FrameDeltaSeconds(elapsedSeconds);
this->TotalElapsedSeconds += elapsedSeconds;
this->Input->EarlyUpdate();
FPSComponent::RecordUpdateFrame();
this->ObjectManager->Update();
    if (this->SceneManager != nullptr)
    {
this->SceneManager->FlushPendingOperations();
    }
this->UpdatePhysics(elapsedSeconds);
this->Input->Update();
this->PointerInteractionSystem->Update();
}

::PhysicsFixedStepScheduler* Core::CreatePhysicsScheduler(::CoreInitializationOptions* options)
{
    if (options == nullptr)
    {
throw new ArgumentNullException("options");
    }
return new ::PhysicsFixedStepScheduler(options->get_PhysicsFixedStepSeconds());}

::SceneManager* Core::CreateSceneManager(::ContentManager* contentManager, ::RuntimeSceneCatalog* sceneCatalog)
{
    if (contentManager == nullptr)
    {
throw new ArgumentNullException("contentManager");
    }
    if (sceneCatalog == nullptr)
    {
return nullptr;    }
return new ::SceneManager(sceneCatalog, contentManager, this->SceneLoadService, this->ObjectManager);}

void Core::UpdatePhysics(double elapsedSeconds)
{
    if (this->PhysicsRuntimeValue == nullptr)
    {
return;    }
this->PhysicsSchedulerValue->AddElapsedSeconds(elapsedSeconds);
while (this->PhysicsSchedulerValue->TryConsumeStep()) {
this->PhysicsRuntimeValue->Step(this->PhysicsSchedulerValue->get_StepSeconds());
}
}

void Core::ValidateElapsedSeconds(double elapsedSeconds)
{
    if (Number::IsNaN(elapsedSeconds) || Number::IsInfinity(elapsedSeconds))
    {
throw ([&]() {
auto __ctor_arg_0000020F = "elapsedSeconds";
auto __ctor_arg_00000210 = "Elapsed frame time must be finite.";
return new ArgumentOutOfRangeException(__ctor_arg_0000020F, __ctor_arg_00000210);
})();
    }
    if (elapsedSeconds < 0.0)
    {
throw ([&]() {
auto __ctor_arg_00000211 = "elapsedSeconds";
auto __ctor_arg_00000212 = "Elapsed frame time cannot be negative.";
return new ArgumentOutOfRangeException(__ctor_arg_00000211, __ctor_arg_00000212);
})();
    }
}

