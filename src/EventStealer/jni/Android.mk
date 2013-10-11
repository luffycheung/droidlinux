LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_LDLIBS := -llog
LOCAL_MODULE    := ReadEvent
LOCAL_SRC_FILES := ReadEvent.c
include $(BUILD_SHARED_LIBRARY)

