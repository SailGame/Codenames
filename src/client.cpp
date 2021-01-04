#include "client.h"

namespace SailGame { namespace Network {

Client::Client(const std::string &endpoint, const OnNewMsgT &newMsgCallback)
    : OnNewMsg(newMsgCallback), 
    mChannel(grpc::CreateChannel(endpoint, grpc::InsecureChannelCredentials())),
    mStub(GameCore::NewStub(mChannel)),
    mContext(std::make_shared<ClientContext>())
{}

void Client::Connect()
{
    mStream = mStub->Provider(mContext.get());
    spdlog::info("connection established.");
}

ProviderMsg Client::Receive()
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

void Client::Send(const ProviderMsg &msg)
{
    // std::cout << "send: " << msg.Operation_case() << std::endl;
    mStream->Write(msg);
}
}}