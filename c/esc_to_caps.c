#include <stdio.h>
#include <windows.h>

#define ACTIVE (condition ? (TRUE) : (FALSE))
BOOL condition = TRUE;

LRESULT hook_proc(int code, WPARAM wParam, LPARAM lParam);
void send_input(int vk, int flags);
void notify();

int main() {
  HMODULE module = GetModuleHandleW(NULL);
  HHOOK hook = SetWindowsHookExW(WH_KEYBOARD_LL, hook_proc, module, 0);

  if (RegisterHotKey(NULL, 1, MOD_ALT | MOD_WIN, 0x43) <= 0) {
    printf("Failed to register hotkey");
    return EXIT_FAILURE;
  }

  MSG msg;
  while (GetMessageW(&msg, NULL, 0, 0) != 0) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);

    if (msg.message == WM_HOTKEY) {
      if (msg.wParam == 1) {
        condition = !condition;
        notify();
      }
    }
  }

  UnregisterHotKey(NULL, 1);

  return EXIT_SUCCESS;
}

void notify() {
  const wchar_t *title = L"esc_to_caps";
  if (condition) {
    const wchar_t *body = L"Capslock -> Esc\nEsc -> Capslock";
    MessageBoxW(NULL, body, title, MB_OK);
  } else {
    const wchar_t *body = L"Back to normal";
    MessageBoxW(NULL, body, title, MB_OK);
  }
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
