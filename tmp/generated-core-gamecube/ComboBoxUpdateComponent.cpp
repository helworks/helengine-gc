#ifdef DrawText
#undef DrawText
#endif
#include "ComboBoxUpdateComponent.hpp"
#include "ComboBoxComponent.hpp"
#include "runtime/native_exceptions.hpp"

ComboBoxUpdateComponent::ComboBoxUpdateComponent(::ComboBoxComponent* comboBox) : comboBox()
{
    if (comboBox == nullptr)
    {
throw new ArgumentNullException("comboBox");
    }
this->comboBox = comboBox;
}

void ComboBoxUpdateComponent::Update()
{
this->comboBox->Update();
}

uint8_t ComboBoxUpdateComponent::get_UpdateOrder()
{
return this->UpdateComponent::get_UpdateOrder();
}

void ComboBoxUpdateComponent::set_UpdateOrder(uint8_t value)
{
this->UpdateComponent::set_UpdateOrder(value);
}

::Entity* ComboBoxUpdateComponent::get_Parent()
{
return this->Component::get_Parent();
}

void ComboBoxUpdateComponent::set_Parent(::Entity* value)
{
this->Component::set_Parent(value);
}

