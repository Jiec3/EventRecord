#include <stdio.h>
#include <string.h>
#include <jni.h>
#include <android/log.h>
#define LOG_TAG "debug"
#define LOGI(fmt, args...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, fmt, ##args)

JNIEnv *env;

void initEnv(JNIEnv *_env) {
	env = _env;
}

const char * getEventPath() {
	const char * channelIdStr = "/mnt/sdcard/events";

	jmethodID mid;
	jclass	_cls = (*env)->FindClass(env, "com/example/demo/JniHelper");
	mid = (*env)->GetMethodID(env, _cls, "<init>", "()V");
	jobject _object = (*env)->NewObject(env, _cls, mid);
	mid = (*env)->GetMethodID(env, _cls, "getEventPath",
			"()Ljava/lang/String;");
	jstring msg = (*env)->CallObjectMethod(env, _object, mid);
	channelIdStr = (*env)->GetStringUTFChars(env, msg, 0);

	(*env)->DeleteLocalRef(env, msg);

	return channelIdStr;
}

void runShell(const char *command) {
	jmethodID mid;
	jclass	_cls = (*env)->FindClass(env, "com/example/demo/JniHelper");
	mid = (*env)->GetMethodID(env, _cls, "<init>", "()V");
	jobject _object = (*env)->NewObject(env, _cls, mid);
	mid = (*env)->GetMethodID(env, _cls, "runShell",
			"(Ljava/lang/String;)V");
	jstring _command = (*env)->NewStringUTF(env, command);
	(*env)->CallVoidMethod(env, _object, mid, _command);

	(*env)->DeleteLocalRef(env, _command);
}

void saveDevicesInfo(const char *deviceName, int deviceClass, const char *func) {
	jmethodID mid;
	jclass	_cls = (*env)->FindClass(env, "com/example/demo/JniHelper");
	mid = (*env)->GetMethodID(env, _cls, "<init>", "()V");
	jobject _object = (*env)->NewObject(env, _cls, mid);
	mid = (*env)->GetMethodID(env, _cls, "saveDevicesInfo",
			"(Ljava/lang/String;Ljava/lang/String;I)V");
	jstring _deveceName = (*env)->NewStringUTF(env, deviceName);
	jstring _func = (*env)->NewStringUTF(env, func);
	(*env)->CallVoidMethod(env, _object, mid, _deveceName, _func, deviceClass);

	(*env)->DeleteLocalRef(env, _deveceName);
	(*env)->DeleteLocalRef(env, _func);
}

/**
 * jni 调用java的方法，将读取到的event发送到java层上面
 */
void saveEventItem(const char *deviceName, long secTime, long msTime, int type, int code, int value) {
	jmethodID mid;
	jclass	_cls = (*env)->FindClass(env, "com/syouquan/script/ScriptEngine");
	mid = (*env)->GetMethodID(env, _cls, "<init>", "()V");
	jobject _object = (*env)->NewObject(env, _cls, mid);
	mid = (*env)->GetMethodID(env, _cls, "saveEventItem",
			"(Ljava/lang/String;IIIII)V");
	jstring _deveceName = (*env)->NewStringUTF(env, deviceName);

	(*env)->CallVoidMethod(env, _object, mid, _deveceName, secTime, msTime, type, code, value);

	//delete all the res
	(*env)->DeleteLocalRef(env, _deveceName);
	(*env)->DeleteLocalRef(env, _object);
	(*env)->DeleteLocalRef(env, _cls);
}

