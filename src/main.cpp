#include <windows.h>
#include <string>
#include <stdexcept>

#include "ciphers.h"

namespace
{
    constexpr int ID_EDIT_INPUT = 1001;
    constexpr int ID_EDIT_KEY = 1002;
    constexpr int ID_EDIT_OUTPUT = 1003;
    constexpr int ID_COMBO_CIPHER = 1004;
    constexpr int ID_BTN_ENCRYPT = 1005;
    constexpr int ID_BTN_DECRYPT = 1006;
    constexpr int ID_EDIT_ALPHABET = 1007;

    enum class CipherType
    {
        Caesar = 0,
        Vigenere = 1,
        Atbash = 2
    };

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

    void UpdateKeyFieldState(HWND hwndCombo, HWND hwndKey)
    {
        const CipherType type = GetSelectedCipher(hwndCombo);
        const BOOL enable = (type != CipherType::Atbash);
        EnableWindow(hwndKey, enable);
        if (!enable)
        {
            SetWindowTextString(hwndKey, L"");
        }
    }

    void ShowError(HWND hwndOwner, const std::wstring& message)
    {
        MessageBoxW(hwndOwner, message.c_str(), L"Cipher Error", MB_ICONERROR | MB_OK);
    }

    void ProcessCipher(HWND hwnd, HWND hwndCombo, HWND hwndKey, HWND hwndAlphabet, HWND hwndInput, HWND hwndOutput, bool encrypt)
    {
        const CipherType type = GetSelectedCipher(hwndCombo);
        const std::wstring input = GetWindowTextString(hwndInput);
        const std::wstring keyText = GetWindowTextString(hwndKey);
        const std::wstring alphabetText = GetWindowTextString(hwndAlphabet);

        try
        {
            std::wstring result;
            switch (type)
            {
            case CipherType::Caesar:
            {
                int shift = 0;
                if (!keyText.empty())
                {
                    shift = std::stoi(keyText);
                }
                result = not_enigma::ciphers::CaesarCipher(input, encrypt ? shift : -shift, alphabetText);
                break;
            }
            case CipherType::Vigenere:
            {
                if (keyText.empty())
                {
                    throw std::invalid_argument("Vigenère key cannot be empty.");
                }
                result = not_enigma::ciphers::VigenereCipher(input, keyText, encrypt, alphabetText);
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
        static HWND hwndKey = nullptr;
        static HWND hwndOutput = nullptr;
        static HWND hwndCombo = nullptr;
        static HWND hwndAlphabet = nullptr;

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
            SendMessageW(hwndCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Vigenère"));
            SendMessageW(hwndCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Atbash"));
            SendMessageW(hwndCombo, CB_SETCURSEL, 0, 0);

            currentY += 40;

            CreateWindowExW(0, L"STATIC", L"Key:", WS_CHILD | WS_VISIBLE,
                padding, currentY, 100, labelHeight, hwnd, nullptr, GetModuleHandleW(nullptr), nullptr);

            currentY += labelHeight + 5;

            hwndKey = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
                padding, currentY, 200, keyHeight, hwnd, reinterpret_cast<HMENU>(ID_EDIT_KEY), GetModuleHandleW(nullptr), nullptr);

            currentY += keyHeight + sectionSpacing;

            CreateWindowExW(0, L"STATIC", L"Alphabet:", WS_CHILD | WS_VISIBLE,
                padding, currentY, 100, labelHeight, hwnd, nullptr, GetModuleHandleW(nullptr), nullptr);

            currentY += labelHeight + 5;

            hwndAlphabet = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
                padding, currentY, 480, keyHeight, hwnd, reinterpret_cast<HMENU>(ID_EDIT_ALPHABET), GetModuleHandleW(nullptr), nullptr);

            SetWindowTextString(hwndAlphabet, L"ABCDEFGHIJKLMNOPQRSTUVWXYZ");

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

            UpdateKeyFieldState(hwndCombo, hwndKey);
            break;
        }
        case WM_COMMAND:
        {
            const int controlId = LOWORD(wParam);
            const int notification = HIWORD(wParam);

            if (controlId == ID_COMBO_CIPHER && notification == CBN_SELCHANGE)
            {
                UpdateKeyFieldState(hwndCombo, hwndKey);
                return 0;
            }

            if (controlId == ID_BTN_ENCRYPT && notification == BN_CLICKED)
            {
                ProcessCipher(hwnd, hwndCombo, hwndKey, hwndAlphabet, hwndInput, hwndOutput, true);
                return 0;
            }

            if (controlId == ID_BTN_DECRYPT && notification == BN_CLICKED)
            {
                ProcessCipher(hwnd, hwndCombo, hwndKey, hwndAlphabet, hwndInput, hwndOutput, false);
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
