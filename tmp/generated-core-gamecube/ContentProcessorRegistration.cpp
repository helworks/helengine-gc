#ifdef DrawText
#undef DrawText
#endif
#include "ContentProcessorRegistration.hpp"
#include "runtime/array.hpp"
#include "runtime/native_string.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_type.hpp"

Array<std::string>* ContentProcessorRegistration::get_Extensions()
{
return this->ExtensionsValue;
}

Type* ContentProcessorRegistration::get_OutputType()
{
return this->ProcessorValue->get_OutputType();
}

::IContentProcessor* ContentProcessorRegistration::get_Processor()
{
return this->ProcessorValue;
}

std::string ContentProcessorRegistration::get_ProcessorId()
{
return this->ProcessorIdValue;
}

ContentProcessorRegistration::ContentProcessorRegistration(std::string processorId, ::IContentProcessor* processor, Array<std::string>* extensions) : ExtensionsValue(), ProcessorIdValue(), ProcessorValue()
{
    if (String::IsNullOrWhiteSpace(processorId))
    {
throw ([&]() {
auto __ctor_arg_00000036 = "Processor id must be provided.";
auto __ctor_arg_00000037 = "processorId";
return new ArgumentException(__ctor_arg_00000036, __ctor_arg_00000037);
})();
    }
    if (processor == nullptr)
    {
throw new ArgumentNullException("processor");
    }
this->ProcessorIdValue = processorId;
this->ProcessorValue = processor;
this->ExtensionsValue = extensions == nullptr ? Array<std::string>::Empty() : this->NormalizeExtensions(extensions);
}

std::string ContentProcessorRegistration::NormalizeExtension(std::string extension)
{
    if (String::Equals(extension, "*", StringComparison::Ordinal))
    {
return extension;    }
    if (!String::StartsWith(extension, "."))
    {
extension = String::Concat(".", extension);
    }
return String::ToLowerInvariant(extension);}

Array<std::string>* ContentProcessorRegistration::NormalizeExtensions(Array<std::string>* sourceExtensions)
{
Array<std::string> *normalized = new Array<std::string>(sourceExtensions->Length);
for (int32_t extensionIndex = 0; extensionIndex < sourceExtensions->Length; extensionIndex++) {
const std::string extension = (*sourceExtensions)[extensionIndex];
    if (String::IsNullOrWhiteSpace(extension))
    {
throw ([&]() {
auto __ctor_arg_00000038 = "Extension values must be non-empty.";
auto __ctor_arg_00000039 = "sourceExtensions";
return new ArgumentException(__ctor_arg_00000038, __ctor_arg_00000039);
})();
    }
(*normalized)[extensionIndex] = this->NormalizeExtension(extension);
}
return normalized;}

