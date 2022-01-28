#include "wordleSolver.h"
#include <omp.h>

void* __cdecl operator new[](size_t size, const char* name, int flags, unsigned debugFlags, const char* file, int line)
{
    return new uint8_t[size];
}
void* __cdecl operator new[](size_t size, size_t size1, size_t size2, const char* name, int flags, unsigned debugFlags, const char* file, int line)
{
    return new uint8_t[size];
}

extern "C"
{
    __declspec(dllexport) void test()
    {

    }
    __declspec(dllexport) void loadWords(const char** szWords, int count)
    {
        wordler::initialize(szWords, count);
    }

    __declspec(dllexport) int solve(int count)
    {
        int step = 0;

        for (int i = 0; i < count; i++)
        {
            wordler::WordHash uiWord = wordler::getRecommendedSeedWord();
            wordler::GuessSession session = wordler::beginGuessSession(false);

            while (session.m_vWordList.size() > 1)
            {
                wordler::step(session, uiWord);
                uiWord = wordler::pickRandomWord(session);
                step++;

                if (uiWord == session.m_uiTargetWord)
                {
                    break;
                }
            }
        }

        return step;
    }

    __declspec(dllexport) int solveParallel(int count)
    {
        int step = 0;

        omp_set_num_threads(16);
        #pragma omp parallel for num_threads(16)
        for (int i = 0; i < count; i++)
        {
            wordler::WordHash uiWord = wordler::getRecommendedSeedWord();
            wordler::GuessSession session = wordler::beginGuessSession(false);

            while (session.m_vWordList.size() > 1)
            {
                wordler::step(session, uiWord);
                uiWord = wordler::pickRandomWord(session);
                step++;

                if (uiWord == session.m_uiTargetWord)
                {
                    break;
                }
            }
        }

        return step;
    }
}
