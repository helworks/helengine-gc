#ifdef DrawText
#undef DrawText
#endif
#include "MenuComponent.hpp"
#include "UpdateComponent.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"
#include "InputSystem.hpp"
#include "Core.hpp"
#include "Entity.hpp"
#include "runtime/native_dictionary.hpp"
#include "runtime/native_list.hpp"
#include "runtime/array.hpp"
#include "MenuPanelRuntime.hpp"
#include "MenuItemRuntime.hpp"
#include "CoreInitializationOptions.hpp"
#include "ISceneIdPathResolver.hpp"
#include "SceneAsset.hpp"
#include "ContentManager.hpp"
#include "RuntimeSceneLoadService.hpp"
#include "SceneManager.hpp"
#include "InputGamepadState.hpp"
#include "float3.hpp"
#include "RoundedRectComponent.hpp"
#include "int2.hpp"
#include "TextComponent.hpp"
#include "ScrollComponent.hpp"
#include "MenuItemComponent.hpp"
#include "MenuSelectedDescriptionComponent.hpp"
#include "MenuPanelComponent.hpp"
#include "MenuActionKind.hpp"
#include "InputGamepadButton.hpp"
#include "Keys.hpp"
#include "ComponentExecutionContext.hpp"
#include "ComponentExecutionMode.hpp"
#include "RuntimeContentProcessorIds.hpp"
#include "SceneLoadMode.hpp"
#include "DemoMenuLayout.hpp"
#include "runtime/array.hpp"
#include "runtime/native_cast.hpp"
#include "runtime/native_dictionary.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_type.hpp"
#include "system/string_comparer.hpp"

uint8_t MenuComponent::CurrentVersion = 1;

std::string MenuComponent::SerializedComponentTypeId = "helengine.MenuComponent";

std::string MenuComponent::get_ActivePanelId()
{
return this->ActivePanelIdValue;}

std::string MenuComponent::get_InitialPanelId()
{
return this->InitialPanelIdValue;}

void MenuComponent::set_InitialPanelId(std::string value)
{
this->InitialPanelIdValue = value;
}

bool MenuComponent::get_IsInitialized()
{
return this->IsInitialized;
}

void MenuComponent::set_IsInitialized(bool value)
{
this->IsInitialized = value;
}

std::string MenuComponent::get_ProviderTypeName()
{
return this->ProviderTypeNameValue;}

void MenuComponent::set_ProviderTypeName(std::string value)
{
this->ProviderTypeNameValue = value;
}

std::string MenuComponent::get_SelectedItemId()
{
return this->SelectedItemIdValue;}

void MenuComponent::ComponentAdded(::Entity* entity)
{
UpdateComponent::ComponentAdded(entity);
    if (entity == nullptr)
    {
throw new ArgumentNullException("entity");
    }
    if (String::IsNullOrWhiteSpace(this->InitialPanelIdValue))
    {
throw new InvalidOperationException("Menu components require an initial panel id.");
    }
}

MenuComponent::MenuComponent() : IsInitialized(), ActivePanel(), ActivePanelIdValue(), InitialPanelIdValue(), PanelHistory(), PanelRuntimes(), PanelsById(), PressedPointerItem(), PreviousGamepadState(), ProviderTypeNameValue(), SelectedItemIdValue()
{
this->PanelsById = new Dictionary<std::string, ::MenuPanelRuntime*>(StringComparer::Ordinal);
this->PanelRuntimes = new List<::MenuPanelRuntime*>();
this->PanelHistory = new List<std::string>();
this->ProviderTypeNameValue = String::Empty;
this->InitialPanelIdValue = String::Empty;
this->ActivePanelIdValue = String::Empty;
this->SelectedItemIdValue = String::Empty;
}

void MenuComponent::Update()
{
    if (!this->IsInitialized)
    {
this->TryInitialize();
    if (!this->IsInitialized)
    {
return;    }
    }
::InputSystem *inputSystem = Core::get_Instance()->get_Input();
    if (inputSystem == nullptr)
    {
return;    }
this->HandleKeyboardInput(inputSystem);
this->HandleMouseInput(inputSystem);
this->HandleGamepadInput(inputSystem);
}

uint8_t MenuComponent::get_UpdateOrder()
{
return this->UpdateComponent::get_UpdateOrder();
}

