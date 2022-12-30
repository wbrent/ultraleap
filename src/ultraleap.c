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
    ConnectionCallbacks.on_connection = &ultraleap_onConnect;
    ConnectionCallbacks.on_tracking_mode = &ultraleap_onTrackingMode;

    // initialize the last frame ID
    x->x_lastFrameID = 0;
    // open a connection and keep a handle for it
    x->x_leapConnection = OpenConnection();
    SetTrackingMode (eLeapTrackingMode_Desktop);

    x->x_handTypeFlag = 0.0;
    x->x_handPalmDirectionFlag = 0.0;
    x->x_handPalmNormalFlag = 0.0;
    x->x_handPalmPositionFlag = 1.0;
    x->x_handPalmVelocityFlag = 0.0;
    x->x_handFingerCountFlag = 0.0;

    x->x_fingerDirectionFlag = 0.0;
    x->x_fingerPositionFlag = 0.0;
    x->x_fingerVelocityFlag = 0.0;
    x->x_fingerSizeFlag = 0.0;

    x->x_generalFlag = 1.0;

    result = LeapGetVersion (*(x->x_leapConnection), eLeapVersionPart_ClientLibrary, &leapVersion);

    post ("\n****************");
    post ("[ultraleap] for Pd %s", PD_ULTRALEAP_VERSION);
    if (result == eLeapRS_Success)
        post ("built using LeapC API: %d.%d.%d", leapVersion.major, leapVersion.minor, leapVersion.patch);
    post ("William Brent, Dec 2022");
    post ("****************\n");

    return (void*) x;
}

