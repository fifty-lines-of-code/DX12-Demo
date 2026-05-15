#pragma once

#include "../../MathHelper.h"

struct EntityConstantBufferData {
	DirectX::XMFLOAT4X4 WorldViewProjection = MathHelper::Identity4x4();
};