void MenuComponent::set_UpdateOrder(uint8_t value)
{
this->UpdateComponent::set_UpdateOrder(value);
}

::Entity* MenuComponent::get_Parent()
{
return this->Component::get_Parent();
}

void MenuComponent::set_Parent(::Entity* value)
{
this->Component::set_Parent(value);
}

void MenuComponent::ActivatePanel(std::string panelId, bool pushHistory)
{
::MenuPanelRuntime* nextPanel;
    if (!this->PanelsById->TryGetValue(panelId, nextPanel))
    {
throw new InvalidOperationException(std::string("Baked menu panel '") + panelId + std::string("' was not registered."));
    }
    if (this->ActivePanel != nullptr)
    {
    if (pushHistory && !String::Equals(this->ActivePanel->get_Definition()->get_PanelId(), panelId, StringComparison::Ordinal))
    {
this->PanelHistory->Add(this->ActivePanel->get_Definition()->get_PanelId());
    }
    }
for (int32_t panelIndex = 0; panelIndex < this->PanelRuntimes->Count(); panelIndex++) {
::MenuPanelRuntime *panelRuntime = (*this->PanelRuntimes)[panelIndex];
panelRuntime->get_RootEntity()->set_Enabled(false);
this->ClearSelectionVisuals(panelRuntime);
}
this->ActivePanel = nextPanel;
this->ActivePanel->get_RootEntity()->set_Enabled(true);
this->ActivePanelIdValue = nextPanel->get_Definition()->get_PanelId();
this->PressedPointerItem = nullptr;
this->SetSelection(nextPanel, this->ResolveSelectedIndex(nextPanel));
}

void MenuComponent::ApplyItemVisualState(::MenuItemRuntime* runtimeItem, bool isSelected)
{
    if (isSelected)
    {
runtimeItem->get_Background()->set_FillColor(runtimeItem->get_Definition()->get_SelectedFillColor());
runtimeItem->get_Background()->set_BorderColor(runtimeItem->get_Definition()->get_SelectedBorderColor());
    }
else {
runtimeItem->get_Background()->set_FillColor(runtimeItem->get_Definition()->get_IdleFillColor());
runtimeItem->get_Background()->set_BorderColor(runtimeItem->get_Definition()->get_IdleBorderColor());
}
}

void MenuComponent::ApplyItemsScrollOffset(::Entity* itemsRootEntity, int32_t scrollOffset)
{
    if (itemsRootEntity == nullptr)
    {
throw new ArgumentNullException("itemsRootEntity");
    }
const float itemStep = this->ResolveItemsScrollStep(itemsRootEntity);
itemsRootEntity->set_LocalPosition(::float3(0.0f, -scrollOffset * itemStep, 0.0f));
}

Array<::MenuItemRuntime*>* MenuComponent::BindItems(::Entity* panelEntity, std::string panelId)
{
List<::Entity*> *itemEntities = new List<::Entity*>();
CollectEntitiesWithComponent<MenuItemComponent*>(panelEntity, itemEntities);
Array<::MenuItemRuntime*> *itemRuntimes = new Array<MenuItemRuntime*>(itemEntities->Count());
for (int32_t itemIndex = 0; itemIndex < itemEntities->Count(); itemIndex++) {
::Entity *itemEntity = (*itemEntities)[itemIndex];
::MenuItemComponent *itemComponent = FindRequiredComponent<MenuItemComponent*>(itemEntity);
    if (!String::Equals(itemComponent->get_PanelId(), panelId, StringComparison::Ordinal))
    {
throw new InvalidOperationException(std::string("Baked menu item '") + itemComponent->get_ItemId() + std::string("' does not match panel '") + panelId + std::string("'."));
    }
::RoundedRectComponent *backgroundComponent = FindRequiredComponent<RoundedRectComponent*>(itemEntity);
(*itemRuntimes)[itemIndex] = new ::MenuItemRuntime(itemComponent, itemIndex, itemEntity, backgroundComponent);
}
    if (itemRuntimes->Length == 0)
    {
throw new InvalidOperationException(std::string("Baked menu panel '") + panelId + std::string("' does not contain any items."));
    }
return itemRuntimes;}

