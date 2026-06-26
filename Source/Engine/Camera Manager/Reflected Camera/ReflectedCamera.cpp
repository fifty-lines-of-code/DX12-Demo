#include "ReflectedCamera.h"

namespace Engine::EngineCamera {

	void ReflectedCamera::UpdateWithMainCameraData(
		const Vector3& mainCamCenter,
		const Vector3& mainCamForward,
		const Vector3& mainCamUp,
		float          fovY,
		float          aspectRatio,
		float          nearZ,
		float          farZ,
		const EngineSimulation::MirrorPlaneQueryResult& mirrorPlanes
	) {
		if (mirrorPlanes.Count == 0) { return; }


	}
}