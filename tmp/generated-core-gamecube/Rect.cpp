#ifdef DrawText
#undef DrawText
#endif
#include "Rect.hpp"

Rect::Rect() : H(0), W(0), X(0), Y(0)
{
}

Rect::Rect(int32_t x, int32_t y, int32_t w, int32_t h) : H(0), W(0), X(0), Y(0)
{
this->X = x;
this->Y = y;
this->W = w;
this->H = h;
}