void MenuComponent::BindPanels(::Entity* rootEntity)
{
this->PanelsById->Clear();
this->PanelRuntimes->Clear();
this->PanelHistory->Clear();
::Entity *generatedRootEntity = this->FindGeneratedRootEntity(rootEntity);
    if (generatedRootEntity == nullptr)
    {
throw new InvalidOperationException(std::string("Menu root '") + this->DescribeEntity(rootEntity) + std::string("' is missing the generated menu subtree."));
    }
List<::Entity*> *panelEntities = new List<::Entity*>();
CollectEntitiesWithComponent<MenuPanelComponent*>(generatedRootEntity, panelEntities);
for (int32_t panelIndex = 0; panelIndex < panelEntities->Count(); panelIndex++) {
::Entity *panelEntity = (*panelEntities)[panelIndex];
::MenuPanelComponent *panelComponent = FindRequiredComponent<MenuPanelComponent*>(panelEntity);
::TextComponent *selectedDescriptionText = this->ResolveSelectedDescriptionText(panelEntity);
Array<::MenuItemRuntime*> *itemRuntimes = this->BindItems(panelEntity, panelComponent->get_PanelId());
::ScrollComponent *itemsScrollComponent = this->ResolveItemsScrollComponent(panelEntity, panelComponent->get_PanelId());
itemsScrollComponent->set_ItemCount(itemRuntimes->Length);
::MenuPanelRuntime *panelRuntime = new ::MenuPanelRuntime(panelComponent, panelEntity, selectedDescriptionText, itemsScrollComponent->Parent, itemsScrollComponent, itemRuntimes);
panelRuntime->get_ItemsScrollComponent()->ScrollOffsetChanged += &MenuComponent::HandleItemsScrollOffsetChanged;
this->ApplyItemsScrollOffset(panelRuntime->get_ItemsRootEntity(), panelRuntime->get_ItemsScrollComponent()->get_ScrollOffset());
    if (this->PanelsById->ContainsKey(panelComponent->get_PanelId()))
    {
throw new InvalidOperationException(std::string("Duplicate baked menu panel id '") + panelComponent->get_PanelId() + std::string("' was found."));
    }
this->PanelsById->Add(panelComponent->get_PanelId(), panelRuntime);
this->PanelRuntimes->Add(panelRuntime);
}
    if (this->PanelsById->Count() == 0)
    {
throw new InvalidOperationException("The baked menu scene does not contain any panel metadata.");
    }
}

void MenuComponent::ClearSelectionVisuals(::MenuPanelRuntime* panelRuntime)
{
for (int32_t itemIndex = 0; itemIndex < panelRuntime->get_Items()->Length; itemIndex++) {
this->ApplyItemVisualState((*panelRuntime->get_Items())[itemIndex], false);
}
}

template <typename TComponent>
void MenuComponent::CollectEntitiesWithComponent(::Entity* entity, List<::Entity*>* entities)
{
    if (entity == nullptr)
    {
throw new ArgumentNullException("entity");
    }
    if (entities == nullptr)
    {
throw new ArgumentNullException("entities");
    }
TComponent component;
    if (TryFindComponent<TComponent>(entity, component))
    {
entities->Add(entity);
    }
    if (entity->get_Children() == nullptr)
    {
return;    }
for (int32_t childIndex = 0; childIndex < entity->get_Children()->Count(); childIndex++) {
CollectEntitiesWithComponent<TComponent>((*entity->get_Children())[childIndex], entities);
}
}

void MenuComponent::ConfirmSelection(::Keys key)
{
    if (this->ActivePanel == nullptr)
    {
return;    }
    if (this->ActivePanel->get_SelectedItemIndex() < 0 || this->ActivePanel->get_SelectedItemIndex() >= this->ActivePanel->get_Items()->Length)
    {
return;    }
this->ExecuteAction((*this->ActivePanel->get_Items())[this->ActivePanel->get_SelectedItemIndex()]->get_Definition());
}

bool MenuComponent::ContainsPointer(::MenuItemRuntime* runtimeItem, int32_t pointerX, int32_t pointerY)
{
    if (runtimeItem == nullptr)
    {
throw new ArgumentNullException("runtimeItem");
    }
::float3 position = runtimeItem->get_Entity()->get_Position();
const int32_t width = runtimeItem->get_Background()->get_Size().X;
const int32_t height = runtimeItem->get_Background()->get_Size().Y;
return pointerX >= position.X && pointerX < position.X + width && pointerY >= position.Y && pointerY < position.Y + height;}

