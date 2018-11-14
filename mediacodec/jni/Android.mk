LOCAL_PATH := $(call my-dir)

# Program
include $(CLEAR_VARS)
LOCAL_MODULE := mediacodec
LOCAL_SRC_FILES := ../MediaCodec.cpp

LOCAL_LDLIBS := -llog -lz
include $(BUILD_STATIC_LIBRARY)

