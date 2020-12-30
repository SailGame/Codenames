#pragma once

#include <string>
#include <memory>
#include <functional>
#include <spdlog/spdlog.h>
#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>
#include <core/provider.pb.h>
#include <core/core.grpc.pb.h>

#include "types.h"

namespace SailGame { namespace Common {

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientReaderWriterInterface;
using grpc::ClientWriter;
using grpc::Status;
using Core::ProviderMsg;
using Core::GameCore;

class NetworkInterfaceSubscriber {
public:
    virtual void OnEventHappens(const ProviderMsgPtr &) = 0;
};

class NetworkInterface {
public:
    NetworkInterface(const std::shared_ptr<GameCore::StubInterface> &stub) 
        : mStub(stub)
    {}

    static std::shared_ptr<GameCore::StubInterface> CreateStub(const std::string &endpoint) {
        auto channel = grpc::CreateChannel(endpoint, grpc::InsecureChannelCredentials());
        return GameCore::NewStub(channel);
    }

    static std::shared_ptr<NetworkInterface> Create(
        const std::shared_ptr<GameCore::StubInterface> &stub) {
        return std::make_shared<NetworkInterface>(stub);
    }

    void Connect() {
        mStream = mStub->Provider(&mContext);
    }

    bool IsConnected() const {
        return mStream != nullptr;
    }

    void AsyncListen() {
        Connect();
        /// XXX: remove this
        // mStream->WritesDone();

        auto listenFunc = [this] {
            while (true) {
                OnEventHappens(ReceiveMsg());
            }
        };

        /// TODO: where to join the thread
        mListenThread = std::make_unique<std::thread>(listenFunc);
        spdlog::info("listen thread created");
    }

    void SendMsg(const ProviderMsg &msg) {
        mStream->Write(msg);
        spdlog::info("msg sent, type = {}", msg.Msg_case());
    }

    ProviderMsg ReceiveMsg()
    {
        ProviderMsg msg;
        if (mStream->Read(&msg)) {
            return msg;
        }
        if (!mStream->Finish().ok()) {
            std::cout << "rpc failed." << std::endl;
            std::exit(-1);
        }
        auto error_msg = "Stream ends normally, which indicates error in core side.";
        throw std::runtime_error(error_msg);
        return msg;
    }

    void OnEventHappens(const ProviderMsg &msg) {
        mSubscriber->OnEventHappens(std::make_shared<ProviderMsg>(msg));
        spdlog::info("msg received, type = {}", msg.Msg_case());
    }

    void SetSubscriber(NetworkInterfaceSubscriber *subscriber) {
        mSubscriber = subscriber;
    }

private:
    ClientContext mContext;
    std::shared_ptr<GameCore::StubInterface> mStub;
    std::shared_ptr<ClientReaderWriterInterface<ProviderMsg, ProviderMsg>> mStream;
    std::unique_ptr<std::thread> mListenThread;
    NetworkInterfaceSubscriber *mSubscriber{nullptr};
};
}}