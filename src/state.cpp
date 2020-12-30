#include "state.h"
#include <set>

namespace SailGame { namespace Game {

using ::Codenames::LeaderWord;
using ::Codenames::FollowerWord;

static const size_t MATRIX_SIZE = 25;

struct comp {
    template<typename T>
    bool operator()(const T &lh, const T &rh) const
    {
        return lh.second > rh.second;
    }
};

GameState::GameState(const std::vector<unsigned int> &userIds, const StartGameSettings &settings)
    : mPlayerNum(userIds.size()), mSettings(settings){}

GameState::StartStateInfo GameState::GameStart()
{
    std::map<unsigned int, Turn> userIdToRole;
    // 0->blue leader; 1->red leader; odd->blue follower; even->red follower
    for(auto id = 0; id < mPlayerNum; id++)
    {
        userIdToRole[id] = (id == 0) ? Turn::BLUE_LEADER : (
            (id == 1) ? Turn::RED_LEADER : (
                (id % 2) ? Turn::BLUE_FOLLOWER : Turn::RED_FOLLOWER));
    }

    std::vector<Card> cardInfo;
    // draw from word set and party set
    auto words = mWordSet.Draw();
    auto parties = mPartySet.Draw();
    mBluePartyWordsRemain = mRedPartyWordsRemain = 0;
    for(size_t i = 0; i < MATRIX_SIZE; i++)
    {
        // init internal state
        mWordToParty[words[i]] = parties[i];
        mBluePartyWordsRemain += (parties[i] == Codenames::Party::BLUE);
        mRedPartyWordsRemain +=(parties[i] == Codenames::Party::RED);
        // prepare return msg
        cardInfo.emplace_back(words[i], parties[i]);
    }

    mCurrentTurn = (mBluePartyWordsRemain > mRedPartyWordsRemain) ? Codenames::Turn::BLUE_LEADER : Codenames::Turn::RED_LEADER;
    mCurrentState = Codenames::State::NOT_DETERMINED;
    return {userIdToRole, cardInfo, mCurrentTurn};
}

template<>
void GameState::process<LeaderWord>(const ::Codenames::Word &word)
{
    mCurrentTurn = (mCurrentTurn == Turn::BLUE_LEADER) ? Turn::BLUE_FOLLOWER : Turn::RED_FOLLOWER;
}

template<>
void GameState::process<FollowerWord>(const ::Codenames::Word &word)
{
    mVoteQueue.push_back(word.word());
    auto teamMember = (mCurrentTurn == Turn::BLUE_FOLLOWER) ? (mPlayerNum / 2 - 1 + mPlayerNum % 2) : (mPlayerNum / 2 - 1);
    if(mVoteQueue.size() == teamMember)
    {
        std::map<Word, unsigned int> wordToCounts;
        for(auto word : mVoteQueue)
        {
            if(wordToCounts.count(word)) {
                wordToCounts[word]++;
            }
            else {
                wordToCounts[word] = 1;
            }
        }
        std::set<std::pair<Word, unsigned int>, comp> helper(wordToCounts.begin(), wordToCounts.end());
        mVotedWord = helper.begin()->first;
        // core judgement
        if(mWordToParty[mVotedWord] == Party::ASSASSIN) {
            mCurrentState = (mCurrentTurn == Turn::BLUE_FOLLOWER) ? State::RED_WINS : State::BULE_WINS;
        } else if(mWordToParty[mVotedWord] == Party::BYSTANDER) {
            mCurrentTurn = (mCurrentTurn == Turn::BLUE_FOLLOWER) ? Turn::RED_LEADER : Turn::BLUE_LEADER;
        } else {
            mBluePartyWordsRemain -= (mWordToParty[mVotedWord] == Party::BLUE);
            mRedPartyWordsRemain -= (mWordToParty[mVotedWord] == Party::RED);
            if(!mBluePartyWordsRemain) {
                mCurrentState = State::BULE_WINS;
            } else if(!mRedPartyWordsRemain) {
                mCurrentState = State::RED_WINS;
            } else {
                // oolong guess
                if(!(mCurrentTurn == Turn::BLUE_FOLLOWER && mWordToParty[mVotedWord] == Party::BLUE) && 
                    !(mCurrentTurn == Turn::RED_FOLLOWER && mWordToParty[mVotedWord] == Party::RED) ) {
                    mCurrentTurn = (mCurrentTurn == Turn::BLUE_FOLLOWER) ? Turn::RED_LEADER : Turn::BLUE_LEADER;
                }
            }
        }
    }
}

GameState::StartStateInfo GlobalState::NewGame(int roomId, const std::vector<unsigned int> &userIds,
    const StartGameSettings &settings)
{
    assert(mRoomIdToGameState.find(roomId) == mRoomIdToGameState.end());
    mRoomIdToGameState.emplace(roomId, GameState{userIds, settings});
    return mRoomIdToGameState.at(roomId).GameStart();
}

}}