#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <input.h>
#include <android/log.h>
#include <jni.h>
#include "com_example_demo_native_event_JniHelper.h"
#include "uinput.h"
#define LOG_TAG "debug"
#define LOGI(fmt, args...) ;
//__android_log_print(ANDROID_LOG_INFO, LOG_TAG, fmt, ##args)



/* this macro is used to tell if "bit" is set in "array"
 * it selects a byte from the array, and does a boolean AND
 * operation with a byte that only has the relevant bit set.
 * eg. to check for the 12th bit, we do (array[1] & 1<<4)
 */
#define test_bit(bit, array)    (array[bit/8] & (1<<(bit%8)))

/* this macro computes the number of bytes needed to represent a bit array of the specified size */
#define sizeof_bit_array(bits)  ((bits + 7) / 8)

unsigned char m_keyBitmask[(KEY_MAX + 1) / 8];
unsigned char m_absBitmask[(ABS_MAX + 1) / 8];
unsigned char m_relBitmask[(REL_MAX + 1) / 8];
unsigned char m_swBitmask[(SW_MAX + 1) / 8];
unsigned char m_ledBitmask[(LED_MAX + 1) / 8];
unsigned char m_ffBitmask[(FF_MAX + 1) / 8];
unsigned char m_propBitmask[(INPUT_PROP_MAX + 1) / 8];

/**
 * 判断数组array中，从startIndex到endIndex是否有不为零的
 * 如果有不为零的就返回1
 */
int containsNonZeroByte(const uint8_t* array, uint32_t startIndex, uint32_t endIndex) {
	const uint8_t* end = array + endIndex;
	array += startIndex;
	while (array != end) {
		if (*(array++) != 0) {
			return 1;
		}
	}
	return 0;
}

int getAbsAxisUsage(int axis, int deviceClasses) {
	// Touch devices get dibs on touch-related axes.
	if (deviceClasses & INPUT_DEVICE_CLASS_TOUCH) {
		switch (axis) {
		case ABS_X:
		case ABS_Y:
		case ABS_PRESSURE:
		case ABS_TOOL_WIDTH:
		case ABS_DISTANCE:
		case ABS_TILT_X:
		case ABS_TILT_Y:
		case ABS_MT_SLOT:
		case ABS_MT_TOUCH_MAJOR:
		case ABS_MT_TOUCH_MINOR:
		case ABS_MT_WIDTH_MAJOR:
		case ABS_MT_WIDTH_MINOR:
		case ABS_MT_ORIENTATION:
		case ABS_MT_POSITION_X:
		case ABS_MT_POSITION_Y:
		case ABS_MT_TOOL_TYPE:
		case ABS_MT_BLOB_ID:
		case ABS_MT_TRACKING_ID:
		case ABS_MT_PRESSURE:
		case ABS_MT_DISTANCE:
			return INPUT_DEVICE_CLASS_TOUCH;
		}
	}

	// Joystick devices get the rest.
	return deviceClasses & INPUT_DEVICE_CLASS_JOYSTICK;
}

/**
 * 初始化获取手机input设备的信息
 * 返回格式为json格式
 *{infos:[{name:"/dev/input/event0",class:"1"},,{name:"/dev/input/event4",class:"9"}]}
 */
