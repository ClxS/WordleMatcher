#include "wordleSolver.h"

#include <cstdint>
#include <vector>
#include <fstream>
#include <cstdlib>     /* srand, rand */
#include <cassert>
#include <bitset>
#include <algorithm>
#include <iostream>

#define VALIDATION_CHECKS 0

static eastl::vector<wordler::WordHash> gs_words;
static wordler::WordHash gs_defaultWord = wordler::composeWord("irate");

constexpr uint8_t c_invalid = 0b11111;
constexpr char c_baseChar = 'a' - 1;

constexpr uint32_t composeCharacter(char ch, int index)
{
    return (ch - c_baseChar) << (index * 5);
}

constexpr char getCharacter(uint32_t hash, int index)
{
    return (char)(c_baseChar + ((hash >> (index * 5)) & 0b11111));
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

wordler::WordHash wordler::composeWord(const std::string& str)
{
    return composeCharacter(str[0], 0) |
        composeCharacter(str[1], 1) |
        composeCharacter(str[2], 2) |
        composeCharacter(str[3], 3) |
        composeCharacter(str[4], 4);
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

int32_t scoreWord(uint32_t word, uint32_t unscoreMask = 0)
{
    int32_t score = 0;
    for (int i = 0; i < 5; i++)
    {
        if (reduceSlot(unscoreMask, i) != 0)
        {
            continue;
        }

        for (int j = 0; j < 5; j++)
        {
            if (i != j && reduceSlot(word, i) == reduceSlot(word, j))
            {
                score -= 100;
            }
        }

        uint32_t letter = reduceSlot(word, i);
        if (letter == composeCharacter('e', 0))
        {
            score += 10;
        }

        if (letter == composeCharacter('t', 0))
        {
            score += 8;
        }

        if (letter == composeCharacter('a', 0) ||
            letter == composeCharacter('i', 0) ||
            letter == composeCharacter('n', 0) ||
            letter == composeCharacter('o', 0) ||
            letter == composeCharacter('s', 0))
        {
            score += 6;
        }

        if (letter == composeCharacter('h', 0))
        {
            score += 5;
        }

        if (letter == composeCharacter('r', 0))
        {
            score += 5;
        }

        if (letter == composeCharacter('d', 0))
        {
            score += 4;
        }

        if (letter == composeCharacter('l', 0))
        {
            score += 4;
        }

        if (letter == composeCharacter('u', 0))
        {
            score += 3;
        }

        if (letter == composeCharacter('c', 0) || letter == composeCharacter('m', 0))
        {
            score += 3;
        }

        if (letter == composeCharacter('f', 0))
        {
            score += 2;
        }

        if (letter == composeCharacter('w', 0) || letter == composeCharacter('y', 0))
        {
            score += 3;
        }
    }

    return score;
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

    std::sort(gs_words.begin(), gs_words.end(), [](uint32_t a, uint32_t b) {
        return scoreWord(a) > scoreWord(b);
    });

    srand(time(NULL));
}

wordler::WordHash wordler::pickRandomWord()
{
    int iRandomIndex = rand() % gs_words.size();
    return gs_words[iRandomIndex];
}

wordler::WordHash wordler::getRecommendedSeedWord()
{
    return gs_words[0];
}

wordler::WordHash wordler::pickRandomWord(GuessSession& session)
{
    int iRandomIndex = rand() % session.m_vWordList.size();
    return session.m_vWordList[0];
}

wordler::GuessSession wordler::beginGuessSession()
{
    GuessSession session;
    session.m_vWordList = gs_words;
    session.m_uiTargetWord = wordler::pickRandomWord();
    session.m_uiCurrentInclusionMask =
        c_invalid << (0 * 5) |
        c_invalid << (1 * 5) |
        c_invalid << (2 * 5) |
        c_invalid << (3 * 5) |
        c_invalid << (4 * 5);
    return session;
}

void wordler::step(GuessSession& session, WordHash guessedWord)
{
    bool bNewKnowns = false;
    uint32_t uiExclusionMask = 0;
    for (int i = 0; i < 5; i++)
    {
        if (reduceSlot(session.m_uiCurrentInclusionMask, i) != c_invalid)
        {
            orSlot(uiExclusionMask, c_invalid, i);
            continue;
        }

        uint32_t letter = reduceSlot(guessedWord, i);
        if (reduceSlot(session.m_uiTargetWord, i) == letter)
        {
            bNewKnowns = true;
            setSlot(session.m_uiCurrentInclusionMask, letter, i);
            orSlot(uiExclusionMask, c_invalid, i);
        }
        else
        {
            orSlot(uiExclusionMask, letter, i);
        }
    }

    uint32_t uiFoatingGuessMask = c_invalid << (0 * 5) |
        c_invalid << (1 * 5) |
        c_invalid << (2 * 5) |
        c_invalid << (3 * 5) |
        c_invalid << (4 * 5);
    uint32_t mutableFloatingTarget = session.m_uiTargetWord;
    for (int i = 0; i < 5; i++)
    {
        const uint32_t guessLetter = reduceSlot(uiExclusionMask, i);
        if (guessLetter == c_invalid)
        {
            continue;
        }

        for (int j = 0; j < 5; j++)
        {
            if (reduceSlot(session.m_uiCurrentInclusionMask, j) != c_invalid)
            {
                continue;
            }

            const uint32_t hashLetter = reduceSlot(mutableFloatingTarget, j);
            if (guessLetter == hashLetter)
            {
                setSlot(uiFoatingGuessMask, guessLetter, i);
                setSlot(mutableFloatingTarget, 0, j);
                setSlot(uiExclusionMask, c_invalid, i);
                break;
            }
        }
    }

    uint32_t uiInclusionMask = session.m_uiCurrentInclusionMask;
    auto endIt = eastl::remove_if(
        session.m_vWordList.begin(),
        session.m_vWordList.end(),
        [uiInclusionMask, uiExclusionMask, uiFoatingGuessMask](uint32_t hash)
        {
            for (int i = 0; i < 5; i++)
            {
                uint32_t hashCell = reduceSlot(hash, i);
                uint32_t inclusionCell = reduceSlot(uiInclusionMask, i);
                if (inclusionCell != c_invalid && hashCell != inclusionCell)
                {
                    return true;
                }

                for (int j = 0; j < 5; j++)
                {
                    if (inclusionCell == c_invalid && hashCell == reduceSlot(uiExclusionMask, j))
                    {
                        return true;
                    }
                }

                uint32_t floatingLetter = reduceSlot(uiFoatingGuessMask, i);
                if (floatingLetter == c_invalid)
                {
                    continue;
                }

                if (floatingLetter == hashCell)
                {
                    return true;
                }

                bool bFound = false;
                for (int j = 0; j < 5; j++)
                {
                    if (reduceSlot(uiInclusionMask, j) != c_invalid)
                    {
                        continue;
                    }

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
