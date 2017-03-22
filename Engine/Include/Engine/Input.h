#ifndef ENGINE_INPUT_H
#define ENGINE_INPUT_H

/*
 * GLFW key codes, mouse button and joystick buttons put into enums.
 */

/*************************************************************************
 * GLFW 3.2 - www.glfw.org
 * A library for OpenGL, window and input
 *------------------------------------------------------------------------
 * Copyright (c) 2002-2006 Marcus Geelnard
 * Copyright (c) 2006-2016 Camilla Berglund <elmindreda@glfw.org>
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would
 *    be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not
 *    be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 *    distribution.
 *
 *************************************************************************/

namespace Engine {

	/*
	 * Key or button action
	 */
	enum class KeyAction : int {
		Release = 0,
		Press = 1,
		Repeat = 2
	};

	enum class Key : int {
		/* The unknown key */
		Unknown = -1,

		/* Printable keys */
		Space = 32,
		Apostrophe = 39, /* ' */
		Comma = 44, /* , */
		Minus = 45, /* - */
		Period = 46, /* . */
		Slash = 47, /* / */
		Number0 = 48,
		Number1 = 49,
		Number2 = 50,
		Number3 = 51,
		Number4 = 52,
		Number5 = 53,
		Number6 = 54,
		Number7 = 55,
		Number8 = 56,
		Number9 = 57,
		Semicolon = 59, /* ; */
		Equal = 61, /* = */
		A = 65,
		B = 66,
		C = 67,
		D = 68,
		E = 69,
		F = 70,
		G = 71,
		H = 72,
		I = 73,
		J = 74,
		K = 75,
		L = 76,
		M = 77,
		N = 78,
		O = 79,
		P = 80,
		Q = 81,
		R = 82,
		S = 83,
		T = 84,
		U = 85,
		V = 86,
		W = 87,
		X = 88,
		Y = 89,
		Z = 90,
		LeftBracket = 91, /* [ */
		Backslash = 92, /* \ */
		RightBracket = 93, /* ] */
		GraveAccent = 96, /* ` */
		World1 = 161, /* non-US #1 */
		World2 = 162, /* non-US #2 */

		/* Function keys */
		Escape = 256,
		Enter = 257,
		Tab = 258,
		Backspace = 259,
		Insert = 260,
		Delete = 261,
		Right = 262,
		Left = 263,
		Down = 264,
		Up = 265,
		PageUp = 266,
		PageDown = 267,
		Home = 268,
		End = 269,
		CapsLock = 280,
		ScrollLock = 281,
		NumLock = 282,
		PrintScreen = 283,
		Pause = 284,
		F1 = 290,
		F2 = 291,
		F3 = 292,
		F4 = 293,
		F5 = 294,
		F6 = 295,
		F7 = 296,
		F8 = 297,
		F9 = 298,
		F10 = 299,
		F11 = 300,
		F12 = 301,
		F13 = 302,
		F14 = 303,
		F15 = 304,
		F16 = 305,
		F17 = 306,
		F18 = 307,
		F19 = 308,
		F20 = 309,
		F21 = 310,
		F22 = 311,
		F23 = 312,
		F24 = 313,
		F25 = 314,
		Numpad0 = 320,
		Numpad1 = 321,
		Numpad2 = 322,
		Numpad3 = 323,
		Numpad4 = 324,
		Numpad5 = 325,
		Numpad6 = 326,
		Numpad7 = 327,
		Numpad8 = 328,
		Numpad9 = 329,
		NumpadDecimal = 330,
		NumpadDivide = 331,
		NumpadMultiply = 332,
		NumpadSubtract = 333,
		NumpadAdd = 334,
		NumpadEnter = 335,
		NumpadEqual = 336,
		LeftShift = 340,
		LeftControl = 341,
		LeftAlt = 342,
		LeftSuper = 343,
		RightShift = 344,
		RightControl = 345,
		RightAlt = 346,
		RightSuper = 347,
		Menu = 348,
		Last = Menu
	};

	/*
	 * Modifier key flags
	 */
	enum class Modifier : int {
		None = 0x0000,
		Shift = 0x0001,
		Control = 0x0002,
		Alt = 0x0004,
		Super = 0x0008
	};

	Modifier operator|(Modifier a, Modifier b);
	Modifier operator&(Modifier a, Modifier b);
	Modifier operator^(Modifier a, Modifier b);
	Modifier operator~(Modifier a);
	Modifier& operator|=(Modifier& a, Modifier b);
	Modifier& operator&=(Modifier& a, Modifier b);
	Modifier& operator^=(Modifier& a, Modifier b);

	/*
	 * Mouse buttons
	 */
	enum class MouseButton : int {
		Button1 = 0,
		Button2 = 1,
		Button3 = 2,
		Button4 = 3,
		Button5 = 4,
		Button6 = 5,
		Button7 = 6,
		Button8 = 7,
		Last = Button8,
		Left = Button1,
		Right = Button2,
		Middle = Button3
	};

	/*
	 * Joysticks
	 */
	enum class Joystick : int {
		Joystick1 = 0,
		Joystick2 = 1,
		Joystick3 = 2,
		Joystick4 = 3,
		Joystick5 = 4,
		Joystick6 = 5,
		Joystick7 = 6,
		Joystick8 = 7,
		Joystick9 = 8,
		Joystick10 = 9,
		Joystick11 = 10,
		Joystick12 = 11,
		Joystick13 = 12,
		Joystick14 = 13,
		Joystick15 = 14,
		Joystick16 = 15,
		Last = Joystick16,
	};
}

#endif
