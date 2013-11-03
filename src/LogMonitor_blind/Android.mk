LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS) 
# give module name
LOCAL_MODULE    := hello_world  
# list your C files to compile
LOCAL_SRC_FILES := LogMonitor.c
include $(BUILD_EXECUTABLE)
