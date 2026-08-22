//
// Created by TsCat on 2026/7/17.
//

#include "app/rule/state_hub.h"

namespace app::rule
{
    StateHub::StateHub() : snapshot_(std::make_shared<const StateSnapshot>()) {}

    void StateHub::publish(StateSnapshot snapshot)
    {
        if (*snapshot_ == snapshot) return;
        snapshot_ = std::make_shared<const StateSnapshot>(std::move(snapshot));
    }

    void StateHub::mark_reset_pending()
    {
        auto next = std::make_shared<StateSnapshot>(*snapshot_);
        next->reset_pending = true;
        snapshot_ = std::move(next);
    }

    std::shared_ptr<const StateSnapshot> StateHub::snapshot() const
    {
        return snapshot_;
    }
}
