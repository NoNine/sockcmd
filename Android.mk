LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    demosockcmd-server.c

LOCAL_STATIC_LIBRARIES := \
    libsockcmd \
    liblog

LOCAL_MODULE:= demosockcmd-server
LOCAL_32_BIT_ONLY := true

LOCAL_CFLAGS :=

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/include

include $(BUILD_EXECUTABLE)


include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    demosockcmd-client.c

LOCAL_STATIC_LIBRARIES := \
    libsockcmd \
    liblog

LOCAL_MODULE:= demosockcmd-client
LOCAL_32_BIT_ONLY := true

LOCAL_CFLAGS :=

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/include

include $(BUILD_EXECUTABLE)

include $(call all-makefiles-under,$(LOCAL_PATH))
