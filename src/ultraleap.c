#include "ultraleap.h"

// constructor
static void* ultraleap_new (t_symbol* s, int argc, t_atom* argv)
{
    t_ultraleap* x = (t_ultraleap*) pd_new (ultraleap_class);
    LEAP_VERSION leapVersion;
    eLeapRS result;

    x->x_outletGeneral = outlet_new (&x->x_obj, &s_list);
  	x->x_outletHands = outlet_new (&x->x_obj, &s_list);

    x->x_objSymbol = s;

    // set up callback Functions
    LeapC_ConnectionCallbacks.on_connection = &ultraleap_onConnect;
    LeapC_ConnectionCallbacks.on_device_found = &ultraleap_onDevice;
    LeapC_ConnectionCallbacks.on_tracking_mode = &ultraleap_onTrackingMode;

    // initialize the last frame ID
    x->x_lastFrameID = 0;
    // open a connection and keep a handle for it
    x->x_leapConnection = LeapC_OpenConnection();
    LeapC_SetTrackingMode (eLeapTrackingMode_Desktop);
    result = LeapGetVersion (*(x->x_leapConnection), eLeapVersionPart_ClientLibrary, &leapVersion);

    x->x_generalFlag = 1.0;

    x->x_handTypeFlag = 0.0;
    x->x_handFingerCountFlag = 0.0;
    x->x_handTimeVisibleFlag = 0.0;
    x->x_handGrabStrengthFlag = 0.0;
    x->x_handGrabAngleFlag = 0.0;
    x->x_handPinchStrengthFlag = 0.0;
    x->x_handPinchDistanceFlag = 0.0;

    x->x_armWristPositionFlag = 0.0;
    x->x_armElbowPositionFlag = 0.0;
    x->x_armWidthFlag = 0.0;

    x->x_palmDirectionFlag = 0.0;
    x->x_palmNormalFlag = 0.0;
    x->x_palmPositionFlag = 1.0;
    x->x_palmStabilizedPositionFlag = 0.0;
    x->x_palmVelocityFlag = 0.0;
    x->x_palmWidthFlag = 0.0;

    x->x_fingerTypeFlag = 0.0;
    x->x_fingerDirectionFlag = 0.0;
    x->x_fingerPositionFlag = 0.0;
    x->x_fingerVelocityFlag = 0.0;
    x->x_fingerSizeFlag = 0.0;
    x->x_fingerIsExtendedFlag = 0.0;

    post ("\n****************");
    post ("[ultraleap] version %s", PD_ULTRALEAP_VERSION);
    if (result == eLeapRS_Success)
        post ("built using LeapC API: %d.%d.%d", leapVersion.major, leapVersion.minor, leapVersion.patch);
    post ("William Brent, %s", PD_ULTRALEAP_RELEASE_DATE);
    post ("****************\n");

    return (void*) x;
}

// destructor
static void ultraleap_free (t_ultraleap* x)
{
    // just calling CloseConnection() seems to work without any issues. is that all the cleanup that's required?
    LeapC_CloseConnection();
}

