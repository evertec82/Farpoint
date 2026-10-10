// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include <optional>
#include "shader_recompiler/frontend/control_flow_graph.h"
#include "shader_recompiler/ir/basic_block.h"

namespace Shader::Optimization {

// Recognize only i = phi(initial, i + 1), with the back edge guarded by i < limit.
// The guard must be the sole predecessor of the incoming back-edge block.
inline std::optional<std::pair<u32, u32>> BoundedInductionRange(const IR::Inst* phi) {
    if (!phi || phi->GetOpcode() != IR::Opcode::Phi || phi->NumArgs() != 2) {
        return {};
    }
    const size_t initial_idx = phi->Arg(0).IsImmediate() ? 0 : 1;
    if (!phi->Arg(initial_idx).IsImmediate()) {
        return {};
    }
    const u32 initial = phi->Arg(initial_idx).U32();
    const auto* increment = phi->Arg(1 - initial_idx).TryInst();
    if (!increment || increment->GetOpcode() != IR::Opcode::IAdd32 ||
        increment->Arg(0) != IR::Value{const_cast<IR::Inst*>(phi)} ||
        !increment->Arg(1).IsImmediate() || increment->Arg(1).U32() != 1) {
        return {};
    }
    auto* back = phi->PhiBlock(1 - initial_idx);
    if (back->ImmPredecessors().size() != 1 || !back->cfg_block) {
        return {};
    }
    auto* guard = back->ImmPredecessors()[0];
    const auto* cfg = guard->cfg_block;
    if (!cfg || cfg->branch_true == cfg->branch_false) {
        return {};
    }
    const bool guarded_by_true_scc =
        (cfg->cond == IR::Condition::Scc1 && cfg->branch_true == back->cfg_block) ||
        (cfg->cond == IR::Condition::Scc0 && cfg->branch_false == back->cfg_block);
    if (!guarded_by_true_scc) {
        return {};
    }
    // Only the final SCC write controls the edge. Do not infer a bound merely from
    // a comparison somewhere in the loop, or from a conditional exit on another path.
    for (auto it = guard->rbegin(); it != guard->rend(); ++it) {
        if (it->GetOpcode() != IR::Opcode::SetScc) {
            continue;
        }
        const auto* compare = it->Arg(0).TryInst();
        if (!compare || compare->GetOpcode() != IR::Opcode::SLessThan32 ||
            compare->Arg(0) != IR::Value{const_cast<IR::Inst*>(phi)} ||
            !compare->Arg(1).IsImmediate()) {
            return {};
        }
        const u32 limit = compare->Arg(1).U32();
        if (initial > limit || limit >= 0x7fffffffU) {
            return {};
        }
        return std::pair{initial, limit};
    }
    return {};
}
} // namespace Shader::Optimization