void identifyDevices(char *infos)
{
	char          buf[256] = { 0, };  /* RATS: Use ok */
	unsigned char mask[EV_MAX/8 + 1]; /* RATS: Use ok */

	int deviceClass;

	int           version;
	int           fd = 0;
	int           rc;
	int           i, j;
	char          *tmp;

	int haveKeyboardKeys;
	int haveGamepadButtons;
	int identifyId;

	sprintf(infos, "{infos:[");

	//scan all devices
	for (i = 0; i < 32; i++) {
		LOGI("<<<<<<<<<<<<<<<<<begin read event %d", i);
		char          name[64];           /* RATS: Use ok, but could be better */
		sprintf(name, "/dev/input/event%d", i);
		if ((fd = open(name, O_RDONLY, 0)) >= 0) {
			ioctl(fd, EVIOCGVERSION, &version);
			ioctl(fd, EVIOCGNAME(sizeof(buf)), buf);
			ioctl(fd, EVIOCGBIT(0, sizeof(mask)), mask);
			LOGI("%s\n", name);
			LOGI("    evdev version: %d.%d.%d\n",
					version >> 16, (version >> 8) & 0xff, version & 0xff);
			LOGI("    name: %s\n", buf);
			LOGI("    features:");

			char func[256];
			int typeCount = 0;
			deviceClass   = 0;
			for (j = 0; j < EV_MAX; j++) {
				if (test_bit(j, mask)) {
					const char *type = "unkowned";
					switch(j) {
					case EV_KEY: type = "keys/buttons"; break;
					case EV_REL:
					{
						type = "relative";
						deviceClass |= INPUT_DEVICE_CLASS_CURSOR;
						break;
					}
					case EV_ABS: type = "absolute";     break;
					case EV_MSC: type = "reserved";     break;
					case EV_LED: type = "leds";         break;
					case EV_SND: type = "sound";        break;
					case EV_REP: type = "repeat";       break;
					case EV_FF:  type = "feedback";     break;
					}
					LOGI(" %s", type);
					typeCount++;
					if (typeCount == 1)
						sprintf(func, "%s", type);
					else
						sprintf(func, "%s,%s", func, type);
				}
			}

			ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(m_keyBitmask)), m_keyBitmask);
			ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(m_absBitmask)), m_absBitmask);
			ioctl(fd, EVIOCGBIT(EV_REL, sizeof(m_relBitmask)), m_relBitmask);
			ioctl(fd, EVIOCGBIT(EV_SW, sizeof(m_swBitmask)), m_swBitmask);
			ioctl(fd, EVIOCGBIT(EV_LED, sizeof(m_ledBitmask)), m_ledBitmask);
			ioctl(fd, EVIOCGBIT(EV_FF, sizeof(m_ffBitmask)), m_ffBitmask);
			ioctl(fd, EVIOCGPROP(sizeof(m_propBitmask)), m_propBitmask);

			haveKeyboardKeys = containsNonZeroByte(m_keyBitmask, 0, sizeof_bit_array(BTN_MISC))
                        										|| containsNonZeroByte(m_keyBitmask, sizeof_bit_array(KEY_OK),
                        												sizeof_bit_array(KEY_MAX + 1));
			haveGamepadButtons = containsNonZeroByte(m_keyBitmask, sizeof_bit_array(BTN_MISC),
					sizeof_bit_array(BTN_MOUSE))
					|| containsNonZeroByte(m_keyBitmask, sizeof_bit_array(BTN_JOYSTICK),
							sizeof_bit_array(BTN_DIGI));
			if (haveKeyboardKeys || haveGamepadButtons) {
				deviceClass |= INPUT_DEVICE_CLASS_KEYBOARD;
				LOGI("INPUT_DEVICE_CLASS_KEYBOARD;");
			}


			// See if this is a cursor device such as a trackball or mouse.
			if (test_bit(BTN_MOUSE, m_keyBitmask)
					&& test_bit(REL_X, m_relBitmask)
					&& test_bit(REL_Y, m_relBitmask)) {
				deviceClass |= INPUT_DEVICE_CLASS_CURSOR;
				LOGI("INPUT_DEVICE_CLASS_CURSOR;");
			}

			// See if this is a touch pad.
			// Is this a new modern multi-touch driver?
			if (test_bit(ABS_MT_POSITION_X, m_absBitmask)
					&& test_bit(ABS_MT_POSITION_Y, m_absBitmask)) {
				// Some joysticks such as the PS3 controller report axes that conflict
				// with the ABS_MT range.  Try to confirm that the device really is
				// a touch screen.
				if (test_bit(BTN_TOUCH, m_keyBitmask) || !haveGamepadButtons) {
					deviceClass |= INPUT_DEVICE_CLASS_TOUCH | INPUT_DEVICE_CLASS_TOUCH_MT;
					LOGI("NPUT_DEVICE_CLASS_TOUCH | INPUT_DEVICE_CLASS_TOUCH_MT");
				}
				// Is this an old style single-touch driver?
			} else if (test_bit(BTN_TOUCH, m_keyBitmask)
					&& test_bit(ABS_X, m_absBitmask)
					&& test_bit(ABS_Y, m_absBitmask)) {
				deviceClass |= INPUT_DEVICE_CLASS_TOUCH;
				LOGI("INPUT_DEVICE_CLASS_TOUCH");
			}

			// See if this device is a joystick.
			// Assumes that joysticks always have gamepad buttons in order to distinguish them
			// from other devices such as accelerometers that also have absolute axes.
			if (haveGamepadButtons) {
				unsigned char assumedClasses = deviceClass | INPUT_DEVICE_CLASS_JOYSTICK;
				for (j = 0; j <= ABS_MAX; j++) {
					if (test_bit(j, m_absBitmask)
							&& (getAbsAxisUsage(j, assumedClasses) & INPUT_DEVICE_CLASS_JOYSTICK)) {
						deviceClass = assumedClasses;
						LOGI("INPUT_DEVICE_CLASS_JOYSTICK");
						break;
					}
				}
			}

			// Check whether this device has switches.

			for (j = 0; j <= SW_MAX; j++) {
				if (test_bit(j, m_swBitmask)) {
					deviceClass |= INPUT_DEVICE_CLASS_SWITCH;
					LOGI("INPUT_DEVICE_CLASS_SWITCH");
					break;
				}
			}

			// Check whether this device supports the vibrator.
			if (test_bit(FF_RUMBLE, m_ffBitmask)) {
				deviceClass |= INPUT_DEVICE_CLASS_VIBRATOR;
				LOGI("INPUT_DEVICE_CLASS_VIBRATOR");
			}

			//LOGI("name = %s, deviceClass = %d, func = %s", name, deviceClass, func);
			//saveDevicesInfo(name, deviceClass, func);

			LOGI("<<<<<<<<<<<<<<<<<<<<<end read %s \n \n", name);


			if (i) {
				sprintf(infos, "%s,", infos);
			}
			sprintf(infos, "%s{name:\"%s\",class:\"%d\"}", infos, name, deviceClass);
			close(fd);
		}
	}

	sprintf(infos, "%s]}", infos);
}

