#ifdef DrawText
#undef DrawText
#endif
#include "ButtonComponent.hpp"
#include "runtime/native_exceptions.hpp"
#include "Component.hpp"
#include "RenderOrder2D.hpp"
#include "Entity.hpp"
#include "float3.hpp"
#include "byte4.hpp"
#include "FontTightMetrics.hpp"
#include "FontAsset.hpp"
#include "system/math.hpp"
#include "ThemeManager.hpp"
#include "RoundedRectCorners.hpp"
#include "Keys.hpp"
#include "RoundedRectComponent.hpp"
#include "InteractableComponent.hpp"
#include "TextComponent.hpp"
#include "int2.hpp"
#include "PointerInteraction.hpp"
#include "system/math.hpp"
#include "runtime/native_event.hpp"
#include "runtime/native_exceptions.hpp"

::int2 ButtonComponent::get_AnchorSize()
{
return this->size;
}

bool ButtonComponent::get_CanReceiveFocus()
{
return Parent != nullptr && Parent->get_IsHierarchyEnabled() && this->interactableComponent != nullptr;
}

::RoundedRectCorners ButtonComponent::get_Corners()
{
return this->Corners;
}

void ButtonComponent::set_Corners(::RoundedRectCorners value)
{
this->Corners = value;
}

::IFocusGroup* ButtonComponent::get_FocusGroup()
{
return this->FocusGroup;
}

void ButtonComponent::set_FocusGroup(::IFocusGroup* value)
{
this->FocusGroup = value;
}

::FontAsset* ButtonComponent::get_Font()
{
return this->font;}

void ButtonComponent::set_Font(::FontAsset* value)
{
    if (value == nullptr)
    {
throw new ArgumentNullException("value");
    }
this->font = value;
    if (this->textComponent != nullptr)
    {
this->textComponent->set_Font(this->font);
    }
this->ApplyTextLayout();
}

bool ButtonComponent::get_IsDefaultTarget()
{
return this->IsDefaultTarget;
}

void ButtonComponent::set_IsDefaultTarget(bool value)
{
this->IsDefaultTarget = value;
}

bool ButtonComponent::get_IsKeyboardFocused()
{
return this->IsKeyboardFocused;
}

void ButtonComponent::set_IsKeyboardFocused(bool value)
{
this->IsKeyboardFocused = value;
}

::int2 ButtonComponent::get_Size()
{
return this->size;
}

int32_t ButtonComponent::get_TabIndex()
{
return this->TabIndex;
}

void ButtonComponent::set_TabIndex(int32_t value)
{
this->TabIndex = value;
}

void ButtonComponent::ActivateFromKey(::Keys key)
{
    if (!this->CanActivateWithKey(key))
    {
return;    }
if (this->onClickAction != nullptr)
{
(*this->onClickAction)();
}
}

ButtonComponent::ButtonComponent(std::string text, ::int2 size, ::FontAsset* font, Action<>* onClickAction, float borderThickness) : Corners(), FocusGroup(), Hovered(), IsDefaultTarget(), IsKeyboardFocused(), TabIndex(0), BackgroundRenderOrder(), ButtonTextColor(), CornerRadius(), FocusedBorderColor(), FocusedFillColor(), HasRenderOrderOverrides(), HoverCursorKind(), HoverFillColor(), IdleBorderColor(), IdleFillColor(), PressedFillColor(), TextRenderOrder(), UsesHoverOnlyBackground(), borderThickness(), font(), interactableComponent(), isHovering(), isPressed(), onClickAction(), roundedRect(), size(), text(), textComponent(), textEntity()
{
this->text = text;
this->size = size;
this->font = font;
this->onClickAction = onClickAction;
this->borderThickness = borderThickness;
this->ButtonTextColor = ThemeManager::get_Colors()->get_TextOnAccent();
this->IdleFillColor = ThemeManager::get_Colors()->get_AccentSecondary();
this->HoverFillColor = ThemeManager::get_Colors()->get_AccentPrimary();
this->PressedFillColor = ThemeManager::get_Colors()->get_AccentTertiary();
this->FocusedFillColor = this->IdleFillColor;
this->IdleBorderColor = ThemeManager::get_Colors()->get_AccentTertiary();
this->FocusedBorderColor = ThemeManager::get_Colors()->get_AccentPrimary();
this->set_Corners(RoundedRectCorners::All);
this->UpdateCornerRadius();
}

