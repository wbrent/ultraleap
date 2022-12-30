/* Copyright (C) 2012-2017 Ultraleap Limited. All rights reserved.
 *
 * Use of this code is subject to the terms of the Ultraleap SDK agreement
 * available at https://central.leapmotion.com/agreements/SdkAgreement unless
 * Ultraleap has signed a separate license agreement with you or your
 * organisation.
 *
 */

#include "leapConnection.h"

// forward declarations
#if defined(_MSC_VER)
    static void serviceMessageLoop (void* unused);
#else
    static void* serviceMessageLoop (void* unused);
#endif
static void setFrame (const LEAP_TRACKING_EVENT* frame);
static void setDevice (const LEAP_DEVICE_INFO* deviceProps);

// external state
bool LeapC_IsConnected = false;

// internal state
static volatile bool _isRunning = false;
static LEAP_CONNECTION connectionHandle = NULL;
static LEAP_TRACKING_EVENT* lastFrame = NULL;
static LEAP_DEVICE_INFO* lastDevice = NULL;

// callback function pointers
struct Callbacks LeapC_ConnectionCallbacks;

//Threading variables
#if defined(_MSC_VER)
    static HANDLE pollingThread;
    static CRITICAL_SECTION dataLock;
#else
    static pthread_t pollingThread;
    static pthread_mutex_t dataLock;
#endif

// creates the connection handle and opens a connection to the Leap Motion service. on success, creates a thread to service the LeapC message pump.
LEAP_CONNECTION* LeapC_OpenConnection (void)
{
    if (_isRunning)
    {
        return &connectionHandle;
    }

    // must call LeapCreateConnection() to set connectionHandle, then pass connectionHandle to LeapOpenConnection() in order to actually open the connection. it may be that the connectionHandle is already set, in which case we just pass it to LeapOpenConnection() without the need to create a new connection.
    if (connectionHandle || LeapCreateConnection (NULL, &connectionHandle) == eLeapRS_Success)
    {
        eLeapRS result = LeapOpenConnection (connectionHandle);
        if (result == eLeapRS_Success)
        {
            _isRunning = true;
            #if defined(_MSC_VER)
                InitializeCriticalSection (&dataLock);
                pollingThread = (HANDLE)_beginthread (serviceMessageLoop, 0, NULL);
            #else
                pthread_mutex_init (&dataLock, NULL);
                pthread_create (&pollingThread, NULL, serviceMessageLoop, NULL);
            #endif
        }
    }

    return &connectionHandle;
}

void LeapC_CloseConnection (void)
{
    if (! _isRunning)
    {
        return;
    }

    _isRunning = false;
    LeapCloseConnection (connectionHandle);
    #if defined(_MSC_VER)
        WaitForSingleObject (pollingThread, INFINITE);
        CloseHandle (pollingThread);
    #else
        pthread_join (pollingThread, NULL);
        pthread_mutex_destroy (&dataLock);
    #endif
}

void LeapC_DestroyConnection (void)
{
    LeapC_CloseConnection();
    LeapDestroyConnection (connectionHandle);
}


// close the connection and let message thread function end
void CloseConnectionHandle (LEAP_CONNECTION* thisHandle)
{
    LeapDestroyConnection (*thisHandle);
    _isRunning = false;
}

// called by serviceMessageLoop() when a connection event is returned by LeapPollConnection()
static void handleConnectionEvent (const LEAP_CONNECTION_EVENT* connection_event)
{
    LeapC_IsConnected = true;
    if (LeapC_ConnectionCallbacks.on_connection)
    {
        LeapC_ConnectionCallbacks.on_connection();
    }
}

// called by serviceMessageLoop() when a connection lost event is returned by LeapPollConnection()
static void handleConnectionLostEvent (const LEAP_CONNECTION_LOST_EVENT* connection_lost_event)
{
    LeapC_IsConnected = false;
    if (LeapC_ConnectionCallbacks.on_connection_lost)
    {
        LeapC_ConnectionCallbacks.on_connection_lost();
    }
}

