##############################################################################
#
#    Copyright 2012 - 2013 Vivante Corporation, Sunnyvale, California.
#    All Rights Reserved.
#
#    Permission is hereby granted, free of charge, to any person obtaining
#    a copy of this software and associated documentation files (the
#    'Software'), to deal in the Software without restriction, including
#    without limitation the rights to use, copy, modify, merge, publish,
#    distribute, sub license, and/or sell copies of the Software, and to
#    permit persons to whom the Software is furnished to do so, subject
#    to the following conditions:
#
#    The above copyright notice and this permission notice (including the
#    next paragraph) shall be included in all copies or substantial
#    portions of the Software.
#
#    THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
#    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
#    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
#    IN NO EVENT SHALL VIVANTE AND/OR ITS SUPPLIERS BE LIABLE FOR ANY
#    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
#    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
#    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
##############################################################################


#########################################################################
# OpenGL ES JNI sample
# This makefile builds both an activity and a shared library.
#########################################################################
ifneq ($(TARGET_SIMULATOR),true) # not 64 bit clean

#TOP_LOCAL_PATH:= $(call my-dir)

# Build activity

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-subdir-java-files)

LOCAL_PACKAGE_NAME := GL2Tutorial1

LOCAL_JNI_SHARED_LIBRARIES := libgl2tutorial1_jni

ifneq ($(USER_INSTALL_PATH),true)
LOCAL_MODULE_PATH := $(LOCAL_PATH)/bin
else
LOCAL_MODULE_PATH := $(LOCAL_INSTALL_PATH)
endif #

include $(BUILD_PACKAGE)

#########################################################################
# Build JNI Shared Library
#########################################################################

LOCAL_PATH:= $(LOCAL_PATH)/jni

include $(CLEAR_VARS)

# Optional tag would mean it doesn't get installed by default
LOCAL_MODULE_TAGS := optional

LOCAL_CFLAGS := -Werror -DANDROID_SDK_VERSION=$(PLATFORM_SDK_VERSION) -DANDROID_JNI

LOCAL_SRC_FILES:= tutorial1.cpp \
					../../tutorial1.cpp

LOCAL_SHARED_LIBRARIES := \
	libutils \
	libEGL \
	libGLESv2

LOCAL_MODULE := libgl2tutorial1_jni

LOCAL_PRELINK_MODULE := false

LOCAL_MODULE_PATH := $(LOCAL_PATH)/../libs/armeabi

include $(BUILD_SHARED_LIBRARY)

endif # TARGET_SIMULATOR
