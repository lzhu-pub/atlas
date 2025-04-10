#pragma once

#include <stdint.h>
#include <iostream>

namespace atlas
{
    struct VectorState
    {
        // vtype
        uint8_t lmul = 8; // unit: one 8th
        uint8_t sew = 0;  // unit: one bit
        bool vta = false;
        bool vma = false;
        // vl
        uint8_t vl = 0;
        // vstart
        uint8_t vstart = 0;

        uint8_t index = 0;

        template <typename VLEN> uint8_t getVLMAX() { return sizeof(VLEN) * lmul / sew; }

    }; // struct VectorState

    std::ostream & operator<<(std::ostream & os, const VectorState & vector_state);

} // namespace atlas