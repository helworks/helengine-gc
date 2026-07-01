#include "platform/gamecube/GameCubeFramePlan.hpp"

#include <unordered_set>

#include "IDrawable3D.hpp"
#include "LightComponent.hpp"
#include "RenderFrame.hpp"
#include "RenderFrameBatchingMetadata.hpp"
#include "RenderFrameShadowCasterSubmission.hpp"

namespace {
    /// Releases the extracted render-frame graph owned by one GameCube frame plan.
    void DeleteExtractionResult(RenderFrameExtractionResult*& extractionResult) {
        if (extractionResult == nullptr) {
            return;
        }

        std::unordered_set<RenderFrameDrawableSubmission*> deletedDrawableSubmissions;
        std::unordered_set<RenderFrameLightSubmission*> deletedLightSubmissions;
        std::unordered_set<RenderFrameShadowCasterSubmission*> deletedShadowCasterSubmissions;
        List<RenderFrame*>* frames = extractionResult->get_Frames();
        if (frames != nullptr) {
            for (int32_t frameIndex = 0; frameIndex < frames->get_Count(); frameIndex++) {
                RenderFrame* frame = (*frames)[frameIndex];
                if (frame == nullptr) {
                    continue;
                }

                List<RenderFrameDrawableSubmission*>* drawableSubmissions = frame->get_DrawableSubmissions();
                if (drawableSubmissions != nullptr) {
                    for (int32_t submissionIndex = 0; submissionIndex < drawableSubmissions->get_Count(); submissionIndex++) {
                        RenderFrameDrawableSubmission* drawableSubmission = (*drawableSubmissions)[submissionIndex];
                        if (drawableSubmission == nullptr || !deletedDrawableSubmissions.insert(drawableSubmission).second) {
                            continue;
                        }

                        RenderFrameBatchingMetadata* batchingMetadata = drawableSubmission->get_BatchingMetadata();
                        if (batchingMetadata != nullptr) {
                            delete batchingMetadata;
                        }

                        delete drawableSubmission;
                    }

                    delete drawableSubmissions;
                }

                List<RenderFrameLightSubmission*>* lightSubmissions = frame->get_LightSubmissions();
                if (lightSubmissions != nullptr) {
                    for (int32_t submissionIndex = 0; submissionIndex < lightSubmissions->get_Count(); submissionIndex++) {
                        RenderFrameLightSubmission* lightSubmission = (*lightSubmissions)[submissionIndex];
                        if (lightSubmission == nullptr || !deletedLightSubmissions.insert(lightSubmission).second) {
                            continue;
                        }

                        delete lightSubmission;
                    }

                    delete lightSubmissions;
                }

                List<RenderFrameShadowCasterSubmission*>* shadowCasterSubmissions = frame->get_ShadowCasterSubmissions();
                if (shadowCasterSubmissions != nullptr) {
                    for (int32_t submissionIndex = 0; submissionIndex < shadowCasterSubmissions->get_Count(); submissionIndex++) {
                        RenderFrameShadowCasterSubmission* shadowCasterSubmission = (*shadowCasterSubmissions)[submissionIndex];
                        if (shadowCasterSubmission == nullptr || !deletedShadowCasterSubmissions.insert(shadowCasterSubmission).second) {
                            continue;
                        }

                        delete shadowCasterSubmission;
                    }

                    delete shadowCasterSubmissions;
                }

                delete frame;
            }

            delete frames;
        }

        delete extractionResult;
        extractionResult = nullptr;
    }
}

namespace helengine::gamecube {
    /// Releases the temporary scene snapshots used to build this frame plan.
    GameCubeFramePlan::~GameCubeFramePlan() {
        if (Cameras != nullptr) {
            delete Cameras;
            Cameras = nullptr;
        }

        if (Drawables != nullptr) {
            delete Drawables;
            Drawables = nullptr;
        }

        if (Lights != nullptr) {
            delete Lights;
            Lights = nullptr;
        }

        DeleteExtractionResult(ExtractionResult);
        DrawableSubmissions = nullptr;
        LightSubmissions = nullptr;
    }
}
