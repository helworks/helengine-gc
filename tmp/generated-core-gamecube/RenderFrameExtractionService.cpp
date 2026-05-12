#ifdef DrawText
#undef DrawText
#endif
#include "RenderFrameExtractionService.hpp"
#include "runtime/native_exceptions.hpp"
#include "RenderFrameDrawableClassifier.hpp"
#include "runtime/native_list.hpp"
#include "RenderFrameLightClassifier.hpp"
#include "runtime/array.hpp"
#include "RenderFrameExtractionResult.hpp"
#include "RenderFrameDrawableSubmission.hpp"
#include "RenderFrameShadowCasterSubmission.hpp"
#include "IDrawable3D.hpp"
#include "RenderFrameLightSubmission.hpp"
#include "RenderFrame.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"

::RenderFrameExtractionResult* RenderFrameExtractionService::Extract(List<::CameraComponent*>* cameras, List<::IDrawable3D*>* drawables, List<::LightComponent*>* lights, ::RendererBackendCapabilityProfile* backendCapabilities)
{
    if (cameras == nullptr)
    {
throw new ArgumentNullException("cameras");
    }
else     if (drawables == nullptr)
    {
throw new ArgumentNullException("drawables");
    }
else     if (lights == nullptr)
    {
throw new ArgumentNullException("lights");
    }
else     if (backendCapabilities == nullptr)
    {
throw new ArgumentNullException("backendCapabilities");
    }
::RenderFrameDrawableClassifier *classifier = new ::RenderFrameDrawableClassifier();
List<::RenderFrameDrawableSubmission*> *drawableSubmissions = new List<::RenderFrameDrawableSubmission*>(drawables->get_Count());
List<::RenderFrameShadowCasterSubmission*> *shadowCasterSubmissions = new List<::RenderFrameShadowCasterSubmission*>(drawables->get_Count());
for (int32_t drawableIndex = 0; drawableIndex < drawables->get_Count(); drawableIndex++) {
::IDrawable3D *drawable = (*drawables)[drawableIndex];
Array<::RenderFrameDrawableSubmission*> *submissions = classifier->Classify(drawable);
for (int32_t submissionIndex = 0; submissionIndex < submissions->Length; submissionIndex++) {
::RenderFrameDrawableSubmission *submission = (*submissions)[submissionIndex];
drawableSubmissions->Add(submission);
    if (!submission->get_IsTransparent())
    {
shadowCasterSubmissions->Add(new ::RenderFrameShadowCasterSubmission(submission->get_Drawable(), submission->get_SubmeshIndex(), submission->get_Material()));
    }
}
}
::RenderFrameLightClassifier *lightClassifier = new ::RenderFrameLightClassifier();
Array<::RenderFrameLightSubmission*> *lightSubmissions = new Array<RenderFrameLightSubmission*>(lights->get_Count());
for (int32_t lightIndex = 0; lightIndex < lights->get_Count(); lightIndex++) {
(*lightSubmissions)[lightIndex] = lightClassifier->Classify((*lights)[lightIndex]);
}
Array<::RenderFrame*> *frames = new Array<RenderFrame*>(cameras->get_Count());
for (int32_t index = 0; index < cameras->get_Count(); index++) {
(*frames)[index] = ([&]() {
auto __ctor_arg_00000115 = (*cameras)[index];
auto __ctor_arg_00000116 = new List<RenderFrameDrawableSubmission*>(drawableSubmissions->ToArray());
auto __ctor_arg_00000117 = new List<RenderFrameLightSubmission*>(lightSubmissions);
auto __ctor_arg_00000118 = new List<RenderFrameShadowCasterSubmission*>(shadowCasterSubmissions->ToArray());
return new ::RenderFrame(__ctor_arg_00000115, __ctor_arg_00000116, __ctor_arg_00000117, __ctor_arg_00000118);
})();
}
return new ::RenderFrameExtractionResult(new List<RenderFrame*>(frames), backendCapabilities);}