// called by serviceMessageLoop() when a device event is returned by LeapPollConnection(). demonstrates how to access device properties.
static void handleDeviceEvent (const LEAP_DEVICE_EVENT* device_event)
{
    LEAP_DEVICE deviceHandle;
    // open device using LEAP_DEVICE_REF from event struct.
    eLeapRS result = LeapOpenDevice (device_event->device, &deviceHandle);

    if (result != eLeapRS_Success)
    {
        printf("Could not open device %s.\n", LeapC_eLeapRSResultString (result));
        return;
    }

    // create a struct to hold the device properties, we have to provide a buffer for the serial string
    LEAP_DEVICE_INFO deviceProperties = { sizeof (deviceProperties) };
    // start with a length of 1 (pretending we don't know a priori what the length is).
    // currently device serial numbers are all the same length, but that could change in the future
    deviceProperties.serial_length = 1;
    deviceProperties.serial = malloc (deviceProperties.serial_length);
    // this will fail since the serial buffer is only 1 character long
    // but deviceProperties is updated to contain the required buffer length
    result = LeapGetDeviceInfo (deviceHandle, &deviceProperties);
    if (result == eLeapRS_InsufficientBuffer)
    {
        // try again with correct buffer size
        deviceProperties.serial = realloc (deviceProperties.serial, deviceProperties.serial_length);

        result = LeapGetDeviceInfo (deviceHandle, &deviceProperties);
        if (result != eLeapRS_Success)
        {
            printf ("Failed to get device info %s.\n", LeapC_eLeapRSResultString (result));
            free (deviceProperties.serial);
            return;
        }
    }

    setDevice (&deviceProperties);
    if (LeapC_ConnectionCallbacks.on_device_found)
    {
        LeapC_ConnectionCallbacks.on_device_found (&deviceProperties);
    }

    free (deviceProperties.serial);
    LeapCloseDevice (deviceHandle);
}

// called by serviceMessageLoop() when a device lost event is returned by LeapPollConnection()
static void handleDeviceLostEvent (const LEAP_DEVICE_EVENT* device_event)
{
    if (LeapC_ConnectionCallbacks.on_device_lost)
    {
        LeapC_ConnectionCallbacks.on_device_lost();
    }
}

// called by serviceMessageLoop() when a device failure event is returned by LeapPollConnection()
static void handleDeviceFailureEvent (const LEAP_DEVICE_FAILURE_EVENT* device_failure_event)
{
    if (LeapC_ConnectionCallbacks.on_device_failure)
    {
        LeapC_ConnectionCallbacks.on_device_failure (device_failure_event->status, device_failure_event->hDevice);
    }
}

// called by serviceMessageLoop() when a tracking event is returned by LeapPollConnection()
static void handleTrackingEvent (const LEAP_TRACKING_EVENT* tracking_event)
{
    setFrame (tracking_event); //support polling tracking data from different thread
    if (LeapC_ConnectionCallbacks.on_frame)
    {
        LeapC_ConnectionCallbacks.on_frame (tracking_event);
    }
}

// called by serviceMessageLoop() when a log event is returned by LeapPollConnection()
static void handleLogEvent (const LEAP_LOG_EVENT* log_event)
{
    if (LeapC_ConnectionCallbacks.on_log_message)
    {
        LeapC_ConnectionCallbacks.on_log_message (log_event->severity, log_event->timestamp, log_event->message);
    }
}

// called by serviceMessageLoop() when a log event is returned by LeapPollConnection()
static void handleLogEvents (const LEAP_LOG_EVENTS* log_events)
{
    if (LeapC_ConnectionCallbacks.on_log_message)
    {
        for (int i = 0; i < (int)(log_events->nEvents); i++)
        {
            const LEAP_LOG_EVENT* log_event = &log_events->events[i];
            LeapC_ConnectionCallbacks.on_log_message(log_event->severity, log_event->timestamp, log_event->message);
        }
    }
}

// called by serviceMessageLoop() when a policy event is returned by LeapPollConnection()
static void handlePolicyEvent (const LEAP_POLICY_EVENT* policy_event)
{
    if (LeapC_ConnectionCallbacks.on_policy)
    {
        LeapC_ConnectionCallbacks.on_policy (policy_event->current_policy);
    }
}

// called by serviceMessageLoop() when a config change event is returned by LeapPollConnection()
static void handleConfigChangeEvent (const LEAP_CONFIG_CHANGE_EVENT* config_change_event)
{
    if (LeapC_ConnectionCallbacks.on_config_change)
    {
        LeapC_ConnectionCallbacks.on_config_change (config_change_event->requestID, config_change_event->status);
    }
}

// called by serviceMessageLoop() when a config response event is returned by LeapPollConnection()
static void handleConfigResponseEvent (const LEAP_CONFIG_RESPONSE_EVENT* config_response_event)
{
    if (LeapC_ConnectionCallbacks.on_config_response)
    {
        LeapC_ConnectionCallbacks.on_config_response (config_response_event->requestID, config_response_event->value);
    }
}