// object initialization
void ultraleap_setup (void)
{
    // call class_new to get a pointer to this object
    ultraleap_class = (t_class*) class_new (
        gensym ("ultraleap"),
        (t_newmethod) ultraleap_new, // new method
        (t_method) ultraleap_free, // free method
        sizeof (t_ultraleap),
        CLASS_DEFAULT,
        A_GIMME,
        0
    );


    // info
    class_addmethod (
        ultraleap_class,
        (t_method) ultraleapInfo,
        gensym ("info"),
        A_NULL
    );


    // poll
    class_addbang (
        ultraleap_class,
        (t_method) ultraleapPoll
    );


    // set methods
    // tracking mode
    class_addmethod (
        ultraleap_class,
        (t_method) ultraleapSetTrackingMode,
        gensym ("tracking_mode"),
        A_DEFSYMBOL,
        A_NULL
    );

    // general
    class_addmethod (
        ultraleap_class,
        (t_method) ultraleapSetGeneralFlag,
        gensym ("general"),
        A_DEFFLOAT,
        A_NULL
    );

    // hand
    class_addmethod (
        ultraleap_class,
        (t_method) ultraleapSetHandTypeFlag,
        gensym ("hand_type"),
        A_DEFFLOAT,
        A_NULL
    );

    class_addmethod (
        ultraleap_class,
        (t_method) ultraleapSetHandFingerCountFlag,
        gensym ("finger_count"),
        A_DEFFLOAT,
        A_NULL
    );

    class_addmethod (
        ultraleap_class,
        (t_method) ultraleapSetHandTimeVisibleFlag,
        gensym ("time_visible"),
        A_DEFFLOAT,
        A_NULL
    );

    class_addmethod (
        ultraleap_class,
        (t_method) ultraleapSetHandGrabStrengthFlag,
        gensym ("grab_strength"),
        A_DEFFLOAT,
        A_NULL
    );

    class_addmethod (
        ultraleap_class,
        (t_method) ultraleapSetHandGrabAngleFlag,
        gensym ("grab_angle"),
        A_DEFFLOAT,
        A_NULL
    );

    class_addmethod (
        ultraleap_class,
        (t_method) ultraleapSetHandPinchStrengthFlag,
        gensym ("pinch_strength"),
        A_DEFFLOAT,
        A_NULL
    );

    class_addmethod (
        ultraleap_class,
        (t_method) ultraleapSetHandPinchDistanceFlag,
        gensym ("pinch_distance"),
        A_DEFFLOAT,
        A_NULL
    );

    // arm
    class_addmethod (
        ultraleap_class,
        (t_method) ultraleapSetArmWristPositionFlag,
        gensym ("wrist_position"),
        A_DEFFLOAT,
        A_NULL
    );

    class_addmethod (
        ultraleap_class,
        (t_method) ultraleapSetArmElbowPositionFlag,
        gensym ("elbow_position"),
        A_DEFFLOAT,
        A_NULL
    );

    class_addmethod (
        ultraleap_class,
        (t_method) ultraleapSetArmWidthFlag,
        gensym ("arm_width"),
        A_DEFFLOAT,
        A_NULL
    );

    // palm
    class_addmethod (
        ultraleap_class,
        (t_method) ultraleapSetPalmDirectionFlag,
        gensym ("palm_direction"),
        A_DEFFLOAT,
        A_NULL
    );

    class_addmethod (
        ultraleap_class,
        (t_method) ultraleapSetPalmNormalFlag,
        gensym ("palm_normal"),
        A_DEFFLOAT,
        A_NULL
    );

    class_addmethod (
        ultraleap_class,
        (t_method) ultraleapSetPalmPositionFlag,
        gensym ("palm_position"),
        A_DEFFLOAT,
        A_NULL
    );

    class_addmethod (
        ultraleap_class,
        (t_method) ultraleapSetPalmStabilizedPositionFlag,
        gensym ("palm_stabilized_position"),
        A_DEFFLOAT,
        A_NULL
    );

    class_addmethod (
        ultraleap_class,
        (t_method) ultraleapSetPalmVelocityFlag,
        gensym ("palm_velocity"),
        A_DEFFLOAT,
        A_NULL
    );

    class_addmethod (
        ultraleap_class,
        (t_method) ultraleapSetPalmWidthFlag,
        gensym ("palm_width"),
        A_DEFFLOAT,
        A_NULL
    );

    // fingers
    class_addmethod (
        ultraleap_class,
        (t_method) ultraleapSetFingerTypeFlag,
        gensym ("finger_type"),
        A_DEFFLOAT,
        A_NULL
    );

    class_addmethod (
        ultraleap_class,
        (t_method) ultraleapSetFingerDirectionFlag,
        gensym ("finger_direction"),
        A_DEFFLOAT,
        A_NULL
    );

    class_addmethod (
        ultraleap_class,
        (t_method) ultraleapSetFingerPositionFlag,
        gensym ("finger_position"),
        A_DEFFLOAT,
        A_NULL
    );

    class_addmethod (
        ultraleap_class,
        (t_method) ultraleapSetFingerVelocityFlag,
        gensym ("finger_velocity"),
        A_DEFFLOAT,
        A_NULL
    );

    class_addmethod (
        ultraleap_class,
        (t_method) ultraleapSetFingerSizeFlag,
        gensym ("finger_size"),
        A_DEFFLOAT,
        A_NULL
    );

    class_addmethod (
        ultraleap_class,
        (t_method) ultraleapSetFingerIsExtendedFlag,
        gensym ("finger_is_extended"),
        A_DEFFLOAT,
        A_NULL
    );
}


