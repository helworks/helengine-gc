#ifdef DrawText
#undef DrawText
#endif
#include "AssetContentProcessor_1.hpp"
#include "runtime/native_exceptions.hpp"
#include "Asset.hpp"
#include "AssetSerializer.hpp"
#include "runtime/native_cast.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_type.hpp"

template <typename TAsset>
Type* AssetContentProcessor_1<TAsset>::get_OutputType()
{
return he_cpp_type_of<TAsset>("TAsset");
}

template <typename TAsset>
TAsset AssetContentProcessor_1<TAsset>::Read(::Stream* stream)
{
    if (stream == nullptr)
    {
throw new ArgumentNullException("stream");
    }
::Asset *asset = AssetSerializer::Deserialize(stream);
    TAsset typedAsset = he_cpp_try_cast<TAsset>(asset);
    if (typedAsset != nullptr)
    {
return typedAsset;    }
throw new InvalidOperationException(std::string("Serialized asset did not contain '") + he_cpp_type_of<TAsset>("TAsset")->get_Name() + std::string("'."));
}

template <typename TAsset>
void* AssetContentProcessor_1<TAsset>::ReadObject(::Stream* stream)
{
return this->Read(stream);}

