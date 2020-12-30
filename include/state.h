#pragma once

#include <map>
#include <codenames/codenames.pb.h>

#include "types.h"

namespace SailGame { namespace Game {

using ::Codenames::StartGameSettings;
using ::Codenames::Turn;
using ::Codenames::State;
using Game::Card;
using Game::Word;

class GameState
{
public:
    using StartStateInfo = std::tuple<
        std::map<unsigned int, Turn>,
        std::vector<Card>,
        enum Turn
    >;

    GameState(const std::vector<unsigned int> &userIds, const StartGameSettings &settings);

    StartStateInfo GameStart();

    template<typename MsgT>
    void process(const ::Codenames::Word &word)
    {
        throw std::runtime_error("Unsupported msg type");
    }

    unsigned int mPlayerNum;
    StartGameSettings mSettings;
    enum Turn mCurrentTurn;
    enum State mCurrentState;
    Word mVotedWord{"DISCUSSING..."};
private:
    std::map<Word, enum Party> mWordToParty;
    int mBluePartyWordsRemain{-1};
    int mRedPartyWordsRemain{-1};
    std::vector<Word> mVoteQueue;
    WordSet mWordSet{"data/word.txt"};
    PartySet mPartySet{"data/party.txt"};
};

class GlobalState
{
public:
    GameState::StartStateInfo NewGame(int roomId, const std::vector<unsigned int> &userIds,
        const StartGameSettings &settings);
    
    std::map<int, GameState> mRoomIdToGameState;
    int mCurrentRoomId{-1};
};

}}