// destructor
static void ultraleap_free (t_ultraleap* x)
{
    // calling this crashes Pd when deleting/recreating [ultraleap]
    // DestroyConnection();
    // DestroyConnection() calls CloseConnection(), but also LeapDestroyConnection(), which destroys the underlying connectionHandle object and releases all resources associated with it. not sure why this causes a crash when deleting/recreating [ultraleap], but it does.

    // just calling CloseConnection() seems to work without any issues. is that all the cleanup that's required?
    CloseConnection();
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
    // general
    class_addmethod (
        ultraleap_class,
        (t_method) ultraleapSetGeneralFlag,
        gensym ("general"),
        A_DEFFLOAT,
        A_NULL
    );


    // hands
    class_addmethod (
        ultraleap_class,
        (t_method) ultraleapSetHandTypeFlag,
        gensym ("hand_type"),
        A_DEFFLOAT,
        A_NULL
    );

    class_addmethod (
        ultraleap_class,
        (t_method) ultraleapSetHandPalmDirectionFlag,
        gensym ("hand_palm_direction"),
        A_DEFFLOAT,
        A_NULL
    );

    class_addmethod (
        ultraleap_class,
        (t_method) ultraleapSetHandPalmNormalFlag,
        gensym ("hand_palm_normal"),
        A_DEFFLOAT,
        A_NULL
    );

    class_addmethod (
        ultraleap_class,
        (t_method) ultraleapSetHandPalmPositionFlag,
        gensym ("hand_palm_position"),
        A_DEFFLOAT,
        A_NULL
    );

    class_addmethod (
        ultraleap_class,
        (t_method) ultraleapSetHandPalmVelocityFlag,
        gensym ("hand_palm_velocity"),
        A_DEFFLOAT,
        A_NULL
    );

    class_addmethod (
        ultraleap_class,
        (t_method) ultraleapSetHandFingerCountFlag,
        gensym ("hand_finger_count"),
        A_DEFFLOAT,
        A_NULL
    );


    // fingers
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

static void ultraleapSetHandPalmDirectionFlag (t_ultraleap* x, t_float state)
{
    state = (state < 0.0) ? 0.0 : state;
    state = (state > 1.0) ? 1.0 : state;

    x->x_handPalmDirectionFlag = state;
}

static void ultraleapSetHandPalmNormalFlag (t_ultraleap* x, t_float state)
{
    state = (state < 0.0) ? 0.0 : state;
    state = (state > 1.0) ? 1.0 : state;

    x->x_handPalmNormalFlag = state;
}

static void ultraleapSetHandPalmPositionFlag (t_ultraleap* x, t_float state)
{
    state = (state < 0.0) ? 0.0 : state;
    state = (state > 1.0) ? 1.0 : state;

    x->x_handPalmPositionFlag = state;
}

static void ultraleapSetHandPalmVelocityFlag (t_ultraleap* x, t_float state)
{
    state = (state < 0.0) ? 0.0 : state;
    state = (state > 1.0) ? 1.0 : state;

    x->x_handPalmVelocityFlag = state;
}

static void ultraleapSetHandFingerCountFlag (t_ultraleap* x, t_float state)
{
    state = (state < 0.0) ? 0.0 : state;
    state = (state > 1.0) ? 1.0 : state;

    x->x_handFingerCountFlag = state;
}


// set methods: fingers
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


// info method
static void ultraleapInfo (t_ultraleap* x)
{
    post ("\n\n[ultraleap] %s\n", PD_ULTRALEAP_VERSION);

    post ("general: %1.0f\n", x->x_generalFlag);

    post ("hand_type: %1.0f", x->x_handTypeFlag);
    post ("hand_palm_direction: %1.0f", x->x_handPalmDirectionFlag);
    post ("hand_palm_normal: %1.0f", x->x_handPalmNormalFlag);
    post ("hand_palm_position: %1.0f", x->x_handPalmPositionFlag);
    post ("hand_palm_velocity: %1.0f", x->x_handPalmVelocityFlag);
    post ("hand_finger_count: %1.0f\n", x->x_handFingerCountFlag);

    post ("finger_direction: %1.0f", x->x_fingerDirectionFlag);
    post ("finger_position: %1.0f", x->x_fingerPositionFlag);
    post ("finger_velocity: %1.0f", x->x_fingerVelocityFlag);
    post ("finger_size: %1.0f\n", x->x_fingerSizeFlag);

    LEAP_DEVICE_INFO* deviceProps = GetDeviceProperties();
    if (deviceProps)
      post ("Using device: %s", deviceProps->serial);

    const char* result = SetTrackingMode (0);
    if (result)
      post ("SetTrackingMode() result: %s", result);

    // Requesting the current tracking mode is asynchronous. After you call this function, a subsequent call to LeapPollConnection provides a LEAP_TRACKING_MODE_EVENT containing the current tracking mode, reflecting any changes.
    GetTrackingMode();
}


// poll method
static void ultraleapPoll (t_ultraleap* x)
{
    uint32_t nHands;
    LEAP_TRACKING_EVENT* frame = GetFrame();

    if (frame && (frame->tracking_frame_id > x->x_lastFrameID))
    {
        x->x_lastFrameID = frame->tracking_frame_id;
        nHands = frame->nHands;

        if (nHands > 0)
            ultraleapProcessHands (x, frame);

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

        // post ("thumb: %f, index: %f, middle: %f, ring: %f, pinky: %f", hand->thumb.distal.next_joint.x, hand->index.distal.next_joint.x, hand->middle.distal.next_joint.x, hand->ring.distal.next_joint.x, hand->pinky.distal.next_joint.x);

/*
        // from LeapSDK/docs/cpp/devguide/Leap_Pointables.html: "As of version 2.0 of the Leap Motion SDK, all five fingers are are always present in the list of fingers for a hand."
        // therefore, Leap::FingerList.count() will not work as it did in the previous SDK. can use Leap::Finger.isExtended() to determine which ones are actually out.
        // numFingersPerHand = fingerList.count();
*/
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

        // TODO: add hand_visible_time flag
        SETSYMBOL (&handInfo[1], gensym ("visible_time"));
        SETFLOAT (&handInfo[2], (t_float) (hand.visible_time / (t_float)1000000));

        outlet_list (x->x_outletHands, 0, numHandInfoAtoms - 3, handInfo);

        // TODO: add hand_pinch_distance flag
        SETSYMBOL (&handInfo[1], gensym ("pinch_distance"));
        SETFLOAT (&handInfo[2], hand.pinch_distance);

        outlet_list (x->x_outletHands, 0, numHandInfoAtoms - 3, handInfo);

        // TODO: what's the difference between _distance and _strength?
        // strength is normalized 0..1, distance is a measurement in mm
        SETSYMBOL (&handInfo[1], gensym ("pinch_strength"));
        SETFLOAT (&handInfo[2], hand.pinch_strength);

        outlet_list (x->x_outletHands, 0, numHandInfoAtoms - 3, handInfo);

        // TODO: add hand_grab_angle flag
        SETSYMBOL (&handInfo[1], gensym ("grab_angle"));
        SETFLOAT (&handInfo[2], hand.grab_angle);

        outlet_list (x->x_outletHands, 0, numHandInfoAtoms - 3, handInfo);

        // TODO: what's the difference between grab_angle and grab_strength?
        // angle is a measure of the grab angle in radians. strength is normalized 0..1
        SETSYMBOL (&handInfo[1], gensym ("grab_strength"));
        SETFLOAT (&handInfo[2], hand.grab_strength);

        outlet_list (x->x_outletHands, 0, numHandInfoAtoms - 3, handInfo);

        // PALM
        // direction
        if (x->x_handPalmDirectionFlag)
        {
            SETSYMBOL (&handInfo[1], gensym ("palm"));
            SETSYMBOL (&handInfo[2], gensym ("direction"));
            SETFLOAT (&handInfo[3], hand.palm.direction.x);
            SETFLOAT (&handInfo[4], hand.palm.direction.y);
            SETFLOAT (&handInfo[5], hand.palm.direction.z);

            outlet_list (x->x_outletHands, 0, numHandInfoAtoms, handInfo);
        }

        // palm normal
        if (x->x_handPalmNormalFlag)
        {
            SETSYMBOL (&handInfo[1], gensym ("palm"));
            SETSYMBOL (&handInfo[2], gensym ("normal"));
            SETFLOAT (&handInfo[3], hand.palm.normal.x);
            SETFLOAT (&handInfo[4], hand.palm.normal.y);
            SETFLOAT (&handInfo[5], hand.palm.normal.z);

            outlet_list (x->x_outletHands, 0, numHandInfoAtoms, handInfo);
        }

        // palm position
        if (x->x_handPalmPositionFlag)
        {
            SETSYMBOL (&handInfo[1], gensym ("palm"));
            SETSYMBOL (&handInfo[2], gensym ("position"));
            SETFLOAT (&handInfo[3], hand.palm.position.x);
            SETFLOAT (&handInfo[4], hand.palm.position.y);
            SETFLOAT (&handInfo[5], hand.palm.position.z);

            outlet_list (x->x_outletHands, 0, numHandInfoAtoms, handInfo);
        }

        // palm velocity
        if (x->x_handPalmVelocityFlag)
        {
            SETSYMBOL (&handInfo[1], gensym ("palm"));
            SETSYMBOL (&handInfo[2], gensym ("velocity"));
            SETFLOAT (&handInfo[3], hand.palm.velocity.x);
            SETFLOAT (&handInfo[4], hand.palm.velocity.y);
            SETFLOAT (&handInfo[5], hand.palm.velocity.z);

            outlet_list (x->x_outletHands, 0, numHandInfoAtoms, handInfo);
        }

        // TODO: add hand_palm_width flag
        SETSYMBOL (&handInfo[1], gensym ("palm"));
        SETSYMBOL (&handInfo[2], gensym ("width"));
        SETFLOAT (&handInfo[3], hand.palm.width);

        outlet_list (x->x_outletHands, 0, numHandInfoAtoms - 2, handInfo);

        // TODO: add hand_palm_stabilized_position flag
        SETSYMBOL (&handInfo[1], gensym ("palm"));
        SETSYMBOL (&handInfo[2], gensym ("stabilized_position"));
        SETFLOAT (&handInfo[3], hand.palm.stabilized_position.x);
        SETFLOAT (&handInfo[4], hand.palm.stabilized_position.y);
        SETFLOAT (&handInfo[5], hand.palm.stabilized_position.z);

        outlet_list (x->x_outletHands, 0, numHandInfoAtoms, handInfo);

        // process fingers per hand
        ultraleapProcessFingers (x, handIdx, fingerList);
    }
}

// process fingers
static void ultraleapProcessFingers (t_ultraleap* x, int handIdx, LEAP_DIGIT* fingerList)
{
    for (int fingerIdx = 0; fingerIdx < NUM_FINGERS; fingerIdx++)
    {
        LEAP_DIGIT finger = fingerList[fingerIdx];
        int numFingerInfoAtoms = 7;
        t_atom fingerInfo[numFingerInfoAtoms];

        SETFLOAT (&fingerInfo[0], handIdx);

        // TODO: is the finger_id the same as the type? (i.e., 0 = thumb, 4 = pinky)

        // TODO: how do we get the direction of a finger or bone? see the Cxx solution
        /*
        if (x->x_fingerDirectionFlag)
        {
            SETSYMBOL (&fingerInfo[1], gensym ("finger"));
            SETFLOAT (&fingerInfo[2], finger.finger_id);
            SETSYMBOL (&fingerInfo[3], gensym ("direction"));
            SETFLOAT (&fingerInfo[4], finger.direction().x);
            SETFLOAT (&fingerInfo[5], finger.direction().y);
            SETFLOAT (&fingerInfo[6], finger.direction().z);

            outlet_anything (x->x_outletHands, gensym ("hand"), numFingerInfoAtoms, fingerInfo);
        }
        */

        if (x->x_fingerPositionFlag)
        {
            // each finger has 4 bones. the distal bone is the tip. each bone has two joints/ends. next_joint is the end further from the body
            LEAP_BONE distalBone = finger.distal;

            SETSYMBOL (&fingerInfo[1], gensym ("finger"));
            // finger_id seems to be consistent, where thumb = 0, index = 1, middle = 2, ring = 3, and pinky = 4
            SETFLOAT (&fingerInfo[2], finger.finger_id);
            SETSYMBOL (&fingerInfo[3], gensym ("position"));
            SETFLOAT (&fingerInfo[4], distalBone.next_joint.x);
            SETFLOAT (&fingerInfo[5], distalBone.next_joint.y);
            SETFLOAT (&fingerInfo[6], distalBone.next_joint.z);

            outlet_list (x->x_outletHands, 0, numFingerInfoAtoms, fingerInfo);
        }

        // TODO: how do we get the velocity of a finger or bone? see the Cxx solution
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

        // TODO: how do we get the width/length of a finger or bone? see the Cxx solution
        if (x->x_fingerSizeFlag)
        {
            SETFLOAT (&fingerInfo[0], handIdx);
            SETSYMBOL (&fingerInfo[1], gensym ("finger"));
            SETFLOAT (&fingerInfo[2], finger.type());
            SETSYMBOL (&fingerInfo[3], gensym ("size"));
            SETFLOAT (&fingerInfo[4], finger.width());
            SETFLOAT (&fingerInfo[5], finger.length());

            outlet_anything (x->x_outletHands, gensym ("hand"), numFingerInfoAtoms - 1, fingerInfo);
        }
*/
        // TODO: add is_extended flag
        SETSYMBOL (&fingerInfo[1], gensym ("finger"));
        // finger_id seems to be consistent, where thumb = 0, index = 1, middle = 2, ring = 3, and pinky = 4
        SETFLOAT (&fingerInfo[2], finger.finger_id);
        SETSYMBOL (&fingerInfo[3], gensym ("is_extended"));
        SETFLOAT (&fingerInfo[4], finger.is_extended);
        outlet_list (x->x_outletHands, 0, numFingerInfoAtoms - 2, fingerInfo);
    }
}

// process general data
static void ultraleapProcessGeneral (t_ultraleap* x, LEAP_TRACKING_EVENT* frame)
{
    int numGeneralInfoAtoms = 4;
    t_atom generalInfo[numGeneralInfoAtoms];

    SETFLOAT (&generalInfo[0], (t_float) frame->tracking_frame_id);
    SETFLOAT (&generalInfo[1], (t_float) (frame->info.timestamp / (t_float)1000000));
    SETFLOAT (&generalInfo[2], (t_float) frame->framerate);
    SETFLOAT (&generalInfo[3], (t_float) frame->nHands);

    outlet_list (x->x_outletGeneral, 0, numGeneralInfoAtoms, &generalInfo[0]);
}


// callback functions
// callback for when the connection opens
static void ultraleap_onConnect (void)
{
    post ("Leap Controller: Connected");
}

// callback for when tracking mode changes
static void ultraleap_onTrackingMode (const LEAP_TRACKING_MODE_EVENT* mode_event)
{
    const char* trackingMode = eLeapTrackingModeString (mode_event->current_tracking_mode);
    post ("Leap Tracking Mode: %s", trackingMode);
}
