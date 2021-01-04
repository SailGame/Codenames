#include "msg_builder.h"

namespace SailGame { namespace Game {

ProviderMsgPtr MsgBuilder::CreateRegisterArgs(int seqId, const std::string &id,
    const std::string &gameName, int maxUsers, int minUsers)
{
    auto msg = std::make_shared<ProviderMsg>();
    msg->set_sequenceid(seqId);

    auto registerArgs = msg->mutable_registerargs();
    registerArgs->set_id(id);
    registerArgs->set_gamename(gameName);

    auto gameSettings = registerArgs->mutable_gamesetting();
    gameSettings->set_maxusers(maxUsers);
    gameSettings->set_minusers(minUsers);

    return msg;
}

ProviderMsgPtr MsgBuilder::CreateNotifyMsgArgs(int seqId, ErrorNumber err,
    int roomId, int userId, const NotifyMsg &custom)
{
    auto msg = std::make_shared<ProviderMsg>();
    msg->set_sequenceid(seqId);

    auto args = msg->mutable_notifymsgargs();
    args->set_err(err);
    args->set_roomid(roomId);
    args->set_userid(userId);
    args->mutable_custom()->PackFrom(custom);

    return msg;
}

NotifyMsg MsgBuilder::CreateGameStart(const std::vector<Card> &cards, const Turn &role,
    const enum Turn &turn)
{
    NotifyMsg msg;
    auto gameStart = msg.mutable_start();
    for(auto & card : cards)
    {
        gameStart->mutable_cardinfo()->add_cards()->CopyFrom(card.ConvertToGrpcCard());
    }
    gameStart->set_role(role);
    gameStart->set_turn(turn);
    return msg;
}

NotifyMsg MsgBuilder::CreateProcessState(const ::Codenames::Word &word, const enum State &state, 
    const enum Turn &turn)
{
    NotifyMsg msg;
    auto processState = msg.mutable_process();
    processState->mutable_word()->CopyFrom(word);
    processState->set_state(state);
    processState->set_turn(turn);
    return msg;
}
}}
