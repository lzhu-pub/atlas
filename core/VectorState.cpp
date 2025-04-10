#include "core/VectorState.hpp"

namespace atlas
{
    std::ostream & operator<<(std::ostream & os, const VectorState & vector_state)
    {
        os << "LMUL: ";
        if (vector_state.lmul < 8)
        {
            os << "1/" << 8 / vector_state.lmul << " ";
        }
        else
        {
            os << vector_state.lmul / 8 << " ";
        }
        os << "SEW: " << vector_state.sew << " ";
        os << "VTA: " << vector_state.vta << " "
           << "VMA: " << vector_state.vma << " ";
        os << "VL: " << vector_state.vl << " "
           << "VSTART: " << vector_state.vstart << "; ";

        os << "index: " << vector_state.index;

        return os;
    }
} // namespace atlas