#include "wordleSolver.h"
#include <iostream>
#include <chrono>
#include <omp.h>

#define PRINT_STEPS 0

void* __cdecl operator new[](size_t size, const char* name, int flags, unsigned debugFlags, const char* file, int line)
{
    return new uint8_t[size];
}
void* __cdecl operator new[](size_t size, size_t size1, size_t size2, const char* name, int flags, unsigned debugFlags, const char* file, int line)
{
    return new uint8_t[size];
}

template <typename T>
std::string to_string_with_precision(const T a_value, const int n = 2)
{
    std::ostringstream out;
    out.precision(n);
    out << std::fixed << a_value;
    return out.str();
}

int main()
{
    wordler::initialize("data/words.txt");

    constexpr int c_iterations = 10000;
    constexpr int c_successCutOff = 6;

    int success = 0;
    int failure = 0;
    int fastestSolve = 9999;
    int slowestSolve = 0;
    int totalSolveStepCount = 0;

    auto start = std::chrono::high_resolution_clock::now();

    omp_set_num_threads(16);
    #pragma omp parallel for num_threads(16)
    for (int i = 0; i < c_iterations; i++)
    {
        wordler::WordHash uiWord = wordler::getRecommendedSeedWord();
        wordler::GuessSession session = wordler::beginGuessSession(false);

        #if PRINT_STEPS
        std::cout << "Random word is: " << wordler::decomposeWord(session.m_uiTargetWord) << "\n";
        #endif

        int step = 0;
        while (session.m_vWordList.size() > 1)
        {
            #if PRINT_STEPS
            std::cout << "Picked Word: " << wordler::decomposeWord(uiWord) << " out of " << session.m_vWordList.size() << " words\n";
            #endif

            wordler::step(session, uiWord);
            uiWord = wordler::pickRandomWord(session);
            step++;

            if (uiWord == session.m_uiTargetWord)
            {
                break;
            }
        }

        if (step > c_successCutOff)
        {
            failure++;
        }
        else
        {
            success++;
        }

        slowestSolve = std::max(slowestSolve, step);
        fastestSolve = std::min(fastestSolve, step);
        totalSolveStepCount += step;

        #if PRINT_STEPS
        std::cout << "Found " << wordler::decomposeWord(session.m_vWordList[0]) << " after " << step << " steps!\n";
        #endif
}
    auto end = std::chrono::high_resolution_clock::now();

    std::cout << 
        std::format(
            "\nIterations: {}\nSolve Average ms: {}\nSuccess Rate: {}/{}\nFastest Solve: {} steps\nSlowest Solve: {} steps\nAverage Solve: {} steps",
            c_iterations,
            to_string_with_precision(std::chrono::duration_cast<std::chrono::microseconds>((end - start) / (double)c_iterations)),
            success,
            failure + success,
            fastestSolve,
            slowestSolve,
            to_string_with_precision(totalSolveStepCount / (double)c_iterations));

    return 0;
}
