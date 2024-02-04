toggle := 1 ; start CapsLock -> Esc 
capsLockStatus := 0

$CapsLock::
	if (toggle) {
		SendInput, {Esc}
	} else {
		SendInput, {CapsLock}
		
		if (!capsLockStatus) {
			capsLockStatus := !toggleCapsLock
			SetCapsLockState, On
			
		} else {
			capsLockStatus := 0
			SetCapsLockState, Off
		}
	}
return

$Esc::
	if(toggle) {
		SendInput, {CapsLock}
		if (!capsLockStatus) {
			capsLockStatus := !toggleCapsLock
			SetCapsLockState, On
		} else {
			capsLockStatus := 0
			SetCapsLockState, Off
		}
	} else {
		SendInput, {Esc}
	}
return

; shift + capslock behaviour
+CapsLock::
  if(toggle) {
	SendInput, {Esc}
  } else {
		if (!capsLockStatus) {
			capsLockStatus := !toggleCapsLock
			SetCapsLockState, On
		} else {
			capsLockStatus := 0
			SetCapsLockState, Off
		}
  }
return

; win + shift + c
#!c::
	toggle := !toggle
	if (toggle) {
		MsgBox, , , % "Esc -> CapsLock`nCapsLock -> Esc", 3s	
	} else {
		MsgBox, , , % "Esc -> Esc`nCapsLock -> CapsLock", 3s
	}
return
