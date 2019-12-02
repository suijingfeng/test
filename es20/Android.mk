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
TOP_LOCAL_PATH := $(LOCAL_PATH)

USER_INSTALL_PATH  := true

ifeq ($(USER_INSTALL_PATH),true)
LOCAL_INSTALL_PATH := $(TOP_LOCAL_PATH)/install/apk
endif

include $(TOP_LOCAL_PATH)/samples/sample1/android/Android.mk
include $(TOP_LOCAL_PATH)/samples/sample2/android/Android.mk
include $(TOP_LOCAL_PATH)/samples/sample3/android/Android.mk
include $(TOP_LOCAL_PATH)/samples/sample4/android/Android.mk
include $(TOP_LOCAL_PATH)/samples/sample5/android/Android.mk
include $(TOP_LOCAL_PATH)/samples/sample6/android/Android.mk
include $(TOP_LOCAL_PATH)/samples/sample7/android/Android.mk
include $(TOP_LOCAL_PATH)/samples/sample8/android/Android.mk
include $(TOP_LOCAL_PATH)/samples/sample9/android/Android.mk
include $(TOP_LOCAL_PATH)/samples/sample10/android/Android.mk
include $(TOP_LOCAL_PATH)/tutorial1/android/Android.mk
include $(TOP_LOCAL_PATH)/tutorial2/android/Android.mk
include $(TOP_LOCAL_PATH)/tutorial3/android/Android.mk
include $(TOP_LOCAL_PATH)/tutorial4/android/Android.mk
include $(TOP_LOCAL_PATH)/tutorial5/android/Android.mk
#Dont support to run tutorial6 on Android whith JNI
#include $(TOP_LOCAL_PATH)/tutorial6/android/Android.mk
include $(TOP_LOCAL_PATH)/tutorial7/android/Android.mk
