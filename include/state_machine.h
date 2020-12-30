#pragma once

#include <memory>

#include "types.h"

namespace SailGame { namespace Common {

template<typename StateT>
class StateMachine {
public:
    StateMachine() = default;

    static std::shared_ptr<StateMachine> Create() {
        return std::make_shared<StateMachine>();
    }

    template<typename MsgT>
    ProviderMsgPtrs Transition(const MsgT &event) {
        throw std::runtime_error("Unsupported msg type");
    }

    const StateT &GetState() const { return mState; } 

private:
    StateT mState;
};
}}