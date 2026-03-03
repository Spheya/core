#pragma once

#include <string>

#include "platform.hpp"

enum class InputButton {
	None = 0x0,

	KeyEscape = 0x001,
	KeyBackspace = 0x00e,
	KeyTab = 0x00f,

	KeyEnter = 0x01c,
	KeyLeftControl = 0x01d,
	KeyLeftShift = 0x02a,
	KeyRightShift = 0x036,
	KeyLeftAlt = 0x038,
	KeySpace = 0x039,

	Key1 = 0x002,
	Key2 = 0x003,
	Key3 = 0x004,
	Key4 = 0x005,
	Key5 = 0x006,
	Key6 = 0x007,
	Key7 = 0x008,
	Key8 = 0x009,
	Key9 = 0x00a,
	Key0 = 0x00b,

	KeyQ = 0x010,
	KeyW = 0x011,
	KeyE = 0x012,
	KeyR = 0x013,
	KeyT = 0x014,
	KeyY = 0x015,
	KeyU = 0x016,
	KeyI = 0x017,
	KeyO = 0x018,
	KeyP = 0x019,
	KeyA = 0x01e,
	KeyS = 0x01f,
	KeyD = 0x020,
	KeyF = 0x021,
	KeyG = 0x022,
	KeyH = 0x023,
	KeyJ = 0x024,
	KeyK = 0x025,
	KeyL = 0x026,
	KeyZ = 0x02c,
	KeyX = 0x02d,
	KeyC = 0x02e,
	KeyV = 0x02f,
	KeyB = 0x030,
	KeyN = 0x031,
	KeyM = 0x032,

	KeyUp = 0x148,
	KeyDown = 0x150,
	KeyLeft = 0x14b,
	KeyRight = 0x14d,

	MouseButtonLeft = 0x201,
	MouseButtonRight = 0x202,
	MouseButtonMiddle = 0x203
};

// Returns a user friendly name for the button
// to get an id corresponding with the functionality (for sprite lookup), convert the button to a virtual key code
inline std::string getButtonName(InputButton button) {
	switch(button) {
	case InputButton::MouseButtonLeft: return "Left Mouse Button";
	case InputButton::MouseButtonRight: return "Right Mouse Button";
	case InputButton::MouseButtonMiddle: return "Middle Mouse Button";
	default: {
		unsigned scanCode = unsigned(button);
		long lParam = scanCode << 16;
		wchar_t buffer[128];
		GetKeyNameText(lParam, buffer, sizeof(buffer));
		return wcharPtrToStr(buffer);
	}
	}
}
