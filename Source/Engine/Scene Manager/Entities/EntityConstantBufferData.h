#pragma once

#include "../../../Helper/MathHelper.h"

struct EntityConstantBufferData {
	DirectX::XMFLOAT4X4 WorldViewProjection = MathHelper::Identity4x4();
};