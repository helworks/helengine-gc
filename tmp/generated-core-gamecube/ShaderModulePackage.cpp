#ifdef DrawText
#undef DrawText
#endif
#include "ShaderModulePackage.hpp"
#include "ShaderProgramBinary.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_exceptions.hpp"
#include "system/io/path.hpp"
#include "runtime/array.hpp"
#include "ContentManager.hpp"
#include "RawByteContent.hpp"
#include "ShaderTargetNames.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"
#include "system/io/path.hpp"

::ShaderModuleDefinition* ShaderModulePackage::get_Definition()
{
return this->Definition;
}

std::string ShaderModulePackage::get_RootPath()
{
return this->RootPath;
}

::ShaderCompileTarget ShaderModulePackage::get_Target()
{
return this->Target;
}

::ShaderProgramBinary* ShaderModulePackage::GetBinary(std::string programName, std::string variant)
{
return this->Definition->GetBinary(programName, this->targetName, variant);}

Array<uint8_t>* ShaderModulePackage::GetBinaryData(std::string programName, std::string variant)
{
::ShaderProgramBinary *binary = this->GetBinary(programName, variant);
    if (binary->get_Bytecode() != nullptr && binary->get_Bytecode()->Length > 0)
    {
return binary->get_Bytecode();    }
const std::string path = this->GetBinaryPath(programName, variant);
::ContentManager *contentManager = new ::ContentManager(this->RootPath);
::RawByteContent *binaryContent = contentManager->Load<RawByteContent*>(path, nullptr);
return binaryContent->get_Bytes();}

std::string ShaderModulePackage::GetBinaryPath(std::string programName, std::string variant)
{
::ShaderProgramBinary *binary = this->GetBinary(programName, variant);
    if (String::IsNullOrWhiteSpace(binary->get_Path()))
    {
throw new InvalidOperationException("The shader binary does not include a file path.");
    }
    if (Path::IsPathRooted(binary->get_Path()))
    {
return binary->get_Path();    }
return Path::Combine(this->RootPath, binary->get_Path());}

ShaderModulePackage::ShaderModulePackage(::ShaderCompileTarget target, std::string rootPath, ::ShaderModuleDefinition* definition) : Definition(), RootPath(), Target(), targetName()
{
    if (String::IsNullOrWhiteSpace(rootPath))
    {
throw ([&]() {
auto __ctor_arg_000001CA = "Package root path must be provided.";
auto __ctor_arg_000001CB = "rootPath";
return new ArgumentException(__ctor_arg_000001CA, __ctor_arg_000001CB);
})();
    }
    if (definition == nullptr)
    {
throw new ArgumentNullException("definition");
    }
this->Target = target;
this->RootPath = rootPath;
this->Definition = definition;
this->targetName = ShaderTargetNames::GetTargetName(target);
}

