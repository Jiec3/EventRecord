/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class com_example_demo_native_event_JniHelper */

#ifndef _Included_com_example_demo_native_event_JniHelper
#define _Included_com_example_demo_native_event_JniHelper
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com_example_demo_native_event_JniHelper
 * Method:    nativeEventRecord
 * Signature: ()V
 */
JNIEXPORT jstring JNICALL Java_com_syouquan_script_ScriptEngine_naitveInitDevice
  (JNIEnv *, jclass);

JNIEXPORT jstring JNICALL Java_com_syouquan_script_ScriptEngine_nativeGetDeviceName
  (JNIEnv *, jclass, jint);

JNIEXPORT jint JNICALL Java_com_syouquan_script_ScriptEngine_nativeIsCodeInDevice
  (JNIEnv *, jclass, jint, jstring);

JNIEXPORT void JNICALL Java_com_syouquan_script_ScriptEngine_nativeEventStartRecord
  (JNIEnv *, jclass);

JNIEXPORT void JNICALL Java_com_syouquan_script_ScriptEngine_nativeEventStopRecord
  (JNIEnv *, jclass);

JNIEXPORT void JNICALL Java_com_syouquan_script_ScriptEngine_nativeReplay
  (JNIEnv *, jclass);

JNIEXPORT void JNICALL Java_com_syouquan_script_ScriptEngine_nativeEventStopReplay
  (JNIEnv *, jclass);

#ifdef __cplusplus
}
#endif
#endif