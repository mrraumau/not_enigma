#include "ciphers.h"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cwctype>
#include <iostream>
#include <locale>
#include <random>
#include <sstream>
#include <string>
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

namespace
{
    constexpr wchar_t NORMAL_ALPHABET[] = L"ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    void ConfigureLocale()
    {
        try
        {
            const std::locale loc("");
            std::locale::global(loc);
            std::wcin.imbue(loc);
            std::wcout.imbue(loc);
        }
        catch (const std::exception&)
        {
            // Fallback to the classic locale if the user locale is unavailable.
            std::locale::global(std::locale::classic());
            std::wcin.imbue(std::locale());
            std::wcout.imbue(std::locale());
        }
    }

    std::wstring ReadLine(const std::wstring& prompt)
    {
        std::wcout << prompt;
        std::wcout.flush();
        std::wstring line;
        if (!std::getline(std::wcin, line))
        {
            throw std::runtime_error("Unable to read input from console.");
        }
        return line;
    }

    int ReadChoice(const std::wstring& prompt, int min, int max)
    {
        while (true)
        {
            const std::wstring line = ReadLine(prompt);
            std::wistringstream stream(line);
            int value = 0;
            if (stream >> value && value >= min && value <= max)
            {
                return value;
            }
            std::wcout << L"Please enter a number between " << min << L" and " << max << L".\n";
        }
    }

    int ReadInteger(const std::wstring& prompt)
    {
        while (true)
        {
            const std::wstring line = ReadLine(prompt);
            std::wistringstream stream(line);
            int value = 0;
            if (stream >> value)
            {
                return value;
            }
            std::wcout << L"Please enter a valid integer value.\n";
        }
    }

    std::wstring FilterLetters(const std::wstring& value)
    {
        std::wstring filtered;
        filtered.reserve(value.size());
        for (wchar_t ch : value)
        {
            if (std::iswalpha(ch))
            {
                filtered.push_back(ch);
            }
        }
        return filtered;
    }

    std::wstring ReadLettersOnly(
        const std::wstring& prompt,
        const std::wstring& inputLabel,
        bool allowEmpty)
    {
        while (true)
        {
            std::wstring value = ReadLine(prompt);
            if (value.empty())
            {
                if (allowEmpty)
                {
                    return value;
                }

                std::wcout << inputLabel << L" cannot be empty.\n";
                continue;
            }

            std::wstring invalid;
            invalid.reserve(value.size());
            for (wchar_t ch : value)
            {
                if (!std::iswalpha(ch))
                {
                    invalid.push_back(ch);
                }
            }

            if (!invalid.empty())
            {
                std::wcout << L"The following characters are not letters: " << invalid << L"\n";
                const int action = ReadChoice(
                    L"How would you like to proceed?\n"
                    L"  1) Re-enter\n"
                    L"  2) Remove invalid characters automatically\n\n"
                    L"Selection: ",
                    1, 2);

                if (action == 1)
                {
                    continue;
                }

                std::wstring filtered = FilterLetters(value);
                if (filtered.empty() && !allowEmpty)
                {
                    std::wcout << L"All characters would be removed. Please enter only letters.\n";
                    continue;
                }

                value.swap(filtered);
            }

            if (!allowEmpty && value.empty())
            {
                std::wcout << inputLabel << L" cannot be empty.\n";
                continue;
            }

            return value;
        }
    }

    std::mt19937& RandomEngine()
    {
        using Clock = std::chrono::steady_clock;
        static thread_local std::mt19937 engine = [] {
            try
            {
                std::random_device rd;
                return std::mt19937(rd());
            }
            catch (...)
            {
                const auto seed = static_cast<unsigned int>(Clock::now().time_since_epoch().count());
                return std::mt19937(seed);
            }
        }();
        return engine;
    }

    std::wstring GenerateRandomAlphabet()
    {
        std::wstring alphabet = NORMAL_ALPHABET;
        auto& engine = RandomEngine();
        std::shuffle(alphabet.begin(), alphabet.end(), engine);
        return alphabet;
    }

    std::wstring ChooseAlphabetPreset()
    {
        while (true)
        {
            const int choice = ReadChoice(
                L"\nChoose alphabet preset:\n"
                L"  1) Normal (ABCDEFGHIJKLMNOPQRSTUVWXYZ)\n"
                L"  2) Reverse (ZYXWVUTSRQPONMLKJIHGFEDCBA)\n"
                L"  3) Random\n"
                L"  4) Custom\n\n"
                L"Selection: ",
                1, 4);

            switch (choice)
            {
            case 1:
                return NORMAL_ALPHABET;
            case 2:
            {
                std::wstring reversed = NORMAL_ALPHABET;
                std::reverse(reversed.begin(), reversed.end());
                return reversed;
            }
            case 3:
            {
                const std::wstring randomAlphabet = GenerateRandomAlphabet();
                std::wcout << L"Generated alphabet: " << randomAlphabet << L"\n";
                return randomAlphabet;
            }
            case 4:
            {
                const std::wstring custom = ReadLettersOnly(
                    L"Enter custom alphabet (leave empty to use default): ",
                    L"Alphabet",
                    /*allowEmpty*/ true);
                if (custom.empty())
                {
                    return NORMAL_ALPHABET;
                }
                return custom;
            }
            default:
                break;
            }
        }
    }

}

int main()
{
    ConfigureLocale();

    std::wcout << L"Not Enigma Console\n";
    std::wcout << L"===================\n\n";

    try
    {
        const int cipherChoice = ReadChoice(
            L"Choose cipher:\n"
            L"  1) Caesar\n"
            L"  2) Vigenere\n"
            L"  3) Atbash\n\n"
            L"Selection: ",
            1, 3);

        const int actionChoice = ReadChoice(
            L"\nChoose action:\n"
            L"  1) Encrypt\n"
            L"  2) Decrypt\n\n"
            L"Selection: ",
            1, 2);
        const bool encrypt = actionChoice == 1;

        const std::wstring input = ReadLettersOnly(L"\nEnter text: ", L"Text", /*allowEmpty*/ false);

        std::wstring output;

        switch (cipherChoice)
        {
        case 1: // Caesar
        {
            const int shift = ReadInteger(L"Enter shift value: ");
            const std::wstring alphabet = ChooseAlphabetPreset();
            const int appliedShift = encrypt ? shift : -shift;
            output = not_enigma::ciphers::CaesarCipher(input, appliedShift, alphabet);
            break;
        }
        case 2: // Vigenere
        {
            const std::wstring keyword = ReadLettersOnly(L"Enter keyword: ", L"Keyword", /*allowEmpty*/ false);
            output = not_enigma::ciphers::VigenereCipher(input, keyword, encrypt, std::wstring());
            break;
        }
        case 3: // Atbash
        {
            const std::wstring alphabet = ChooseAlphabetPreset();
            output = not_enigma::ciphers::AtbashCipher(input, alphabet);
            break;
        }
        default:
            throw std::logic_error("Unknown cipher selection.");
        }

        std::wcout << L"\nResult:\n" << output << L"\n";
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Error: " << ex.what() << '\n';
        return 1;
    }

#ifdef _WIN32
    if (_isatty(_fileno(stdin)))
#else
    if (isatty(fileno(stdin)))
#endif
    {
        std::wcout << L"\nPress Enter to exit...";
        std::wcout.flush();
        std::wstring discard;
        std::getline(std::wcin, discard);
    }

    return 0;
}
