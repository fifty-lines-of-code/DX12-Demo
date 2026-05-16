#pragma once

#include "../../../Helper/MathHelper.h"

struct EntityConstantBufferData {
	DirectX::XMFLOAT4X4 World = MathHelper::Identity4x4();
};

struct PerPassConstantBufferData {
    DirectX::XMFLOAT4X4 ViewProjection = MathHelper::Identity4x4();
};