bool ButtonComponent::CanActivateWithKey(::Keys key)
{
return key == Keys::Enter || key == Keys::Space;}

void ButtonComponent::ComponentAdded(::Entity* entity)
{
Component::ComponentAdded(entity);
    if (!entity->get_Enabled())
    {
return;    }
uint8_t backgroundOrder = RenderOrder2D::PanelSurface;
uint8_t textOrder = RenderOrder2D::PanelForeground;
    if (this->HasRenderOrderOverrides)
    {
backgroundOrder = this->BackgroundRenderOrder;
textOrder = this->TextRenderOrder;
    }
this->roundedRect = new ::RoundedRectComponent();
this->roundedRect->set_Size(this->size);
this->roundedRect->set_Corners(this->Corners);
this->roundedRect->set_Radius(this->CornerRadius);
this->roundedRect->set_BorderThickness(this->borderThickness);
this->roundedRect->set_FillColor(ThemeManager::get_Colors()->get_AccentSecondary());
this->roundedRect->set_BorderColor(ThemeManager::get_Colors()->get_AccentTertiary());
this->roundedRect->set_RenderOrder2D(backgroundOrder);
entity->AddComponent(this->roundedRect);
this->UpdateButtonColor();
this->interactableComponent = new ::InteractableComponent();
this->interactableComponent->set_Size(this->size);
this->interactableComponent->set_HoverCursor(this->HoverCursorKind);
this->interactableComponent->CursorEvent += &ButtonComponent::OnCursorEvent;
entity->AddComponent(this->interactableComponent);
this->textEntity = new ::Entity();
this->textEntity->set_LayerMask(entity->get_LayerMask());
this->textEntity->set_Enabled(true);
this->textEntity->InitComponents();
entity->InitChildren();
entity->AddChild(this->textEntity);
this->textComponent = new ::TextComponent();
this->textComponent->set_Text(this->text);
this->textComponent->set_Font(this->font);
this->textComponent->set_Color(this->ButtonTextColor);
this->textComponent->set_Size(::int2(1, 1));
this->textComponent->set_RenderOrder2D(textOrder);
this->textEntity->AddComponent(this->textComponent);
this->ApplyTextLayout();
}

void ButtonComponent::ComponentRemoved(::Entity* entity)
{
Component::ComponentRemoved(entity);
this->isHovering = false;
this->isPressed = false;
this->SetTargetFocused(false);
}

bool ButtonComponent::ContainsScreenPoint(int32_t x, int32_t y)
{
    if (Parent == nullptr)
    {
return false;    }
::float3 position = Parent->get_Position();
return x >= position.X && x < position.X + this->size.X && y >= position.Y && y < position.Y + this->size.Y;}

void ButtonComponent::ParentEnabledChange(bool newEnabled)
{
Component::ParentEnabledChange(newEnabled);
    if (!newEnabled)
    {
this->isHovering = false;
this->isPressed = false;
this->SetTargetFocused(false);
    }
    if (this->textEntity != nullptr)
    {
this->textEntity->set_Enabled(newEnabled);
    }
}

