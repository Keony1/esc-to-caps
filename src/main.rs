#![windows_subsystem = "windows"]

use windows::core::PCWSTR;
use windows::Win32::Foundation::{HWND, LPARAM, LRESULT, WPARAM};
use windows::Win32::UI::Input::KeyboardAndMouse::{
    RegisterHotKey, SendInput, UnregisterHotKey, INPUT, INPUT_0, INPUT_KEYBOARD, KEYBDINPUT,
    KEYBD_EVENT_FLAGS, KEYEVENTF_KEYUP, MOD_ALT, MOD_WIN, VIRTUAL_KEY, VK_CAPITAL, VK_ESCAPE,
};
use windows::Win32::UI::WindowsAndMessaging::{
    CallNextHookEx, DispatchMessageW, GetMessageW, MessageBoxW, SetWindowsHookExW,
    TranslateMessage, UnhookWindowsHookEx, HHOOK, KBDLLHOOKSTRUCT, MB_OK, MSG, WH_KEYBOARD_LL,
    WM_HOTKEY, WM_KEYDOWN, WM_KEYUP,
};

use windows::Win32::System::LibraryLoader::GetModuleHandleW;

static mut ACTIVE: bool = true;

const CBSIZE: i32 = std::mem::size_of::<INPUT>() as i32;

unsafe fn send_input(vk: VIRTUAL_KEY, flags: KEYBD_EVENT_FLAGS) {
    let input = INPUT {
        r#type: INPUT_KEYBOARD,
        Anonymous: INPUT_0 {
            ki: KEYBDINPUT {
                wVk: vk,
                wScan: 0,
                dwFlags: flags,
                time: 0,
                dwExtraInfo: 0,
            },
        },
    };
    SendInput(&[input], CBSIZE);
}

unsafe extern "system" fn keyboard_hook_proc(code: i32, wparam: WPARAM, lparam: LPARAM) -> LRESULT {
    if ACTIVE && code >= 0 {
        let kbd_struct = lparam.0 as *const KBDLLHOOKSTRUCT;
        let vk_code = (*kbd_struct).vkCode as u16;
        let vk_code = VIRTUAL_KEY(vk_code);

        match vk_code {
            VK_CAPITAL => {
                if wparam.0 as u32 == WM_KEYDOWN {
                    send_input(VK_ESCAPE, KEYBD_EVENT_FLAGS(0));
                } else if wparam.0 as u32 == WM_KEYUP {
                    send_input(VK_ESCAPE, KEYEVENTF_KEYUP);
                }
                return LRESULT(1);
            }
            VK_ESCAPE => {
                if wparam.0 as u32 == WM_KEYDOWN {
                    send_input(VK_CAPITAL, KEYBD_EVENT_FLAGS(0));
                } else if wparam.0 as u32 == WM_KEYUP {
                    send_input(VK_CAPITAL, KEYEVENTF_KEYUP);
                }
                return LRESULT(1);
            }
            _ => {}
        };
    }

    let null_hhook: HHOOK = HHOOK(std::ptr::null::<isize>() as isize);
    CallNextHookEx(null_hhook, code, wparam, lparam)
}

unsafe fn notification(hwnd: HWND) {
    let title: Vec<u16> = "esc_to_caps".encode_utf16().chain(Some(0)).collect();

    let body: Vec<u16> = match ACTIVE {
        true => "Capslock -> Esc\nEsc -> Capslock"
            .encode_utf16()
            .chain(Some(0))
            .collect(),
        false => "Capslock -> Capslock\nEsc -> Esc"
            .encode_utf16()
            .chain(Some(0))
            .collect(),
    };

    let body_ptr = body.first().unwrap();
    let title_ptr = title.first().unwrap();

    MessageBoxW(hwnd, PCWSTR(body_ptr), PCWSTR(title_ptr), MB_OK);
}

fn main() {
    unsafe {
        let module = GetModuleHandleW(PCWSTR(std::ptr::null::<u16>())).unwrap();
        let hook = SetWindowsHookExW(WH_KEYBOARD_LL, Some(keyboard_hook_proc), module, 0);

        if let Err(e) = hook {
            // criar um arquivo de log?
            panic!("Failed to install hook: {e}");
        }

        let letter_c = 0x43;
        let null_hwnd: HWND = HWND(std::ptr::null::<isize>() as isize);
        if let Err(e) = RegisterHotKey(null_hwnd, 1, MOD_ALT | MOD_WIN, letter_c) {
            println!("Failed to register hotkey: {e}");
        }

        let mut msg = MSG::default();
        loop {
            let null_hwnd: HWND = HWND(std::ptr::null::<isize>() as isize);
            if GetMessageW(&mut msg, null_hwnd, 0, 0).0 == 0 {
                break;
            }

            TranslateMessage(&msg);
            DispatchMessageW(&msg);

            match msg.message {
                WM_HOTKEY => {
                    let id = msg.wParam.0;

                    if id == 1 {
                        ACTIVE = !ACTIVE;
                        notification(null_hwnd)
                    }
                }
                _ => {}
            }
        }

        if let Err(e) = UnhookWindowsHookEx(hook.unwrap()) {
            println!("Failed to unhook: {e}")
        }

        if let Err(e) = UnregisterHotKey(null_hwnd, 1) {
            println!("Failed to unregister hotkey: {e}")
        }
    }
}