std::string MenuComponent::DescribeEntity(::Entity* entity)
{
return he_cpp_type_of<Entity>("Entity")->Name;}

void MenuComponent::EnsureSelectedItemVisible(::MenuPanelRuntime* panelRuntime, int32_t selectedItemIndex)
{
    if (panelRuntime == nullptr)
    {
throw new ArgumentNullException("panelRuntime");
    }
    if (selectedItemIndex < 0 || selectedItemIndex >= panelRuntime->get_Items()->Length)
    {
throw ([&]() {
auto __ctor_arg_00000217 = "selectedItemIndex";
auto __ctor_arg_00000218 = "Selected baked menu item index must be valid.";
return new ArgumentOutOfRangeException(__ctor_arg_00000217, __ctor_arg_00000218);
})();
    }
const int32_t visibleItemCount = panelRuntime->get_ItemsScrollComponent()->get_VisibleItemCount();
const int32_t scrollOffset = panelRuntime->get_ItemsScrollComponent()->get_ScrollOffset();
const int32_t visibleEndExclusive = scrollOffset + visibleItemCount;
    if (selectedItemIndex < scrollOffset)
    {
panelRuntime->get_ItemsScrollComponent()->ScrollTo(selectedItemIndex);
return;    }
    if (selectedItemIndex >= visibleEndExclusive)
    {
panelRuntime->get_ItemsScrollComponent()->ScrollTo(selectedItemIndex - visibleItemCount + 1);
    }
}

void MenuComponent::ExecuteAction(::MenuItemComponent* itemComponent)
{
    if (itemComponent == nullptr)
    {
throw new ArgumentNullException("itemComponent");
    }
    if (itemComponent->get_ActionKind() == MenuActionKind::None)
    {
return;    }
else     if (itemComponent->get_ActionKind() == MenuActionKind::OpenPanel)
    {
this->ActivatePanel(itemComponent->get_TargetId(), true);
    }
else     if (itemComponent->get_ActionKind() == MenuActionKind::LoadScene)
    {
this->LoadScene(itemComponent->get_TargetId());
    }
else     if (itemComponent->get_ActionKind() == MenuActionKind::Back)
    {
this->NavigateBack();
    }
else {
throw new InvalidOperationException(std::string("Unsupported baked menu action kind '") + std::to_string(static_cast<int32_t>(itemComponent->get_ActionKind())) + std::string("'."));
}
}

template <typename TComponent>
TComponent MenuComponent::FindFirstComponent(::Entity* entity)
{
TComponent component;
    if (TryFindComponent<TComponent>(entity, component))
    {
return component;    }
return nullptr;}

::Entity* MenuComponent::FindGeneratedRootEntity(::Entity* rootEntity)
{
    if (rootEntity->get_Children() == nullptr)
    {
return nullptr;    }
    if (rootEntity->get_Children()->Count() == 1)
    {
return (*rootEntity->get_Children())[0];    }
return nullptr;}

::MenuItemRuntime* MenuComponent::FindHoveredItem(::MenuPanelRuntime* panelRuntime, int32_t pointerX, int32_t pointerY)
{
    if (panelRuntime == nullptr)
    {
throw new ArgumentNullException("panelRuntime");
    }
for (int32_t itemIndex = 0; itemIndex < panelRuntime->get_Items()->Length; itemIndex++) {
::MenuItemRuntime *runtimeItem = (*panelRuntime->get_Items())[itemIndex];
    if (!this->ContainsPointer(runtimeItem, pointerX, pointerY))
    {
continue;
    }
return runtimeItem;}
return nullptr;}

template <typename TComponent>
TComponent MenuComponent::FindRequiredComponent(::Entity* entity)
{
TComponent component;
    if (TryFindComponent<TComponent>(entity, component))
    {
return component;    }
throw new InvalidOperationException(std::string("Entity '") + this->DescribeEntity(entity) + std::string("' is missing required component '") + he_cpp_type_of<TComponent>("TComponent")->get_Name() + std::string("'."));
}

