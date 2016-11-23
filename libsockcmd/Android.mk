LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    sockcmd-client.c \
    sockcmd-server.c

LOCAL_STATIC_LIBRARIES := \
    libcutils \
    liblog

LOCAL_MODULE:= libsockcmd
LOCAL_32_BIT_ONLY := true

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/../include

include $(BUILD_STATIC_LIBRARY)
