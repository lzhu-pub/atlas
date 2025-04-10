#include "core/inst_handlers/v/RvvInsts.hpp"
#include "core/AtlasState.hpp"
#include "core/ActionGroup.hpp"
#include "include/ActionTags.hpp"
#include "include/AtlasUtils.hpp"

namespace atlas
{
    template <typename XLEN>
    void RvvInsts::getInstComputeAddressHandlers(std::map<std::string, Action> & inst_handlers)
    {
        static_assert(std::is_same_v<XLEN, RV64> || std::is_same_v<XLEN, RV32>);
        (void)inst_handlers;
    }

    template <typename XLEN>
    void RvvInsts::getInstHandlers(std::map<std::string, Action> & inst_handlers)
    {
        static_assert(std::is_same_v<XLEN, RV64> || std::is_same_v<XLEN, RV32>);

        //TODO: decide VLEN
        using VLEN = XLEN;

        inst_handlers.emplace(
            "vsetvl", atlas::Action::createAction<&RvvInsts::vsetvl_handler<XLEN, VLEN>, RvvInsts>(
                          nullptr, "vsetvl", ActionTags::EXECUTE_TAG));
        inst_handlers.emplace(
            "vsetvli", atlas::Action::createAction<&RvvInsts::vsetvli_handler<XLEN, VLEN>, RvvInsts>(
                           nullptr, "vsetvli", ActionTags::EXECUTE_TAG));
        inst_handlers.emplace(
            "vsetivl", atlas::Action::createAction<&RvvInsts::vsetivl_handler<XLEN, VLEN>, RvvInsts>(
                           nullptr, "vsetivl", ActionTags::EXECUTE_TAG));
        inst_handlers.emplace(
            "vsetivli", atlas::Action::createAction<&RvvInsts::vsetivli_handler<XLEN, VLEN>, RvvInsts>(
                            nullptr, "vsetivli", ActionTags::EXECUTE_TAG));
    }

    template void RvvInsts::getInstComputeAddressHandlers<RV32>(std::map<std::string, Action> &);
    template void RvvInsts::getInstComputeAddressHandlers<RV64>(std::map<std::string, Action> &);
    template void RvvInsts::getInstHandlers<RV32>(std::map<std::string, Action> &);
    template void RvvInsts::getInstHandlers<RV64>(std::map<std::string, Action> &);

    template <typename XLEN> void RvvInsts::vsetVTYPE(atlas::AtlasState* state, XLEN vtype)
    {
        WRITE_CSR_REG<XLEN>(state, VTYPE, vtype);
        VectorState* vector_state = state->getVectorState();
        vector_state->sew = 8 << READ_CSR_FIELD<XLEN>(state, VTYPE, "vsew");
        uint8_t vlmul = READ_CSR_FIELD<XLEN>(state, VTYPE, "vlmul");
        switch (vlmul)
        {
            case 0b101:
                vector_state->lmul = 1;
                break;
            case 0b110:
                vector_state->lmul = 2;
                break;
            case 0b111:
                vector_state->lmul = 4;
                break;
            case 0b000:
                vector_state->lmul = 8;
                break;
            case 0b001:
                vector_state->lmul = 16;
                break;
            case 0b010:
                vector_state->lmul = 32;
                break;
            case 0b011:
                vector_state->lmul = 64;
                break;
            default:
                break;
        }
        vector_state->vta = READ_CSR_FIELD<XLEN>(state, VTYPE, "vta");
        vector_state->vma = READ_CSR_FIELD<XLEN>(state, VTYPE, "vma");
    }

    template <typename XLEN, typename VLEN>
    XLEN RvvInsts::vsetAVL(atlas::AtlasState* state, bool set_max, XLEN avl)
    {
        VectorState* vector_state = state->getVectorState();
        if (set_max)
        {
            vector_state->vl = std::min<uint8_t>(vector_state->getVLMAX<VLEN>(), avl);
        }
        else
        {
            vector_state->vl = vector_state->getVLMAX<VLEN>();
        }
        WRITE_CSR_REG<XLEN>(state, VL, vector_state->vl);
        return vector_state->vl;
    }

    template <typename XLEN, typename VLEN> ActionGroup* RvvInsts::vsetvl_handler(atlas::AtlasState* state)
    {
        const AtlasInstPtr & inst = state->getCurrentInst();
        const XLEN vtype_val = READ_INT_REG<XLEN>(state, inst->getRs2());
        vsetVTYPE<XLEN>(state, vtype_val);
        if (inst->getRs1() == 0)
        {
            if (inst->getRd() != 0)
            {
                WRITE_INT_REG<XLEN>(state, inst->getRd(), vsetAVL<XLEN, VLEN>(state, true));
            }
        }
        else
        {
            vsetAVL<XLEN, VLEN>(state, false, READ_INT_REG<XLEN>(state, inst->getRs1()));
        }
        return nullptr;
    }

    template <typename XLEN, typename VLEN> ActionGroup* RvvInsts::vsetvli_handler(atlas::AtlasState* state)
    {
        const AtlasInstPtr & inst = state->getCurrentInst();
        const XLEN vtype_val = inst->getImmediate();
        vsetVTYPE<XLEN>(state, vtype_val);
        if (inst->getRs1() == 0)
        {
            if (inst->getRd() != 0)
            {
                WRITE_INT_REG<XLEN>(state, inst->getRd(), vsetAVL<XLEN, VLEN>(state, true));
            }
        }
        else
        {
            vsetAVL<XLEN, VLEN>(state, false, READ_INT_REG<XLEN>(state, inst->getRs1()));
        }
        return nullptr;
    }

    template <typename XLEN, typename VLEN> ActionGroup* RvvInsts::vsetivl_handler(atlas::AtlasState* state)
    {
        const AtlasInstPtr & inst = state->getCurrentInst();
        const XLEN vtype_val = READ_INT_REG<XLEN>(state, inst->getRs2());
        vsetVTYPE<XLEN>(state, vtype_val);
        vsetAVL<XLEN, VLEN>(state, false, READ_INT_REG<XLEN>(state, inst->getAVL()));
        return nullptr;
    }

    template <typename XLEN, typename VLEN> ActionGroup* RvvInsts::vsetivli_handler(atlas::AtlasState* state)
    {
        const AtlasInstPtr & inst = state->getCurrentInst();
        const XLEN vtype_val = READ_INT_REG<XLEN>(state, inst->getImmediate());
        vsetVTYPE<XLEN>(state, vtype_val);
        vsetAVL<XLEN, VLEN>(state, false, READ_INT_REG<XLEN>(state, inst->getAVL()));
        return nullptr;
    }

} // namespace atlas
