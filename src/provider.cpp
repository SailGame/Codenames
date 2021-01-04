#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

#include "game_manager.h"
#include "state.h"
#include "msg_builder.h"

using SailGame::Common::NetworkInterface;
using SailGame::Common::GameManager;
using SailGame::Game::GlobalState;
using SailGame::Common::EventLoop;
using SailGame::Common::StateMachine;
using SailGame::Game::MsgBuilder;

int main(int argc, char** argv)
{
    spdlog::info("Hello, I'm Codenames Server!");

    std::string endpoint = "localhost:50051";
    auto stub = NetworkInterface::CreateStub(endpoint);
    GameManager<GlobalState> gameManager(
        EventLoop::Create(),
        StateMachine<GlobalState>::Create(),
        NetworkInterface::Create(stub));
        
    gameManager.StartWithRegisterArgs(
        MsgBuilder::CreateRegisterArgs(0, "codename", "CODENAMES", 10, 4));

    return 0;
}