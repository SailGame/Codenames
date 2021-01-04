#include <spdlog/spdlog.h>

#include <sailgame_pb/core/provider.pb.h>
#include <sailgame_pb/core/error.pb.h>

#include "sailgame/common/state_machine.h"
#include "sailgame/common/util.h"
#include "sailgame/codenames/state.h"
#include "sailgame/codenames/msg_builder.h"

namespace SailGame { namespace Common {

using SailGame::Game::GlobalState;
using Core::ProviderMsg;
using Core::RegisterRet;
using Core::StartGameArgs;
using Core::UserOperationArgs;
using Core::CloseGameArgs;
using Core::QueryStateArgs;
using ::Codenames::UserOperation;
using ::Codenames::LeaderWord;
using ::Codenames::FollowerWord;
using ::Codenames::StartGameSettings;
using Game::MsgBuilder;

#define TransitionFor(MsgT) \
    template<> \
    template<> \
    ProviderMsgPtrs StateMachine<GlobalState>::Transition<MsgT>(const MsgT &msg)

TransitionFor(ProviderMsg);
TransitionFor(RegisterRet);
TransitionFor(StartGameArgs);
// TransitionFor(CloseGameArgs);
// TransitionFor(QueryStateArgs);
TransitionFor(UserOperationArgs);

TransitionFor(UserOperation);
TransitionFor(LeaderWord);
TransitionFor(FollowerWord);

TransitionFor(ProviderMsg)
{
    switch (msg.Msg_case()) {
        case ProviderMsg::MsgCase::kRegisterRet:
            return Transition<RegisterRet>(msg.registerret());
        case ProviderMsg::MsgCase::kStartGameArgs:
            return Transition<StartGameArgs>(msg.startgameargs());
        case ProviderMsg::MsgCase::kCloseGameArgs:
            return Transition<CloseGameArgs>(msg.closegameargs());
        case ProviderMsg::MsgCase::kQueryStateArgs:
            return Transition<QueryStateArgs>(msg.querystateargs());
        case ProviderMsg::MsgCase::kUserOperationArgs:
            return Transition<UserOperationArgs>(msg.useroperationargs());
    }
    throw std::runtime_error("Unsupported msg type");
    return {};
}

TransitionFor(RegisterRet)
{
    if (msg.err() != Core::ErrorNumber::OK) {
        spdlog::error("Register Failure");
    }
    return {};   
}

TransitionFor(StartGameArgs)
{
    auto [userIdToRole, cardInfo, turn] = mState.NewGame(
        msg.roomid(), Util::ConvertGrpcRepeatedFieldToVector(msg.userid()),
        Util::UnpackGrpcAnyTo<StartGameSettings>(msg.custom()));

    ProviderMsgPtrs msgs;
    for (const auto &entry : userIdToRole) {
        msgs.push_back(
            MsgBuilder::CreateNotifyMsgArgs(0, Core::ErrorNumber::OK, msg.roomid(), entry.first,
                MsgBuilder::CreateGameStart(cardInfo, entry.second, turn)));
    }
    return msgs;
}

TransitionFor(UserOperationArgs)
{
    mState.mCurrentRoomId = msg.roomid();
    return Transition(Util::UnpackGrpcAnyTo<UserOperation>(msg.custom()));
}

TransitionFor(UserOperation)
{
    /**
     * leader word -> broadcast
     * follower word -> collect and vote automatically
     */
    switch(msg.Operation_case())
    {
        case UserOperation::OperationCase::kLword:
            return Transition<LeaderWord>(msg.lword());
        case UserOperation::OperationCase::kFword:
            return Transition<FollowerWord>(msg.fword());
    }
    throw std::runtime_error("Unsupported msg type");
    return {};
}

TransitionFor(LeaderWord)
{
    auto roomId = mState.mCurrentRoomId;
    auto word = msg.word();
    auto &gameState = mState.mRoomIdToGameState.at(roomId);
    gameState.process<LeaderWord>(word);
    // broadcast word
    ProviderMsgPtrs msgs;
    msgs.push_back(MsgBuilder::CreateNotifyMsgArgs(0, Core::ErrorNumber::OK, roomId, 0,
            MsgBuilder::CreateProcessState(word, gameState.mCurrentState, gameState.mCurrentTurn)));
    return msgs;
}

TransitionFor(FollowerWord)
{
    auto roomId = mState.mCurrentRoomId;
    auto word = msg.word();
    auto &gameState = mState.mRoomIdToGameState.at(roomId);
    gameState.process<FollowerWord>(word);
    ProviderMsgPtrs msgs;
    msgs.push_back(MsgBuilder::CreateNotifyMsgArgs(0, Core::ErrorNumber::OK, roomId, 0,
        MsgBuilder::CreateProcessState(gameState.mVotedWord.ConvertToGrpcWord(), gameState.mCurrentState, gameState.mCurrentTurn)));
    return msgs;
}
}}