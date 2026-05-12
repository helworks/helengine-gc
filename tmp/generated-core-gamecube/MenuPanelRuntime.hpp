#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class MenuPanelComponent;
class MenuItemRuntime;
class Entity;
class ScrollComponent;
class TextComponent;

#include "MenuPanelComponent.hpp"
#include "runtime/array.hpp"
#include "MenuItemRuntime.hpp"
#include "Entity.hpp"
#include "ScrollComponent.hpp"
#include "TextComponent.hpp"

class MenuPanelRuntime
{
public:
    virtual ~MenuPanelRuntime() = default;

    ::MenuPanelComponent* Definition;

    ::MenuPanelComponent* get_Definition();

    Array<::MenuItemRuntime*>* Items;

    Array<::MenuItemRuntime*>* get_Items();

    ::Entity* ItemsRootEntity;

    ::Entity* get_ItemsRootEntity();

    ::ScrollComponent* ItemsScrollComponent;

    ::ScrollComponent* get_ItemsScrollComponent();

    ::Entity* RootEntity;

    ::Entity* get_RootEntity();

    ::TextComponent* SelectedDescriptionText;

    ::TextComponent* get_SelectedDescriptionText();

    int32_t SelectedItemIndex;

    int32_t get_SelectedItemIndex();
    void set_SelectedItemIndex(int32_t value);

    MenuPanelRuntime(::MenuPanelComponent* definition, ::Entity* rootEntity, ::TextComponent* selectedDescriptionText, ::Entity* itemsRootEntity, ::ScrollComponent* itemsScrollComponent, Array<::MenuItemRuntime*>* items);
};
