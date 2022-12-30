#include "m_pd.h"
#include "LeapC.h"
#include "leapConnection.h"

#define PD_ULTRALEAP_VERSION "version 0.1.0"
#define NUM_FINGERS 5

static t_class* ultraleap_class;

typedef struct _ultraleap
{
    t_object x_obj;
    t_symbol* x_objSymbol;

    int64_t x_lastFrameID;

    t_float x_handTypeFlag;
    t_float x_handPalmDirectionFlag;
    t_float x_handPalmNormalFlag;
    t_float x_handPalmPositionFlag;
    t_float x_handPalmVelocityFlag;
    t_float x_handFingerCountFlag;

    t_float x_fingerDirectionFlag;
    t_float x_fingerPositionFlag;
    t_float x_fingerVelocityFlag;
    t_float x_fingerSizeFlag;
    // TODO: add fingerIsExtendedFlag

    t_float x_generalFlag;

    t_outlet* x_outletGeneral;
    t_outlet* x_outletHands;
} t_ultraleap;


// object setup functions
static void* ultraleap_new (t_symbol* s, int argc, t_atom* argv);
static void ultraleap_free (t_ultraleap* x);
void ultraleap_setup (void);

// set methods: general
static void ultraleapSetGeneralFlag (t_ultraleap* x, t_float state);

// set methods: hand
static void ultraleapSetHandTypeFlag (t_ultraleap* x, t_float state);
static void ultraleapSetHandPalmDirectionFlag (t_ultraleap* x, t_float state);
static void ultraleapSetHandPalmNormalFlag (t_ultraleap* x, t_float state);
static void ultraleapSetHandPalmPositionFlag (t_ultraleap* x, t_float state);
static void ultraleapSetHandPalmVelocityFlag (t_ultraleap* x, t_float state);
static void ultraleapSetHandFingerCountFlag (t_ultraleap* x, t_float state);

// set methods: fingers
static void ultraleapSetFingerDirectionFlag (t_ultraleap* x, t_float state);
static void ultraleapSetFingerPositionFlag (t_ultraleap* x, t_float state);
static void ultraleapSetFingerVelocityFlag (t_ultraleap* x, t_float state);
static void ultraleapSetFingerSizeFlag (t_ultraleap* x, t_float state);

// post object info
static void ultraleapInfo (t_ultraleap* x);

// leapConnection callback functions
static void ultraleap_onConnect (void);
static void ultraleap_onTrackingMode (const LEAP_TRACKING_MODE_EVENT* mode_event);
// static void OnDevice (const LEAP_DEVICE_INFO *props);
// static void OnFrame (const LEAP_TRACKING_EVENT *frame);

// poll Leap device
static void ultraleapPoll (t_ultraleap* x);

// sub-routinese to extract data from a Leap::Frame
static void ultraleapProcessHands (t_ultraleap* x, LEAP_TRACKING_EVENT* frame);
static void ultraleapProcessFingers (t_ultraleap* x, int handIdx, LEAP_DIGIT* fingerList);
static void ultraleapProcessGeneral(t_ultraleap* x, LEAP_TRACKING_EVENT* frame);
