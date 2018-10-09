LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
prebuilt_stdcxx_PATH := prebuilts/ndk/current/sources/cxx-stl
LOCAL_SRC_FILES := \
	CRC.cpp \
	MD5Checksum.cpp \
	RKBoot.cpp \
	RKImage.cpp \
	RKLog.cpp \
	RKComm.cpp \
	RKDevice.cpp \
	RKAndroidDevice.cpp \
	Upgrade.cpp \
	RKSparse.cpp \
	../roots.cpp \
	mounts.cpp

LOCAL_CFLAGS := -Wno-error

LOCAL_C_INCLUDES := \
	bootable/recovery \
    external/e2fsprogs/lib \
	system/extras/ext4_utils/include/ext4_utils \
	system/core/base/include \
	system/vold

LOCAL_STATIC_LIBRARIES := \
	libext4_utils \
	libcutils \
	libmtdutils \
	libselinux \
	libfs_mgr \
	libext2_uuid \

LOCAL_MODULE := librkupdate

LOCAL_CPPFLAGS += -Wall -D_FILE_OFFSET_BITS=64 -D_LARGE_FILE -fexceptions

include $(BUILD_STATIC_LIBRARY)
