#ifdef DrawText
#undef DrawText
#endif
#include "InputTextState.hpp"
#include "runtime/array.hpp"

InputTextState::InputTextState() : CharacterCount(0), Characters()
{
}

int32_t InputTextState::get_CharacterCount()
{
return this->CharacterCount;
}

void InputTextState::set_CharacterCount(int32_t value)
{
this->CharacterCount = value;
}

Array<char>* InputTextState::get_Characters()
{
return this->Characters;
}

void InputTextState::set_Characters(Array<char>* value)
{
this->Characters = value;
}

