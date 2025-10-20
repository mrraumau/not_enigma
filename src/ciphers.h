#pragma once

#include <string>

namespace not_enigma::ciphers
{
std::wstring CaesarCipher(const std::wstring& input, int shift, const std::wstring& alphabet);
std::wstring VigenereCipher(const std::wstring& input, const std::wstring& key, bool encrypt, const std::wstring& alphabet);
std::wstring AtbashCipher(const std::wstring& input, const std::wstring& alphabet);
}