void ButtonComponent::SetCornerRadius(float cornerRadius)
{
    if (cornerRadius < 0.0f)
    {
throw ([&]() {
auto __ctor_arg_000002AE = "cornerRadius";
auto __ctor_arg_000002AF = "Corner radius must not be negative.";
return new ArgumentOutOfRangeException(__ctor_arg_000002AE, __ctor_arg_000002AF);
})();
    }
this->CornerRadius = cornerRadius;
    if (this->roundedRect != nullptr)
    {
this->roundedRect->set_Radius(this->CornerRadius);
    }
}

void ButtonComponent::SetHoverCursor(::PointerCursorKind cursor)
{
this->HoverCursorKind = cursor;
    if (this->interactableComponent != nullptr)
    {
this->interactableComponent->set_HoverCursor(cursor);
    }
}

void ButtonComponent::SetRenderOrders(uint8_t backgroundOrder, uint8_t textOrder)
{
this->HasRenderOrderOverrides = true;
this->BackgroundRenderOrder = backgroundOrder;
this->TextRenderOrder = textOrder;
    if (this->roundedRect != nullptr)
    {
this->roundedRect->set_RenderOrder2D(backgroundOrder);
    }
    if (this->textComponent != nullptr)
    {
this->textComponent->set_RenderOrder2D(textOrder);
    }
}

void ButtonComponent::SetSize(::int2 newSize)
{
    if (newSize.X < 1 || newSize.Y < 1)
    {
throw ([&]() {
auto __ctor_arg_000002B0 = "newSize";
auto __ctor_arg_000002B1 = "Button size must be positive.";
return new ArgumentOutOfRangeException(__ctor_arg_000002B0, __ctor_arg_000002B1);
})();
    }
this->size = newSize;
    if (this->Corners != RoundedRectCorners::None)
    {
this->UpdateCornerRadius();
    }
    if (this->roundedRect != nullptr)
    {
this->roundedRect->set_Size(this->size);
this->roundedRect->set_Corners(this->Corners);
this->roundedRect->set_Radius(this->CornerRadius);
    }
    if (this->interactableComponent != nullptr)
    {
this->interactableComponent->set_Size(this->size);
this->interactableComponent->set_HoverCursor(this->HoverCursorKind);
    }
    if (this->textEntity == nullptr || this->textComponent == nullptr)
    {
return;    }
this->ApplyTextLayout();
}

void ButtonComponent::SetTargetFocused(bool isFocused)
{
    if (this->IsKeyboardFocused == isFocused)
    {
this->UpdateButtonColor();
return;    }
this->set_IsKeyboardFocused(isFocused);
this->UpdateButtonColor();
}

void ButtonComponent::SetTextColor(::byte4 color)
{
this->ButtonTextColor = color;
    if (this->textComponent != nullptr)
    {
this->textComponent->set_Color(color);
    }
}

void ButtonComponent::SetVisualPalette(::byte4 idleFillColor, ::byte4 hoverFillColor, ::byte4 pressedFillColor, ::byte4 focusedFillColor, ::byte4 idleBorderColor, ::byte4 focusedBorderColor)
{
this->IdleFillColor = idleFillColor;
this->HoverFillColor = hoverFillColor;
this->PressedFillColor = pressedFillColor;
this->FocusedFillColor = focusedFillColor;
this->IdleBorderColor = idleBorderColor;
this->FocusedBorderColor = focusedBorderColor;
this->UpdateButtonColor();
}

void ButtonComponent::UseHoverOnlyBackground()
{
this->UsesHoverOnlyBackground = true;
this->UpdateButtonColor();
}

void ButtonComponent::UseSquareCorners()
{
this->set_Corners(RoundedRectCorners::None);
this->CornerRadius = 0.0f;
    if (this->roundedRect != nullptr)
    {
this->roundedRect->set_Corners(this->Corners);
this->roundedRect->set_Radius(this->CornerRadius);
    }
}

void ButtonComponent::UseTopCorners()
{
this->set_Corners(static_cast<RoundedRectCorners>((static_cast<int32_t>(RoundedRectCorners::TopLeft) + static_cast<int32_t>(RoundedRectCorners::TopRight))));
this->UpdateCornerRadius();
    if (this->roundedRect != nullptr)
    {
this->roundedRect->set_Corners(this->Corners);
this->roundedRect->set_Radius(this->CornerRadius);
    }
}

