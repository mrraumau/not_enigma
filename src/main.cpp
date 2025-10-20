#include <windows.h>
#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>

#include "ciphers.h"

namespace
{
    constexpr int ID_EDIT_INPUT = 1001;
    constexpr int ID_EDIT_KEYWORD = 1002;
    constexpr int ID_EDIT_SHIFT = 1003;
    constexpr int ID_EDIT_OUTPUT = 1004;
    constexpr int ID_COMBO_CIPHER = 1005;
    constexpr int ID_BTN_ENCRYPT = 1006;
    constexpr int ID_BTN_DECRYPT = 1007;
    constexpr int ID_EDIT_ALPHABET = 1008;
    constexpr int ID_COMBO_ALPHABET_PRESET = 1009;

    enum class CipherType
    {
        Caesar = 0,
        Vigenere = 1,
        Atbash = 2
    };

    constexpr wchar_t NORMAL_ALPHABET[] = L"ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    std::wstring GetWindowTextString(HWND hwnd)
    {
        const int length = GetWindowTextLengthW(hwnd);
        if (length <= 0)
        {
            return L"";
        }

        std::wstring buffer(static_cast<std::size_t>(length) + 1, L'\0');
        const int copied = GetWindowTextW(hwnd, buffer.data(), length + 1);
        if (copied < 0)
        {
            return L"";
        }
        buffer.resize(static_cast<std::size_t>(copied));
        return buffer;
    }

    void SetWindowTextString(HWND hwnd, const std::wstring& text)
    {
        SetWindowTextW(hwnd, text.c_str());
    }

    std::wstring ToWide(const std::string& text)
    {
        if (text.empty())
        {
            return L"";
        }

        const int required = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, nullptr, 0);
        if (required <= 0)
        {
            return L"";
        }

