#include "IRenderer.h"

namespace Engine::EngineRenderer {

	IRenderer::IRenderer() {
		mRendererPipelinePassStringValues = {
			"SHADOW_PASS",
			"OPAQUE_RENDER_PASS",
			"DEBUG_SYSTEM_PASS",
			"BLUR_UI_PASS"
		};
	}

	std::string* IRenderer::RendererPipelinePass_ToString(RendererPipelinePass pass) {
		if (pass >= RendererPipelinePass::COUNT) { return nullptr; }

		return &mRendererPipelinePassStringValues[(uint8_t)pass];
	}
}