void MenuComponent::HandleGamepadInput(::InputSystem* inputSystem)
{
::InputGamepadState currentGamepadState = inputSystem->GetGamepadState(0);
    if (!currentGamepadState.get_Connected())
    {
this->PreviousGamepadState = currentGamepadState;
return;    }
    if (this->WasGamepadButtonPressed(currentGamepadState, this->PreviousGamepadState, InputGamepadButton::DPadUp))
    {
this->MoveSelection(-1);
    }
else     if (this->WasGamepadButtonPressed(currentGamepadState, this->PreviousGamepadState, InputGamepadButton::DPadDown))
    {
this->MoveSelection(1);
    }
else     if (this->WasGamepadButtonPressed(currentGamepadState, this->PreviousGamepadState, InputGamepadButton::South))
    {
this->ConfirmSelection(Keys::Enter);
    }
else     if (this->WasGamepadButtonPressed(currentGamepadState, this->PreviousGamepadState, InputGamepadButton::East) || this->WasGamepadButtonPressed(currentGamepadState, this->PreviousGamepadState, InputGamepadButton::Select))
    {
this->NavigateBack();
    }
this->PreviousGamepadState = currentGamepadState;
}

void MenuComponent::HandleItemsScrollOffsetChanged(::ScrollComponent* scrollComponent, int32_t scrollOffset)
{
    if (scrollComponent == nullptr)
    {
throw new ArgumentNullException("scrollComponent");
    }
this->ApplyItemsScrollOffset(scrollComponent->Parent, scrollOffset);
}

void MenuComponent::HandleKeyboardInput(::InputSystem* inputSystem)
{
    if (inputSystem->WasKeyPressed(Keys::Up) || inputSystem->WasKeyPressed(Keys::W))
    {
this->MoveSelection(-1);
    }
else     if (inputSystem->WasKeyPressed(Keys::Down) || inputSystem->WasKeyPressed(Keys::S))
    {
this->MoveSelection(1);
    }
else     if (inputSystem->WasKeyPressed(Keys::Enter))
    {
this->ConfirmSelection(Keys::Enter);
    }
else     if (inputSystem->WasKeyPressed(Keys::Space))
    {
this->ConfirmSelection(Keys::Space);
    }
else     if (inputSystem->WasKeyPressed(Keys::Escape) || inputSystem->WasKeyPressed(Keys::Back))
    {
this->NavigateBack();
    }
}

void MenuComponent::HandleMouseInput(::InputSystem* inputSystem)
{
    if (this->ActivePanel == nullptr)
    {
this->PressedPointerItem = nullptr;
return;    }
::MenuItemRuntime *hoveredItem = this->FindHoveredItem(this->ActivePanel, inputSystem->GetMouseX(), inputSystem->GetMouseY());
    if (hoveredItem != nullptr && hoveredItem->get_Index() != this->ActivePanel->get_SelectedItemIndex() && this->IsMouseHoverSelectionUpdateRequired(inputSystem))
    {
this->SetSelection(this->ActivePanel, hoveredItem->get_Index());
    }
    if (inputSystem->WasMouseLeftButtonPressed())
    {
this->PressedPointerItem = hoveredItem;
return;    }
    if (inputSystem->WasMouseLeftButtonReleased())
    {
    if (this->IsSameRuntimeItem(this->PressedPointerItem, hoveredItem))
    {
this->ExecuteAction(hoveredItem->get_Definition());
    }
this->PressedPointerItem = nullptr;
    }
}

bool MenuComponent::IsMouseHoverSelectionUpdateRequired(::InputSystem* inputSystem)
{
    if (inputSystem == nullptr)
    {
throw new ArgumentNullException("inputSystem");
    }
    if (inputSystem->GetMouseDeltaX() != 0 || inputSystem->GetMouseDeltaY() != 0)
    {
return true;    }
return inputSystem->WasMouseLeftButtonPressed();}

bool MenuComponent::IsSameRuntimeItem(::MenuItemRuntime* left, ::MenuItemRuntime* right)
{
    if (left == nullptr || right == nullptr)
    {
return false;    }
return left->get_Index() == right->get_Index() && String::Equals(left->get_Definition()->get_PanelId(), right->get_Definition()->get_PanelId(), StringComparison::Ordinal) && String::Equals(left->get_Definition()->get_ItemId(), right->get_Definition()->get_ItemId(), StringComparison::Ordinal);}

