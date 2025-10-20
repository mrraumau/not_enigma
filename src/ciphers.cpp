#include "ciphers.h"

#include <algorithm>
#include <cwctype>
#include <stdexcept>
#include <vector>

namespace not_enigma::ciphers
{
namespace
{
    constexpr wchar_t DEFAULT_ALPHABET[] = L"ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    std::wstring PrepareAlphabet(const std::wstring& alphabet)
    {
        const std::wstring source = alphabet.empty() ? std::wstring(DEFAULT_ALPHABET) : alphabet;

        std::wstring normalized;
        normalized.reserve(source.size());

        for (wchar_t ch : source)
        {
            if (std::iswspace(ch))
            {
                continue;
            }

            const wchar_t upper = std::towupper(ch);
            if (std::find(normalized.begin(), normalized.end(), upper) == normalized.end())
            {
                normalized.push_back(upper);
            }
        }

        if (normalized.size() < 2)
        {
            throw std::invalid_argument("Alphabet must contain at least two unique characters.");
        }

        return normalized;
    }

    wchar_t ApplyShift(wchar_t ch, int shift, const std::wstring& alphabet)
    {
        const bool wasLower = std::iswlower(ch) != 0;
        const bool wasUpper = std::iswupper(ch) != 0;
        const wchar_t upper = std::towupper(ch);

        const std::size_t pos = alphabet.find(upper);
        if (pos == std::wstring::npos)
        {
            return ch;
        }

        const int size = static_cast<int>(alphabet.size());
        int index = static_cast<int>(pos);
        index = (index + shift) % size;
        if (index < 0)
        {
            index += size;
        }

        wchar_t mapped = alphabet[static_cast<std::size_t>(index)];
        if (wasLower)
        {
            mapped = static_cast<wchar_t>(std::towlower(mapped));
        }
        else if (!wasUpper)
        {
            // Non-alphabetic characters (such as digits) are preserved as-is in the alphabet.
            return mapped;
        }

        return mapped;
    }
}

std::wstring CaesarCipher(const std::wstring& input, int shift, const std::wstring& alphabet)
{
    const std::wstring preparedAlphabet = PrepareAlphabet(alphabet);
    const int size = static_cast<int>(preparedAlphabet.size());
    const int normalizedShift = size == 0 ? 0 : shift % size;

    std::wstring output;
    output.reserve(input.size());
    for (wchar_t ch : input)
    {
        output.push_back(ApplyShift(ch, normalizedShift, preparedAlphabet));
    }
    return output;
}

std::wstring VigenereCipher(const std::wstring& input, const std::wstring& key, bool encrypt, const std::wstring& alphabet)
{
    const std::wstring preparedAlphabet = PrepareAlphabet(alphabet);

    std::vector<int> shifts;
    shifts.reserve(key.size());
    for (wchar_t ch : key)
    {
        const wchar_t upper = std::towupper(ch);
        const std::size_t pos = preparedAlphabet.find(upper);
        if (pos != std::wstring::npos)
        {
            const int shift = static_cast<int>(pos) * (encrypt ? 1 : -1);
            shifts.push_back(shift);
        }
    }

    if (shifts.empty())
    {
        throw std::invalid_argument("Vigenère key must contain characters from the alphabet.");
    }

    std::wstring output;
    output.reserve(input.size());

    std::size_t key_index = 0;
    for (wchar_t ch : input)
    {
        const wchar_t upper = std::towupper(ch);
        const std::size_t pos = preparedAlphabet.find(upper);
        if (pos != std::wstring::npos)
        {
            const int shift = shifts[key_index % shifts.size()];
            output.push_back(ApplyShift(ch, shift, preparedAlphabet));
            ++key_index;
        }
        else
        {
            output.push_back(ch);
        }
    }

    return output;
}

std::wstring AtbashCipher(const std::wstring& input, const std::wstring& alphabet)
{
    const std::wstring preparedAlphabet = PrepareAlphabet(alphabet);
    std::wstring output;
    output.reserve(input.size());
    const std::size_t lastIndex = preparedAlphabet.size() - 1;
    for (wchar_t ch : input)
    {
        const bool wasLower = std::iswlower(ch) != 0;
        const bool wasUpper = std::iswupper(ch) != 0;
        const wchar_t upper = std::towupper(ch);
        const std::size_t pos = preparedAlphabet.find(upper);
        if (pos == std::wstring::npos)
        {
            output.push_back(ch);
            continue;
        }

        wchar_t mapped = preparedAlphabet[lastIndex - pos];
        if (wasLower)
        {
            mapped = static_cast<wchar_t>(std::towlower(mapped));
        }
        else if (!wasUpper)
        {
            output.push_back(mapped);
            continue;
        }

        output.push_back(mapped);
    }
    return output;
}
}

