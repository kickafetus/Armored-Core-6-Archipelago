#include "configui.h"
#include "dllmain.h"    // Log(), AC6_ApplyConnection()
#include "apclient.h"   // APClient_StatusText()

#include <windows.h>
#include <thread>
#include <atomic>
#include <string>

// A small dialog-style window with Host/Port/Slot/Password fields and a Connect
// button. Runs on its own thread with a normal message loop. Hidden until the
// hotkey toggles it; closing (X) hides rather than destroys.

namespace {

constexpr wchar_t kClass[] = L"AC6AP_Settings";
enum { IDC_HOST = 1001, IDC_PORT, IDC_SLOT, IDC_PASS, IDC_CONNECT, IDC_STATUS };
enum { WM_TOGGLE = WM_APP + 1 };

std::atomic<bool> g_running{ false };
std::thread       g_thread;
HWND g_wnd = nullptr, g_eHost = nullptr, g_ePort = nullptr,
     g_eSlot = nullptr, g_ePass = nullptr, g_status = nullptr;
std::wstring g_iHost, g_iPort, g_iSlot, g_iPass;   // initial field values

std::wstring ToW(const char* s) {
    if (!s) return L"";
    int n = MultiByteToWideChar(CP_UTF8, 0, s, -1, nullptr, 0);
    std::wstring w(n > 0 ? n - 1 : 0, 0);
    if (n > 0) MultiByteToWideChar(CP_UTF8, 0, s, -1, &w[0], n);
    return w;
}
std::string ToA(const std::wstring& w) {
    int n = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string s(n > 0 ? n - 1 : 0, 0);
    if (n > 0) WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, &s[0], n, nullptr, nullptr);
    return s;
}
std::wstring GetText(HWND h) {
    int n = GetWindowTextLengthW(h);
    if (n <= 0) return L"";
    std::wstring w(n, 0);
    GetWindowTextW(h, &w[0], n + 1);
    return w;
}

HWND Label(HWND p, const wchar_t* t, int y) {
    return CreateWindowExW(0, L"STATIC", t, WS_CHILD | WS_VISIBLE,
                           12, y + 2, 74, 20, p, nullptr, nullptr, nullptr);
}
HWND Edit(HWND p, int id, int y, const std::wstring& val, bool password) {
    DWORD st = WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | (password ? ES_PASSWORD : 0);
    return CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", val.c_str(), st,
                           92, y, 252, 23, p, (HMENU)(INT_PTR)id, nullptr, nullptr);
}
void ApplyGuiFont(HWND h) {
    HFONT f = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    EnumChildWindows(h, [](HWND c, LPARAM lp) -> BOOL {
        SendMessageW(c, WM_SETFONT, (WPARAM)lp, TRUE); return TRUE;
    }, (LPARAM)f);
}

LRESULT CALLBACK Proc(HWND h, UINT m, WPARAM wp, LPARAM lp) {
    switch (m) {
    case WM_CREATE:
        Label(h, L"Host", 14);     g_eHost = Edit(h, IDC_HOST, 12, g_iHost, false);
        Label(h, L"Port", 46);     g_ePort = Edit(h, IDC_PORT, 44, g_iPort, false);
        Label(h, L"Slot", 78);     g_eSlot = Edit(h, IDC_SLOT, 76, g_iSlot, false);
        Label(h, L"Password", 110);g_ePass = Edit(h, IDC_PASS, 108, g_iPass, true);
        CreateWindowExW(0, L"BUTTON", L"Connect",
                        WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                        92, 144, 120, 30, h, (HMENU)IDC_CONNECT, nullptr, nullptr);
        g_status = CreateWindowExW(0, L"STATIC", L"Disconnected",
                        WS_CHILD | WS_VISIBLE, 12, 186, 332, 20, h,
                        (HMENU)IDC_STATUS, nullptr, nullptr);
        ApplyGuiFont(h);
        SetTimer(h, 1, 500, nullptr);
        return 0;

    case WM_COMMAND:
        if (LOWORD(wp) == IDC_CONNECT) {
            std::string host = ToA(GetText(g_eHost)), port = ToA(GetText(g_ePort));
            std::string slot = ToA(GetText(g_eSlot)), pass = ToA(GetText(g_ePass));
            AC6_ApplyConnection(host.c_str(), port.c_str(), slot.c_str(), pass.c_str());
            SetWindowTextW(g_status, L"Connecting...");
        }
        return 0;

    case WM_TIMER:
        if (g_status) SetWindowTextW(g_status, ToW(APClient_StatusText().c_str()).c_str());
        return 0;

    case WM_TOGGLE:
        if (IsWindowVisible(h)) {
            ShowWindow(h, SW_HIDE);
        } else {
            ShowWindow(h, SW_SHOW);
            SetForegroundWindow(h);
            SetFocus(g_eHost);
        }
        return 0;

    case WM_CLOSE:                 // X button hides; we keep the window alive
        ShowWindow(h, SW_HIDE);
        return 0;

    case WM_DESTROY:
        KillTimer(h, 1);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(h, m, wp, lp);
}

void ThreadMain() {
    WNDCLASSEXW wc{};
    wc.cbSize        = sizeof(wc);
    wc.lpfnWndProc   = Proc;
    wc.hInstance     = GetModuleHandleW(nullptr);
    wc.lpszClassName = kClass;
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    RegisterClassExW(&wc);

    RECT r{ 0, 0, 360, 224 };
    AdjustWindowRect(&r, WS_CAPTION | WS_SYSMENU, FALSE);
    g_wnd = CreateWindowExW(WS_EX_TOPMOST, kClass, L"Archipelago - Armored Core VI",
                            WS_CAPTION | WS_SYSMENU | WS_POPUP,
                            140, 140, r.right - r.left, r.bottom - r.top,
                            nullptr, nullptr, wc.hInstance, nullptr);
    if (!g_wnd) { Log("ConfigUI: CreateWindow failed (%lu)", GetLastError()); return; }
    Log("ConfigUI ready - press F8 in-game to open Archipelago settings");

    MSG msg;
    while (g_running && GetMessageW(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    g_wnd = nullptr;
}

} // namespace

void ConfigUI_Start(const char* host, const char* port,
                    const char* slot, const char* password) {
    if (g_running.exchange(true)) return;
    g_iHost = ToW(host); g_iPort = ToW(port);
    g_iSlot = ToW(slot); g_iPass = ToW(password);
    g_thread = std::thread(ThreadMain);
}

void ConfigUI_Stop() {
    if (!g_running.exchange(false)) return;
    if (g_wnd) PostMessageW(g_wnd, WM_DESTROY, 0, 0);
    if (g_thread.joinable()) g_thread.join();
}

void ConfigUI_Toggle() {
    if (g_wnd) PostMessageW(g_wnd, WM_TOGGLE, 0, 0);
}
