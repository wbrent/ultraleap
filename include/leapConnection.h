/* Copyright (C) 2012-2017 Ultraleap Limited. All rights reserved.
 *
 * Use of this code is subject to the terms of the Ultraleap SDK agreement
 * available at https://central.leapmotion.com/agreements/SdkAgreement unless
 * Ultraleap has signed a separate license agreement with you or your
 * organisation.
 *
 */


#ifndef LEAP_CONNECTION_H
#define LEAP_CONNECTION_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#if defined(_MSC_VER)
    #include <Windows.h>
    #include <process.h>
    #define LockMutex EnterCriticalSection
    #define UnlockMutex LeaveCriticalSection
#else
    #include <unistd.h>
    #include <pthread.h>
    #define LockMutex pthread_mutex_lock
    #define UnlockMutex pthread_mutex_unlock
#endif

#include "LeapC.h"

// client functions
LEAP_CONNECTION* LeapC_OpenConnection (void);
void LeapC_CloseConnection (void);
void LeapC_DestroyConnection (void);
LEAP_TRACKING_EVENT* LeapC_GetFrame (void);
LEAP_DEVICE_INFO* LeapC_GetDeviceProperties (void);
const char* LeapC_SetTrackingMode (eLeapTrackingMode m);
const char* LeapC_GetTrackingMode (void);
const char* LeapC_eLeapRSResultString (eLeapRS r);
const char* LeapC_eLeapTrackingModeString (eLeapTrackingMode m);

extern bool LeapC_IsConnected;

// callback function pointers
typedef void (*connection_callback)     (void);
typedef void (*device_callback)         (const LEAP_DEVICE_INFO* device);
typedef void (*device_lost_callback)    (void);
typedef void (*device_failure_callback) (const eLeapDeviceStatus failure_code,
                                         const LEAP_DEVICE failed_device);
typedef void (*policy_callback)         (const uint32_t current_policies);
typedef void (*tracking_callback)       (const LEAP_TRACKING_EVENT* tracking_event);
typedef void (*log_callback)            (const eLeapLogSeverity severity,
                                         const int64_t timestamp,
                                         const char* message);
typedef void (*config_change_callback)  (const uint32_t requestID, const bool success);
typedef void (*config_response_callback)(const uint32_t requestID, LEAP_VARIANT value);
typedef void (*image_callback)          (const LEAP_IMAGE_EVENT* image_event);
typedef void (*point_mapping_change_callback)(const LEAP_POINT_MAPPING_CHANGE_EVENT* point_mapping_change_event);
typedef void (*head_pose_callback)(const LEAP_HEAD_POSE_EVENT* head_pose_event);
typedef void (*imu_callback)(const LEAP_IMU_EVENT* imu_event);
typedef void (*tracking_mode_callback)(const LEAP_TRACKING_MODE_EVENT* mode_event);

struct Callbacks{
  connection_callback      on_connection;
  connection_callback      on_connection_lost;
  device_callback          on_device_found;
  device_lost_callback     on_device_lost;
  device_failure_callback  on_device_failure;
  policy_callback          on_policy;
  tracking_callback        on_frame;
  log_callback             on_log_message;
  config_change_callback   on_config_change;
  config_response_callback on_config_response;
  image_callback           on_image;
  point_mapping_change_callback on_point_mapping_change;
  head_pose_callback       on_head_pose;
  imu_callback             on_imu;
  tracking_mode_callback   on_tracking_mode;
};

extern struct Callbacks LeapC_ConnectionCallbacks;

#endif /* LEAP_CONNECTION_H */
