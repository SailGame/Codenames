#pragma once

#include <variant>
#include <memory>
#include <vector>

#include <codenames/codenames.pb.h>

namespace Core {
struct ProviderMsg;
}

namespace SailGame { namespace Common {

using ProviderMsgPtr = std::shared_ptr<Core::ProviderMsg>;
using ProviderMsgPtrs = std::vector<ProviderMsgPtr>;
}}

namespace SailGame { namespace Game {

using ::Codenames::Party;

class Word
{
public:
    Word(std::string word) : mStr(word) {}
    ::Codenames::Word ConvertToGrpcWord() const;
    bool operator==(const Word &other) const
    {
        return mStr == other.get_str();
    }
    bool operator<(const Word &other) const
    {
        return mStr < other.get_str();
    }
    std::string get_str() const {return mStr;}
private:
    std::string mStr;
};

class Card
{
public:
    Card(std::string word, Party party) : mWord(word), mParty(party) {}
    Card(Word word, Party party) : mWord(word), mParty(party) {}
    ::Codenames::Card ConvertToGrpcCard() const;
private:
    Word mWord;
    Party mParty;
};

template<typename T>
class Set
{
public:
    virtual std::vector<T> Draw() = 0;
};

class WordSet : public Set<Word>
{
public:
    WordSet(std::string filepath);
    std::vector<Word> Draw();
private:
    std::vector<Word> mWords;
};

class PartySet : public Set<Party>
{
public:
    PartySet(std::string filepath);
    std::vector<Party> Draw();
private:
    std::vector<Party> mParties;
};

}}