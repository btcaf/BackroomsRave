﻿#ifndef UNICODE
#define UNICODE
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <mmsystem.h>

#pragma comment(lib, "winmm.lib")

#include "ApplicationD3D.h"
#include "SoundWrapper.h"
inline constinit TCHAR const APP_NAME[] = TEXT(
    "Backrooms Rave"
);

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);

namespace {

    constexpr TCHAR CLASS_NAME[] = TEXT("MainWindowClass");

    /*
     * Registers a window class.
     */
    BOOL register_window_class(HINSTANCE instance, WNDCLASSEX& wc) {
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        wc.lpfnWndProc = WindowProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = instance;
        wc.hIcon = nullptr;
        wc.hCursor = static_cast<HCURSOR>(
            LoadImage(
                nullptr, IDC_ARROW, IMAGE_CURSOR, LR_DEFAULTSIZE,
                LR_DEFAULTSIZE, LR_DEFAULTSIZE | LR_SHARED)
            );
        wc.hbrBackground = nullptr;
        wc.lpszMenuName = nullptr;
        wc.lpszClassName = CLASS_NAME;
        wc.hIconSm = nullptr;

        if (!RegisterClassEx(&wc))
            return FALSE;
        return TRUE;
    }

    /*
     * Creates the main window of the application.
     */
    BOOL create_window(HINSTANCE instance, HWND& hwnd) {
        RECT desktop;
        GetClientRect(GetDesktopWindow(), &desktop);

        hwnd = CreateWindowEx(
            0, CLASS_NAME, APP_NAME,
            WS_POPUP,
            0, 0,
            desktop.right - desktop.left,
            desktop.bottom - desktop.top,
            nullptr, nullptr, instance, nullptr
        );

        if (hwnd == nullptr)
            return FALSE;
        return TRUE;
    }

    /*
    * Handles the application's message loop.
    * Returns `wParam` value of the last message, i.e. `WM_QUIT`,
    * or 1 if message error occured.
    */
    INT message_loop() {
        MSG msg = { };
        BOOL msg_result;
        while ((msg_result = GetMessage(&msg, nullptr, 0, 0)) > 0) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        return msg_result == 0 ? static_cast<INT>(msg.wParam) : 1;
    }

} /* anonymous namespace */

/*
 * The main function of the application.
 */
int WINAPI wWinMain(
    _In_     HINSTANCE instance,
    _In_opt_ [[maybe_unused]] HINSTANCE prev_instance,
    _In_ [[maybe_unused]] PWSTR cmd_line,
    _In_     INT cmd_show
) {
    WNDCLASSEX wc;
    HWND hwnd;

    if (!register_window_class(instance, wc))
        return 1;
    if (!create_window(instance, hwnd))
        return 1;

    ShowWindow(hwnd, cmd_show);
    UpdateWindow(hwnd);

    // hide cursor
    while (ShowCursor(FALSE) >= 0);

    // play music
    SoundWrapper sound(TEXT("assets/caramelldansen.wav"));

    return message_loop();
}

/*
 * The main window procedure
 */
LRESULT CALLBACK WindowProc(
    HWND hwnd, UINT msg, WPARAM w_param, LPARAM l_param
) {
    switch (msg) {
    case WM_CREATE:
        InitDirect3D(hwnd);
        InitTimer(hwnd);
        return 0;
    case WM_TIMER:
        OnTimer();
        InvalidateRect(hwnd, nullptr, FALSE);
        return 0;
    case WM_PAINT:
        OnPaint();
        ValidateRect(hwnd, nullptr);
        return 0;
    case WM_DESTROY:
        ReleaseTimer(hwnd);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, w_param, l_param);
}