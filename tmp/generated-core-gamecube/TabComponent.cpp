#ifdef DrawText
#undef DrawText
#endif
#include "TabComponent.hpp"
#include "ThemeManager.hpp"

bool TabComponent::get_IsSelected()
{
return IsKeyboardFocused;
}

void TabComponent::SetSelected(bool isSelected)
{
SetTargetFocused(isSelected);
}

TabComponent::TabComponent(std::string text, ::int2 size, ::FontAsset* font, Action<>* onClickAction, float borderThickness) : ButtonComponent(text, size, font, onClickAction, borderThickness)
{
UseTopCorners();
SetCornerRadius(static_cast<float>((size.Y * 0.3)));
SetTextColor(ThemeManager::get_Colors()->get_AccentQuaternary());
SetVisualPalette(ThemeManager::get_Colors()->get_SurfacePrimary(), ThemeManager::get_Colors()->get_AccentSecondary(), ThemeManager::get_Colors()->get_AccentTertiary(), ThemeManager::get_Colors()->get_SurfaceInput(), ThemeManager::get_Colors()->get_AccentTertiary(), ThemeManager::get_Colors()->get_AccentTertiary());
}

::int2 TabComponent::get_AnchorSize()
{
return this->ButtonComponent::get_AnchorSize();
}

bool TabComponent::get_CanReceiveFocus()
{
return this->ButtonComponent::get_CanReceiveFocus();
}

::RoundedRectCorners TabComponent::get_Corners()
{
return this->ButtonComponent::get_Corners();
}

void TabComponent::set_Corners(::RoundedRectCorners value)
{
this->ButtonComponent::set_Corners(value);
}

::IFocusGroup* TabComponent::get_FocusGroup()
{
return this->ButtonComponent::get_FocusGroup();
}

void TabComponent::set_FocusGroup(::IFocusGroup* value)
{
this->ButtonComponent::set_FocusGroup(value);
}

::FontAsset* TabComponent::get_Font()
{
return this->ButtonComponent::get_Font();
}

void TabComponent::set_Font(::FontAsset* value)
{
this->ButtonComponent::set_Font(value);
}

bool TabComponent::get_IsDefaultTarget()
{
return this->ButtonComponent::get_IsDefaultTarget();
}

void TabComponent::set_IsDefaultTarget(bool value)
{
this->ButtonComponent::set_IsDefaultTarget(value);
}

bool TabComponent::get_IsKeyboardFocused()
{
return this->ButtonComponent::get_IsKeyboardFocused();
}

void TabComponent::set_IsKeyboardFocused(bool value)
{
this->ButtonComponent::set_IsKeyboardFocused(value);
}

::int2 TabComponent::get_Size()
{
return this->ButtonComponent::get_Size();
}

int32_t TabComponent::get_TabIndex()
{
return this->ButtonComponent::get_TabIndex();
}

void TabComponent::set_TabIndex(int32_t value)
{
this->ButtonComponent::set_TabIndex(value);
}

::Entity* TabComponent::get_Parent()
{
return this->Component::get_Parent();
}

void TabComponent::set_Parent(::Entity* value)
{
this->Component::set_Parent(value);
}

