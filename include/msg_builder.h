#pragma once

#include <core/provider.pb.h>
#include <core/error.pb.h>
#include <codenames/codenames.pb.h>

#include "types.h"

namespace SailGame { namespace Game {
using ::Codenames::NotifyMsg;
using ::Codenames::CardInfo;
using ::Codenames::State;
using ::Codenames::Turn;
using Core::ErrorNumber;
using Core::NotifyMsgArgs;
using Core::ProviderMsg;
using Common::ProviderMsgPtr;
using Game::Card;

class MsgBuilder
{
public:
    static ProviderMsgPtr CreateRegisterArgs(int seqId, const std::string &id,
        const std::string &gameName, int maxUsers, int minUsers);
    
    static ProviderMsgPtr CreateNotifyMsgArgs(int seqId, ErrorNumber err,
        int roomId, int userId, const NotifyMsg &custom);
    
    static NotifyMsg CreateGameStart(const std::vector<Card> &cards, const Turn &role, 
        const Turn &turn);
    
    static NotifyMsg CreateProcessState(const ::Codenames::Word &word, const State &state, 
        const Turn &turn);
};
}}