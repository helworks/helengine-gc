#ifdef DrawText
#undef DrawText
#endif
#include "RenderList2D.hpp"
#include "runtime/native_list.hpp"
#include "Logger.hpp"
#include "runtime/native_exceptions.hpp"
#include "IDrawable2D.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"

int32_t RenderList2D::get_Count()
{
return this->items->Count();}

void RenderList2D::Add(::IDrawable2D* drawable)
{
const int32_t insertIndex = this->FindInsertIndex(drawable != nullptr ? drawable->get_RenderOrder2D() : static_cast<uint8_t>(0));
this->items->Insert(insertIndex, drawable);
}

void RenderList2D::Clear()
{
this->items->Clear();
}

void RenderList2D::EnsureCapacity(int32_t desiredCount)
{
this->EnsureCapacity(desiredCount, false);
}

void RenderList2D::EnsureCapacity(int32_t desiredCount, bool warnOnExpand)
{
    if (desiredCount <= this->items->Capacity())
    {
return;    }
const int32_t oldCap = this->items->Capacity();
this->items->SetCapacity(desiredCount);
    if (warnOnExpand)
    {
Logger::WriteWarning(std::string("RenderList2D expanded from ") + std::to_string(oldCap) + std::string(" to ") + std::to_string(items->Capacity()) + std::string("."));
    }
}

bool RenderList2D::Remove(::IDrawable2D* drawable)
{
const int32_t index = this->FindIndexByReference(drawable);
    if (index < 0)
    {
return false;    }
this->items->RemoveAt(index);
return true;}

RenderList2D::RenderList2D(int32_t initialCapacity) : items()
{
    if (initialCapacity < 0)
    {
throw new ArgumentOutOfRangeException("initialCapacity");
    }
this->items = new List<::IDrawable2D*>(initialCapacity);
}

void RenderList2D::VisitOrdered(::IRenderVisitor2D* visitor)
{
    if (visitor == nullptr)
    {
throw new ArgumentNullException("visitor");
    }
for (int32_t i = 0; i < this->items->Count(); i++) {
visitor->Visit((*this->items)[i]);
}
}

int32_t RenderList2D::FindIndexByReference(::IDrawable2D* drawable)
{
for (int32_t i = 0; i < this->items->Count(); i++) {
    if (((*this->items)[i] == drawable))
    {
return i;    }
}
return -1;}

int32_t RenderList2D::FindInsertIndex(uint8_t renderOrder)
{
for (int32_t i = 0; i < this->items->Count(); i++) {
::IDrawable2D *current = (*this->items)[i];
const uint8_t currentOrder = current != nullptr ? current->get_RenderOrder2D() : static_cast<uint8_t>(0);
    if (renderOrder < currentOrder)
    {
return i;    }
}
return this->items->Count();}

