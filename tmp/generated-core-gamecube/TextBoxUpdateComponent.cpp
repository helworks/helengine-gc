#ifdef DrawText
#undef DrawText
#endif
#include "TextBoxUpdateComponent.hpp"
#include "TextBoxComponent.hpp"
#include "InputSystem.hpp"
#include "Core.hpp"

TextBoxUpdateComponent::TextBoxUpdateComponent(::TextBoxComponent* textBox) : textBox()
{
this->textBox = textBox;
}

void TextBoxUpdateComponent::Update()
{
this->textBox->Update();
    if (!this->textBox->get_IsFocused())
    {
return;    }
::InputSystem *input = Core::get_Instance()->get_Input();
    if (!input->WasMouseLeftButtonPressed())
    {
return;    }
const int32_t pointerX = input->GetMouseX();
const int32_t pointerY = input->GetMouseY();
    if (!this->textBox->ContainsScreenPoint(pointerX, pointerY))
    {
this->textBox->set_IsFocused(false);
    }
}

uint8_t TextBoxUpdateComponent::get_UpdateOrder()
{
return this->UpdateComponent::get_UpdateOrder();
}

void TextBoxUpdateComponent::set_UpdateOrder(uint8_t value)
{
this->UpdateComponent::set_UpdateOrder(value);
}

::Entity* TextBoxUpdateComponent::get_Parent()
{
return this->Component::get_Parent();
}

void TextBoxUpdateComponent::set_Parent(::Entity* value)
{
this->Component::set_Parent(value);
}

