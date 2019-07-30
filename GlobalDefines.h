//Defines accross the entire project

//#define GET_PERFORMANCE_TIME

//color text
#define RED_COLOR_TEXT printf("\033[31m");
#define GREEN_COLOR_TEXT printf("\033[32m");
#define BLUE_COLOR_TEXT printf("\033[34m");
#define BLACK_COLOR_TEXT printf("\033[m");

//#define AS_FAST_AS_POSSIBLE

#define USE_LOCAL_EVENTS

//#define ALLOW_HURRY_UP

//#define USE_EVENT_DEBUG_MSG

//#define USE_CGLX_CULLING

//#define USE_CCLIVE

#define USE_TVM_DECODING
#define NUM_TVM_THREADS 1
#define VIDS_PER_THREAD 1000
#define DEFAULT_NUM_UPLOADED_PER_EVENT VIDS_PER_THREAD

#define SEEK_BUFFER_OFFSET_TIME 0.0


//for speeding up decoding when lots of vids are on the screen
//#define HURREY_UP_WHEN_LONG_LIST
//#define HURRY_UP_LONG_LIST_SIZE 10

//#define HURRY_UP_WHEN_BEHIND

#define SEEK_WHEN_OFF
#define SEEK_WHEN_OFF_BY_SEC 1
