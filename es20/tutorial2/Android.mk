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


LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

ifeq ($(AQROOT),)
$(warning Need vdk headers for this build)
$(error export AQROOT before build)
endif

LOCAL_SRC_FILES := \
	tutorial2.cpp

LOCAL_C_INCLUDES := \
	$(AQROOT)/sdk/inc

LOCAL_SHARED_LIBRARIES := \
    libVDK \
    libEGL \
    libGLESv2

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE      := tutorial2_es20
include $(BUILD_EXECUTABLE)

