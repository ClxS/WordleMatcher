#include "wordleSolver.h"
#include <iostream>
#include <chrono>

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

    int success = 0;
    int failure = 0;

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 100; i++)
    {
        wordler::WordHash uiWord = wordler::getRecommendedSeedWord();
        wordler::GuessSession session = wordler::beginGuessSession();

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
        }

        if (step > 5)
        {
            failure++;
        }
        else
        {
            success++;
        }

        #if PRINT_STEPS
        std::cout << "Found " << wordler::decomposeWord(session.m_vWordList[0]) << " after " << step << " steps!\n";
        #endif
    }
    auto end = std::chrono::high_resolution_clock::now();

    std::cout << 
        std::format(
            "\nSolve Average ms: {}\nSuccess Rate: {}/{}",
            to_string_with_precision(std::chrono::duration_cast<std::chrono::microseconds>((end - start) / 100.0)),
            success,
            failure + success);

    return 0;
}
