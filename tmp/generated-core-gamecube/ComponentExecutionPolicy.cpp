#ifdef DrawText
#undef DrawText
#endif
#include "ComponentExecutionPolicy.hpp"
#include "runtime/native_exceptions.hpp"
#include "ComponentExecutionContext.hpp"
#include "ComponentExecutionMode.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_type.hpp"

bool ComponentExecutionPolicy::ShouldRunComponentLifecycle(::Component* component, ::Entity* entity)
{
    if (component == nullptr)
    {
throw new ArgumentNullException("component");
    }
    if (entity == nullptr)
    {
throw new ArgumentNullException("entity");
    }
    if (ComponentExecutionContext::get_CurrentMode() != ComponentExecutionMode::Editor)
    {
return true;    }
    if (!entity->get_SuppressUpdateComponentExecutionInEditor())
    {
return true;    }
return false;}