int isCodeInDevice(int code, const char * deviceName) {
	int           fd = 0;

	if ((fd = open(deviceName, O_RDONLY, 0)) >= 0) {

		ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(m_keyBitmask)), m_keyBitmask);
		ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(m_absBitmask)), m_absBitmask);
		ioctl(fd, EVIOCGBIT(EV_REL, sizeof(m_relBitmask)), m_relBitmask);
		ioctl(fd, EVIOCGBIT(EV_SW, sizeof(m_swBitmask)), m_swBitmask);
		ioctl(fd, EVIOCGBIT(EV_LED, sizeof(m_ledBitmask)), m_ledBitmask);
		ioctl(fd, EVIOCGBIT(EV_FF, sizeof(m_ffBitmask)), m_ffBitmask);
		ioctl(fd, EVIOCGPROP(sizeof(m_propBitmask)), m_propBitmask);

		if (test_bit(code, m_keyBitmask) || test_bit(code, m_absBitmask) ||
				test_bit(code, m_relBitmask) || test_bit(code, m_swBitmask) ||
				test_bit(code, m_ledBitmask) || test_bit(code, m_ffBitmask) ||
				test_bit(code, m_propBitmask)) {
			return 1;
		}
		close(fd);
	}

	return 0;
}

void getDeviceNameByClass(int class, char *deviceNames)
{
	char          buf[256] = { 0, };  /* RATS: Use ok */
	unsigned char mask[EV_MAX/8 + 1]; /* RATS: Use ok */

	int deviceClass;

	int           version;
	int           fd = 0;
	int           rc;
	int           i, j;
	char          *tmp;

	int haveKeyboardKeys;
	int haveGamepadButtons;
	int identifyId;

	sprintf(deviceNames, "{devices:[");

	int first = 1;

	//scan all devices
	for (i = 0; i < 32; i++) {
		char          name[64];           /* RATS: Use ok, but could be better */
		sprintf(name, "/dev/input/event%d", i);
		if ((fd = open(name, O_RDONLY, 0)) >= 0) {
			ioctl(fd, EVIOCGVERSION, &version);
			ioctl(fd, EVIOCGNAME(sizeof(buf)), buf);
			ioctl(fd, EVIOCGBIT(0, sizeof(mask)), mask);

			char func[256];
			int typeCount = 0;
			deviceClass   = 0;
			for (j = 0; j < EV_MAX; j++) {
				if (test_bit(j, mask)) {
					const char *type = "unkowned";
					switch(j) {
					case EV_KEY: type = "keys/buttons"; break;
					case EV_REL:
					{
						type = "relative";
						deviceClass |= INPUT_DEVICE_CLASS_CURSOR;
						LOGI("INPUT_DEVICE_CLASS_CURSOR");
						break;
					}
					case EV_ABS: type = "absolute";     break;
					case EV_MSC: type = "reserved";     break;
					case EV_LED: type = "leds";         break;
					case EV_SND: type = "sound";        break;
					case EV_REP: type = "repeat";       break;
					case EV_FF:  type = "feedback";     break;
					}
					typeCount++;
					if (typeCount == 1)
						sprintf(func, "%s", type);
					else
						sprintf(func, "%s,%s", func, type);
				}
			}

			ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(m_keyBitmask)), m_keyBitmask);
			ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(m_absBitmask)), m_absBitmask);
			ioctl(fd, EVIOCGBIT(EV_REL, sizeof(m_relBitmask)), m_relBitmask);
			ioctl(fd, EVIOCGBIT(EV_SW, sizeof(m_swBitmask)), m_swBitmask);
			ioctl(fd, EVIOCGBIT(EV_LED, sizeof(m_ledBitmask)), m_ledBitmask);
			ioctl(fd, EVIOCGBIT(EV_FF, sizeof(m_ffBitmask)), m_ffBitmask);
			ioctl(fd, EVIOCGPROP(sizeof(m_propBitmask)), m_propBitmask);

			haveKeyboardKeys = containsNonZeroByte(m_keyBitmask, 0, sizeof_bit_array(BTN_MISC))
                        										|| containsNonZeroByte(m_keyBitmask, sizeof_bit_array(KEY_OK),
                        												sizeof_bit_array(KEY_MAX + 1));
			haveGamepadButtons = containsNonZeroByte(m_keyBitmask, sizeof_bit_array(BTN_MISC),
					sizeof_bit_array(BTN_MOUSE))
					|| containsNonZeroByte(m_keyBitmask, sizeof_bit_array(BTN_JOYSTICK),
							sizeof_bit_array(BTN_DIGI));
			if (haveKeyboardKeys || haveGamepadButtons) {
				deviceClass |= INPUT_DEVICE_CLASS_KEYBOARD;
				LOGI("INPUT_DEVICE_CLASS_KEYBOARD;");
			}


			// See if this is a cursor device such as a trackball or mouse.
			if (test_bit(BTN_MOUSE, m_keyBitmask)
					&& test_bit(REL_X, m_relBitmask)
					&& test_bit(REL_Y, m_relBitmask)) {
				deviceClass |= INPUT_DEVICE_CLASS_CURSOR;
				LOGI("INPUT_DEVICE_CLASS_CURSOR;");
			}

			// See if this is a touch pad.
			// Is this a new modern multi-touch driver?
			if (test_bit(ABS_MT_POSITION_X, m_absBitmask)
					&& test_bit(ABS_MT_POSITION_Y, m_absBitmask)) {
				// Some joysticks such as the PS3 controller report axes that conflict
				// with the ABS_MT range.  Try to confirm that the device really is
				// a touch screen.
				if (test_bit(BTN_TOUCH, m_keyBitmask) || !haveGamepadButtons) {
					deviceClass |= INPUT_DEVICE_CLASS_TOUCH | INPUT_DEVICE_CLASS_TOUCH_MT;
					LOGI("NPUT_DEVICE_CLASS_TOUCH | INPUT_DEVICE_CLASS_TOUCH_MT");
				}
				// Is this an old style single-touch driver?
			} else if (test_bit(BTN_TOUCH, m_keyBitmask)
					&& test_bit(ABS_X, m_absBitmask)
					&& test_bit(ABS_Y, m_absBitmask)) {
				deviceClass |= INPUT_DEVICE_CLASS_TOUCH;
				LOGI("INPUT_DEVICE_CLASS_TOUCH");
			}

			// See if this device is a joystick.
			// Assumes that joysticks always have gamepad buttons in order to distinguish them
			// from other devices such as accelerometers that also have absolute axes.
			if (haveGamepadButtons) {
				unsigned char assumedClasses = deviceClass | INPUT_DEVICE_CLASS_JOYSTICK;
				for (j = 0; j <= ABS_MAX; j++) {
					if (test_bit(j, m_absBitmask)
							&& (getAbsAxisUsage(j, assumedClasses) & INPUT_DEVICE_CLASS_JOYSTICK)) {
						deviceClass = assumedClasses;
						LOGI("INPUT_DEVICE_CLASS_JOYSTICK");
						break;
					}
				}
			}

			// Check whether this device has switches.

			for (j = 0; j <= SW_MAX; j++) {
				if (test_bit(j, m_swBitmask)) {
					deviceClass |= INPUT_DEVICE_CLASS_SWITCH;
					LOGI("INPUT_DEVICE_CLASS_SWITCH");
					break;
				}
			}

			// Check whether this device supports the vibrator.
			if (test_bit(FF_RUMBLE, m_ffBitmask)) {
				deviceClass |= INPUT_DEVICE_CLASS_VIBRATOR;
				LOGI("INPUT_DEVICE_CLASS_VIBRATOR");
			}

			//LOGI("name = %s, deviceClass = %d, func = %s", name, deviceClass, func);
			//saveDevicesInfo(name, deviceClass, func);

			LOGI("<<<<<<<<<<<<<<<<<<<<<class  %d deviceClass %d and = %d \n \n", class, deviceClass, class & deviceClass);

			if (class & deviceClass) {
				if (!first) {
					sprintf(deviceNames, "%s,", deviceNames);
				}
				first = 0;
				sprintf(deviceNames, "%s{name:\"%s\"}", deviceNames, name);
			}


			close(fd);
		}
	}

	sprintf(deviceNames, "%s]}", deviceNames);
}

JNIEXPORT jstring JNICALL Java_com_syouquan_script_ScriptEngine_naitveInitDevice
  (JNIEnv *env, jobject thiz)
{
	initEnv(env);
	char infos[2048];
	identifyDevices(infos);
	jstring _deviceInfos = (*env)->NewStringUTF(env, infos);
	return _deviceInfos;
}

JNIEXPORT int JNICALL  Java_com_syouquan_script_ScriptEngine_nativeIsCodeInDevice
  (JNIEnv * env, jobject thiz, jint code, jstring device)
{
	LOGI("---call native method isCodeInDevice---");
	char *_device = (char*)(*env)->GetStringUTFChars(env, device, NULL);
	if (isCodeInDevice(code, _device)) {
		return 1;
	}
}

JNIEXPORT jstring JNICALL Java_com_syouquan_script_ScriptEngine_nativeGetDeviceName
  (JNIEnv *env, jclass thiz, jint deviceClass)
{
	char deviceNames[1024];
	getDeviceNameByClass(deviceClass, deviceNames);

	jstring _deviceNames = (*env)->NewStringUTF(env, deviceNames);
	return _deviceNames;
}
