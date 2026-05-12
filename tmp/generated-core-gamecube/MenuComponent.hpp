#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class UpdateComponent;
class IUpdateable;
class InputSystem;
class Core;
class Entity;
class MenuPanelRuntime;
class MenuItemRuntime;
class CoreInitializationOptions;
class ISceneIdPathResolver;
class SceneAsset;
class ContentManager;
class RuntimeSceneLoadService;
class SceneManager;
class InputGamepadState;
class float3;
class RoundedRectComponent;
class int2;
class TextComponent;
class ScrollComponent;
class MenuItemComponent;

#include "UpdateComponent.hpp"
#include "IUpdateable.hpp"
#include "UpdateComponent.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_exceptions.hpp"
#include "InputSystem.hpp"
#include "Core.hpp"
#include "Core.hpp"
#include "Entity.hpp"
#include "runtime/native_dictionary.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_list.hpp"
#include "runtime/array.hpp"
#include "runtime/array.hpp"
#include "MenuPanelRuntime.hpp"
#include "runtime/native_exceptions.hpp"
#include "MenuItemRuntime.hpp"
#include "CoreInitializationOptions.hpp"
#include "ISceneIdPathResolver.hpp"
#include "SceneAsset.hpp"
#include "ContentManager.hpp"
#include "RuntimeSceneLoadService.hpp"
#include "SceneManager.hpp"
#include "InputSystem.hpp"
#include "InputGamepadState.hpp"
#include "MenuItemRuntime.hpp"
#include "float3.hpp"
#include "Entity.hpp"
#include "RoundedRectComponent.hpp"
#include "int2.hpp"
#include "TextComponent.hpp"
#include "ScrollComponent.hpp"
#include "ScrollComponent.hpp"
#include "RoundedRectComponent.hpp"
#include "runtime/native_list.hpp"
#include "InputGamepadState.hpp"
#include "MenuPanelRuntime.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_dictionary.hpp"
#include "runtime/array.hpp"
#include "Keys.hpp"
#include "MenuItemComponent.hpp"
#include "TextComponent.hpp"
#include "InputGamepadButton.hpp"

class MenuComponent : public UpdateComponent
{
public:
    virtual ~MenuComponent() = default;

    static uint8_t CurrentVersion;

    static std::string SerializedComponentTypeId;

    std::string get_ActivePanelId();

    std::string get_InitialPanelId();

    void set_InitialPanelId(std::string value);

    bool IsInitialized;

    bool get_IsInitialized();
    void set_IsInitialized(bool value);

    std::string get_ProviderTypeName();

    void set_ProviderTypeName(std::string value);

    std::string get_SelectedItemId();

    void ComponentAdded(::Entity* entity);

    MenuComponent();

    void Update();

    uint8_t get_UpdateOrder();

    void set_UpdateOrder(uint8_t value);

    ::Entity* get_Parent();

    void set_Parent(::Entity* value);
private:
    ::MenuPanelRuntime* ActivePanel;

    std::string ActivePanelIdValue;

    std::string InitialPanelIdValue;

    List<std::string>* PanelHistory;

    List<::MenuPanelRuntime*>* PanelRuntimes;

    Dictionary<std::string, ::MenuPanelRuntime*>* PanelsById;

    ::MenuItemRuntime* PressedPointerItem;

    ::InputGamepadState PreviousGamepadState;

    std::string ProviderTypeNameValue;

    std::string SelectedItemIdValue;

    void ActivatePanel(std::string panelId, bool pushHistory);

    void ApplyItemVisualState(::MenuItemRuntime* runtimeItem, bool isSelected);

    void ApplyItemsScrollOffset(::Entity* itemsRootEntity, int32_t scrollOffset);

    Array<::MenuItemRuntime*>* BindItems(::Entity* panelEntity, std::string panelId);

    void BindPanels(::Entity* rootEntity);

    void ClearSelectionVisuals(::MenuPanelRuntime* panelRuntime);

    template <typename TComponent>
    void CollectEntitiesWithComponent(::Entity* entity, List<::Entity*>* entities);

    void ConfirmSelection(::Keys key);

    bool ContainsPointer(::MenuItemRuntime* runtimeItem, int32_t pointerX, int32_t pointerY);

    std::string DescribeEntity(::Entity* entity);

    void EnsureSelectedItemVisible(::MenuPanelRuntime* panelRuntime, int32_t selectedItemIndex);

    void ExecuteAction(::MenuItemComponent* itemComponent);

    template <typename TComponent>
    TComponent FindFirstComponent(::Entity* entity);

    ::Entity* FindGeneratedRootEntity(::Entity* rootEntity);

    ::MenuItemRuntime* FindHoveredItem(::MenuPanelRuntime* panelRuntime, int32_t pointerX, int32_t pointerY);

    template <typename TComponent>
    TComponent FindRequiredComponent(::Entity* entity);

    void HandleGamepadInput(::InputSystem* inputSystem);

    void HandleItemsScrollOffsetChanged(::ScrollComponent* scrollComponent, int32_t scrollOffset);

    void HandleKeyboardInput(::InputSystem* inputSystem);

    void HandleMouseInput(::InputSystem* inputSystem);

    bool IsMouseHoverSelectionUpdateRequired(::InputSystem* inputSystem);

    bool IsSameRuntimeItem(::MenuItemRuntime* left, ::MenuItemRuntime* right);

    void LoadScene(std::string sceneId);

    void MoveSelection(int32_t delta);

    void NavigateBack();

    ::InputGamepadState ReadPrimaryGamepadState();

    ::ScrollComponent* ResolveItemsScrollComponent(::Entity* panelEntity, std::string panelId);

    float ResolveItemsScrollStep(::Entity* itemsRootEntity);

    ::TextComponent* ResolveSelectedDescriptionText(::Entity* panelEntity);

    int32_t ResolveSelectedIndex(::MenuPanelRuntime* panelRuntime);

    void SetSelection(::MenuPanelRuntime* panelRuntime, int32_t itemIndex);

    template <typename TComponent>
    bool TryFindComponent(::Entity* entity, TComponent& component);

    void TryInitialize();

    bool WasGamepadButtonPressed(::InputGamepadState currentState, ::InputGamepadState previousState, ::InputGamepadButton button);
};
