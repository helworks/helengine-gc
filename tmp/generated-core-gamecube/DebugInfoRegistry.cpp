#ifdef DrawText
#undef DrawText
#endif
#include "DebugInfoRegistry.hpp"
#include "runtime/native_list.hpp"
#include "IDebugInfoProvider.hpp"
#include "runtime/native_list.hpp"

void DebugInfoRegistry::Register(::IDebugInfoProvider* provider)
{
    if (provider == nullptr)
    {
return;    }
// Lock omitted in TypeScript
providers->Add(provider);
}

List<ValueTuple<std::string, std::string, std::string>*>* DebugInfoRegistry::Snapshot()
{
List<ValueTuple<std::string, std::string, std::string>*> *result = new List<ValueTuple<std::string, std::string, std::string>*>();
// Lock omitted in TypeScript
for (int32_t i = 0; i < providers->Count(); i++) {
::IDebugInfoProvider *p = (*providers)[i];
List<ValueTuple<std::string, std::string>*> *items = new List<ValueTuple<std::string, std::string>*>();
try {
p->AppendInfo(items);
}
catch (...) {
}
for (int32_t j = 0; j < items->Count(); j++) {
ValueTuple<std::string, std::string> *it = (*items)[j];
result->Add(new ValueTuple<std::string, std::string, std::string>(p->get_Category(), it->Item1, it->Item2));
}
}
return result;}

List<::IDebugInfoProvider*>* DebugInfoRegistry::providers = new List<::IDebugInfoProvider*>();

void* DebugInfoRegistry::sync = new char[1];