::Entity* ButtonComponent::get_Parent()
{
return this->Component::get_Parent();
}

void ButtonComponent::set_Parent(::Entity* value)
{
this->Component::set_Parent(value);
}

::byte4 ButtonComponent::TransparentBackgroundColor = ::byte4(255, 255, 255, 0);

void ButtonComponent::ApplyTextLayout()
{
    if (this->textEntity == nullptr || this->textComponent == nullptr)
    {
return;    }
::FontTightMetrics tight = this->font->MeasureTight(this->text);
const double lineHeight = Math::Max(static_cast<double>(this->font->get_LineHeight()), 1.0);
double px = (static_cast<double>(this->size.X) - tight.Width) / 2.0;
double py = (static_cast<double>(this->size.Y) - lineHeight) / 2.0;
px = Math::Round(px);
py = Math::Round(py);
this->textEntity->set_Position(::float3(static_cast<float>(px), static_cast<float>(py), 0.1f));
this->textComponent->set_Size(([&]() {
auto __ctor_arg_000002B2 = static_cast<int32_t>(Math::Ceiling(tight.Width));
auto __ctor_arg_000002B3 = static_cast<int32_t>(Math::Ceiling(lineHeight));
return ::int2(__ctor_arg_000002B2, __ctor_arg_000002B3);
})());
}

::byte4 ButtonComponent::GetIdleBorderColor()
{
    if (this->UsesHoverOnlyBackground)
    {
return TransparentBackgroundColor;    }
return this->IdleBorderColor;}

::byte4 ButtonComponent::GetIdleFillColor()
{
    if (this->UsesHoverOnlyBackground)
    {
return TransparentBackgroundColor;    }
return this->IdleFillColor;}

void ButtonComponent::OnCursorEvent(::int2 relPos, ::int2 delta, ::PointerInteraction state)
{
switch (state) {
case PointerInteraction::Hover: {
    if (!this->isHovering)
    {
this->isHovering = true;
this->UpdateButtonColor();
this->RaiseHovered();
    }
break;
}
case PointerInteraction::Press: {
this->isPressed = true;
this->UpdateButtonColor();
break;
}
case PointerInteraction::Release: {
    if (this->isPressed && this->isHovering)
    {
if (this->onClickAction != nullptr)
{
(*this->onClickAction)();
}
    }
this->isPressed = false;
this->UpdateButtonColor();
break;
}
case PointerInteraction::Leave: {
    if (this->isHovering || this->isPressed)
    {
this->isHovering = false;
this->isPressed = false;
this->UpdateButtonColor();
    }
break;
}
case PointerInteraction::None: {
break;
}
}

}

void ButtonComponent::RaiseHovered()
{
    if (true)
    {
this->Hovered.Invoke();
    }
}

void ButtonComponent::UpdateButtonColor()
{
    if (this->roundedRect == nullptr)
    {
return;    }
this->roundedRect->set_BorderColor(this->IsKeyboardFocused ? FocusedBorderColor : GetIdleBorderColor());
    if (this->isPressed)
    {
this->roundedRect->set_FillColor(this->PressedFillColor);
    }
else     if (this->isHovering)
    {
this->roundedRect->set_FillColor(this->HoverFillColor);
    }
else     if (this->IsKeyboardFocused)
    {
this->roundedRect->set_FillColor(this->FocusedFillColor);
    }
else {
this->roundedRect->set_FillColor(this->GetIdleFillColor());
}
}

void ButtonComponent::UpdateCornerRadius()
{
this->CornerRadius = static_cast<float>((Math::Min(static_cast<double>(this->size.X), static_cast<double>(this->size.Y)) * 0.15));
}

