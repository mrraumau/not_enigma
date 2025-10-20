#include <windows.h>
#include <algorithm>
#include <chrono>
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

    HFONT g_uiFont = nullptr;
    HFONT g_headingFont = nullptr;
    HBRUSH g_backgroundBrush = nullptr;

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

    HFONT CreateUIFont(HWND hwnd, int pointSize, bool bold)
    {
        HDC hdc = GetDC(hwnd);
        const int height = -MulDiv(pointSize, GetDeviceCaps(hdc, LOGPIXELSY), 72);
        ReleaseDC(hwnd, hdc);
        return CreateFontW(height, 0, 0, 0, bold ? FW_SEMIBOLD : FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
            DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    }

    void ApplyFont(HWND hwnd, HFONT font)
    {
        if (font != nullptr)
        {
            SendMessageW(hwnd, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
        }
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
        EnableWindow(hwndKeywordLabel, enableKeyword);
        const BOOL enableShift = (type == CipherType::Caesar);
        EnableWindow(hwndShiftEdit, enableShift);
        EnableWindow(hwndShiftLabel, enableShift);

        const BOOL enablePreset = (type != CipherType::Vigenere);
        EnableWindow(hwndAlphabetPreset, enablePreset);
        EnableWindow(hwndAlphabetPresetLabel, enablePreset);
        const BOOL enableAlphabet = (type != CipherType::Vigenere);
        EnableWindow(hwndAlphabet, enableAlphabet);
        if (enablePreset)
        {
            if (SendMessageW(hwndAlphabetPreset, CB_GETCURSEL, 0, 0) == CB_ERR)
            {
                SendMessageW(hwndAlphabetPreset, CB_SETCURSEL, 0, 0);
                SetWindowTextString(hwndAlphabet, NORMAL_ALPHABET);
            }
        }
        else
        {
            SetWindowTextString(hwndAlphabet, NORMAL_ALPHABET);
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
            if (!g_uiFont)
            {
                g_uiFont = CreateUIFont(hwnd, 10, false);
            }
            if (!g_headingFont)
            {
                g_headingFont = CreateUIFont(hwnd, 14, true);
            }
            if (!g_backgroundBrush)
            {
                g_backgroundBrush = CreateSolidBrush(RGB(246, 247, 251));
            }

            const int padding = 16;
            const int groupInnerPadding = 14;
            const int labelHeight = 20;
            const int controlHeight = 28;
            const int comboDropHeight = 160;
            const int editHeight = 110;
            const int shortSpacing = 6;
            const int sectionSpacing = 18;
            const int buttonWidth = 110;
            const int buttonHeight = 34;
            const int windowWidth = 640;
            const int groupWidth = windowWidth - padding * 2;
            int currentY = padding;

            HWND hwndTitle = CreateWindowExW(0, L"STATIC", L"Not Enigma Cipher Suite",
                WS_CHILD | WS_VISIBLE | SS_LEFT,
                padding, currentY, groupWidth, 32,
                hwnd, nullptr, GetModuleHandleW(nullptr), nullptr);
            ApplyFont(hwndTitle, g_headingFont);

            currentY += 36;

            HWND hwndDescription = CreateWindowExW(0, L"STATIC",
                L"Explore classical substitution ciphers with customizable alphabets.",
                WS_CHILD | WS_VISIBLE | SS_LEFT,
                padding, currentY, groupWidth, 40,
                hwnd, nullptr, GetModuleHandleW(nullptr), nullptr);
            ApplyFont(hwndDescription, g_uiFont);

            currentY += 36;

            HWND hwndCipherGroup = CreateWindowExW(0, L"BUTTON", L"Cipher Options",
                WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
                padding, currentY, groupWidth, 220,
                hwnd, nullptr, GetModuleHandleW(nullptr), nullptr);
            ApplyFont(hwndCipherGroup, g_uiFont);

            int groupY = currentY + 30;
            const int innerX = padding + groupInnerPadding;
            const int innerWidth = groupWidth - groupInnerPadding * 2;
            int groupContentBottom = groupY;

            HWND hwndCipherLabel = CreateWindowExW(0, L"STATIC", L"Cipher:", WS_CHILD | WS_VISIBLE,
                innerX, groupY, 100, labelHeight,
                hwnd, nullptr, GetModuleHandleW(nullptr), nullptr);
            ApplyFont(hwndCipherLabel, g_uiFont);
            groupY += labelHeight + shortSpacing;

            hwndCombo = CreateWindowExW(0, L"COMBOBOX", nullptr,
                CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE | WS_TABSTOP,
                innerX, groupY, 220, comboDropHeight,
                hwnd, reinterpret_cast<HMENU>(ID_COMBO_CIPHER), GetModuleHandleW(nullptr), nullptr);
            ApplyFont(hwndCombo, g_uiFont);

            SendMessageW(hwndCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Caesar"));
            SendMessageW(hwndCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Vigenere"));
            SendMessageW(hwndCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Atbash"));
            SendMessageW(hwndCombo, CB_SETCURSEL, 0, 0);

            groupY += controlHeight + sectionSpacing;

            hwndKeywordLabel = CreateWindowExW(0, L"STATIC", L"Keyword:", WS_CHILD | WS_VISIBLE,
                innerX, groupY, 120, labelHeight,
                hwnd, nullptr, GetModuleHandleW(nullptr), nullptr);
            ApplyFont(hwndKeywordLabel, g_uiFont);
            groupY += labelHeight + shortSpacing;

            hwndKeyword = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
                innerX, groupY, innerWidth, controlHeight,
                hwnd, reinterpret_cast<HMENU>(ID_EDIT_KEYWORD), GetModuleHandleW(nullptr), nullptr);
            ApplyFont(hwndKeyword, g_uiFont);

            groupY += controlHeight + sectionSpacing;

            hwndShiftLabel = CreateWindowExW(0, L"STATIC", L"Shift:", WS_CHILD | WS_VISIBLE,
                innerX, groupY, 120, labelHeight,
                hwnd, nullptr, GetModuleHandleW(nullptr), nullptr);
            ApplyFont(hwndShiftLabel, g_uiFont);
            groupY += labelHeight + shortSpacing;

            hwndShift = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
                innerX, groupY, 140, controlHeight,
                hwnd, reinterpret_cast<HMENU>(ID_EDIT_SHIFT), GetModuleHandleW(nullptr), nullptr);
            ApplyFont(hwndShift, g_uiFont);

            groupY += controlHeight + sectionSpacing;

            hwndAlphabetPresetLabel = CreateWindowExW(0, L"STATIC", L"Alphabet Preset:", WS_CHILD | WS_VISIBLE,
                innerX, groupY, 160, labelHeight,
                hwnd, nullptr, GetModuleHandleW(nullptr), nullptr);
            ApplyFont(hwndAlphabetPresetLabel, g_uiFont);
            groupY += labelHeight + shortSpacing;

            hwndAlphabetPreset = CreateWindowExW(0, L"COMBOBOX", nullptr,
                CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE | WS_TABSTOP,
                innerX, groupY, 220, comboDropHeight,
                hwnd, reinterpret_cast<HMENU>(ID_COMBO_ALPHABET_PRESET), GetModuleHandleW(nullptr), nullptr);
            ApplyFont(hwndAlphabetPreset, g_uiFont);

            SendMessageW(hwndAlphabetPreset, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Normal"));
            SendMessageW(hwndAlphabetPreset, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Reverse"));
            SendMessageW(hwndAlphabetPreset, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Random"));
            SendMessageW(hwndAlphabetPreset, CB_SETCURSEL, 0, 0);

            groupY += controlHeight;
            groupContentBottom = groupY;

            const int cipherGroupHeight = (groupContentBottom - currentY) + groupInnerPadding;
            SetWindowPos(hwndCipherGroup, nullptr, 0, 0, groupWidth, cipherGroupHeight, SWP_NOMOVE | SWP_NOZORDER);
            currentY += cipherGroupHeight + sectionSpacing;

            HWND hwndAlphabetGroup = CreateWindowExW(0, L"BUTTON", L"Alphabet",
                WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
                padding, currentY, groupWidth, 160,
                hwnd, nullptr, GetModuleHandleW(nullptr), nullptr);
            ApplyFont(hwndAlphabetGroup, g_uiFont);

            int alphabetY = currentY + 30;

            HWND hwndAlphabetInfo = CreateWindowExW(0, L"STATIC",
                L"Adjust the working alphabet or leave the default ordering.",
                WS_CHILD | WS_VISIBLE | SS_LEFT,
                innerX, alphabetY, innerWidth, labelHeight,
                hwnd, nullptr, GetModuleHandleW(nullptr), nullptr);
            ApplyFont(hwndAlphabetInfo, g_uiFont);

            alphabetY += labelHeight + shortSpacing;

            hwndAlphabet = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
                innerX, alphabetY, innerWidth, controlHeight,
                hwnd, reinterpret_cast<HMENU>(ID_EDIT_ALPHABET), GetModuleHandleW(nullptr), nullptr);
            ApplyFont(hwndAlphabet, g_uiFont);
            SetWindowTextString(hwndAlphabet, NORMAL_ALPHABET);

            alphabetY += controlHeight;

            const int alphabetGroupHeight = (alphabetY - currentY) + groupInnerPadding;
            SetWindowPos(hwndAlphabetGroup, nullptr, 0, 0, groupWidth, alphabetGroupHeight, SWP_NOMOVE | SWP_NOZORDER);
            currentY += alphabetGroupHeight + sectionSpacing;

            HWND hwndMessageGroup = CreateWindowExW(0, L"BUTTON", L"Message",
                WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
                padding, currentY, groupWidth, 2 * editHeight + 80,
                hwnd, nullptr, GetModuleHandleW(nullptr), nullptr);
            ApplyFont(hwndMessageGroup, g_uiFont);

            int messageY = currentY + 30;

            HWND hwndInputLabel = CreateWindowExW(0, L"STATIC", L"Input:", WS_CHILD | WS_VISIBLE,
                innerX, messageY, 100, labelHeight,
                hwnd, nullptr, GetModuleHandleW(nullptr), nullptr);
            ApplyFont(hwndInputLabel, g_uiFont);
            messageY += labelHeight + shortSpacing;

            hwndInput = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | WS_VSCROLL | WS_HSCROLL,
                innerX, messageY, innerWidth, editHeight,
                hwnd, reinterpret_cast<HMENU>(ID_EDIT_INPUT), GetModuleHandleW(nullptr), nullptr);
            ApplyFont(hwndInput, g_uiFont);

            messageY += editHeight + sectionSpacing;

            HWND hwndOutputLabel = CreateWindowExW(0, L"STATIC", L"Output:", WS_CHILD | WS_VISIBLE,
                innerX, messageY, 100, labelHeight,
                hwnd, nullptr, GetModuleHandleW(nullptr), nullptr);
            ApplyFont(hwndOutputLabel, g_uiFont);
            messageY += labelHeight + shortSpacing;

            hwndOutput = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
                WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | WS_VSCROLL | WS_HSCROLL | ES_READONLY,
                innerX, messageY, innerWidth, editHeight,
                hwnd, reinterpret_cast<HMENU>(ID_EDIT_OUTPUT), GetModuleHandleW(nullptr), nullptr);
            ApplyFont(hwndOutput, g_uiFont);

            messageY += editHeight;
            const int messageGroupHeight = (messageY - currentY) + groupInnerPadding + 20;
            SetWindowPos(hwndMessageGroup, nullptr, 0, 0, groupWidth, messageGroupHeight, SWP_NOMOVE | SWP_NOZORDER);
            currentY += messageGroupHeight + sectionSpacing;

            HWND hwndEncrypt = CreateWindowExW(0, L"BUTTON", L"Encrypt",
                WS_CHILD | WS_VISIBLE | WS_TABSTOP,
                padding, currentY, buttonWidth, buttonHeight,
                hwnd, reinterpret_cast<HMENU>(ID_BTN_ENCRYPT), GetModuleHandleW(nullptr), nullptr);
            ApplyFont(hwndEncrypt, g_uiFont);

            HWND hwndDecrypt = CreateWindowExW(0, L"BUTTON", L"Decrypt",
                WS_CHILD | WS_VISIBLE | WS_TABSTOP,
                padding + buttonWidth + 12, currentY, buttonWidth, buttonHeight,
                hwnd, reinterpret_cast<HMENU>(ID_BTN_DECRYPT), GetModuleHandleW(nullptr), nullptr);
            ApplyFont(hwndDecrypt, g_uiFont);

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
        case WM_CTLCOLORDLG:
            return reinterpret_cast<LRESULT>(g_backgroundBrush);
        case WM_CTLCOLORSTATIC:
        case WM_CTLCOLORBTN:
        {
            HDC hdc = reinterpret_cast<HDC>(wParam);
            SetBkMode(hdc, TRANSPARENT);
            return reinterpret_cast<LRESULT>(g_backgroundBrush);
        }
        case WM_DESTROY:
            if (g_uiFont)
            {
                DeleteObject(g_uiFont);
                g_uiFont = nullptr;
            }
            if (g_headingFont)
            {
                DeleteObject(g_headingFont);
                g_headingFont = nullptr;
            }
            if (g_backgroundBrush)
            {
                DeleteObject(g_backgroundBrush);
                g_backgroundBrush = nullptr;
            }
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

    const DWORD windowStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    HWND hwnd = CreateWindowExW(0, CLASS_NAME, L"Not Enigma",
        windowStyle,
        CW_USEDEFAULT, CW_USEDEFAULT, 700, 680,
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