void MenuComponent::LoadScene(std::string sceneId)
{
    if (String::IsNullOrWhiteSpace(sceneId))
    {
throw new InvalidOperationException("Scene-loading baked menu items must provide a scene id.");
    }
    if (Core::get_Instance() == nullptr)
    {
throw new InvalidOperationException("A core instance must exist before loading a scene from the baked menu.");
    }
    if (ComponentExecutionContext::get_CurrentMode() == ComponentExecutionMode::Editor)
    {
    if (Core::get_Instance()->get_SceneLoadService() == nullptr)
    {
throw new InvalidOperationException("Core scene loading services must be initialized before loading a scene from the baked menu.");
    }
    if (Core::get_Instance()->get_InitializationOptions()->get_ScenePathResolver() == nullptr)
    {
throw new InvalidOperationException("An editor scene-id path resolver must be configured before editor menu scene loading can occur.");
    }
const std::string authoredScenePath = Core::get_Instance()->get_InitializationOptions()->get_ScenePathResolver()->ResolveScenePath(sceneId);
::SceneAsset *sceneAsset = Core::get_Instance()->get_ContentManager()->Load<SceneAsset*>(authoredScenePath, RuntimeContentProcessorIds::SceneAsset);
Core::get_Instance()->get_SceneLoadService()->Load(sceneAsset);
    if (Parent != nullptr)
    {
Parent->set_Enabled(false);
    }
    }
else     if (Core::get_Instance()->get_SceneManager() == nullptr)
    {
throw new InvalidOperationException("Core scene manager must be initialized before runtime menu scene loading can occur.");
    }
else {
Core::get_Instance()->get_SceneManager()->LoadScene(sceneId, SceneLoadMode::Single);
}
}

void MenuComponent::MoveSelection(int32_t delta)
{
    if (this->ActivePanel == nullptr || this->ActivePanel->get_Items()->Length == 0)
    {
return;    }
    if (delta == 0)
    {
return;    }
int32_t nextIndex = this->ActivePanel->get_SelectedItemIndex();
    if (nextIndex < 0)
    {
nextIndex = 0;
    }
nextIndex += delta;
    if (nextIndex < 0)
    {
nextIndex = this->ActivePanel->get_Items()->Length - 1;
    }
else     if (nextIndex >= this->ActivePanel->get_Items()->Length)
    {
nextIndex = 0;
    }
this->SetSelection(this->ActivePanel, nextIndex);
}

void MenuComponent::NavigateBack()
{
    if (this->PanelHistory->Count() == 0)
    {
return;    }
const std::string previousPanelId = (*this->PanelHistory)[this->PanelHistory->Count() - 1];
this->PanelHistory->RemoveAt(this->PanelHistory->Count() - 1);
this->ActivatePanel(previousPanelId, false);
}

::InputGamepadState MenuComponent::ReadPrimaryGamepadState()
{
    if (Core::get_Instance() == nullptr || Core::get_Instance()->get_Input() == nullptr)
    {
return ::InputGamepadState();    }
return Core::get_Instance()->get_Input()->GetGamepadState(0);}

::ScrollComponent* MenuComponent::ResolveItemsScrollComponent(::Entity* panelEntity, std::string panelId)
{
List<::Entity*> *scrollEntities = new List<::Entity*>();
CollectEntitiesWithComponent<ScrollComponent*>(panelEntity, scrollEntities);
    if (scrollEntities->Count() != 1)
    {
throw new InvalidOperationException(std::string("Baked menu panel '") + panelId + std::string("' must contain exactly one scroll component."));
    }
::ScrollComponent *scrollComponent = FindRequiredComponent<ScrollComponent*>((*scrollEntities)[0]);
    if (scrollComponent->get_VisibleItemCount() < 1)
    {
throw new InvalidOperationException(std::string("Baked menu panel '") + panelId + std::string("' must expose at least one visible item row."));
    }
return scrollComponent;}

