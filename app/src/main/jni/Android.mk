LOCAL_PATH:= $(call my-dir)

INCLUDE_PATH:=D:\ffmpeg-android\armv7-a\include
FFMPEG_LIB_PATH:=D:\ffmpeg-android\armv7-a\lib

include $(CLEAR_VARS)
LOCAL_MODULE:= libavcodec
LOCAL_SRC_FILES:= $(FFMPEG_LIB_PATH)/libavcodec.so
LOCAL_EXPORT_C_INCLUDES := $(INCLUDE_PATH)
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE:= libavformat
LOCAL_SRC_FILES:= $(FFMPEG_LIB_PATH)/libavformat.so
LOCAL_EXPORT_C_INCLUDES := $(INCLUDE_PATH)
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE:= libswscale
LOCAL_SRC_FILES:= $(FFMPEG_LIB_PATH)/libswscale.so
LOCAL_EXPORT_C_INCLUDES := $(INCLUDE_PATH)
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE:= libavutil
LOCAL_SRC_FILES:= $(FFMPEG_LIB_PATH)/libavutil.so
LOCAL_EXPORT_C_INCLUDES := $(INCLUDE_PATH)
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE:= libavfilter
LOCAL_SRC_FILES:= $(FFMPEG_LIB_PATH)/libavfilter.so
LOCAL_EXPORT_C_INCLUDES := $(INCLUDE_PATH)
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE:= libpostproc
LOCAL_SRC_FILES:= $(FFMPEG_LIB_PATH)/libpostproc.so
LOCAL_EXPORT_C_INCLUDES := $(INCLUDE_PATH)
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE:= libswresample
LOCAL_SRC_FILES:= $(FFMPEG_LIB_PATH)/libswresample.so
LOCAL_EXPORT_C_INCLUDES := $(INCLUDE_PATH)
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE:= libavdevice
LOCAL_SRC_FILES:= $(FFMPEG_LIB_PATH)/libavdevice.so
LOCAL_EXPORT_C_INCLUDES := $(INCLUDE_PATH)
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := ffmpeg
LOCAL_SRC_FILES := com_zhougang_camera2encode_H264Encoder.cpp \
                   cmdutils.c \
                   ffmpeg_filter.c \
                   ffmpeg_opt.c \
                   ffmpeg_hw.c \
                   ffmpeg.c
LOCAL_C_INCLUDES := D:\ffmpeg-4.0.2
LOCAL_LDLIBS := -lm -llog
LOCAL_SHARED_LIBRARIES := libavcodec libavfilter libavformat libavutil libswresample libpostproc libswscale libavdevice
include $(BUILD_SHARED_LIBRARY)