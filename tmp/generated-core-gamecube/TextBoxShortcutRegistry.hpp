#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class TextBoxShortcutBinding;

#include "TextBoxShortcutBinding.hpp"

class TextBoxShortcutRegistry
{
public:
    virtual ~TextBoxShortcutRegistry() = default;

    ::TextBoxShortcutBinding* CopyShortcut;

    ::TextBoxShortcutBinding* get_CopyShortcut();
    void set_CopyShortcut(::TextBoxShortcutBinding* value);

    ::TextBoxShortcutBinding* PasteShortcut;

    ::TextBoxShortcutBinding* get_PasteShortcut();
    void set_PasteShortcut(::TextBoxShortcutBinding* value);

    ::TextBoxShortcutBinding* SelectAllShortcut;

    ::TextBoxShortcutBinding* get_SelectAllShortcut();
    void set_SelectAllShortcut(::TextBoxShortcutBinding* value);

    TextBoxShortcutRegistry();
};
