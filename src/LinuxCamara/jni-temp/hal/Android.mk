

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)


LOCAL_C_INCLUDES += $(LOCAL_PATH)/../include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../libs3cjpeg


LOCAL_SRC_FILES:= \
	SecCamera.cpp \
	SecCameraHWInterface.cpp


LOCAL_SHARED_LIBRARIES:= libutils libui liblog libbinder libcutils
LOCAL_SHARED_LIBRARIES+= libs3cjpeg
LOCAL_SHARED_LIBRARIES+= libcamera_client



LOCAL_MODULE:= libcamera

LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

