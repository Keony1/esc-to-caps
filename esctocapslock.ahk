toggle := 1 ; inicia CapsLock -> Esc 
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

#!c::
	toggle := !toggle
	if (toggle) {
		MsgBox, , , % "Esc -> CapsLock`nCapsLock -> Esc", 3s	
	} else {
		MsgBox, , , % "Esc -> Esc`nCapsLock -> CapsLock", 3s
	}
	
