#include "ciphers.h"

#include <exception>
#include <iostream>
#include <string>

namespace
{
    int ExpectEqual(const std::wstring& actual, const std::wstring& expected, const char* description)
    {
        if (actual == expected)
        {
            return 0;
        }

        std::wcerr << L"Test failed for " << description << L"\n"
                   << L"  Expected: " << expected << L"\n"
                   << L"  Actual:   " << actual << L"\n";
        return 1;
    }
}

int main()
{
    int failures = 0;

    try
    {
        // Caesar cipher with classic alphabet.
        failures += ExpectEqual(
            not_enigma::ciphers::CaesarCipher(L"Hello World", 3, L"ABCDEFGHIJKLMNOPQRSTUVWXYZ"),
            L"Khoor Zruog",
            "Caesar encrypt shift +3");

        failures += ExpectEqual(
            not_enigma::ciphers::CaesarCipher(L"Khoor Zruog", -3, L"ABCDEFGHIJKLMNOPQRSTUVWXYZ"),
            L"Hello World",
            "Caesar decrypt shift -3");

        // Vigenere cipher using the classic ATTACK AT DAWN example.
        failures += ExpectEqual(
            not_enigma::ciphers::VigenereCipher(L"ATTACKATDAWN", L"LEMON", true, L"ABCDEFGHIJKLMNOPQRSTUVWXYZ"),
            L"LXFOPVEFRNHR",
            "Vigenere encrypt");

        failures += ExpectEqual(
            not_enigma::ciphers::VigenereCipher(L"LXFOPVEFRNHR", L"LEMON", false, L"ABCDEFGHIJKLMNOPQRSTUVWXYZ"),
            L"ATTACKATDAWN",
            "Vigenere decrypt");

        // Atbash cipher using the standard alphabet.
        failures += ExpectEqual(
            not_enigma::ciphers::AtbashCipher(L"Hello World", L"ABCDEFGHIJKLMNOPQRSTUVWXYZ"),
            L"Svool Dliow",
            "Atbash classic alphabet");

        // Atbash with a reversed custom alphabet matches the traditional Latin mapping.
        failures += ExpectEqual(
            not_enigma::ciphers::AtbashCipher(L"Cipher", L"ZYXWVUTSRQPONMLKJIHGFEDCBA"),
            L"Xrksvi",
            "Atbash reversed alphabet");

        if (failures == 0)
        {
            std::wcout << L"All cipher tests passed.\n";
        }
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Unexpected exception: " << ex.what() << '\n';
        return 1;
    }

    return failures == 0 ? 0 : 1;
}

