LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_LDLIBS += -L$(SYSROOT)/usr/lib -llog
LOCAL_MODULE    := event

LOCAL_SRC_FILES := record.c \
				   JNIUtils.c \
				   identifyDevices.c 

include $(BUILD_SHARED_LIBRARY)

