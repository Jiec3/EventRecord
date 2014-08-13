#ifndef U_INPUT_H__
#define U_INPUT_H__
/* Android munges Linux headers to avoid copyright issues, but doesn't munge linux/uinput.h,
 * so constants reproduced here.
*/
#define UI_SET_EVBIT   0x40045564
#define UI_SET_KEYBIT  0x40045565
#define UI_SET_RELBIT  0x40045566
#define UI_SET_ABSBIT  0x40045567

/*
 * Input device classes.
 */
enum {
	/* The input device is a keyboard or has buttons. */
	INPUT_DEVICE_CLASS_KEYBOARD      = 0x00000001,

	/* The input device is an alpha-numeric keyboard (not just a dial pad). */
	INPUT_DEVICE_CLASS_ALPHAKEY      = 0x00000002,

	/* The input device is a touchscreen or a touchpad (either single-touch or multi-touch). */
	INPUT_DEVICE_CLASS_TOUCH         = 0x00000004,

	/* The input device is a cursor device such as a trackball or mouse. */
	INPUT_DEVICE_CLASS_CURSOR        = 0x00000008,

	/* The input device is a multi-touch touchscreen. */
	INPUT_DEVICE_CLASS_TOUCH_MT      = 0x00000010,

	/* The input device is a directional pad (implies keyboard, has DPAD keys). */
	INPUT_DEVICE_CLASS_DPAD          = 0x00000020,

	/* The input device is a gamepad (implies keyboard, has BUTTON keys). */
	INPUT_DEVICE_CLASS_GAMEPAD       = 0x00000040,

	/* The input device has switches. */
	INPUT_DEVICE_CLASS_SWITCH        = 0x00000080,

	/* The input device is a joystick (implies gamepad, has joystick absolute axes). */
	INPUT_DEVICE_CLASS_JOYSTICK      = 0x00000100,

	/* The input device has a vibrator (supports FF_RUMBLE). */
	INPUT_DEVICE_CLASS_VIBRATOR      = 0x00000200,

	/* The input device is virtual (not a real device, not part of UI configuration). */
	INPUT_DEVICE_CLASS_VIRTUAL       = 0x40000000,

	/* The input device is external (not built-in). */
	INPUT_DEVICE_CLASS_EXTERNAL      = 0x80000000,
};

#endif
