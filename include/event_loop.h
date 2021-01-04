#pragma once

#include <functional>
#include <memory>
#include <queue>
#include <mutex>

#include <core/provider.pb.h>

#include "types.h"

namespace SailGame { namespace Common {

using Core::ProviderMsg;

class EventLoopSubscriber {
public:
    virtual void OnEventProcessed(const ProviderMsgPtr &) = 0;
};

class EventLoop {
public:
    EventLoop() = default;

    static std::shared_ptr<EventLoop> Create() {
        return std::make_shared<EventLoop>();
    }

    void StartLoop() {
        while (!mShouldStop) {
            if (!mEventQueue.empty()) {
                // std::cout << "[EventLoop] Process Event: " << int(mEventQueue.front()->mType) << std::endl;
                mSubscriber->OnEventProcessed(mEventQueue.front());
                mEventQueue.pop();
            }
        }
    }

    void StopLoop() {
        mShouldStop = true;
    }

    void AppendEvent(const ProviderMsgPtr &event) {
        // multiple threads may invoke this at the same time
        std::lock_guard<std::mutex> lock(mMutex);
        // std::cout << "[EventLoop] Append Event: " << int(event->mType) << std::endl;
        mEventQueue.push(event);
    }

    bool Empty() const { return mEventQueue.empty(); }

    void SetSubscriber(EventLoopSubscriber *subscriber) {
        mSubscriber = subscriber;
    }

private:
    std::queue<ProviderMsgPtr> mEventQueue;
    std::mutex mMutex;
    EventLoopSubscriber *mSubscriber{nullptr};
    bool mShouldStop{false};
};
}}