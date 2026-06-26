#pragma once

#include "Vector.h"

namespace Engine {

    struct BasisVectors {
        Vector3 Right = Vector3(1.f, 0.f, 0.f);
        Vector3 Up = Vector3(0.f, 1.f, 0.f);
        Vector3 Forward = Vector3(0.f, 0.f, 1.f);

        BasisVectors() {}

        BasisVectors(
            Vector3 right, 
            Vector3 up,
            Vector3 fwd
        ) : Right(right), Up(up), Forward(fwd)
        {}
    };
}