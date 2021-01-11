#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>

#include <sailgame_pb/core/core_mock.grpc.pb.h>
#include <sailgame_pb/core/provider.pb.h>
#include <sailgame_pb/core/error.pb.h>
#include <sailgame_pb/codenames/codenames.pb.h>

#include "sailgame/common/game_manager.h"
#include "sailgame/common/util.h"
#include "sailgame/codenames/msg_builder.h"

namespace SailGame { namespace Test {

using namespace testing;
using Common::EventLoop;
using Common::GameManager;
using Common::NetworkInterface;
using Common::StateMachine;
using Common::Util;
using Core::ErrorNumber;
using Core::MockGameCoreStub;
using Core::ProviderMsg;
using Game::GlobalState;
using Game::MsgBuilder;
using Game::Card;
using grpc::testing::MockClientReaderWriter;
using ::Codenames::NotifyMsg;
using ::Codenames::StartStateMsg;
using ::Codenames::ProcessStateMsg;
using ::Codenames::State;


MATCHER_P3(NotifyMsgArgsMatcher, err, roomId, userId, "")
{
    return arg.has_notifymsgargs() && arg.notifymsgargs().err() == err &&
        arg.notifymsgargs().roomid() == roomId &&
        arg.notifymsgargs().userid() == userId &&
        arg.notifymsgargs().custom().template Is<NotifyMsg>();
}

MATCHER(StartStateMsgMatcher, "")
{
    auto msg = Util::UnpackGrpcToAny<StartStateMsg>(arg.notifymsgargs().custom());
    return msg.has_start() && msg.start().has_cardinfo() &&
        msg.start().had_role() && msg.start().has_turn();
}

MATCHER(ProcessStateMsgMatcher, "")
{
    auto msg = Util::UnpackGrpcToAny<ProcessStateMsg>(arg.notifymsgargs().custom());
    return msg.has_process() && msg.process().has_word() && 
        msg.process().has_state() && msg.process().has_turn();   
}

class MockCoreFixture : public ::testing::Test
{
public:
    MockCoreFixture() : mMockStream(new MockClientReaderWriter<ProviderMsg, ProviderMsg>()),
        mMockStub(std::make_shared<MockGameCoreStub>()),
        mStateMachine(std::make_shared<StateMachine<GlobalState>>()),
        mNetworkInterface(NetworkInterface::Create(mMockStub)),
        mGameManager(EventLoop::Create(), mStateMachine, mNetworkInterface) {}
    

    void SetUp() {
        spdlog::set_level(spdlog::level::debug);
        EXPECT_CALL(*mMockStub, ProviderRaw(_))
            .Times(AtLeast(1)).WillOnce(Return(mMockStream));
        mThread = std::make_unique<std::thread>([&] {
            mGameManager.Start();
        });

        while (!mNetworkInterface->IsConnected()) {}
    }

    void TearDown() {
        mGameManager.Stop();
        mThread->join();
    }

    void ProcessMsgFromCore(const ProviderMsg &msg) {
        EXPECT_CALL(*mMockStream, Read(_)).Times(1).WillOnce(
            DoAll(SetArgPointee<0>(msg), Return(true)));
        mNetworkInterface->OnEventHappens(mNetworkInterface->ReceiveMsg());

        while (mGameManager.HasEventToProcess()) {}
    }

    void RegisterAndGameStart(int roomId, 
        const std::vector<unsigned int> &userIds) 
    {
        EXPECT_CALL(*mMockStream, Write(_, _)).Times(AtLeast(1));
        mNetworkInterface->SendMsg(
            *MsgBuilder::CreateRegisterArgs(0, "codename", "CODENAMES", 10, 4));
        ProcessMsgFromCore(*MsgBuilder::CreateRegisterRet(0, ErrorNumber::OK));

        for(auto &userId : userIds)
        {
            EXPECT_CALL(*mMockStream, Write(
                AllOf(NotifyMsgArgsMatcher(ErrorNumber::OK, roomId, userId),
                    StartStateMsgMatcher()), _)).Times(1);            
        }
        
        ProcessMsgFromCore(*MsgBuilder::CreateStartGameArgs(0, roomId, userIds,
            MsgBuilder::CreateStartGameSettings(15)));
    }
protected:
    MockClientReaderWriter<ProviderMsg, ProviderMsg> *mMockStream;
    std::shared_ptr<MockGameCoreStub> mMockStub;
    std::shared_ptr<StateMachine<GlobalState>> mStateMachine;
    std::shared_ptr<NetworkInterface> mNetworkInterface;
    GameManager<GlobalState> mGameManager;
    std::unique_ptr<std::thread> mThread;
};

TEST_F(MockCoreFixture, RegisterAndGameStart) 
{
    auto roomId = 1002;
    std::vector<unsigned int> userIds = {11, 22, 33, 44};
    RegisterAndGameStart(roomId, userIds);

    const auto &state = mStateMachine->GetState();
    EXPECT_EQ(state.mRoomIdToGameState.size(), 1);
    EXPECT_NE(state.mRoomIdToGameState.find(roomId), 
        state.mRoomIdToGameState.end());
    const auto &gameState = state.mRoomIdToGameState.at(roomId);
    EXPECT_EQ(gameState.mPlayerNum, userIds.size());
    EXPECT_EQ(gameState.mCurrentState, State::NOT_DETERMINED);
    EXPECT_TRUE((gameState.mCurrentTurn == Turn::RED_LDEAR) || (gameState.mCurrentTurn == Turn::BLUE_LEADER));
}

TEXT_F(MockCoreFixture, LeaderWord)
{
    auto roomId = 1002;
    std::vector<unsigned int> userIds = {11, 22, 33, 44};
    RegisterAndGameStart(roomId, userIds);

    EXPECT_CALL(*mMockStream, Write(
        AllOf(NotifyMsgArgsMatcher(ErrorNumber::OK, roomId, userIds[0]),
            ProcessStateMatcher()), _)).Times(1);
    
    ProcessMsgFromCore(*MsgBuilder::CreateUserOperationArgs(0, roomId, 
        userIds[0], MsgBuilder::CreateLeaderWord("keyword:2")));
    
    const auto &state = mStateMachine->GetState().mRoomIdToGameState.at(roomId);
    EXPECT_TRUE((state.mCurrentTurn == Turn::RED_FOLLOWER) || (state.mCurrentTurn == Turn::BLUE_FOLLOWER));
}

TEXT_F(MockCoreFixture, FollowerWord)
{
    auto roomId = 1002;
    std::vector<unsigned int> userIds = {11, 22, 33, 44};
    RegisterAndGameStart(roomId, userIds);

    EXPECT_CALL(*mMockStream, Write(
        AllOf(NotifyMsgArgsMatcher(ErrorNumber::OK, roomId, userIds[0]),
            ProcessStateMatcher()), _)).Times(1);
    EXPECT_CALL(*mMockStream, Write(
        AllOf(NotifyMsgArgsMatcher(ErrorNumber::OK, roomId, userIds[0]),
            ProcessStateMatcher()), _)).Times(1);
        
    ProcessMsgFromCore(*MsgBuilder::CreateUserOperationArgs(0, roomId, 
        userIds[0], MsgBuilder::CreateLeaderWord("keyword:2")));
    ProcessMsgFromCore(*MsgBuilder::CreateUserOperationArgs(0, roomId, 
        userIds[0], MsgBuilder::CreateFollowerWord("guess")));
    
}

}}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}