#include "core/Execute.hpp"
#include "core/AtlasInst.hpp"
#include "core/AtlasState.hpp"
#include "core/Translate.hpp"
#include "include/ActionTags.hpp"

#include "sparta/utils/LogUtils.hpp"

#include "core/inst_handlers/rv64/a/RvaInsts.hpp"
#include "core/inst_handlers/rv64/d/RvdInsts.hpp"
#include "core/inst_handlers/rv64/f/RvfInsts.hpp"
#include "core/inst_handlers/rv64/i/RviInsts.hpp"
#include "core/inst_handlers/rv64/m/RvmInsts.hpp"
#include "core/inst_handlers/rv64/zicsr/RvzicsrInsts.hpp"
#include "core/inst_handlers/rv64/zifencei/RvzifenceiInsts.hpp"

namespace atlas
{
    Execute::Execute(sparta::TreeNode* execute_node, const ExecuteParameters* p) :
        sparta::Unit(execute_node)
    {
        (void)p;

        Action execute_action = atlas::Action::createAction<&Execute::execute_>(this, "Execute");
        execute_action.addTag(ActionTags::EXECUTE_TAG);
        execute_action_group_.addAction(execute_action);

        // Get instruction handlers
        RviInsts::getInstHandlers(inst_handlers_);
        RvmInsts::getInstHandlers(inst_handlers_);
        RvaInsts::getInstHandlers(inst_handlers_);
        RvfInsts::getInstHandlers(inst_handlers_);
        RvdInsts::getInstHandlers(inst_handlers_);
        RvzicsrInsts::getInstHandlers(inst_handlers_);
        RvzifenceiInsts::getInstHandlers(inst_handlers_);

        // Get instruction compute address handlers
        RviInsts::getInstComputeAddressHandlers(inst_compute_address_handlers_);
        RvaInsts::getInstComputeAddressHandlers(inst_compute_address_handlers_);
        RvfInsts::getInstComputeAddressHandlers(inst_compute_address_handlers_);
        RvdInsts::getInstComputeAddressHandlers(inst_compute_address_handlers_);
    }

    ActionGroup* Execute::execute_(AtlasState* state)
    {
        // Connect instruction to Fetch
        const auto inst = state->getCurrentInst();
        ActionGroup* inst_action_group = inst->getActionGroup();
        inst_action_group->setNextActionGroup(execute_action_group_.getNextActionGroup());

        // Insert translation Action into instruction's ActionGroup between the compute address
        // handler and the execute handler
        if (inst->isMemoryInst())
        {
            const ActionGroup* data_translate_action_group =
                state->getTranslateUnit()->getDataTranslateActionGroup();
            for (auto it = data_translate_action_group->getActions().rbegin();
                 it != data_translate_action_group->getActions().rend(); ++it)
            {
                auto & action = *it;
                inst_action_group->insertActionAfter(action, ActionTags::COMPUTE_ADDR_TAG);
            }
        }

        state->insertExecuteActions(inst_action_group);

        ILOG(inst);

        // Execute the instruction
        return inst_action_group;
    }
} // namespace atlas
