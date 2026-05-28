#pragma once

#include "../../../../Engine/Math/EngineMath.h"

struct EntityConstantBufferData {
	Engine::Matrix4x4 World = Engine::Matrix4x4::Identity();
};

struct PerPassConstantBufferData {
	Engine::Matrix4x4 World = Engine::Matrix4x4::Identity();
};