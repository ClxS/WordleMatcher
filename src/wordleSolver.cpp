#include "wordleSolver.h"

#include <cstdint>
#include <vector>
#include <fstream>
#include <cstdlib>     /* srand, rand */
#include <cassert>

#define VALIDATION_CHECKS 0

static eastl::vector<wordler::WordHash> gs_words;
static wordler::WordHash gs_defaultWord = wordler::composeWord("doums");

constexpr char c_baseChar = 'a' - 1;

wordler::WordHash wordler::composeWord(const std::string& str)
{
    return (str[0] - c_baseChar) << (0 * 5) |
        (str[1] - c_baseChar) << (1 * 5) |
        (str[2] - c_baseChar) << (2 * 5) |
        (str[3] - c_baseChar) << (3 * 5) |
        (str[4] - c_baseChar) << (4 * 5);
}

constexpr char getCharacter(uint32_t hash, int index)
{
    return (char)(c_baseChar + ((hash >> (index * 5)) & 0b11111));
}

std::string wordler::decomposeWord(wordler::WordHash hash)
{
    char szLetters[6];
    szLetters[5] = 0;

    szLetters[0] = (char)(c_baseChar + ((hash >> (0 * 5)) & 0b11111));
    szLetters[1] = (char)(c_baseChar + ((hash >> (1 * 5)) & 0b11111));
    szLetters[2] = (char)(c_baseChar + ((hash >> (2 * 5)) & 0b11111));
    szLetters[3] = (char)(c_baseChar + ((hash >> (3 * 5)) & 0b11111));
    szLetters[4] = (char)(c_baseChar + ((hash >> (4 * 5)) & 0b11111));

    return std::string(szLetters);
}

void wordler::initialize(const char* szFilePath)
{
    std::ifstream input(szFilePath);

    for (std::string line; std::getline(input, line); )
    {
        gs_words.push_back(composeWord(line));

        #if VALIDATION_CHECKS
        std::string word = decomposeWord(gs_words.back());
        assert(word == line);
        #endif
    }

    srand(time(NULL));
}

wordler::WordHash wordler::pickRandomWord()
{
    int iRandomIndex = rand() % gs_words.size();
    return gs_words[iRandomIndex];
}

wordler::WordHash wordler::getRecommendedSeedWord()
{
    return gs_defaultWord;
}

wordler::WordHash wordler::pickRandomWord(GuessSession& session)
{
    int iRandomIndex = rand() % session.m_vWordList.size();
    return session.m_vWordList[iRandomIndex];
}

wordler::GuessSession wordler::beginGuessSession()
{
    GuessSession session;
    session.m_vWordList = gs_words;
    session.m_uiTargetWord = wordler::pickRandomWord();
    session.m_uiCurrentInclusionMask = 0;
    return session;
}

constexpr uint32_t reduceSlot(uint32_t in, int index)
{
    return ((in >> (index * 5)) & 0b11111);
}

constexpr void orSlot(uint32_t& in, uint32_t value, int index)
{
    in |= value << (index * 5);
}

constexpr void setSlot(uint32_t& in, uint32_t value, int index)
{
    in &= ~(0b11111 << (index * 5));
    in |= value << (index * 5);
}

void wordler::step(GuessSession& session, WordHash guessedWord)
{
    uint32_t uiExclusionMask = 0;
    for (int i = 0; i < 5; i++)
    {
        uint32_t letter = reduceSlot(guessedWord, i);
        if (reduceSlot(session.m_uiTargetWord, i) == letter)
        {
            orSlot(session.m_uiCurrentInclusionMask, letter, i);
            orSlot(uiExclusionMask, 0b11111, i);
        }
        else
        {
            orSlot(uiExclusionMask, letter, i);
        }
    }

    uint32_t uiFoatingGuessMask = 0;
    uint32_t mutableFloatingTarget = session.m_uiTargetWord;
    for (int i = 0; i < 5; i++)
    {
        const uint32_t letter = reduceSlot(uiExclusionMask, i);
        if (letter == 0)
        {
            continue;
        }

        for (int j = 0; j < 5; j++)
        {
            if (reduceSlot(session.m_uiCurrentInclusionMask, j) != 0)
            {
                continue;
            }

            const uint32_t hashLetter = reduceSlot(mutableFloatingTarget, j);
            if (letter == hashLetter)
            {
                uiFoatingGuessMask = letter << (i * 5);
                setSlot(mutableFloatingTarget, 0, j);
            }
        }
    }

    uint32_t uiInclusionMask = session.m_uiCurrentInclusionMask;
    auto endIt = eastl::remove_if(
        session.m_vWordList.begin(),
        session.m_vWordList.end(),
        [uiInclusionMask, uiExclusionMask, uiFoatingGuessMask](uint32_t hash)
        {
            if ((hash & uiInclusionMask) != uiInclusionMask)
            {
                return true;
            }

            for (int i = 0; i < 5; i++)
            {
                if (reduceSlot(hash, i) == reduceSlot(uiExclusionMask, i))
                {
                    return true;
                }
            }

            for (int i = 0; i < 5; i++)
            {
                uint32_t floatingLetter = reduceSlot(uiFoatingGuessMask, i);
                if (floatingLetter == 0)
                {
                    continue;
                }

                bool bFound = false;
                for (int j = 0; j < 5; j++)
                {
                    uint32_t hashLetter = reduceSlot(hash, j);
                    if (hashLetter == floatingLetter)
                    {
                        bFound = true;
                        setSlot(hash, 0, j);
                        break;
                    }
                }

                if (!bFound)
                {
                    return true;
                }
            }

            return false;
        });
    session.m_vWordList.erase(endIt, session.m_vWordList.end());
}
