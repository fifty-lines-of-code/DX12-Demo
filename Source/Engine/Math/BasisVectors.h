#pragma once

#include "Vector.h"

namespace Engine {

    struct BasisVectors {
        Vector3 right = Vector3(1.f, 0.f, 0.f);
        Vector3 up = Vector3(0.f, 1.f, 0.f);
        Vector3 forward = Vector3(0.f, 0.f, 1.f);
    };
}