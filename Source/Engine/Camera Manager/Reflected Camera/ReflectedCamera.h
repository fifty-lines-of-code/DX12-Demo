#pragma once

#include "../../Math/EngineMath.h"
#include "../../Simulation/SimulationDataStructures.h"

namespace Engine::EngineCamera {

	struct ReflectedCameraData {
		Matrix4x4 View;
		Matrix4x4 Projection;
		Matrix4x4 ViewProjection;
		Vector3 Center;
		bool IsValid = false;
	};

	class ReflectedCamera {
	public:
		ReflectedCamera() = default;
		~ReflectedCamera() = default;

		ReflectedCamera(const ReflectedCamera& rhs) = delete;
		ReflectedCamera& operator=(const ReflectedCamera& rhs) = delete;
		ReflectedCamera(ReflectedCamera&&) = delete;
		ReflectedCamera& operator=(ReflectedCamera&&) = delete;

		void UpdateWithMainCameraData(
			const Vector3& mainCamCenter,
			const Vector3& mainCamForward,
			const Vector3& mainCamUp,
			float          fovY,
			float          aspectRatio,
			float          nearPlane,
			float          farPlane,
			const EngineSimulation::MirrorPlaneQueryResult& mirrorPlanes
		) noexcept;

		const Matrix4x4& GetViewProjection() const noexcept;

	private:
		ReflectedCameraData mData;

	private:
		void DebugPrintCameraData() const noexcept;
	};
}