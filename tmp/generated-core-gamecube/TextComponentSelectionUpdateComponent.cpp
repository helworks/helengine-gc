#ifdef DrawText
#undef DrawText
#endif
#include "TextComponentSelectionUpdateComponent.hpp"
#include "TextComponent.hpp"
#include "runtime/native_exceptions.hpp"

::TextComponent* TextComponentSelectionUpdateComponent::get_TextComponent()
{
return this->TextComponent;
}

TextComponentSelectionUpdateComponent::TextComponentSelectionUpdateComponent(::TextComponent* textComponent) : TextComponent()
{
this->TextComponent = (textComponent != nullptr ? textComponent : throw new ArgumentNullException("textComponent"));
}

void TextComponentSelectionUpdateComponent::Update()
{
this->TextComponent->UpdateSelectionInput();
}

uint8_t TextComponentSelectionUpdateComponent::get_UpdateOrder()
{
return this->UpdateComponent::get_UpdateOrder();
}

void TextComponentSelectionUpdateComponent::set_UpdateOrder(uint8_t value)
{
this->UpdateComponent::set_UpdateOrder(value);
}

::Entity* TextComponentSelectionUpdateComponent::get_Parent()
{
return this->Component::get_Parent();
}

void TextComponentSelectionUpdateComponent::set_Parent(::Entity* value)
{
this->Component::set_Parent(value);
}

