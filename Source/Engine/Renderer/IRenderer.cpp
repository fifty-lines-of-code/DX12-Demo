#include "IRenderer.h"

namespace Engine::EngineRenderer {

	IRenderer::IRenderer() {
		mRendererPipelinePassStringValues = {
			"SHADOW_PASS",
			"MIRROR_RENDER_PASS",
			"OPAQUE_RENDER_PASS",
			"DEBUG_SYSTEM_PASS",
			"BLUR_UI_PASS"
		};
	}

	const std::string& IRenderer::RendererPipelinePass_ToString(
		RendererPipelinePass pass
	) {
		if (pass >= RendererPipelinePass::COUNT) {
			return INVALID_PASS_STRING;
		}

		return mRendererPipelinePassStringValues[(uint8_t)pass];
	}
}