// called by serviceMessageLoop() when an image event is returned by LeapPollConnection()
static void handleImageEvent (const LEAP_IMAGE_EVENT* image_event)
{
    if (LeapC_ConnectionCallbacks.on_image)
    {
        LeapC_ConnectionCallbacks.on_image (image_event);
    }
}

// called by serviceMessageLoop() when a point mapping change event is returned by LeapPollConnection()
static void handlePointMappingChangeEvent (const LEAP_POINT_MAPPING_CHANGE_EVENT* point_mapping_change_event)
{
    if (LeapC_ConnectionCallbacks.on_point_mapping_change)
    {
        LeapC_ConnectionCallbacks.on_point_mapping_change (point_mapping_change_event);
    }
}

// called by serviceMessageLoop() when a point mapping change event is returned by LeapPollConnection()
static void handleHeadPoseEvent (const LEAP_HEAD_POSE_EVENT* head_pose_event)
{
      if (LeapC_ConnectionCallbacks.on_head_pose)
      {
          LeapC_ConnectionCallbacks.on_head_pose (head_pose_event);
      }
}

// called by serviceMessageLoop() when an IMU event is returned by LeapPollConnection()
static void handleImuEvent (const LEAP_IMU_EVENT* imu_event)
{
    if (LeapC_ConnectionCallbacks.on_imu)
    {
        LeapC_ConnectionCallbacks.on_imu (imu_event);
    }
}

static void handleTrackingModeEvent (const LEAP_TRACKING_MODE_EVENT* mode_event)
{
    if (LeapC_ConnectionCallbacks.on_tracking_mode)
    {
        LeapC_ConnectionCallbacks.on_tracking_mode (mode_event);
    }
}

