#ifdef DrawText
#undef DrawText
#endif
#include "TextBoxShortcutRegistry.hpp"
#include "TextBoxShortcutBinding.hpp"
#include "Keys.hpp"

::TextBoxShortcutBinding* TextBoxShortcutRegistry::get_CopyShortcut()
{
return this->CopyShortcut;
}

void TextBoxShortcutRegistry::set_CopyShortcut(::TextBoxShortcutBinding* value)
{
this->CopyShortcut = value;
}

::TextBoxShortcutBinding* TextBoxShortcutRegistry::get_PasteShortcut()
{
return this->PasteShortcut;
}

void TextBoxShortcutRegistry::set_PasteShortcut(::TextBoxShortcutBinding* value)
{
this->PasteShortcut = value;
}

::TextBoxShortcutBinding* TextBoxShortcutRegistry::get_SelectAllShortcut()
{
return this->SelectAllShortcut;
}

void TextBoxShortcutRegistry::set_SelectAllShortcut(::TextBoxShortcutBinding* value)
{
this->SelectAllShortcut = value;
}

TextBoxShortcutRegistry::TextBoxShortcutRegistry() : CopyShortcut(), PasteShortcut(), SelectAllShortcut()
{
this->set_SelectAllShortcut(new ::TextBoxShortcutBinding(Keys::A, true, false, false));
this->set_CopyShortcut(new ::TextBoxShortcutBinding(Keys::C, true, false, false));
this->set_PasteShortcut(new ::TextBoxShortcutBinding(Keys::V, true, false, false));
}

