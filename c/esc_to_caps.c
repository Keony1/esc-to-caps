#include <stdio.h>
#include <string.h>
#include <windows.h>

#define ACTIVE (condition ? (TRUE) : (FALSE))
#define WM_APPMSG (WM_USER + 1)
BOOL condition = TRUE;
NOTIFYICONDATA nid;

LRESULT hook_proc(int code, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
HWND create_window_hwnd(HINSTANCE hInstance);

void send_input(int vk, int flags);
void update_icon();

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow) {
  HMODULE module = GetModuleHandleW(NULL);
  HHOOK hook = SetWindowsHookExW(WH_KEYBOARD_LL, hook_proc, module, 0);

  if (RegisterHotKey(NULL, 1, MOD_ALT | MOD_WIN, 0x43) <= 0) { // 0x43 = c
    printf("Failed to register hotkey");
    return EXIT_FAILURE;
  }

  HWND hWnd = create_window_hwnd(hInstance);

  if (!hWnd) {
    MessageBox(NULL, TEXT("Window Creation Failed!"), TEXT("Error"),
               MB_ICONERROR);
    return 0;
  }

  nid.cbSize = sizeof(NOTIFYICONDATA);
  nid.hWnd = hWnd;
  nid.hIcon = LoadIcon(NULL, IDI_ASTERISK), nid.uID = 1;
  nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_INFO;
  nid.uVersion = NOTIFYICON_VERSION;
  nid.uCallbackMessage = WM_APPMSG;
  strcpy(nid.szTip, "esc_to_caps (ON)");

  Shell_NotifyIcon(NIM_ADD, &nid);
  Shell_NotifyIcon(NIM_SETVERSION, &nid);

  MSG msg;
  while (GetMessageW(&msg, NULL, 0, 0) != 0) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);

    if (msg.message == WM_HOTKEY) {
      if (msg.wParam == 1) {
        update_icon();
      }
    }
  }

  UnregisterHotKey(NULL, 1);

  return EXIT_SUCCESS;
}

void update_icon() {
  condition = !condition;
  ACTIVE ? strcpy(nid.szTip, "esc_to_caps (ON)")
         : strcpy(nid.szTip, "esc_to_caps (OFF)");
  nid.hIcon = ACTIVE ? LoadIcon(NULL, IDI_ASTERISK) : LoadIcon(NULL, IDI_HAND);
  Shell_NotifyIcon(NIM_MODIFY, &nid);
}

void send_input(int vk, int flags) {
  INPUT input = {.type = INPUT_KEYBOARD,
                 .ki = {.wVk = vk,
                        .wScan = MapVirtualKeyA(vk, MAPVK_VK_TO_VSC),
                        .dwFlags = flags,
                        .time = 0,
                        .dwExtraInfo = 0}};
  SendInput(1, (INPUT[]){input}, sizeof(input));
}

LRESULT hook_proc(int code, WPARAM wParam, LPARAM lParam) {
  if (ACTIVE && code >= 0x00) {
    KBDLLHOOKSTRUCT *key_pressed = (KBDLLHOOKSTRUCT *)lParam;
    if (key_pressed->vkCode == VK_CAPITAL) {
      if (wParam == WM_KEYDOWN) {
        send_input(VK_ESCAPE, 0);
      } else {
        send_input(VK_ESCAPE, KEYEVENTF_KEYUP);
      }
      return 1;

    } else if (key_pressed->vkCode == VK_ESCAPE) {
      if (wParam == WM_KEYDOWN) {
        send_input(VK_CAPITAL, 0);
      } else {
        send_input(VK_CAPITAL, KEYEVENTF_KEYUP);
      }
      return 1;
    }
  }

  CallNextHookEx(NULL, code, wParam, lParam);
}

HWND create_window_hwnd(HINSTANCE hInstance) {
  LPCSTR class_name = "EscToCapsClass";
  WNDCLASS wc = {0};
  wc.lpfnWndProc = WndProc;
  wc.hInstance = hInstance;
  wc.lpszClassName = class_name;

  if (!RegisterClass(&wc)) {
    MessageBox(NULL, TEXT("Window Registration Failed!"), TEXT("Error"),
               MB_ICONERROR);
    return 0;
  }

  HWND hWnd = CreateWindowEx(0, class_name, TEXT(""), 0, 0, 0, 0, 0,
                             HWND_MESSAGE, NULL, hInstance, NULL);
  return hWnd;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam,
                         LPARAM lParam) {
  switch (message) {
  case WM_DESTROY:
    PostQuitMessage(0);
    break;
  case WM_APPMSG:
    switch (lParam) {
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MOUSEWHEEL:
      update_icon();
      break;
    }
    break;
  default:
    break;
  }

  return DefWindowProc(hWnd, message, wParam, lParam);
}
