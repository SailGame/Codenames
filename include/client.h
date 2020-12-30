#pragma once

#include <memory>
#include <thread>
#include <string>
#include <functional>
#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>
#include <spdlog/spdlog.h>

#include <core/provider.pb.h>
#include <core/core.grpc.pb.h>

namespace SailGame { namespace Network {

using ::grpc::Channel;
using ::grpc::ClientContext;
using ::grpc::ClientReader;
using ::grpc::ClientReaderWriter;
using ::grpc::ClientWriter;
using ::grpc::Status;
using ::Core::ProviderMsg;
using ::Core::GameCore;

class Client {
public:
    using OnNewMsgT = std::function<void(const ProviderMsg &)>;

public:
    Client(const std::string &endpoint, const OnNewMsgT &newMsgCallback);

    void Connect();

    ProviderMsg Receive();

    void Send(const ProviderMsg &msg);

private:
    OnNewMsgT OnNewMsg;

private:
    std::shared_ptr<Channel> mChannel;
    std::shared_ptr<GameCore::Stub> mStub;
    std::shared_ptr<ClientContext> mContext;
    std::shared_ptr<ClientReaderWriter<ProviderMsg, ProviderMsg>> mStream;
};
}}
