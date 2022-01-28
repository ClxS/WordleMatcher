#pragma once
#include <string>
#include <vector>
#include "EASTL/vector.h"

namespace wordler
{
    using WordHash = uint32_t;

    struct GuessSession
    {
        uint32_t m_uiTargetWord;
        eastl::vector<uint32_t> m_vWordList;

        uint32_t m_uiCurrentInclusionMask;
    };

    void initialize(const char* szFilePath);
    void initialize(const char** pSzWords, int count);

    WordHash composeWord(const std::string& str);
    std::string decomposeWord(WordHash hash);

    WordHash pickRandomWord();
    WordHash pickRandomWord(GuessSession& session);
    WordHash getRecommendedSeedWord();

    GuessSession beginGuessSession(bool bUseRawList);
    void step(GuessSession& session, WordHash guessedWord);
}
