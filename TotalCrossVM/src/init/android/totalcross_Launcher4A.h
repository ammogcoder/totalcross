/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class totalcross_Launcher4A */

#ifndef _Included_totalcross_Launcher4A
#define _Included_totalcross_Launcher4A
#ifdef __cplusplus
extern "C" {
#endif
#undef totalcross_Launcher4A_PEN_DOWN
#define totalcross_Launcher4A_PEN_DOWN 1L
#undef totalcross_Launcher4A_PEN_UP
#define totalcross_Launcher4A_PEN_UP 2L
#undef totalcross_Launcher4A_PEN_DRAG
#define totalcross_Launcher4A_PEN_DRAG 3L
#undef totalcross_Launcher4A_KEY_PRESS
#define totalcross_Launcher4A_KEY_PRESS 4L
#undef totalcross_Launcher4A_STOPVM_EVENT
#define totalcross_Launcher4A_STOPVM_EVENT 5L
#undef totalcross_Launcher4A_APP_PAUSED
#define totalcross_Launcher4A_APP_PAUSED 6L
#undef totalcross_Launcher4A_APP_RESUMED
#define totalcross_Launcher4A_APP_RESUMED 7L
#undef totalcross_Launcher4A_SCREEN_CHANGED
#define totalcross_Launcher4A_SCREEN_CHANGED 8L
#undef totalcross_Launcher4A_SIP_CLOSED
#define totalcross_Launcher4A_SIP_CLOSED 9L
#undef totalcross_Launcher4A_MULTITOUCHEVENT_SCALE
#define totalcross_Launcher4A_MULTITOUCHEVENT_SCALE 10L
#undef totalcross_Launcher4A_BARCODE_READ
#define totalcross_Launcher4A_BARCODE_READ 11L
#undef totalcross_Launcher4A_TRANSITION_NONE
#define totalcross_Launcher4A_TRANSITION_NONE 0L
#undef totalcross_Launcher4A_TRANSITION_OPEN
#define totalcross_Launcher4A_TRANSITION_OPEN 1L
#undef totalcross_Launcher4A_TRANSITION_CLOSE
#define totalcross_Launcher4A_TRANSITION_CLOSE 2L
#undef totalcross_Launcher4A_SOFT_EXIT
#define totalcross_Launcher4A_SOFT_EXIT 1073741824L
#undef totalcross_Launcher4A_SIP_HIDE
#define totalcross_Launcher4A_SIP_HIDE 10000L
#undef totalcross_Launcher4A_SIP_TOP
#define totalcross_Launcher4A_SIP_TOP 10001L
#undef totalcross_Launcher4A_SIP_BOTTOM
#define totalcross_Launcher4A_SIP_BOTTOM 10002L
#undef totalcross_Launcher4A_SIP_SHOW
#define totalcross_Launcher4A_SIP_SHOW 10003L
#undef totalcross_Launcher4A_SIP_ENABLE_NUMERICPAD
#define totalcross_Launcher4A_SIP_ENABLE_NUMERICPAD 10004L
#undef totalcross_Launcher4A_SIP_DISABLE_NUMERICPAD
#define totalcross_Launcher4A_SIP_DISABLE_NUMERICPAD 10005L
#undef totalcross_Launcher4A_REBOOT_DEVICE
#define totalcross_Launcher4A_REBOOT_DEVICE 1L
#undef totalcross_Launcher4A_SET_AUTO_OFF
#define totalcross_Launcher4A_SET_AUTO_OFF 2L
#undef totalcross_Launcher4A_SHOW_KEY_CODES
#define totalcross_Launcher4A_SHOW_KEY_CODES 3L
#undef totalcross_Launcher4A_GET_REMAINING_BATTERY
#define totalcross_Launcher4A_GET_REMAINING_BATTERY 4L
#undef totalcross_Launcher4A_IS_KEY_DOWN
#define totalcross_Launcher4A_IS_KEY_DOWN 5L
#undef totalcross_Launcher4A_TURN_SCREEN_ON
#define totalcross_Launcher4A_TURN_SCREEN_ON 6L
#undef totalcross_Launcher4A_GPSFUNC_START
#define totalcross_Launcher4A_GPSFUNC_START 7L
#undef totalcross_Launcher4A_GPSFUNC_STOP
#define totalcross_Launcher4A_GPSFUNC_STOP 8L
#undef totalcross_Launcher4A_GPSFUNC_GETDATA
#define totalcross_Launcher4A_GPSFUNC_GETDATA 9L
#undef totalcross_Launcher4A_CELLFUNC_START
#define totalcross_Launcher4A_CELLFUNC_START 10L
#undef totalcross_Launcher4A_CELLFUNC_STOP
#define totalcross_Launcher4A_CELLFUNC_STOP 11L
#undef totalcross_Launcher4A_VIBRATE
#define totalcross_Launcher4A_VIBRATE 12L
#undef totalcross_Launcher4A_CLIPBOARD
#define totalcross_Launcher4A_CLIPBOARD 13L
/*
 * Class:     totalcross_Launcher4A
 * Method:    pictureTaken
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_totalcross_Launcher4A_pictureTaken
  (JNIEnv *, jclass, jint);

/*
 * Class:     totalcross_Launcher4A
 * Method:    initializeVM
 * Signature: (Landroid/content/Context;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_totalcross_Launcher4A_initializeVM
  (JNIEnv *, jobject, jobject, jstring, jstring, jstring, jstring);

/*
 * Class:     totalcross_Launcher4A
 * Method:    nativeInitSize
 * Signature: (Landroid/graphics/Bitmap;)V
 */
JNIEXPORT void JNICALL Java_totalcross_Launcher4A_nativeInitSize
  (JNIEnv *, jobject, jint, jint);

/*
 * Class:     totalcross_Launcher4A
 * Method:    nativeOnEvent
 * Signature: (IIIIII)V
 */
JNIEXPORT void JNICALL Java_totalcross_Launcher4A_nativeOnEvent
  (JNIEnv *, jobject, jint, jint, jint, jint, jint, jint);

#ifdef __cplusplus
}
#endif
#endif
