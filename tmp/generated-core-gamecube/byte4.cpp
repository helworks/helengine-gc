#ifdef DrawText
#undef DrawText
#endif
#include "byte4.hpp"
#include "runtime/native_string.hpp"

byte4::byte4() : W(), X(), Y(), Z()
{
}

std::string byte4::ToString()
{
return std::to_string(this->X) + std::string(", ") + std::to_string(Y) + std::string(", ") + std::to_string(Z) + std::string(", ") + std::to_string(W);}

byte4::byte4(uint8_t x, uint8_t y, uint8_t z, uint8_t w) : W(), X(), Y(), Z()
{
this->X = x;
this->Y = y;
this->Z = z;
this->W = w;
}

