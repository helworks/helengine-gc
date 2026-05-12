#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class InputSystem;
class CoreInitializationOptions;
class ContentManager;
class RuntimeContentManagerConfiguration;
class RuntimeComponentRegistry;
class PhysicsFixedStepScheduler;
class FPSComponent;
class ObjectManager;
class SceneManager;
class PointerInteractionSystem;
class RenderManager3D;
class RenderManager2D;
class FontAsset;
class IPhysicsRuntime;
class RuntimeSceneAssetReferenceResolver;
class RuntimeSceneLoadService;
class TextBoxShortcutRegistry;
class ITextClipboardService;
class IInputBackend;
class IRuntimeComponentDeserializer;
class RuntimeSceneCatalog;

#include "runtime/native_disposable.hpp"
#include "InputSystem.hpp"
#include "runtime/native_exceptions.hpp"
#include "CoreInitializationOptions.hpp"
#include "ContentManager.hpp"
#include "RuntimeContentManagerConfiguration.hpp"
#include "RuntimeContentManagerConfiguration.hpp"
#include "runtime/native_exceptions.hpp"
#include "RuntimeComponentRegistry.hpp"
#include "ContentManager.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_exceptions.hpp"
#include "system/io/path.hpp"
#include "system/io/path.hpp"
#include "PhysicsFixedStepScheduler.hpp"
#include "FPSComponent.hpp"
#include "FPSComponent.hpp"
#include "ObjectManager.hpp"
#include "SceneManager.hpp"
#include "PointerInteractionSystem.hpp"
#include "RenderManager3D.hpp"
#include "RenderManager2D.hpp"
#include "runtime/native_exceptions.hpp"
#include "FontAsset.hpp"
#include "CoreInitializationOptions.hpp"
#include "InputSystem.hpp"
#include "ObjectManager.hpp"
#include "IPhysicsRuntime.hpp"
#include "PhysicsFixedStepScheduler.hpp"
#include "PointerInteractionSystem.hpp"
#include "RenderManager2D.hpp"
#include "RenderManager3D.hpp"
#include "RuntimeSceneAssetReferenceResolver.hpp"
#include "RuntimeSceneLoadService.hpp"
#include "SceneManager.hpp"
#include "RuntimeComponentRegistry.hpp"
#include "TextBoxShortcutRegistry.hpp"
#include "ITextClipboardService.hpp"
#include "runtime/native_dictionary.hpp"
#include "IInputBackend.hpp"
#include "IRuntimeComponentDeserializer.hpp"
#include "RuntimeSceneCatalog.hpp"

class Core : public IDisposable
{
public:
    virtual ~Core() = default;

    static ::Core* Instance;

    static ::Core* get_Instance();
    static void set_Instance(::Core* value);

    ::ContentManager* get_ContentManager();

    ::FontAsset* get_DefaultFontAsset();

    void set_DefaultFontAsset(::FontAsset* value);

    double FrameDeltaSeconds;

    double get_FrameDeltaSeconds();
    void set_FrameDeltaSeconds(double value);

    ::CoreInitializationOptions* InitializationOptions;

    ::CoreInitializationOptions* get_InitializationOptions();
    void set_InitializationOptions(::CoreInitializationOptions* value);

    ::InputSystem* Input;

    ::InputSystem* get_Input();
    void set_Input(::InputSystem* value);

    ::InputSystem* get_InputSystem();

    ::ObjectManager* ObjectManager;

    ::ObjectManager* get_ObjectManager();
    void set_ObjectManager(::ObjectManager* value);

    ::IPhysicsRuntime* get_PhysicsRuntime();

    ::PhysicsFixedStepScheduler* get_PhysicsScheduler();

    ::PointerInteractionSystem* PointerInteractionSystem;

    ::PointerInteractionSystem* get_PointerInteractionSystem();
    void set_PointerInteractionSystem(::PointerInteractionSystem* value);

    ::RenderManager2D* RenderManager2D;

    ::RenderManager2D* get_RenderManager2D();
    void set_RenderManager2D(::RenderManager2D* value);

    ::RenderManager3D* RenderManager3D;

    ::RenderManager3D* get_RenderManager3D();
    void set_RenderManager3D(::RenderManager3D* value);

    ::RuntimeSceneAssetReferenceResolver* SceneAssetReferenceResolver;

    ::RuntimeSceneAssetReferenceResolver* get_SceneAssetReferenceResolver();
    void set_SceneAssetReferenceResolver(::RuntimeSceneAssetReferenceResolver* value);

    ::RuntimeSceneLoadService* SceneLoadService;

    ::RuntimeSceneLoadService* get_SceneLoadService();
    void set_SceneLoadService(::RuntimeSceneLoadService* value);

    ::SceneManager* SceneManager;

    ::SceneManager* get_SceneManager();
    void set_SceneManager(::SceneManager* value);

    ::RuntimeComponentRegistry* SceneRuntimeComponentRegistry;

    ::RuntimeComponentRegistry* get_SceneRuntimeComponentRegistry();
    void set_SceneRuntimeComponentRegistry(::RuntimeComponentRegistry* value);

    ::TextBoxShortcutRegistry* get_TextBoxShortcutRegistry();

    ::ITextClipboardService* get_TextClipboardService();

    double TotalElapsedSeconds;

    double get_TotalElapsedSeconds();
    void set_TotalElapsedSeconds(double value);

    void AttachPhysicsRuntime(::IPhysicsRuntime* runtime);

    void DetachPhysicsRuntime();

    void Dispose();

    virtual void Draw();

    ::ContentManager* GetContentManager();

    ::ContentManager* GetContentManager(std::string rootDirectory);

    virtual void Initialize(::RenderManager3D* render3D, ::RenderManager2D* render2D, ::IInputBackend* input);

    virtual void Initialize(::RenderManager3D* render3D, ::RenderManager2D* render2D, ::IInputBackend* input, ::CoreInitializationOptions* options);

    Core();

    Core(::CoreInitializationOptions* options);

    void RegisterRuntimeComponentDeserializer(::IRuntimeComponentDeserializer* deserializer);

    void SetTextClipboardService(::ITextClipboardService* clipboardService);

    virtual void Update();

    virtual void Update(double elapsedSeconds);
private:
    void* ContentManagerLock;

    Dictionary<std::string, ::ContentManager*>* ContentManagersByRootPath;

    ::FontAsset* DefaultFontAssetValue;

    ::IPhysicsRuntime* PhysicsRuntimeValue;

    ::PhysicsFixedStepScheduler* PhysicsSchedulerValue;

    ::TextBoxShortcutRegistry* TextBoxShortcutRegistryValue;

    ::ITextClipboardService* TextClipboardServiceValue;

    ::PhysicsFixedStepScheduler* CreatePhysicsScheduler(::CoreInitializationOptions* options);

    ::SceneManager* CreateSceneManager(::ContentManager* contentManager, ::RuntimeSceneCatalog* sceneCatalog);

    void UpdatePhysics(double elapsedSeconds);

    void ValidateElapsedSeconds(double elapsedSeconds);
};