// utility functions
static t_float ultraleapGetVectorMagnitude (LEAP_VECTOR v)
{
    t_float mag = 0.0;
    mag = sqrt ((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
    return mag;
}

static t_float ultraleapGetEuclideanDistance (LEAP_VECTOR a, LEAP_VECTOR b)
{
    t_float diffX = a.x - b.x;
    t_float diffY = a.y - b.y;
    t_float diffZ = a.z - b.z;
    t_float dist = sqrt ((diffX * diffX) + (diffY * diffY) + (diffZ * diffZ));

    return dist;
}

static LEAP_VECTOR ultraleapGetVectorCentroid (LEAP_VECTOR a, LEAP_VECTOR b)
{
    t_float meanX = (a.x + b.x) * 0.5;
    t_float meanY = (a.y + b.y) * 0.5;
    t_float meanZ = (a.z + b.z) * 0.5;
    LEAP_VECTOR centroid;

    centroid.x = meanX;
    centroid.y = meanY;
    centroid.z = meanZ;

    return centroid;
}

static LEAP_VECTOR ultraleapNormalizeVector (LEAP_VECTOR v)
{
    LEAP_VECTOR normVec = v;
    t_float mag = ultraleapGetVectorMagnitude (v);
    t_float denom;

    if (mag <= EPSILON)
    {
        normVec.x = 0.0;
        normVec.y = 0.0;
        normVec.z = 0.0;

        return normVec;
    }

    denom = 1.0f / mag;

    normVec.x *= denom;
    normVec.y *= denom;
    normVec.z *= denom;

    return normVec;
}


// set methods: tracking mode
static void ultraleapSetTrackingMode (t_ultraleap* x, t_symbol* m)
{
    const char* result;

    if (! strcmp (m->s_name, "desktop"))
        result = LeapC_SetTrackingMode (eLeapTrackingMode_Desktop);
    else if (! strcmp (m->s_name, "hmd"))
        result = LeapC_SetTrackingMode (eLeapTrackingMode_HMD);
    else if (! strcmp (m->s_name, "screentop"))
        result = LeapC_SetTrackingMode (eLeapTrackingMode_ScreenTop);
    else
        result = "invalid";

    if (strcmp (result, "eLeapRS_Success"))
        pd_error (x, "[%s]: failed to set tracking mode. unknown mode type: %s", x->x_objSymbol->s_name, m->s_name);
}


// set methods: general
static void ultraleapSetGeneralFlag (t_ultraleap* x, t_float state)
{
    state = (state < 0.0) ? 0.0 : state;
    state = (state > 1.0) ? 1.0 : state;

    x->x_generalFlag = state;
}


// set methods: hands
static void ultraleapSetHandTypeFlag (t_ultraleap* x, t_float state)
{
    state = (state < 0.0) ? 0.0 : state;
    state = (state > 1.0) ? 1.0 : state;

    x->x_handTypeFlag = state;
}

static void ultraleapSetHandFingerCountFlag (t_ultraleap* x, t_float state)
{
    state = (state < 0.0) ? 0.0 : state;
    state = (state > 1.0) ? 1.0 : state;

    x->x_handFingerCountFlag = state;
}

static void ultraleapSetHandTimeVisibleFlag (t_ultraleap* x, t_float state)
{
    state = (state < 0.0) ? 0.0 : state;
    state = (state > 1.0) ? 1.0 : state;

    x->x_handTimeVisibleFlag = state;
}

static void ultraleapSetHandGrabStrengthFlag (t_ultraleap* x, t_float state)
{
    state = (state < 0.0) ? 0.0 : state;
    state = (state > 1.0) ? 1.0 : state;

    x->x_handGrabStrengthFlag = state;
}

static void ultraleapSetHandGrabAngleFlag (t_ultraleap* x, t_float state)
{
    state = (state < 0.0) ? 0.0 : state;
    state = (state > 1.0) ? 1.0 : state;

    x->x_handGrabAngleFlag = state;
}

static void ultraleapSetHandPinchStrengthFlag (t_ultraleap* x, t_float state)
{
    state = (state < 0.0) ? 0.0 : state;
    state = (state > 1.0) ? 1.0 : state;

    x->x_handPinchStrengthFlag = state;
}

static void ultraleapSetHandPinchDistanceFlag (t_ultraleap* x, t_float state)
{
    state = (state < 0.0) ? 0.0 : state;
    state = (state > 1.0) ? 1.0 : state;

    x->x_handPinchDistanceFlag = state;
}

// set methods: arms
static void ultraleapSetArmWristPositionFlag (t_ultraleap* x, t_float state)
{
    state = (state < 0.0) ? 0.0 : state;
    state = (state > 1.0) ? 1.0 : state;

    x->x_armWristPositionFlag = state;
}

static void ultraleapSetArmElbowPositionFlag (t_ultraleap* x, t_float state)
{
    state = (state < 0.0) ? 0.0 : state;
    state = (state > 1.0) ? 1.0 : state;

    x->x_armElbowPositionFlag = state;
}

static void ultraleapSetArmWidthFlag (t_ultraleap* x, t_float state)
{
    state = (state < 0.0) ? 0.0 : state;
    state = (state > 1.0) ? 1.0 : state;

    x->x_armWidthFlag = state;
}

// set methods: palms
static void ultraleapSetPalmDirectionFlag (t_ultraleap* x, t_float state)
{
    state = (state < 0.0) ? 0.0 : state;
    state = (state > 1.0) ? 1.0 : state;

    x->x_palmDirectionFlag = state;
}

static void ultraleapSetPalmNormalFlag (t_ultraleap* x, t_float state)
{
    state = (state < 0.0) ? 0.0 : state;
    state = (state > 1.0) ? 1.0 : state;

    x->x_palmNormalFlag = state;
}

static void ultraleapSetPalmPositionFlag (t_ultraleap* x, t_float state)
{
    state = (state < 0.0) ? 0.0 : state;
    state = (state > 1.0) ? 1.0 : state;

    x->x_palmPositionFlag = state;
}

static void ultraleapSetPalmStabilizedPositionFlag (t_ultraleap* x, t_float state)
{
    state = (state < 0.0) ? 0.0 : state;
    state = (state > 1.0) ? 1.0 : state;

    x->x_palmStabilizedPositionFlag = state;
}

static void ultraleapSetPalmVelocityFlag (t_ultraleap* x, t_float state)
{
    state = (state < 0.0) ? 0.0 : state;
    state = (state > 1.0) ? 1.0 : state;

    x->x_palmVelocityFlag = state;
}

static void ultraleapSetPalmWidthFlag (t_ultraleap* x, t_float state)
{
    state = (state < 0.0) ? 0.0 : state;
    state = (state > 1.0) ? 1.0 : state;

    x->x_palmWidthFlag = state;
}

// set methods: fingers
static void ultraleapSetFingerTypeFlag (t_ultraleap* x, t_float state)
{
    state = (state < 0.0) ? 0.0 : state;
    state = (state > 1.0) ? 1.0 : state;

    x->x_fingerTypeFlag = state;
}

static void ultraleapSetFingerDirectionFlag (t_ultraleap* x, t_float state)
{
    state = (state < 0.0) ? 0.0 : state;
    state = (state > 1.0) ? 1.0 : state;

    x->x_fingerDirectionFlag = state;
}

static void ultraleapSetFingerPositionFlag (t_ultraleap* x, t_float state)
{
    state = (state < 0.0) ? 0.0 : state;
    state = (state > 1.0) ? 1.0 : state;

    x->x_fingerPositionFlag = state;
}

static void ultraleapSetFingerVelocityFlag (t_ultraleap* x, t_float state)
{
    state = (state < 0.0) ? 0.0 : state;
    state = (state > 1.0) ? 1.0 : state;

    x->x_fingerVelocityFlag = state;
}

static void ultraleapSetFingerSizeFlag (t_ultraleap* x, t_float state)
{
    state = (state < 0.0) ? 0.0 : state;
    state = (state > 1.0) ? 1.0 : state;

    x->x_fingerSizeFlag = state;
}

static void ultraleapSetFingerIsExtendedFlag (t_ultraleap* x, t_float state)
{
    state = (state < 0.0) ? 0.0 : state;
    state = (state > 1.0) ? 1.0 : state;

    x->x_fingerIsExtendedFlag = state;
}


// info method
static void ultraleapInfo (t_ultraleap* x)
{
    LEAP_DEVICE_INFO* deviceProps = LeapC_GetDeviceProperties();

    post ("\n[ultraleap] version %s\n", PD_ULTRALEAP_VERSION);

    post ("general: %1.0f\n", x->x_generalFlag);

    post ("hand_type: %1.0f", x->x_handTypeFlag);
    post ("finger_count: %1.0f", x->x_handFingerCountFlag);
    post ("time_visible: %1.0f", x->x_handTimeVisibleFlag);
    post ("grab_strength: %1.0f", x->x_handGrabStrengthFlag);
    post ("grab_angle: %1.0f", x->x_handGrabAngleFlag);
    post ("pinch_strength: %1.0f", x->x_handPinchStrengthFlag);
    post ("pinch_distance: %1.0f\n", x->x_handPinchDistanceFlag);

    post ("wrist_position: %1.0f", x->x_armWristPositionFlag);
    post ("elbow_position: %1.0f", x->x_armElbowPositionFlag);
    post ("arm_width: %1.0f\n", x->x_armWidthFlag);

    post ("palm_direction: %1.0f", x->x_palmDirectionFlag);
    post ("palm_normal: %1.0f", x->x_palmNormalFlag);
    post ("palm_position: %1.0f", x->x_palmPositionFlag);
    post ("palm_stabilized_position: %1.0f", x->x_palmStabilizedPositionFlag);
    post ("palm_velocity: %1.0f", x->x_palmVelocityFlag);
    post ("palm_width: %1.0f\n", x->x_palmWidthFlag);

    post ("finger_type: %1.0f", x->x_fingerTypeFlag);
    post ("finger_direction: %1.0f", x->x_fingerDirectionFlag);
    post ("finger_position: %1.0f", x->x_fingerPositionFlag);
    post ("finger_velocity: %1.0f", x->x_fingerVelocityFlag);
    post ("finger_size: %1.0f", x->x_fingerSizeFlag);
    post ("finger_is_extended: %1.0f\n", x->x_fingerIsExtendedFlag);

    if (deviceProps)
        post ("Using device: %s", deviceProps->serial);

    // Requesting the current tracking mode is asynchronous. After you call this function, a subsequent call to LeapPollConnection provides a LEAP_TRACKING_MODE_EVENT containing the current tracking mode, reflecting any changes.
    // to trigger the _onTrackingMode callback
    LeapC_GetTrackingMode();
}

// poll method
static void ultraleapPoll (t_ultraleap* x)
{
    uint32_t nHands;
    LEAP_TRACKING_EVENT* frame = LeapC_GetFrame();

    if (frame && (frame->tracking_frame_id > x->x_lastFrameID))
    {
        x->x_lastFrameID = frame->tracking_frame_id;
        nHands = frame->nHands;

        if (nHands > 0)
        {
            ultraleapProcessArms (x, frame);
            ultraleapProcessHands (x, frame);
        }

        if (x->x_generalFlag)
            ultraleapProcessGeneral (x, frame);
    }
}

// process hand data
static void ultraleapProcessHands (t_ultraleap* x, LEAP_TRACKING_EVENT* frame)
{
    LEAP_HAND* handList = frame->pHands;

    for (uint32_t handIdx = 0; handIdx < frame->nHands; handIdx++)
    {
        int numHandInfoAtoms = 6;
        t_atom handInfo[numHandInfoAtoms];

        // get this particular hand
        LEAP_HAND hand = handList[handIdx];
        // get a pointer to the array of digits
        LEAP_DIGIT* fingerList = &hand.digits[0];

        // set first atom to handIdx since all output lists will begin with that
        SETFLOAT (&handInfo[0], handIdx);

        // HAND
        if (x->x_handTypeFlag)
        {
            SETSYMBOL (&handInfo[1], gensym ("type"));

            if (hand.confidence > 0.5)
            {
                if (hand.type == eLeapHandType_Left)
                    SETSYMBOL (&handInfo[2], gensym ("left"));
                else if (hand.type == eLeapHandType_Right)
                    SETSYMBOL (&handInfo[2], gensym ("right"));
            }
            else
                SETSYMBOL (&handInfo[2], gensym ("invalid"));

            outlet_list (x->x_outletHands, 0, numHandInfoAtoms - 3, handInfo);
        }

        // finger count
        if (x->x_handFingerCountFlag)
        {
            uint32_t fingerCount = 0;

            for (int fingerIdx = 0; fingerIdx < NUM_FINGERS; fingerIdx++)
            {
                LEAP_DIGIT finger = fingerList[fingerIdx];
                fingerCount += finger.is_extended;
            }

            SETSYMBOL (&handInfo[1], gensym ("finger_count"));
            SETFLOAT (&handInfo[2], fingerCount);

            outlet_list (x->x_outletHands, 0, numHandInfoAtoms - 3, handInfo);
        }

        // time visible
        if (x->x_handTimeVisibleFlag)
        {
            SETSYMBOL (&handInfo[1], gensym ("time_visible"));
            SETFLOAT (&handInfo[2], (t_float) (hand.visible_time / (t_float)1000000));

            outlet_list (x->x_outletHands, 0, numHandInfoAtoms - 3, handInfo);
        }

        // angle is a measure of the grab angle in radians. strength is normalized 0..1
        // grab strength
        if (x->x_handGrabStrengthFlag)
        {
            SETSYMBOL (&handInfo[1], gensym ("grab_strength"));
            SETFLOAT (&handInfo[2], hand.grab_strength);

            outlet_list (x->x_outletHands, 0, numHandInfoAtoms - 3, handInfo);
        }

        // grab angle
        if (x->x_handGrabAngleFlag)
        {
            SETSYMBOL (&handInfo[1], gensym ("grab_angle"));
            SETFLOAT (&handInfo[2], hand.grab_angle);

            outlet_list (x->x_outletHands, 0, numHandInfoAtoms - 3, handInfo);
        }

        // strength is normalized 0..1, distance is a measurement in mm
        // time visible
        if (x->x_handPinchStrengthFlag)
        {
            SETSYMBOL (&handInfo[1], gensym ("pinch_strength"));
            SETFLOAT (&handInfo[2], hand.pinch_strength);

            outlet_list (x->x_outletHands, 0, numHandInfoAtoms - 3, handInfo);
        }

        // pinch distance
        if (x->x_handPinchDistanceFlag)
        {
            SETSYMBOL (&handInfo[1], gensym ("pinch_distance"));
            SETFLOAT (&handInfo[2], hand.pinch_distance);

            outlet_list (x->x_outletHands, 0, numHandInfoAtoms - 3, handInfo);
        }

        // PALM
        // direction
        if (x->x_palmDirectionFlag)
        {
            SETSYMBOL (&handInfo[1], gensym ("palm"));
            SETSYMBOL (&handInfo[2], gensym ("direction"));
            SETFLOAT (&handInfo[3], hand.palm.direction.x);
            SETFLOAT (&handInfo[4], hand.palm.direction.y);
            SETFLOAT (&handInfo[5], hand.palm.direction.z);

            outlet_list (x->x_outletHands, 0, numHandInfoAtoms, handInfo);
        }

        // palm normal
        if (x->x_palmNormalFlag)
        {
            SETSYMBOL (&handInfo[1], gensym ("palm"));
            SETSYMBOL (&handInfo[2], gensym ("normal"));
            SETFLOAT (&handInfo[3], hand.palm.normal.x);
            SETFLOAT (&handInfo[4], hand.palm.normal.y);
            SETFLOAT (&handInfo[5], hand.palm.normal.z);

            outlet_list (x->x_outletHands, 0, numHandInfoAtoms, handInfo);
        }

        // palm position
        if (x->x_palmPositionFlag)
        {
            SETSYMBOL (&handInfo[1], gensym ("palm"));
            SETSYMBOL (&handInfo[2], gensym ("position"));
            SETFLOAT (&handInfo[3], hand.palm.position.x);
            SETFLOAT (&handInfo[4], hand.palm.position.y);
            SETFLOAT (&handInfo[5], hand.palm.position.z);

            outlet_list (x->x_outletHands, 0, numHandInfoAtoms, handInfo);
        }

        // TODO: why is this vector always (0, 0, 0)? is it necessary to do some configuration for .stabilized_position to be set by LEAP_PALM?
        // palm stabilized position
        if (x->x_palmStabilizedPositionFlag)
        {
            SETSYMBOL (&handInfo[1], gensym ("palm"));
            SETSYMBOL (&handInfo[2], gensym ("stabilized_position"));
            SETFLOAT (&handInfo[3], hand.palm.stabilized_position.x);
            SETFLOAT (&handInfo[4], hand.palm.stabilized_position.y);
            SETFLOAT (&handInfo[5], hand.palm.stabilized_position.z);

            outlet_list (x->x_outletHands, 0, numHandInfoAtoms, handInfo);
        }

        // palm velocity
        if (x->x_palmVelocityFlag)
        {
            SETSYMBOL (&handInfo[1], gensym ("palm"));
            SETSYMBOL (&handInfo[2], gensym ("velocity"));
            SETFLOAT (&handInfo[3], hand.palm.velocity.x);
            SETFLOAT (&handInfo[4], hand.palm.velocity.y);
            SETFLOAT (&handInfo[5], hand.palm.velocity.z);

            outlet_list (x->x_outletHands, 0, numHandInfoAtoms, handInfo);
        }

        // palm width
        if (x->x_palmWidthFlag)
        {
            SETSYMBOL (&handInfo[1], gensym ("palm"));
            SETSYMBOL (&handInfo[2], gensym ("width"));
            SETFLOAT (&handInfo[3], hand.palm.width);

            outlet_list (x->x_outletHands, 0, numHandInfoAtoms - 2, handInfo);
        }

        // process fingers per hand
        ultraleapProcessFingers (x, handIdx, fingerList);
    }
}

// process arm per hand
static void ultraleapProcessArms (t_ultraleap* x, LEAP_TRACKING_EVENT* frame)
{
    LEAP_HAND* handList = frame->pHands;

    for (uint32_t handIdx = 0; handIdx < frame->nHands; handIdx++)
    {
        int numArmInfoAtoms = 6;
        t_atom armInfo[numArmInfoAtoms];

        // get this particular hand
        LEAP_HAND hand = handList[handIdx];

        // set first atom to handIdx since all output lists will begin with that
        SETFLOAT (&armInfo[0], handIdx);

        // TODO: make arm direction flag
        LEAP_VECTOR diffVec;

        // based on LeapImplementationC++.h line 160, we can get the direction of a bone by getting the difference betweeen next_joint and prev_joint in each dimension. it also needs to be normalized.
        diffVec.x = hand.arm.next_joint.x - hand.arm.prev_joint.x;
        diffVec.y = hand.arm.next_joint.y - hand.arm.prev_joint.y;
        diffVec.z = hand.arm.next_joint.z - hand.arm.prev_joint.z;

        diffVec = ultraleapNormalizeVector (diffVec);

        SETSYMBOL (&armInfo[1], gensym ("arm"));
        SETSYMBOL (&armInfo[2], gensym ("direction"));
        SETFLOAT (&armInfo[3], diffVec.x);
        SETFLOAT (&armInfo[4], diffVec.y);
        SETFLOAT (&armInfo[5], diffVec.z);

        outlet_list (x->x_outletHands, 0, numArmInfoAtoms, armInfo);

        if (x->x_armWristPositionFlag)
        {
            SETSYMBOL (&armInfo[1], gensym ("arm"));
            SETSYMBOL (&armInfo[2], gensym ("wrist_position"));
            SETFLOAT (&armInfo[3], hand.arm.next_joint.x);
            SETFLOAT (&armInfo[4], hand.arm.next_joint.y);
            SETFLOAT (&armInfo[5], hand.arm.next_joint.z);

            outlet_list (x->x_outletHands, 0, numArmInfoAtoms, armInfo);
        }

        if (x->x_armElbowPositionFlag)
        {
            SETSYMBOL (&armInfo[1], gensym ("arm"));
            SETSYMBOL (&armInfo[2], gensym ("elbow_position"));
            SETFLOAT (&armInfo[3], hand.arm.prev_joint.x);
            SETFLOAT (&armInfo[4], hand.arm.prev_joint.y);
            SETFLOAT (&armInfo[5], hand.arm.prev_joint.z);

            outlet_list (x->x_outletHands, 0, numArmInfoAtoms, armInfo);
        }

        // TODO: make arm center flag
        // based on LeapImplementationC++.h line 159, we can get the center of a bone by taking the arithmetic mean of the next_joint and prev_joint coordinates
        LEAP_VECTOR centroid = ultraleapGetVectorCentroid (hand.arm.next_joint, hand.arm.prev_joint);

        SETSYMBOL (&armInfo[1], gensym ("arm"));
        SETSYMBOL (&armInfo[2], gensym ("center"));
        SETFLOAT (&armInfo[3], centroid.x);
        SETFLOAT (&armInfo[4], centroid.y);
        SETFLOAT (&armInfo[5], centroid.z);

        outlet_list (x->x_outletHands, 0, numArmInfoAtoms, armInfo);

        if (x->x_armWidthFlag)
        {
            SETSYMBOL (&armInfo[1], gensym ("arm"));
            SETSYMBOL (&armInfo[2], gensym ("width"));
            SETFLOAT (&armInfo[3], hand.arm.width);

            outlet_list (x->x_outletHands, 0, numArmInfoAtoms - 2, armInfo);
        }
    }
}

// process fingers
static void ultraleapProcessFingers (t_ultraleap* x, int handIdx, LEAP_DIGIT* fingerList)
{
    for (int fingerIdx = 0; fingerIdx < NUM_FINGERS; fingerIdx++)
    {
        LEAP_DIGIT finger = fingerList[fingerIdx];
        // each finger has 4 bones. the distal bone is the tip. each bone has two joints/ends. next_joint is the end further from the body
        LEAP_BONE proximalBone = finger.proximal;
        LEAP_BONE distalBone = finger.distal;

        int numFingerInfoAtoms = 7;
        t_atom fingerInfo[numFingerInfoAtoms];

        SETFLOAT (&fingerInfo[0], handIdx);

        // type
        if (x->x_fingerTypeFlag)
        {
            SETSYMBOL (&fingerInfo[1], gensym ("finger"));
            // finger_id seems to be consistent, where thumb = 0, index = 1, middle = 2, ring = 3, and pinky = 4
            SETFLOAT (&fingerInfo[2], finger.finger_id);
            SETSYMBOL (&fingerInfo[3], gensym ("type"));

            switch (finger.finger_id)
            {
                case 0:
                    SETSYMBOL (&fingerInfo[4], gensym ("thumb"));
                    break;
                case 1:
                    SETSYMBOL (&fingerInfo[4], gensym ("index"));
                    break;
                case 2:
                    SETSYMBOL (&fingerInfo[4], gensym ("middle"));
                    break;
                case 3:
                    SETSYMBOL (&fingerInfo[4], gensym ("ring"));
                    break;
                case 4:
                    SETSYMBOL (&fingerInfo[4], gensym ("pinky"));
                    break;
                default:
                    SETSYMBOL (&fingerInfo[4], gensym ("invalid"));
                    break;
            }

            outlet_list (x->x_outletHands, 0, numFingerInfoAtoms - 2, fingerInfo);
        }

        // direction
        if (x->x_fingerDirectionFlag)
        {
            LEAP_VECTOR diffVec;

            // based on LeapImplementationC++.h line 160, we can get the direction of a bone by getting the difference betweeen next_joint and prev_joint in each dimension. here, we use the next_joint of the distal bone and prev_joint of the proximal bone of a finger. it also needs to be normalized.
            diffVec.x = distalBone.next_joint.x - proximalBone.prev_joint.x;
            diffVec.y = distalBone.next_joint.y - proximalBone.prev_joint.y;
            diffVec.z = distalBone.next_joint.z - proximalBone.prev_joint.z;

            diffVec = ultraleapNormalizeVector (diffVec);

            SETSYMBOL (&fingerInfo[1], gensym ("finger"));
            SETFLOAT (&fingerInfo[2], finger.finger_id);
            SETSYMBOL (&fingerInfo[3], gensym ("direction"));

            SETFLOAT (&fingerInfo[4], diffVec.x);
            SETFLOAT (&fingerInfo[5], diffVec.y);
            SETFLOAT (&fingerInfo[6], diffVec.z);

            outlet_list (x->x_outletHands, 0, numFingerInfoAtoms, fingerInfo);
        }

        if (x->x_fingerPositionFlag)
        {
            SETSYMBOL (&fingerInfo[1], gensym ("finger"));
            // finger_id seems to be consistent, where thumb = 0, index = 1, middle = 2, ring = 3, and pinky = 4
            SETFLOAT (&fingerInfo[2], finger.finger_id);
            SETSYMBOL (&fingerInfo[3], gensym ("position"));
            SETFLOAT (&fingerInfo[4], distalBone.next_joint.x);
            SETFLOAT (&fingerInfo[5], distalBone.next_joint.y);
            SETFLOAT (&fingerInfo[6], distalBone.next_joint.z);

            outlet_list (x->x_outletHands, 0, numFingerInfoAtoms, fingerInfo);
        }

        // TODO: since there's no finger velocity in LeapC, need to store the coordinate of each finger on the previous frame and get the difference.
/*
        if (x->x_fingerVelocityFlag)
        {
            SETFLOAT (&fingerInfo[0], handIdx);
            SETSYMBOL (&fingerInfo[1], gensym ("finger"));
            SETFLOAT (&fingerInfo[2], finger.type());
            SETSYMBOL (&fingerInfo[3], gensym ("velocity"));
            SETFLOAT (&fingerInfo[4], finger.tipVelocity().x);
            SETFLOAT (&fingerInfo[5], finger.tipVelocity().y);
            SETFLOAT (&fingerInfo[6], finger.tipVelocity().z);

            outlet_anything (x->x_outletHands, gensym ("hand"), numFingerInfoAtoms, fingerInfo);
        }
*/
        // size
        if (x->x_fingerSizeFlag)
        {
            // based on LeapImplementationC++.h line 161, we can get the length of a bone by getting the euclidean distance betweeen next_joint and prev_joint. for a finger, we need the distance between the proximal and distal bones
            t_float length = ultraleapGetEuclideanDistance (proximalBone.prev_joint, distalBone.next_joint);

            // TODO: this seems to work, but the thumb is longer than it should be. is this related to the fact that the thumb has no metacarpal LEAP_BONE member?
            SETFLOAT (&fingerInfo[0], handIdx);
            SETSYMBOL (&fingerInfo[1], gensym ("finger"));
            SETFLOAT (&fingerInfo[2], finger.finger_id);
            SETSYMBOL (&fingerInfo[3], gensym ("size"));
            SETFLOAT (&fingerInfo[4], distalBone.width);
            SETFLOAT (&fingerInfo[5], length);

            outlet_list (x->x_outletHands, 0, numFingerInfoAtoms - 1, fingerInfo);
        }

        // is extended
        if (x->x_fingerIsExtendedFlag)
        {
            SETSYMBOL (&fingerInfo[1], gensym ("finger"));
            // finger_id seems to be consistent, where thumb = 0, index = 1, middle = 2, ring = 3, and pinky = 4
            SETFLOAT (&fingerInfo[2], finger.finger_id);
            SETSYMBOL (&fingerInfo[3], gensym ("is_extended"));
            SETFLOAT (&fingerInfo[4], finger.is_extended);

            outlet_list (x->x_outletHands, 0, numFingerInfoAtoms - 2, fingerInfo);
        }
    }
}

// process general data
static void ultraleapProcessGeneral (t_ultraleap* x, LEAP_TRACKING_EVENT* frame)
{
    int numGeneralInfoAtoms = 4;
    t_atom generalInfo[numGeneralInfoAtoms];

    SETFLOAT (&generalInfo[0], (t_float) frame->tracking_frame_id);
    SETFLOAT (&generalInfo[1], (t_float) (frame->info.timestamp / (t_float) 1000000));
    SETFLOAT (&generalInfo[2], (t_float) frame->framerate);
    SETFLOAT (&generalInfo[3], (t_float) frame->nHands);

    outlet_list (x->x_outletGeneral, 0, numGeneralInfoAtoms, &generalInfo[0]);
}


//// callback functions
//
// callback for when the connection opens
static void ultraleap_onConnect (void)
{
    post ("Leap Controller: Connected");
}

// callback for when device is found
static void ultraleap_onDevice (const LEAP_DEVICE_INFO* props)
{
    post ("Using device: %s", props->serial);
}

// callback for when tracking mode changes
static void ultraleap_onTrackingMode (const LEAP_TRACKING_MODE_EVENT* mode_event)
{
    const char* trackingMode = LeapC_eLeapTrackingModeString (mode_event->current_tracking_mode);
    post ("Leap Tracking Mode: %s", trackingMode);
}