        std::wstring wide(static_cast<std::size_t>(required), L'\0');
        const int converted = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, wide.data(), required);
        if (converted <= 0)
        {
            return L"";
        }
        if (!wide.empty() && wide.back() == L'\0')
        {
            wide.pop_back();
        }
        return wide;
    }

    CipherType GetSelectedCipher(HWND hwndCombo)
    {
        const int index = static_cast<int>(SendMessageW(hwndCombo, CB_GETCURSEL, 0, 0));
        switch (index)
        {
        case 0: return CipherType::Caesar;
        case 1: return CipherType::Vigenere;
        case 2: return CipherType::Atbash;
        default: return CipherType::Caesar;
        }
    }

    std::wstring GenerateRandomAlphabet()
    {
        std::wstring alphabet = NORMAL_ALPHABET;
        static thread_local std::mt19937 engine(std::random_device{}());
        std::shuffle(alphabet.begin(), alphabet.end(), engine);
        return alphabet;
    }

    void UpdateControlStates(HWND hwndCombo,
        HWND hwndKeywordLabel,
        HWND hwndKeywordEdit,
        HWND hwndShiftLabel,
        HWND hwndShiftEdit,
        HWND hwndAlphabetPresetLabel,
        HWND hwndAlphabetPreset,
        HWND hwndAlphabet)
    {
        const CipherType type = GetSelectedCipher(hwndCombo);

        const BOOL enableKeyword = (type == CipherType::Vigenere);
        EnableWindow(hwndKeywordEdit, enableKeyword);
        ShowWindow(hwndKeywordEdit, enableKeyword ? SW_SHOW : SW_HIDE);
        ShowWindow(hwndKeywordLabel, enableKeyword ? SW_SHOW : SW_HIDE);
        const BOOL enableShift = (type == CipherType::Caesar);
        EnableWindow(hwndShiftEdit, enableShift);
        ShowWindow(hwndShiftEdit, enableShift ? SW_SHOW : SW_HIDE);
        ShowWindow(hwndShiftLabel, enableShift ? SW_SHOW : SW_HIDE);

        const BOOL enablePreset = (type != CipherType::Vigenere);
        EnableWindow(hwndAlphabetPreset, enablePreset);
        ShowWindow(hwndAlphabetPreset, enablePreset ? SW_SHOW : SW_HIDE);
        ShowWindow(hwndAlphabetPresetLabel, enablePreset ? SW_SHOW : SW_HIDE);
        if (enablePreset)
        {
            if (SendMessageW(hwndAlphabetPreset, CB_GETCURSEL, 0, 0) == CB_ERR)
            {
                SendMessageW(hwndAlphabetPreset, CB_SETCURSEL, 0, 0);
                SetWindowTextString(hwndAlphabet, NORMAL_ALPHABET);
            }
        }
    }

    void ShowError(HWND hwndOwner, const std::wstring& message)
    {
        MessageBoxW(hwndOwner, message.c_str(), L"Cipher Error", MB_ICONERROR | MB_OK);
    }

    void ProcessCipher(HWND hwnd,
        HWND hwndCombo,
        HWND hwndKeyword,
        HWND hwndShift,
        HWND hwndAlphabet,
        HWND hwndInput,
        HWND hwndOutput,
        bool encrypt)
    {
        const CipherType type = GetSelectedCipher(hwndCombo);
        const std::wstring input = GetWindowTextString(hwndInput);
        const std::wstring keywordText = GetWindowTextString(hwndKeyword);
        const std::wstring shiftText = GetWindowTextString(hwndShift);
        const std::wstring alphabetText = GetWindowTextString(hwndAlphabet);

        try
        {
            std::wstring result;
            switch (type)
            {
            case CipherType::Caesar:
            {
                if (shiftText.empty())
                {
                    throw std::invalid_argument("Caesar shift cannot be empty.");
                }
                int shift = 0;
                try
                {
                    shift = std::stoi(shiftText);
                }
                catch (const std::exception&)
                {
                    throw std::invalid_argument("Caesar shift must be a valid integer.");
                }
                result = not_enigma::ciphers::CaesarCipher(input, encrypt ? shift : -shift, alphabetText);
                break;
            }
            case CipherType::Vigenere:
            {
                if (keywordText.empty())
                {
                    throw std::invalid_argument("Vigenere keyword cannot be empty.");
                }
                result = not_enigma::ciphers::VigenereCipher(input, keywordText, encrypt, alphabetText);
                break;
            }
            case CipherType::Atbash:
                result = not_enigma::ciphers::AtbashCipher(input, alphabetText);
                break;
            }

            SetWindowTextString(hwndOutput, result);
        }
        catch (const std::exception& ex)
        {
            std::wstring message = L"Error: ";
            message += ToWide(ex.what());
            ShowError(hwnd, message);
        }
        catch (...)
        {
            ShowError(hwnd, L"An unexpected error occurred.");
        }
    }

    LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        static HWND hwndInput = nullptr;
        static HWND hwndKeyword = nullptr;
        static HWND hwndShift = nullptr;
        static HWND hwndOutput = nullptr;
        static HWND hwndCombo = nullptr;
        static HWND hwndAlphabet = nullptr;
        static HWND hwndKeywordLabel = nullptr;
        static HWND hwndShiftLabel = nullptr;
        static HWND hwndAlphabetPreset = nullptr;
        static HWND hwndAlphabetPresetLabel = nullptr;

        switch (msg)
        {
        case WM_CREATE:
        {
            const int padding = 10;
            const int labelHeight = 20;
            const int editHeight = 120;
            const int keyHeight = 25;
            const int buttonWidth = 90;
            const int buttonHeight = 30;
            const int sectionSpacing = 10;
            int currentY = padding;

            hwndCombo = CreateWindowExW(0, L"COMBOBOX", nullptr,
                CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE | WS_TABSTOP,
                padding, currentY, 200, 200, hwnd, reinterpret_cast<HMENU>(ID_COMBO_CIPHER), GetModuleHandleW(nullptr), nullptr);

            SendMessageW(hwndCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Caesar"));
            SendMessageW(hwndCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Vigenere"));
            SendMessageW(hwndCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Atbash"));
            SendMessageW(hwndCombo, CB_SETCURSEL, 0, 0);

            currentY += 40;

            hwndKeywordLabel = CreateWindowExW(0, L"STATIC", L"Keyword:", WS_CHILD | WS_VISIBLE,
                padding, currentY, 100, labelHeight, hwnd, nullptr, GetModuleHandleW(nullptr), nullptr);

            currentY += labelHeight + 5;

            hwndKeyword = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
                padding, currentY, 200, keyHeight, hwnd, reinterpret_cast<HMENU>(ID_EDIT_KEYWORD), GetModuleHandleW(nullptr), nullptr);

            currentY += keyHeight + sectionSpacing;

            hwndShiftLabel = CreateWindowExW(0, L"STATIC", L"Shift:", WS_CHILD | WS_VISIBLE,
                padding, currentY, 100, labelHeight, hwnd, nullptr, GetModuleHandleW(nullptr), nullptr);

            currentY += labelHeight + 5;

            hwndShift = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
                padding, currentY, 200, keyHeight, hwnd, reinterpret_cast<HMENU>(ID_EDIT_SHIFT), GetModuleHandleW(nullptr), nullptr);

            currentY += keyHeight + sectionSpacing;

            CreateWindowExW(0, L"STATIC", L"Alphabet:", WS_CHILD | WS_VISIBLE,
                padding, currentY, 100, labelHeight, hwnd, nullptr, GetModuleHandleW(nullptr), nullptr);

            currentY += labelHeight + 5;

            hwndAlphabet = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
                padding, currentY, 480, keyHeight, hwnd, reinterpret_cast<HMENU>(ID_EDIT_ALPHABET), GetModuleHandleW(nullptr), nullptr);

            SetWindowTextString(hwndAlphabet, NORMAL_ALPHABET);

            currentY += keyHeight + sectionSpacing;

            hwndAlphabetPresetLabel = CreateWindowExW(0, L"STATIC", L"Alphabet Preset:", WS_CHILD | WS_VISIBLE,
                padding, currentY, 150, labelHeight, hwnd, nullptr, GetModuleHandleW(nullptr), nullptr);

            currentY += labelHeight + 5;

            hwndAlphabetPreset = CreateWindowExW(0, L"COMBOBOX", nullptr,
                CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE | WS_TABSTOP,
                padding, currentY, 200, 200, hwnd, reinterpret_cast<HMENU>(ID_COMBO_ALPHABET_PRESET), GetModuleHandleW(nullptr), nullptr);

            SendMessageW(hwndAlphabetPreset, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Normal"));
            SendMessageW(hwndAlphabetPreset, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Reverse"));
            SendMessageW(hwndAlphabetPreset, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Random"));
            SendMessageW(hwndAlphabetPreset, CB_SETCURSEL, 0, 0);

            currentY += keyHeight + sectionSpacing;

            CreateWindowExW(0, L"STATIC", L"Input:", WS_CHILD | WS_VISIBLE,
                padding, currentY, 100, labelHeight, hwnd, nullptr, GetModuleHandleW(nullptr), nullptr);

            currentY += labelHeight + 5;

            hwndInput = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | WS_VSCROLL,
                padding, currentY, 480, editHeight, hwnd, reinterpret_cast<HMENU>(ID_EDIT_INPUT), GetModuleHandleW(nullptr), nullptr);

            currentY += editHeight + sectionSpacing;

            CreateWindowExW(0, L"STATIC", L"Output:", WS_CHILD | WS_VISIBLE,
                padding, currentY, 100, labelHeight, hwnd, nullptr, GetModuleHandleW(nullptr), nullptr);

            currentY += labelHeight + 5;

            hwndOutput = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
                WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | WS_VSCROLL | ES_READONLY,
                padding, currentY, 480, editHeight, hwnd, reinterpret_cast<HMENU>(ID_EDIT_OUTPUT), GetModuleHandleW(nullptr), nullptr);

            currentY += editHeight + sectionSpacing + 20;

            CreateWindowExW(0, L"BUTTON", L"Encrypt",
                WS_CHILD | WS_VISIBLE | WS_TABSTOP,
                padding, currentY, buttonWidth, buttonHeight,
                hwnd, reinterpret_cast<HMENU>(ID_BTN_ENCRYPT), GetModuleHandleW(nullptr), nullptr);

            CreateWindowExW(0, L"BUTTON", L"Decrypt",
                WS_CHILD | WS_VISIBLE | WS_TABSTOP,
                padding + buttonWidth + 10, currentY, buttonWidth, buttonHeight,
                hwnd, reinterpret_cast<HMENU>(ID_BTN_DECRYPT), GetModuleHandleW(nullptr), nullptr);

            SetWindowTextString(hwndShift, L"3");
            UpdateControlStates(hwndCombo, hwndKeywordLabel, hwndKeyword, hwndShiftLabel, hwndShift, hwndAlphabetPresetLabel, hwndAlphabetPreset, hwndAlphabet);
            break;
        }
        case WM_COMMAND:
        {
            const int controlId = LOWORD(wParam);
            const int notification = HIWORD(wParam);

            if (controlId == ID_COMBO_CIPHER && notification == CBN_SELCHANGE)
            {
                UpdateControlStates(hwndCombo, hwndKeywordLabel, hwndKeyword, hwndShiftLabel, hwndShift, hwndAlphabetPresetLabel, hwndAlphabetPreset, hwndAlphabet);
                return 0;
            }

            if (controlId == ID_COMBO_ALPHABET_PRESET && notification == CBN_SELCHANGE)
            {
                if (!IsWindowEnabled(hwndAlphabetPreset))
                {
                    return 0;
                }

                const int selection = static_cast<int>(SendMessageW(hwndAlphabetPreset, CB_GETCURSEL, 0, 0));
                switch (selection)
                {
                case 0:
                    SetWindowTextString(hwndAlphabet, NORMAL_ALPHABET);
                    break;
                case 1:
                {
                    std::wstring reversed = NORMAL_ALPHABET;
                    std::reverse(reversed.begin(), reversed.end());
                    SetWindowTextString(hwndAlphabet, reversed);
                    break;
                }
                case 2:
                    SetWindowTextString(hwndAlphabet, GenerateRandomAlphabet());
                    break;
                default:
                    break;
                }
                return 0;
            }

            if (controlId == ID_BTN_ENCRYPT && notification == BN_CLICKED)
            {
                ProcessCipher(hwnd, hwndCombo, hwndKeyword, hwndShift, hwndAlphabet, hwndInput, hwndOutput, true);
                return 0;
            }

            if (controlId == ID_BTN_DECRYPT && notification == BN_CLICKED)
            {
                ProcessCipher(hwnd, hwndCombo, hwndKeyword, hwndShift, hwndAlphabet, hwndInput, hwndOutput, false);
                return 0;
            }

            break;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        }

        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
}

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow)
{
    const wchar_t CLASS_NAME[] = L"NotEnigmaWindowClass";

    WNDCLASSW wc = {};
    wc.lpfnWndProc = MainWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);

    if (!RegisterClassW(&wc))
    {
        MessageBoxW(nullptr, L"Failed to register window class.", L"Error", MB_ICONERROR | MB_OK);
        return 0;
    }

    HWND hwnd = CreateWindowExW(0, CLASS_NAME, L"Not Enigma",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 520, 620,
        nullptr, nullptr, hInstance, nullptr);

    if (!hwnd)
    {
        MessageBoxW(nullptr, L"Failed to create main window.", L"Error", MB_ICONERROR | MB_OK);
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return static_cast<int>(msg.wParam);
}