// services the LeapC message pump by calling LeapPollConnection(). the average polling time is determined by the framerate of the Ultraleap Tracking service.
#if defined(_MSC_VER)
    static void serviceMessageLoop(void* unused){
#else
    static void* serviceMessageLoop(void* unused){
#endif
    eLeapRS result;
    LEAP_CONNECTION_MESSAGE msg;

    while (_isRunning)
    {
        unsigned int timeout = 1000;
        result = LeapPollConnection (connectionHandle, timeout, &msg);

        if (result != eLeapRS_Success)
        {
            printf ("LeapC PollConnection call was %s.\n", LeapC_eLeapRSResultString (result));
            continue;
        }

        switch (msg.type)
        {
            case eLeapEventType_Connection:
                handleConnectionEvent (msg.connection_event);
                break;
            case eLeapEventType_ConnectionLost:
                handleConnectionLostEvent (msg.connection_lost_event);
                break;
            case eLeapEventType_Device:
                handleDeviceEvent (msg.device_event);
                break;
            case eLeapEventType_DeviceLost:
                handleDeviceLostEvent (msg.device_event);
                break;
            case eLeapEventType_DeviceFailure:
                handleDeviceFailureEvent (msg.device_failure_event);
                break;
            case eLeapEventType_Tracking:
                handleTrackingEvent (msg.tracking_event);
                break;
            case eLeapEventType_ImageComplete:
                // Ignore
                break;
            case eLeapEventType_ImageRequestError:
                // Ignore
                break;
            case eLeapEventType_LogEvent:
                handleLogEvent (msg.log_event);
                break;
            case eLeapEventType_Policy:
                handlePolicyEvent (msg.policy_event);
                break;
            case eLeapEventType_ConfigChange:
                handleConfigChangeEvent (msg.config_change_event);
                break;
            case eLeapEventType_ConfigResponse:
                handleConfigResponseEvent (msg.config_response_event);
                break;
            case eLeapEventType_Image:
                handleImageEvent (msg.image_event);
                break;
            case eLeapEventType_PointMappingChange:
                handlePointMappingChangeEvent (msg.point_mapping_change_event);
                break;
            case eLeapEventType_TrackingMode:
                handleTrackingModeEvent (msg.tracking_mode_event);
                break;
            case eLeapEventType_LogEvents:
                handleLogEvents (msg.log_events);
                break;
            case eLeapEventType_HeadPose:
                handleHeadPoseEvent (msg.head_pose_event);
                break;
            case eLeapEventType_IMU:
                handleImuEvent (msg.imu_event);
                break;
            default:
                //discard unknown message types
                printf ("Unhandled message type %i.\n", msg.type);
        } //switch on msg.type
    }

    #if !defined(_MSC_VER)
        return NULL;
    #endif
}

// caches the newest frame by copying the tracking event struct returned by LeapC
void setFrame (const LEAP_TRACKING_EVENT* frame)
{
    LockMutex (&dataLock);
    if (! lastFrame)
        lastFrame = malloc (sizeof (*frame));

    *lastFrame = *frame;
    UnlockMutex (&dataLock);
}

// returns a pointer to the cached tracking frame
LEAP_TRACKING_EVENT* LeapC_GetFrame()
{
    LEAP_TRACKING_EVENT* currentFrame;

    LockMutex (&dataLock);
    currentFrame = lastFrame;
    UnlockMutex (&dataLock);

    return currentFrame;
}

// caches the last device found by copying the device info struct returned by LeapC
static void setDevice (const LEAP_DEVICE_INFO* deviceProps)
{
    LockMutex (&dataLock);
    if (lastDevice)
    {
        free (lastDevice->serial);
    }
    else
    {
        lastDevice = malloc (sizeof (*deviceProps));
    }

    *lastDevice = *deviceProps;
    lastDevice->serial = malloc (deviceProps->serial_length);
    memcpy (lastDevice->serial, deviceProps->serial, deviceProps->serial_length);
    UnlockMutex (&dataLock);
}

// returns a pointer to the cached device info
LEAP_DEVICE_INFO* LeapC_GetDeviceProperties()
{
    LEAP_DEVICE_INFO* currentDevice;
    LockMutex (&dataLock);
    currentDevice = lastDevice;
    UnlockMutex (&dataLock);
    return currentDevice;
}

const char* LeapC_SetTrackingMode (eLeapTrackingMode m)
{
    eLeapRS result = LeapSetTrackingMode (connectionHandle, m);
    return LeapC_eLeapRSResultString (result);
}

const char* LeapC_GetTrackingMode (void)
{
    eLeapRS result = LeapGetTrackingMode (connectionHandle);
    return LeapC_eLeapRSResultString (result);
}

// translates eLeapRS result codes into a human-readable string
const char* LeapC_eLeapRSResultString (eLeapRS r)
{
    switch (r)
    {
        case eLeapRS_Success:                  return "eLeapRS_Success";
        case eLeapRS_UnknownError:             return "eLeapRS_UnknownError";
        case eLeapRS_InvalidArgument:          return "eLeapRS_InvalidArgument";
        case eLeapRS_InsufficientResources:    return "eLeapRS_InsufficientResources";
        case eLeapRS_InsufficientBuffer:       return "eLeapRS_InsufficientBuffer";
        case eLeapRS_Timeout:                  return "eLeapRS_Timeout";
        case eLeapRS_NotConnected:             return "eLeapRS_NotConnected";
        case eLeapRS_HandshakeIncomplete:      return "eLeapRS_HandshakeIncomplete";
        case eLeapRS_BufferSizeOverflow:       return "eLeapRS_BufferSizeOverflow";
        case eLeapRS_ProtocolError:            return "eLeapRS_ProtocolError";
        case eLeapRS_InvalidClientID:          return "eLeapRS_InvalidClientID";
        case eLeapRS_UnexpectedClosed:         return "eLeapRS_UnexpectedClosed";
        case eLeapRS_UnknownImageFrameRequest: return "eLeapRS_UnknownImageFrameRequest";
        case eLeapRS_UnknownTrackingFrameID:   return "eLeapRS_UnknownTrackingFrameID";
        case eLeapRS_RoutineIsNotSeer:         return "eLeapRS_RoutineIsNotSeer";
        case eLeapRS_TimestampTooEarly:        return "eLeapRS_TimestampTooEarly";
        case eLeapRS_ConcurrentPoll:           return "eLeapRS_ConcurrentPoll";
        case eLeapRS_NotAvailable:             return "eLeapRS_NotAvailable";
        case eLeapRS_NotStreaming:             return "eLeapRS_NotStreaming";
        case eLeapRS_CannotOpenDevice:         return "eLeapRS_CannotOpenDevice";
        default:                               return "unknown result type.";
    }
}

// translates eLeapTrackingMode result codes into a human-readable string
const char* LeapC_eLeapTrackingModeString (eLeapTrackingMode m)
{
    switch (m)
    {
        case eLeapTrackingMode_Desktop:                  return "eLeapTrackingMode_Desktop";
        case eLeapTrackingMode_HMD:             return "eLeapTrackingMode_HMD";
        case eLeapTrackingMode_ScreenTop:          return "eLeapTrackingMode_ScreenTop";
        case eLeapTrackingMode_Unknown:    return "eLeapTrackingMode_Unknown";
        default:                               return "unknown tracking mode type.";
    }
}

/*
// cross-platform sleep function
void millisleep (int milliseconds)
{
    #ifdef _WIN32
        Sleep (milliseconds);
    #else
        usleep (milliseconds * 1000);
    #endif
}
*/