float MenuComponent::ResolveItemsScrollStep(::Entity* itemsRootEntity)
{
    if (itemsRootEntity->get_Children() == nullptr || itemsRootEntity->get_Children()->Count() == 0)
    {
return DemoMenuLayout::ButtonHeight + DemoMenuLayout::ButtonSpacing;    }
    if (itemsRootEntity->get_Children()->Count() >= 2)
    {
const float step = (*itemsRootEntity->get_Children())[1]->get_LocalPosition().Y - (*itemsRootEntity->get_Children())[0]->get_LocalPosition().Y;
    if (step > 0.0f)
    {
return step;    }
    }
::RoundedRectComponent *background = FindFirstComponent<RoundedRectComponent*>((*itemsRootEntity->get_Children())[0]);
    if (background != nullptr && background->get_Size().Y > 0)
    {
return background->get_Size().Y;    }
return DemoMenuLayout::ButtonHeight + DemoMenuLayout::ButtonSpacing;}

::TextComponent* MenuComponent::ResolveSelectedDescriptionText(::Entity* panelEntity)
{
List<::Entity*> *markerEntities = new List<::Entity*>();
CollectEntitiesWithComponent<MenuSelectedDescriptionComponent*>(panelEntity, markerEntities);
    if (markerEntities->Count() != 1)
    {
throw new InvalidOperationException("Each baked menu panel must contain exactly one selected-description marker.");
    }
return FindRequiredComponent<TextComponent*>((*markerEntities)[0]);}

int32_t MenuComponent::ResolveSelectedIndex(::MenuPanelRuntime* panelRuntime)
{
    if (panelRuntime->get_SelectedItemIndex() >= 0 && panelRuntime->get_SelectedItemIndex() < panelRuntime->get_Items()->Length)
    {
return panelRuntime->get_SelectedItemIndex();    }
return 0;}

void MenuComponent::SetSelection(::MenuPanelRuntime* panelRuntime, int32_t itemIndex)
{
    if (panelRuntime == nullptr)
    {
throw new ArgumentNullException("panelRuntime");
    }
    if (itemIndex < 0 || itemIndex >= panelRuntime->get_Items()->Length)
    {
throw ([&]() {
auto __ctor_arg_00000219 = "itemIndex";
auto __ctor_arg_0000021A = "Selected baked menu item index must be valid.";
return new ArgumentOutOfRangeException(__ctor_arg_00000219, __ctor_arg_0000021A);
})();
    }
panelRuntime->set_SelectedItemIndex(itemIndex);
for (int32_t index = 0; index < panelRuntime->get_Items()->Length; index++) {
::MenuItemRuntime *runtimeItem = (*panelRuntime->get_Items())[index];
const bool isSelected = index == itemIndex;
this->ApplyItemVisualState(runtimeItem, isSelected);
}
::MenuItemRuntime *selectedItem = (*panelRuntime->get_Items())[itemIndex];
this->SelectedItemIdValue = selectedItem->get_Definition()->get_ItemId();
panelRuntime->get_SelectedDescriptionText()->set_Text(selectedItem->get_Definition()->get_Description());
this->EnsureSelectedItemVisible(panelRuntime, itemIndex);
this->ApplyItemsScrollOffset(panelRuntime->get_ItemsRootEntity(), panelRuntime->get_ItemsScrollComponent()->get_ScrollOffset());
}

template <typename TComponent>
bool MenuComponent::TryFindComponent(::Entity* entity, TComponent& component)
{
    if (entity != nullptr && entity->get_Components() != nullptr)
    {
for (int32_t componentIndex = 0; componentIndex < entity->get_Components()->Count(); componentIndex++) {
    TComponent typedComponent = he_cpp_try_cast<TComponent>((*entity->get_Components())[componentIndex]);
    if (typedComponent != nullptr)
    {
component = typedComponent;
return true;    }
}
    }
component = nullptr;
return false;}

void MenuComponent::TryInitialize()
{
    if (Parent == nullptr)
    {
return;    }
::Entity *generatedRootEntity = this->FindGeneratedRootEntity(Parent);
    if (generatedRootEntity == nullptr)
    {
return;    }
this->BindPanels(Parent);
this->ActivatePanel(this->InitialPanelIdValue, false);
this->PreviousGamepadState = this->ReadPrimaryGamepadState();
this->set_IsInitialized(true);
}

bool MenuComponent::WasGamepadButtonPressed(::InputGamepadState currentState, ::InputGamepadState previousState, ::InputGamepadButton button)
{
return currentState.IsButtonDown(button) && !previousState.IsButtonDown(button);}

