#include <math.h>
#include "m_pd.h"
#include "LeapC.h"
#include "leapConnection.h"

#define PD_ULTRALEAP_VERSION "0.1.0"
#define PD_ULTRALEAP_RELEASE_DATE "Dec 2022"

#define NUM_FINGERS 5
#define EPSILON 1.192092896e-07f

static t_class* ultraleap_class;

typedef struct _ultraleap
{
    t_object x_obj;
    t_symbol* x_objSymbol;

    LEAP_CONNECTION* x_leapConnection;
    LEAP_TRACKING_EVENT* x_leapFrame;
    LEAP_VECTOR x_fingerVecs[NUM_FINGERS];
    int64_t x_lastFrameID;
    int64_t x_timeStampReference;

    t_float x_generalFlag;

    t_float x_handTypeFlag;
    t_float x_handFingerCountFlag;
    t_float x_handTimeVisibleFlag;
    t_float x_handGrabStrengthFlag;
    t_float x_handGrabAngleFlag;
    t_float x_handPinchStrengthFlag;
    t_float x_handPinchDistanceFlag;

    t_float x_armCenterFlag;
    t_float x_armDirectionFlag;
    t_float x_armWristPositionFlag;
    t_float x_armElbowPositionFlag;
    t_float x_armWidthFlag;

    t_float x_palmDirectionFlag;
    t_float x_palmNormalFlag;
    t_float x_palmPositionFlag;
    t_float x_palmStabilizedPositionFlag;
    t_float x_palmVelocityFlag;
    t_float x_palmWidthFlag;

    t_float x_fingerTypeFlag;
    t_float x_fingerDirectionFlag;
    t_float x_fingerPositionFlag;
    t_float x_fingerVelocityFlag;
    t_float x_fingerSizeFlag;
    t_float x_fingerIsExtendedFlag;

    t_outlet* x_outletGeneral;
    t_outlet* x_outletHands;
} t_ultraleap;


// object setup functions
static void* ultraleap_new (t_symbol* s, int argc, t_atom* argv);
static void ultraleap_free (t_ultraleap* x);
void ultraleap_setup (void);

// utility functions
static t_float ultraleapGetVectorMagnitude (LEAP_VECTOR v);
static t_float ultraleapGetEuclideanDistance (LEAP_VECTOR a, LEAP_VECTOR b);
static LEAP_VECTOR ultraleapGetVectorDiff (LEAP_VECTOR a, LEAP_VECTOR b);
static LEAP_VECTOR ultraleapGetVectorCentroid (LEAP_VECTOR a, LEAP_VECTOR b);
static LEAP_VECTOR ultraleapNormalizeVector (LEAP_VECTOR v);

// timestamp_reset

// set methods: mode
static void ultraleapSetTrackingMode (t_ultraleap* x, t_symbol* m);

// set methods: general
static void ultraleapSetGeneralFlag (t_ultraleap* x, t_float state);

// set methods: hand
static void ultraleapSetHandTypeFlag (t_ultraleap* x, t_float state);
static void ultraleapSetHandFingerCountFlag (t_ultraleap* x, t_float state);
static void ultraleapSetHandTimeVisibleFlag (t_ultraleap* x, t_float state);
static void ultraleapSetHandGrabStrengthFlag (t_ultraleap* x, t_float state);
static void ultraleapSetHandGrabAngleFlag (t_ultraleap* x, t_float state);
static void ultraleapSetHandPinchStrengthFlag (t_ultraleap* x, t_float state);
static void ultraleapSetHandPinchDistanceFlag (t_ultraleap* x, t_float state);

// set methods: arm
static void ultraleapSetArmCenterFlag (t_ultraleap* x, t_float state);
static void ultraleapSetArmDirectionFlag (t_ultraleap* x, t_float state);
static void ultraleapSetArmWristPositionFlag (t_ultraleap* x, t_float state);
static void ultraleapSetArmElbowPositionFlag (t_ultraleap* x, t_float state);
static void ultraleapSetArmWidthFlag (t_ultraleap* x, t_float state);

// set methods: palm
static void ultraleapSetPalmDirectionFlag (t_ultraleap* x, t_float state);
static void ultraleapSetPalmNormalFlag (t_ultraleap* x, t_float state);
static void ultraleapSetPalmPositionFlag (t_ultraleap* x, t_float state);
static void ultraleapSetPalmStabilizedPositionFlag (t_ultraleap* x, t_float state);
static void ultraleapSetPalmVelocityFlag (t_ultraleap* x, t_float state);
static void ultraleapSetPalmWidthFlag (t_ultraleap* x, t_float state);

// set methods: fingers
static void ultraleapSetFingerTypeFlag (t_ultraleap* x, t_float state);
static void ultraleapSetFingerDirectionFlag (t_ultraleap* x, t_float state);
static void ultraleapSetFingerPositionFlag (t_ultraleap* x, t_float state);
static void ultraleapSetFingerVelocityFlag (t_ultraleap* x, t_float state);
static void ultraleapSetFingerSizeFlag (t_ultraleap* x, t_float state);
static void ultraleapSetFingerIsExtendedFlag (t_ultraleap* x, t_float state);

// post object info
static void ultraleapInfo (t_ultraleap* x);

// reset time stamp reference point
static void ultraleapResetTimeStamp (t_ultraleap* x);

// poll Leap device
static void ultraleapPoll (t_ultraleap* x);

// sub-routines to extract data from a Leap::Frame
static void ultraleapProcessHands (t_ultraleap* x);
static void ultraleapProcessArms (t_ultraleap* x);
static void ultraleapProcessFingers (t_ultraleap* x, int handIdx, LEAP_DIGIT* fingerList);
static void ultraleapProcessGeneral(t_ultraleap* x);

// leapConnection callback functions
static void ultraleap_onConnect (void);
static void ultraleap_onTrackingMode (const LEAP_TRACKING_MODE_EVENT* mode_event);
static void ultraleap_onDevice (const LEAP_DEVICE_INFO* props);
// static void OnFrame (const LEAP_TRACKING_EVENT *frame);
