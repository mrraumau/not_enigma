#include "ciphers.h"

#include <algorithm>
#include <cwctype>
#include <iostream>
#include <locale>
#include <random>
#include <sstream>
#include <string>

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

    bool ReadYesNo(const std::wstring& prompt)
    {
        while (true)
        {
            std::wstring line = ReadLine(prompt);
            std::transform(line.begin(), line.end(), line.begin(), [](wchar_t ch) { return static_cast<wchar_t>(std::towlower(ch)); });
            if (line == L"y" || line == L"yes")
            {
                return true;
            }
            if (line == L"n" || line == L"no")
            {
                return false;
            }
            std::wcout << L"Please answer with 'y' or 'n'.\n";
        }
    }

    std::wstring GenerateRandomAlphabet()
    {
        std::wstring alphabet = NORMAL_ALPHABET;
        static thread_local std::mt19937 engine(std::random_device{}());
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
                L"  4) Custom\n"
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
                const std::wstring custom = ReadLine(L"Enter custom alphabet (leave empty to use default): ");
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

    std::wstring PromptOptionalAlphabet()
    {
        if (ReadYesNo(L"\nWould you like to provide a custom alphabet? (y/n): "))
        {
            return ReadLine(L"Enter alphabet characters: ");
        }
        return std::wstring();
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
            L"  3) Atbash\n"
            L"Selection: ",
            1, 3);

        const int actionChoice = ReadChoice(
            L"\nChoose action:\n"
            L"  1) Encrypt\n"
            L"  2) Decrypt\n"
            L"Selection: ",
            1, 2);
        const bool encrypt = actionChoice == 1;

        const std::wstring input = ReadLine(L"\nEnter text: ");

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
            std::wstring keyword;
            while (keyword.empty())
            {
                keyword = ReadLine(L"Enter keyword: ");
                if (keyword.empty())
                {
                    std::wcout << L"Keyword cannot be empty.\n";
                }
            }
            const std::wstring alphabet = PromptOptionalAlphabet();
            output = not_enigma::ciphers::VigenereCipher(input, keyword, encrypt, alphabet);
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

    return 0;
}
