LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS) 
# give module name
LOCAL_MODULE    := pl
# list your C files to compile
LOCAL_SRC_FILES := PortableLogger.c
include $(BUILD_EXECUTABLE)
