package com.sec.android.seccamera;

import java.util.*;

import android.graphics.*;

import java.io.*;
import java.lang.ref.WeakReference;

import android.view.*;
import android.os.*;
import android.util.*;

public class SecCamera {
	public static final String ACTION_NEW_PICTURE = "android.hardware.action.NEW_PICTURE";
	public static final String ACTION_NEW_VIDEO = "android.hardware.action.NEW_VIDEO";
	private static final int ACTION_SHOT_CANCELSERIES = 1031;
	private static final int ACTION_SHOT_CAPTURED = 61509;
	private static final int ACTION_SHOT_CREATING_RESULT_COMPLETED = 61507;
	private static final int ACTION_SHOT_CREATING_RESULT_PROGRESS = 61506;
	private static final int ACTION_SHOT_CREATING_RESULT_STARTED = 61505;
	private static final int ACTION_SHOT_FINALIZE = 1033;
	private static final int ACTION_SHOT_INITIALIZE = 1028;
	private static final int ACTION_SHOT_PROGRESS_ACQUISITION = 61508;
	private static final int ACTION_SHOT_RECT = 61510;
	private static final int ACTION_SHOT_SETRESOLUTION = 1029;
	private static final int ACTION_SHOT_STARTSERIES = 1030;
	private static final int ACTION_SHOT_STOPSERIES = 1032;
	private static final int ADDME_SHOT_CANCEL_CAPTURE = 1038;
	private static final int ADDME_SHOT_CAPUTRED_FIRSTPERSON = 61522;
	private static final int ADDME_SHOT_ERR = 61521;
	private static final int ADDME_SHOT_FINALIZE = 1039;
	private static final int ADDME_SHOT_HANDLE_SNAPSHOT = 1037;
	private static final int ADDME_SHOT_INIT = 1034;
	private static final int ADDME_SHOT_PROGRESS_STITCHING = 61523;
	private static final int ADDME_SHOT_START_CAPTURE = 1035;
	private static final int ADDME_SHOT_SWITCH_POSITION = 1036;
	private static final int BURST_SHOT_CAPTURE = 1044;
	private static final int BURST_SHOT_CAPTURING_PROGRESSED = 61585;
	private static final int BURST_SHOT_CAPTURING_STOPPED = 61586;
	private static final int BURST_SHOT_SAVING_COMPLETED = 61587;
	private static final int BURST_SHOT_START_CAPTURE = 1045;
	private static final int BURST_SHOT_STOP_AND_ENCODING = 1046;
	private static final int BURST_SHOT_STORING = 1043;
	private static final int BURST_SHOT_TERMINATE = 1047;
	public static final int CAMERA_ERROR_PREVIEWFRAME_TIMEOUT = 1001;
	public static final int CAMERA_ERROR_PRIORITY_DIED = 200;
	public static final int CAMERA_ERROR_SERVER_DIED = 100;
	public static final int CAMERA_ERROR_UNKNOWN = 1;
	private static final int CAMERA_FACE_DETECTION_HW = 0;
	private static final int CAMERA_FACE_DETECTION_SW = 1;
	private static final int CAMERA_MSG_ALL_MSGS = 1279;
	private static final int CAMERA_MSG_COMPRESSED_IMAGE = 256;
	private static final int CAMERA_MSG_ERROR = 1;
	private static final int CAMERA_MSG_FOCUS = 4;
	private static final int CAMERA_MSG_POSTVIEW_FRAME = 64;
	private static final int CAMERA_MSG_PREVIEW_FRAME = 16;
	private static final int CAMERA_MSG_PREVIEW_METADATA = 1024;
	private static final int CAMERA_MSG_RAW_IMAGE = 128;
	private static final int CAMERA_MSG_RAW_IMAGE_NOTIFY = 512;
	private static final int CAMERA_MSG_SHUTTER = 2;
	private static final int CAMERA_MSG_VIDEO_FRAME = 32;
	private static final int CAMERA_MSG_ZOOM = 8;
	private static final int CARTOON_SHOT_PROGRESS_RENDERING = 61553;
	private static final int CARTOON_SHOT_SELECT_MODE = 1040;
	private static final int CHECK_MARKER_OF_SAMSUNG_DEFINED_CALLBACK_MSGS = 61440;
	private static final int CONTINUOUS_SHOT_CAPTURING_PROGRESSED = 61489;
	private static final int CONTINUOUS_SHOT_CAPTURING_STOPPED = 61490;
	private static final int CONTINUOUS_SHOT_SAVING_COMPLETED = 61491;
	private static final int CONTINUOUS_SHOT_SOUND = 1027;
	private static final int CONTINUOUS_SHOT_START_CAPTURE = 1024;
	private static final int CONTINUOUS_SHOT_STOP_AND_ENCODING = 1025;
	private static final int CONTINUOUS_SHOT_TERMINATE = 1026;
	private static final int HAL_AE_AWB_LOCK_UNLOCK = 1101;
	private static final int HAL_DISABLE_POSTVIEW_TO_OVERLAY = 1109;
	private static final int HAL_DONE_CHK_DATALINE = 61442;
	private static final int HAL_FACE_DETECT_LOCK_UNLOCK = 1102;
	private static final int HAL_MSG_OBJ_TRACKING = 61441;
	private static final int HAL_OBJECT_POSITION = 1103;
	private static final int HAL_OBJECT_TRACKING_STARTSTOP = 1104;
	private static final int HAL_SET_DEFAULT_IMEI = 1107;
	private static final int HAL_SET_FRONT_SENSOR_MIRROR = 1110;
	private static final int HAL_SET_SAMSUNG_CAMERA = 1108;
	private static final int HAL_STOP_CHK_DATALINE = 1106;
	private static final int HAL_TOUCH_AF_STARTSTOP = 1105;
	private static final int HDR_SHOT_ALL_PROGRESS_COMPLETED = 61572;
	private static final int HDR_SHOT_RESULT_COMPLETED = 61571;
	private static final int HDR_SHOT_RESULT_PROGRESS = 61570;
	private static final int HDR_SHOT_RESULT_STARTED = 61569;
	public static final int PANORAMA_DIRECTION_DOWN = 8;
	public static final int PANORAMA_DIRECTION_DOWN_LEFT = 10;
	public static final int PANORAMA_DIRECTION_DOWN_RIGHT = 9;
	public static final int PANORAMA_DIRECTION_LEFT = 2;
	public static final int PANORAMA_DIRECTION_RIGHT = 1;
	public static final int PANORAMA_DIRECTION_UP = 4;
	public static final int PANORAMA_DIRECTION_UP_LEFT = 6;
	public static final int PANORAMA_DIRECTION_UP_RIGHT = 5;
	private static final int PANORAMA_SHOT_CANCEL = 1023;
	private static final int PANORAMA_SHOT_CAPTURED = 61477;
	private static final int PANORAMA_SHOT_CAPTURED_NEW = 61475;
	private static final int PANORAMA_SHOT_DIR = 61478;
	private static final int PANORAMA_SHOT_ERR = 61473;
	private static final int PANORAMA_SHOT_FINALIZE = 1022;
	private static final int PANORAMA_SHOT_PROGRESS_STITCHING = 61476;
	private static final int PANORAMA_SHOT_RECT_CENTER_POINT = 61474;
	private static final int PANORAMA_SHOT_START = 1020;
	private static final int PANORAMA_SHOT_STOP = 1021;
	private static final int SAMSUNG_SHOT_COMPRESSED_IMAGE = 61697;
	private static final int SET_DISPLAY_ORIENTATION_MIRROR = 1042;
	private static final int SHOT_ACTION = 1010;
	private static final int SHOT_ADDME = 1009;
	private static final int SHOT_BEAUTY = 1007;
	private static final int SHOT_BURST = 1017;
	private static final int SHOT_CARTOON = 1013;
	private static final int SHOT_CONTINUOUS = 1001;
	private static final int SHOT_FACEEFFECT = 1015;
	private static final int SHOT_FRAME = 1005;
	private static final int SHOT_HDR = 1014;
	private static final int SHOT_MOSAIC = 1004;
	private static final int SHOT_PANORAMA = 1002;
	private static final int SHOT_PARTY = 1012;
	private static final int SHOT_SELF = 1006;
	private static final int SHOT_SINGLE = 1000;
	private static final int SHOT_SMARTAF = 1016;
	private static final int SHOT_SMILE = 1003;
	private static final int SHOT_STOPMOTION = 1011;
	private static final int SHOT_VINTAGE = 1008;
	private static final int SMILE_SHOT_DETECTION_REINIT = 1041;
	private static final int SMILE_SHOT_DETECTION_START = 1018;
	private static final int SMILE_SHOT_DETECTION_STOP = 1019;
	private static final int SMILE_SHOT_DETECTION_SUCCESS = 61537;
	private static final int SMILE_SHOT_FACE_RECT = 61538;
	private static final int SMILE_SHOT_SMILE_RECT = 61539;
	private static final String TAG = "SecCamera-JNI-Java";
	private int mAGifMaker;
	private int mArcBlinkShot;
	private int mArcFrameShot;
	private int mArcMosaicShot;
	private AutoFocusCallback mAutoFocusCallback;
	private ErrorCallback mErrorCallback;
	private EventHandler mEventHandler;
	private boolean mFaceDetectionRunning;
	private FaceDetectionListener mFaceListener;
	private PictureCallback mJpegCallback;
	private int mNativeContext;
	private OnActionShotEventListener mOnActionShotEventListener;
	private OnAddMeEventListener mOnAddMeEventListener;
	private OnBlinkDetectionEventListener mOnBlinkDetectionEventListener;
	private OnBurstShotEventListener mOnBurstShotEventListener;
	private OnCartoonShotEventListener mOnCartoonShotEventListener;
	private OnChkDataLineListener mOnChkDataLineListener;
	private OnContinuousShotEventListener mOnContinuousShotEventListener;
	private OnHDRShotEventListener mOnHDRShotEventListener;
	private OnObjectTrackingMsgListener mOnObjectTrackingMsgListener;
	private OnPanoramaEventListener mOnPanoramaEventListener;
	private OnSmileShotDetectionSuccessListener mOnSmileShotDetectionSuccessListener;
	private boolean mOneShot;
	private PictureCallback mPostviewCallback;
	private PreviewCallback mPreviewCallback;
	private PictureCallback mRawImageCallback;
	private ShutterCallback mShutterCallback;
	private boolean mWithBuffer;
	private OnZoomChangeListener mZoomListener;

	static {
		System.loadLibrary("seccamera_jni");
	}

	SecCamera(final int n, final int n2) {
		super();
		this.mFaceDetectionRunning = false;
		this.mOnPanoramaEventListener = null;
		this.mOnContinuousShotEventListener = null;
		this.mOnBurstShotEventListener = null;
		this.mOnAddMeEventListener = null;
		this.mOnActionShotEventListener = null;
		this.mOnBlinkDetectionEventListener = null;
		this.mOnCartoonShotEventListener = null;
		this.mOnObjectTrackingMsgListener = null;
		this.mOnChkDataLineListener = null;
		this.mOnSmileShotDetectionSuccessListener = null;
		this.mOnHDRShotEventListener = null;
		Log.e("SecCamera-JNI-Java", "SecCamera()");
		this.mShutterCallback = null;
		this.mRawImageCallback = null;
		this.mJpegCallback = null;
		this.mPreviewCallback = null;
		this.mPostviewCallback = null;
		this.mZoomListener = null;
		final Looper myLooper = Looper.myLooper();
		if (myLooper != null) {
			this.mEventHandler = new EventHandler(this, myLooper);
		} else {
			final Looper mainLooper = Looper.getMainLooper();
			if (mainLooper != null) {
				this.mEventHandler = new EventHandler(this, mainLooper);
			} else {
				this.mEventHandler = null;
			}
		}
		this.native_setup(new WeakReference(this), n, n2);
		this.native_sendcommand(1108, 0, 0);
	}

	private final native void _addCallbackBuffer(final byte[] p0, final int p1);

	private final native void _startFaceDetection(final int p0);

	private final native void _stopFaceDetection();

	private final native void _stopPreview();

	private final void addCallbackBuffer(final byte[] array, final int n) {
		if (n != 16 && n != 128) {
			throw new IllegalArgumentException("Unsupported message type: " + n);
		}
		this._addCallbackBuffer(array, n);
	}

	public static native void getCameraInfo(final int p0, final CameraInfo p1);

	public static native int getNumberOfCameras();

	private final native void native_autoFocus();

	private final native void native_cancelAutoFocus();

	private final native String native_getParameters();

	private final native void native_initializeFrameShot();

	private final native void native_initializeMosaicShot();

	private final native void native_release();

	private final native void native_setParameters(final String p0);

	private final native void native_setup(final Object p0, final int p1,
			final int p2);

	private final native void native_takePicture(final int p0);

	public static SecCamera open() {
		final int numberOfCameras = getNumberOfCameras();
		final CameraInfo cameraInfo = new CameraInfo();
		for (int i = 0; i < numberOfCameras; ++i) {
			getCameraInfo(i, cameraInfo);
			if (cameraInfo.facing == 0) {
				return new SecCamera(i, 100);
			}
		}
		return null;
	}

	public static SecCamera open(final int n) {
		Log.e("SecCamera-JNI-Java", "SecCamera.open()");
		return new SecCamera(n, 100);
	}

	public static SecCamera open(final int n, final int n2) {
		return new SecCamera(n, n2);
	}

	private static void postBlinkRectFromNative(final Object o, final int n,
			final int[] array, final int[] array2, final int[] array3,
			final int[] array4) {
		Log.e("SecCamera-JNI-Java", "postBlinkRectFromNative");
		final SecCamera secCamera = (SecCamera) ((WeakReference) o).get();
		if (secCamera == null) {
			return;
		}
		if (secCamera.mOnBlinkDetectionEventListener != null) {
			secCamera.mOnBlinkDetectionEventListener.onBlinkDetectionEvent(n,
					array, array2, array3, array4);
			return;
		}
		Log.e("SecCamera-JNI-Java", "mOnBlinkDetectionEventListener is null");
	}

	private static void postEventFromNative(final Object o, final int n,
			final int n2, final int n3, final Object o2) {
		final SecCamera secCamera = (SecCamera) ((WeakReference) o).get();
		if (secCamera == null) {
			return;
		}
		if (secCamera.mEventHandler != null) {
			secCamera.mEventHandler.sendMessage(secCamera.mEventHandler
					.obtainMessage(n, n2, n3, o2));
			return;
		}
		Log.e("SecCamera-JNI-Java", "mEventHandler is null");
	}

	private final native void releaseFrameShot();

	private final native void releaseMosaicShot();

	private final native void setHasPreviewCallback(final boolean p0,
			final boolean p1);

	private final native void setPreviewDisplay(final Surface p0)
			throws IOException;

	public final void addCallbackBuffer(final byte[] array) {
		this._addCallbackBuffer(array, 16);
	}

	public final void addRawImageCallbackBuffer(final byte[] array) {
		this.addCallbackBuffer(array, 128);
	}

	public final void autoFocus(final AutoFocusCallback mAutoFocusCallback) {
		this.mAutoFocusCallback = mAutoFocusCallback;
		this.native_autoFocus();
	}

	public final void cancelAutoFocus() {
		this.native_cancelAutoFocus();
	}

	public void cancelCaptureAddMeShot() {
		this.native_sendcommand(1038, 0, 0);
	}

	public void cancelPanorama() {
		this.native_sendcommand(1023, 0, 0);
	}

	public void cancelSeriesActionShot() {
		this.native_sendcommand(1031, 0, 0);
	}

	public void captureBurstShot() {
		this.native_sendcommand(1044, 0, 0);
	}

	public void doSnapAddMeShot() {
		this.native_sendcommand(1037, 0, 0);
	}

	protected void finalize() {
		this.native_release();
	}

	public void finishActionShot() {
		this.native_sendcommand(1033, 0, 0);
	}

	public void finishAddMeShot() {
		this.native_sendcommand(1039, 0, 0);
	}

	public void finishFrameShot() {
		this.releaseFrameShot();
	}

	public void finishMosaicShot() {
	}

	public Parameters getParameters() {
		final Parameters parameters = new Parameters();
		parameters.unflatten(this.native_getParameters());
		return parameters;
	}

	public void initializeActionShot() {
		this.native_sendcommand(1028, 0, 0);
	}

	public void initializeAddMeShot() {
		this.native_sendcommand(1034, 0, 0);
	}

	public void initializeFrameShot() {
		this.native_initializeFrameShot();
	}

	public void initializeMosaicShot() {
		this.native_initializeMosaicShot();
	}

	public final native void lock();

	public void lockFaceDetect() {
		this.native_sendcommand(1102, 1, 0);
	}

	public final native void native_initializeBlinkDetection();

	public final native void native_sendcommand(final int p0, final int p1,
			final int p2);

	public final native boolean previewEnabled();

	public final native void reconnect() throws IOException;

	public final void release() {
		this.native_release();
		this.mFaceDetectionRunning = false;
	}

	public final native void releaseBlinkDetection();

	public void setAEAWBLockState(final int n, final int n2) {
		this.native_sendcommand(1101, n, n2);
	}

	public final void setAutoFocusCb(final AutoFocusCallback mAutoFocusCallback) {
		this.mAutoFocusCallback = mAutoFocusCallback;
	}

	public void setBurstShotStoring() {
		this.native_sendcommand(1043, 0, 0);
	}

	public void setChkDataLineListener(
			final OnChkDataLineListener mOnChkDataLineListener) {
		this.mOnChkDataLineListener = mOnChkDataLineListener;
	}

	public void setContinuousShotSound(final int n) {
		if (n > 0) {
			this.native_sendcommand(1027, n, 0);
			return;
		}
		this.native_sendcommand(1027, 0, 0);
	}

	public void setDefaultIMEI(final int n) {
		this.native_sendcommand(1107, n, 0);
	}

	public final native void setDisplayOrientation(final int p0);

	public final void setErrorCallback(final ErrorCallback mErrorCallback) {
		this.mErrorCallback = mErrorCallback;
	}

	public final void setFaceDetectionListener(
			final FaceDetectionListener mFaceListener) {
		this.mFaceListener = mFaceListener;
	}

	public final native void setFrameShotIndex(final int p0);

	public void setFrontSensorMirror(final boolean b, final int n) {
		if (b) {
			this.native_sendcommand(1110, 1, 0);
			this.native_sendcommand(1042, n, 1);
			return;
		}
		this.native_sendcommand(1110, 0, 0);
		this.native_sendcommand(1042, n, 0);
	}

	public final native void setMosaicShotBack();

	public final native void setMosaicShotIndex(final int p0);

	public final native void setMosaicShotNext();

	public void setObjectTrackingMsgListener(
			final OnObjectTrackingMsgListener mOnObjectTrackingMsgListener) {
		this.mOnObjectTrackingMsgListener = mOnObjectTrackingMsgListener;
	}

	public void setObjectTrackingPosition(final int n, final int n2) {
		this.native_sendcommand(1103, n, n2);
	}

	public void setOnActionShotEventListener(
			final OnActionShotEventListener mOnActionShotEventListener) {
		this.mOnActionShotEventListener = mOnActionShotEventListener;
	}

	public void setOnAddMeEventListener(
			final OnAddMeEventListener mOnAddMeEventListener) {
		this.mOnAddMeEventListener = mOnAddMeEventListener;
	}

	public void setOnBlinkDetectionEventListener(
			final OnBlinkDetectionEventListener mOnBlinkDetectionEventListener) {
		this.mOnBlinkDetectionEventListener = mOnBlinkDetectionEventListener;
	}

	public void setOnBurstShotEventListener(
			final OnBurstShotEventListener mOnBurstShotEventListener) {
		this.mOnBurstShotEventListener = mOnBurstShotEventListener;
	}

	public void setOnCartoonShotEventListener(
			final OnCartoonShotEventListener mOnCartoonShotEventListener) {
		this.mOnCartoonShotEventListener = mOnCartoonShotEventListener;
	}

	public void setOnContinuousShotEventListener(
			final OnContinuousShotEventListener mOnContinuousShotEventListener) {
		this.mOnContinuousShotEventListener = mOnContinuousShotEventListener;
	}

	public void setOnHDRShotEventListener(
			final OnHDRShotEventListener mOnHDRShotEventListener) {
		this.mOnHDRShotEventListener = mOnHDRShotEventListener;
	}

	public void setOnPanoramaEventListener(
			final OnPanoramaEventListener mOnPanoramaEventListener) {
		this.mOnPanoramaEventListener = mOnPanoramaEventListener;
	}

	public void setOnSmileShotDetectionSuccessListener(
			final OnSmileShotDetectionSuccessListener mOnSmileShotDetectionSuccessListener) {
		this.mOnSmileShotDetectionSuccessListener = mOnSmileShotDetectionSuccessListener;
	}

	public final void setOneShotPreviewCallback(
			final PreviewCallback mPreviewCallback) {
		boolean mOneShot = true;
		this.mPreviewCallback = mPreviewCallback;
		this.mOneShot = mOneShot;
		this.mWithBuffer = false;
		if (mPreviewCallback == null) {
			mOneShot = false;
		}
		this.setHasPreviewCallback(mOneShot, false);
	}

	public void setParameters(final Parameters parameters) {
		this.native_setParameters(parameters.flatten());
	}

	public final void setPreviewCallback(final PreviewCallback mPreviewCallback) {
		this.mPreviewCallback = mPreviewCallback;
		this.mOneShot = false;
		this.mWithBuffer = false;
		final boolean b = mPreviewCallback != null;
		this.setHasPreviewCallback(b, false);
	}

	public final void setPreviewCallbackWithBuffer(
			final PreviewCallback mPreviewCallback) {
		this.mPreviewCallback = mPreviewCallback;
		this.mOneShot = false;
		this.mWithBuffer = true;
		boolean b = false;
		if (mPreviewCallback != null) {
			b = true;
		}
		this.setHasPreviewCallback(b, true);
	}

	public final void setPreviewDisplay(final SurfaceHolder surfaceHolder)
			throws IOException {
		if (surfaceHolder != null) {
			this.setPreviewDisplay(surfaceHolder.getSurface());
			return;
		}
		this.setPreviewDisplay((Surface) null);
	}

	public final native void setPreviewTexture(final SurfaceTexture p0)
			throws IOException;

	public void setResolutionActionShot(final int n, final int n2) {
		this.native_sendcommand(1029, n, n2);
	}

	public void setShootingMode(final int n) {
		this.native_sendcommand(n + 1000, 0, 0);
	}

	public void setShootingModeCallbacks(
			final ShutterCallback mShutterCallback,
			final PictureCallback mRawImageCallback,
			final PictureCallback mJpegCallback) {
		this.mShutterCallback = mShutterCallback;
		this.mRawImageCallback = mRawImageCallback;
		this.mPostviewCallback = null;
		this.mJpegCallback = mJpegCallback;
	}

	public final void setZoomChangeListener(
			final OnZoomChangeListener mZoomListener) {
		this.mZoomListener = mZoomListener;
	}

	public void startBurstShot(final boolean b) {
		if (b) {
			this.native_sendcommand(1045, 0, 0);
			return;
		}
		this.native_sendcommand(1046, 0, 0);
	}

	public void startCaptureAddMeShot() {
		this.native_sendcommand(1035, 0, 0);
	}

	public void startContinuousShot(final boolean b) {
		if (b) {
			this.native_sendcommand(1024, 0, 0);
			return;
		}
		this.native_sendcommand(1025, 0, 0);
	}

	public final void startFaceDetection() {
		if (this.mFaceDetectionRunning) {
			throw new RuntimeException("Face detection is already running");
		}
		if (this.getParameters().getMaxNumDetectedFaces() <= 0) {
			throw new IllegalArgumentException(
					"getMaxNumDetectedFaces() return 0");
		}
		this._startFaceDetection(0);
		this.mFaceDetectionRunning = true;
	}

	public void startObjectTracking() {
		this.native_sendcommand(1104, 1, 0);
	}

	public void startPanorama(final boolean b) {
		if (b) {
			this.native_sendcommand(1020, 0, 0);
			return;
		}
		this.native_sendcommand(1021, 0, 0);
	}

	public final native void startPreview();

	public void startSeriesActionShot() {
		this.native_sendcommand(1030, 0, 0);
	}

	public void startSmileDetection(final boolean b) {
		if (b) {
			this.native_sendcommand(1018, 0, 0);
			return;
		}
		this.native_sendcommand(1019, 0, 0);
	}

	public final native void startSmoothZoom(final int p0);

	public void startTouchAutoFocus() {
		this.native_sendcommand(1105, 1, 0);
	}

	public final void stopFaceDetection() {
		this._stopFaceDetection();
		this.mFaceDetectionRunning = false;
	}

	public void stopObjectTracking() {
		this.native_sendcommand(1104, 0, 0);
	}

	public final void stopPreview() {
		this._stopPreview();
		this.mFaceDetectionRunning = false;
		this.mShutterCallback = null;
		this.mRawImageCallback = null;
		this.mPostviewCallback = null;
		this.mJpegCallback = null;
		this.mAutoFocusCallback = null;
	}

	public void stopSeriesActionShot() {
		this.native_sendcommand(1032, 0, 0);
	}

	public final native void stopSmoothZoom();

	public void stopTouchAutoFocus() {
		this.native_sendcommand(1105, 0, 0);
	}

	public void switchPositionAddMeShot(final int n) {
		this.native_sendcommand(1036, n, 0);
	}

	public final void takePicture(final ShutterCallback shutterCallback,
			final PictureCallback pictureCallback,
			final PictureCallback pictureCallback2) {
		this.takePicture(shutterCallback, pictureCallback, null,
				pictureCallback2);
	}

	public final void takePicture(final ShutterCallback mShutterCallback,
			final PictureCallback mRawImageCallback,
			final PictureCallback mPostviewCallback,
			final PictureCallback mJpegCallback) {
		this.mShutterCallback = mShutterCallback;
		this.mRawImageCallback = mRawImageCallback;
		this.mPostviewCallback = mPostviewCallback;
		this.mJpegCallback = mJpegCallback;
		final ShutterCallback mShutterCallback2 = this.mShutterCallback;
		int n = 0;
		if (mShutterCallback2 != null) {
			n = (0 | 2);
		}
		if (this.mRawImageCallback != null) {
			n = (n | 128);
		}
		if (this.mPostviewCallback != null) {
			n = (n | 64);
		}
		if (this.mJpegCallback != null) {
			n = (n | 256);
		}
		this.native_takePicture(n);
	}

	public void terminateBurstShot() {
		this.native_sendcommand(1047, 0, 0);
	}

	public void terminateContinuousShot() {
		this.native_sendcommand(1026, 0, 0);
	}

	public final native void unlock();

	public void unlockFaceDetect() {
		this.native_sendcommand(1102, 0, 0);
	}

	public static class CameraInfo {
		public static final int CAMERA_FACING_BACK = 0;
		public static final int CAMERA_FACING_FRONT = 1;
		public int facing;
		public int orientation;
	}

	public static class Face {
		public int id;
		public Point leftEye;
		public Point mouth;
		public Rect rect;
		public Point rightEye;
		public int score;

		public Face() {
			super();
			this.id = -1;
			this.leftEye = null;
			this.rightEye = null;
			this.mouth = null;
		}
	}

	private class EventHandler extends Handler {
		private SecCamera mCamera;

		public EventHandler(final SecCamera mCamera, final Looper looper) {
			super(looper);
			this.mCamera = mCamera;
		}

		public void handleMessage(final Message message) {
			boolean b = true;
			switch (message.what) {
			default: {
				Log.e("SecCamera-JNI-Java", "Unknown message type "
						+ message.what);
				break;
			}
			case 2: {
				if (SecCamera.this.mShutterCallback != null) {
					SecCamera.this.mShutterCallback.onShutter();
					return;
				}
				break;
			}
			case 128: {
				if (SecCamera.this.mRawImageCallback != null) {
					SecCamera.this.mRawImageCallback.onPictureTaken(
							(byte[]) message.obj, this.mCamera);
					return;
				}
				break;
			}
			case 256: {
				if (SecCamera.this.mJpegCallback != null) {
					SecCamera.this.mJpegCallback.onPictureTaken(
							(byte[]) message.obj, this.mCamera);
					return;
				}
				break;
			}
			case 16: {
				if (SecCamera.this.mPreviewCallback != null) {
					final PreviewCallback access$300 = SecCamera.this.mPreviewCallback;
					if (SecCamera.this.mOneShot) {
						SecCamera.this.mPreviewCallback = null;
					} else if (!SecCamera.this.mWithBuffer) {
						SecCamera.this.setHasPreviewCallback(b, false);
					}
					access$300.onPreviewFrame((byte[]) message.obj,
							this.mCamera);
					return;
				}
				break;
			}
			case 64: {
				if (SecCamera.this.mPostviewCallback != null) {
					SecCamera.this.mPostviewCallback.onPictureTaken(
							(byte[]) message.obj, this.mCamera);
					return;
				}
				break;
			}
			case 4: {
				if (SecCamera.this.mAutoFocusCallback != null) {
					SecCamera.this.mAutoFocusCallback.onAutoFocus(message.arg1,
							this.mCamera);
					return;
				}
				break;
			}
			case 8: {
				if (SecCamera.this.mZoomListener != null) {
					final OnZoomChangeListener access$2 = SecCamera.this.mZoomListener;
					final int arg1 = message.arg1;
					if (message.arg2 == 0) {
						b = false;
					}
					access$2.onZoomChange(arg1, b, this.mCamera);
					return;
				}
				break;
			}
			case 1024: {
				if (SecCamera.this.mFaceListener != null) {
					SecCamera.this.mFaceListener.onFaceDetection(
							(Face[]) message.obj, this.mCamera);
					return;
				}
				break;
			}
			case 1: {
				Log.e("SecCamera-JNI-Java", "Error " + message.arg1);
				if (SecCamera.this.mErrorCallback != null) {
					SecCamera.this.mErrorCallback.onError(message.arg1,
							this.mCamera);
				}
				if (message.arg1 == 200) {
					SecCamera.this.native_release();
					SecCamera.this.mFaceDetectionRunning = false;
					return;
				}
				break;
			}
			case 61697: {
				Log.w("SecCamera-JNI-Java", "SAMSUNG_SHOT_COMPRESSED_IMAGE");
				if (SecCamera.this.mJpegCallback != null) {
					SecCamera.this.mJpegCallback.onPictureTaken(
							(byte[]) message.obj, this.mCamera);
					return;
				}
				break;
			}
			case 61473: {
				if (SecCamera.this.mOnPanoramaEventListener != null) {
					SecCamera.this.mOnPanoramaEventListener.onPanoramaError(0);
					return;
				}
				break;
			}
			case 61478: {
				if (SecCamera.this.mOnPanoramaEventListener != null) {
					SecCamera.this.mOnPanoramaEventListener
							.onPanoramaDirectionChanged(message.arg1);
					return;
				}
				break;
			}
			case 61474: {
				if (SecCamera.this.mOnPanoramaEventListener != null) {
					SecCamera.this.mOnPanoramaEventListener
							.onPanoramaRectChanged(message.arg1, message.arg2);
					return;
				}
				break;
			}
			case 61475: {
				if (SecCamera.this.mOnPanoramaEventListener != null) {
					SecCamera.this.mOnPanoramaEventListener
							.onPanoramaCapturedNew();
					return;
				}
				break;
			}
			case 61476: {
				if (SecCamera.this.mOnPanoramaEventListener != null) {
					SecCamera.this.mOnPanoramaEventListener
							.onPanoramaProgressStitching(message.arg1);
					return;
				}
				break;
			}
			case 61477: {
				if (SecCamera.this.mOnPanoramaEventListener != null) {
					SecCamera.this.mOnPanoramaEventListener
							.onPanoramaCaptured();
					return;
				}
				break;
			}
			case 61521: {
				if (SecCamera.this.mOnAddMeEventListener != null) {
					SecCamera.this.mOnAddMeEventListener
							.onAddMeError(message.arg1);
					return;
				}
				Log.e("SecCamera-JNI-Java", "mOnAddMeEventListener is null");
				return;
			}
			case 61522: {
				if (SecCamera.this.mOnAddMeEventListener != null) {
					SecCamera.this.mOnAddMeEventListener
							.onAddMeCapturedFirstPerson();
					return;
				}
				Log.e("SecCamera-JNI-Java", "mOnAddMeEventListener is null");
				return;
			}
			case 61523: {
				if (SecCamera.this.mOnAddMeEventListener != null) {
					SecCamera.this.mOnAddMeEventListener
							.onAddMeProgressStitching(message.arg1);
					return;
				}
				Log.e("SecCamera-JNI-Java", "mOnAddMeEventListener is null");
				return;
			}
			case 61489: {
				if (SecCamera.this.mOnContinuousShotEventListener != null) {
					SecCamera.this.mOnContinuousShotEventListener
							.onContinuousShotCapturingProgressed(message.arg1,
									message.arg2);
					return;
				}
				break;
			}
			case 61490: {
				if (SecCamera.this.mOnContinuousShotEventListener != null) {
					SecCamera.this.mOnContinuousShotEventListener
							.onContinuousShotCapturingStopped(message.arg1);
					return;
				}
				break;
			}
			case 61491: {
				if (SecCamera.this.mOnContinuousShotEventListener != null) {
					SecCamera.this.mOnContinuousShotEventListener
							.onContinuousShotSavingCompleted();
					return;
				}
				break;
			}
			case 61505: {
				if (SecCamera.this.mOnActionShotEventListener != null) {
					SecCamera.this.mOnActionShotEventListener
							.onActionShotCreatingResultStarted();
					return;
				}
				break;
			}
			case 61506: {
				if (SecCamera.this.mOnActionShotEventListener != null) {
					SecCamera.this.mOnActionShotEventListener
							.onActionShotCreatingResultProgress(message.arg1);
					return;
				}
				break;
			}
			case 61507: {
				if (SecCamera.this.mOnActionShotEventListener != null) {
					final OnActionShotEventListener access$3 = SecCamera.this.mOnActionShotEventListener;
					if (message.arg1 != (b ? 1 : 0)) {
						b = false;
					}
					access$3.onActionShotCreatingResultCompleted(b);
					return;
				}
				break;
			}
			case 61508: {
				Log.e("SecCamera-JNI-Java", "onActionShotAcquisitionProgress "
						+ message.arg1);
				if (SecCamera.this.mOnActionShotEventListener != null) {
					SecCamera.this.mOnActionShotEventListener
							.onActionShotAcquisitionProgress(message.arg1);
					return;
				}
				break;
			}
			case 61510: {
				if (SecCamera.this.mOnActionShotEventListener != null) {
					SecCamera.this.mOnActionShotEventListener
							.onActionShotRectChanged((byte[]) message.obj);
					return;
				}
				break;
			}
			case 61509: {
				if (SecCamera.this.mOnActionShotEventListener != null) {
					SecCamera.this.mOnActionShotEventListener
							.onActionShotCaptured();
					return;
				}
				break;
			}
			case 61553: {
				if (SecCamera.this.mOnCartoonShotEventListener != null) {
					Log.d("SecCamera-JNI-Java",
							"CARTOON_SHOT_PROGRESS_RENDERING :" + message.arg1);
					SecCamera.this.mOnCartoonShotEventListener
							.onCartoonShotProgressRendering(message.arg1);
					return;
				}
				break;
			}
			case 61537: {
				if (SecCamera.this.mOnSmileShotDetectionSuccessListener != null) {
					SecCamera.this.mOnSmileShotDetectionSuccessListener
							.onSmileShotDetectionSuccess();
					return;
				}
				break;
			}
			case 61538: {
				if (SecCamera.this.mOnSmileShotDetectionSuccessListener != null) {
					SecCamera.this.mOnSmileShotDetectionSuccessListener
							.onSmileShotFaceRectChanged((byte[]) message.obj);
					return;
				}
				break;
			}
			case 61539: {
				if (SecCamera.this.mOnSmileShotDetectionSuccessListener != null) {
					SecCamera.this.mOnSmileShotDetectionSuccessListener
							.onSmileShotSmileRectChanged((byte[]) message.obj);
					return;
				}
				break;
			}
			case 61569: {
				if (SecCamera.this.mOnHDRShotEventListener != null) {
					SecCamera.this.mOnHDRShotEventListener
							.onHDRShotResultStarted();
					return;
				}
				break;
			}
			case 61570: {
				if (SecCamera.this.mOnHDRShotEventListener != null) {
					SecCamera.this.mOnHDRShotEventListener
							.onHDRShotResultProgress(message.arg1);
					return;
				}
				break;
			}
			case 61571: {
				if (SecCamera.this.mOnHDRShotEventListener != null) {
					final OnHDRShotEventListener access$4 = SecCamera.this.mOnHDRShotEventListener;
					if (message.arg1 != (b ? 1 : 0)) {
						b = false;
					}
					access$4.onHDRShotResultCompleted(b);
					return;
				}
				break;
			}
			case 61572: {
				if (SecCamera.this.mOnHDRShotEventListener != null) {
					final OnHDRShotEventListener access$5 = SecCamera.this.mOnHDRShotEventListener;
					if (message.arg1 != (b ? 1 : 0)) {
						b = false;
					}
					access$5.onHDRShotAllProgressCompleted(b);
					return;
				}
				break;
			}
			case 61441: {
				if (SecCamera.this.mOnObjectTrackingMsgListener != null) {
					Log.d("SecCamera-JNI-Java", "HAL_MSG_OBJ_TRACKING :"
							+ message.arg1);
					SecCamera.this.mOnObjectTrackingMsgListener
							.onObjectTrackingStatus(message.arg1);
					return;
				}
				break;
			}
			case 61442: {
				if (SecCamera.this.mOnChkDataLineListener != null) {
					Log.d("SecCamera-JNI-Java", "HAL_DONE_CHK_DATALINE");
					SecCamera.this.mOnChkDataLineListener.onChkDataLineDone();
					return;
				}
				break;
			}
			case 61585: {
				if (SecCamera.this.mOnBurstShotEventListener != null) {
					SecCamera.this.mOnBurstShotEventListener
							.onBurstShotCapturingProgressed(message.arg1,
									message.arg2);
					return;
				}
				break;
			}
			case 61586: {
				if (SecCamera.this.mOnBurstShotEventListener != null) {
					SecCamera.this.mOnBurstShotEventListener
							.onBurstShotCapturingStopped(message.arg1);
					return;
				}
				break;
			}
			case 61587: {
				if (SecCamera.this.mOnBurstShotEventListener != null) {
					SecCamera.this.mOnBurstShotEventListener
							.onBurstShotSavingCompleted();
					return;
				}
				break;
			}
			}
		}
	}

	public class Parameters {
		public static final String ANTIBANDING_50HZ = "50hz";
		public static final String ANTIBANDING_60HZ = "60hz";
		public static final String ANTIBANDING_AUTO = "auto";
		public static final String ANTIBANDING_OFF = "off";
		public static final String EFFECT_AQUA = "aqua";
		public static final String EFFECT_BLACKBOARD = "blackboard";
		public static final String EFFECT_MONO = "mono";
		public static final String EFFECT_NEGATIVE = "negative";
		public static final String EFFECT_NONE = "none";
		public static final String EFFECT_POSTERIZE = "posterize";
		public static final String EFFECT_SEPIA = "sepia";
		public static final String EFFECT_SOLARIZE = "solarize";
		public static final String EFFECT_WHITEBOARD = "whiteboard";
		private static final String FALSE = "false";
		public static final String FLASH_MODE_AUTO = "auto";
		public static final String FLASH_MODE_OFF = "off";
		public static final String FLASH_MODE_ON = "on";
		public static final String FLASH_MODE_RED_EYE = "red-eye";
		public static final String FLASH_MODE_TORCH = "torch";
		public static final int FOCUS_DISTANCE_FAR_INDEX = 2;
		public static final int FOCUS_DISTANCE_NEAR_INDEX = 0;
		public static final int FOCUS_DISTANCE_OPTIMAL_INDEX = 1;
		public static final String FOCUS_MODE_AUTO = "auto";
		public static final String FOCUS_MODE_CONTINUOUS_PICTURE = "continuous-picture";
		public static final String FOCUS_MODE_CONTINUOUS_VIDEO = "continuous-video";
		public static final String FOCUS_MODE_EDOF = "edof";
		public static final String FOCUS_MODE_FIXED = "fixed";
		public static final String FOCUS_MODE_INFINITY = "infinity";
		public static final String FOCUS_MODE_MACRO = "macro";
		private static final String KEY_ANTIBANDING = "antibanding";
		private static final String KEY_AUTO_EXPOSURE_LOCK = "auto-exposure-lock";
		private static final String KEY_AUTO_EXPOSURE_LOCK_SUPPORTED = "auto-exposure-lock-supported";
		private static final String KEY_AUTO_WHITEBALANCE_LOCK = "auto-whitebalance-lock";
		private static final String KEY_AUTO_WHITEBALANCE_LOCK_SUPPORTED = "auto-whitebalance-lock-supported";
		private static final String KEY_EFFECT = "effect";
		private static final String KEY_EXPOSURE_COMPENSATION = "exposure-compensation";
		private static final String KEY_EXPOSURE_COMPENSATION_STEP = "exposure-compensation-step";
		private static final String KEY_FLASH_MODE = "flash-mode";
		private static final String KEY_FOCAL_LENGTH = "focal-length";
		private static final String KEY_FOCUS_AREAS = "focus-areas";
		private static final String KEY_FOCUS_DISTANCES = "focus-distances";
		private static final String KEY_FOCUS_MODE = "focus-mode";
		private static final String KEY_GPS_ALTITUDE = "gps-altitude";
		private static final String KEY_GPS_LATITUDE = "gps-latitude";
		private static final String KEY_GPS_LONGITUDE = "gps-longitude";
		private static final String KEY_GPS_PROCESSING_METHOD = "gps-processing-method";
		private static final String KEY_GPS_TIMESTAMP = "gps-timestamp";
		private static final String KEY_HORIZONTAL_VIEW_ANGLE = "horizontal-view-angle";
		private static final String KEY_JPEG_QUALITY = "jpeg-quality";
		private static final String KEY_JPEG_THUMBNAIL_HEIGHT = "jpeg-thumbnail-height";
		private static final String KEY_JPEG_THUMBNAIL_QUALITY = "jpeg-thumbnail-quality";
		private static final String KEY_JPEG_THUMBNAIL_SIZE = "jpeg-thumbnail-size";
		private static final String KEY_JPEG_THUMBNAIL_WIDTH = "jpeg-thumbnail-width";
		private static final String KEY_MAX_EXPOSURE_COMPENSATION = "max-exposure-compensation";
		private static final String KEY_MAX_NUM_DETECTED_FACES_HW = "max-num-detected-faces-hw";
		private static final String KEY_MAX_NUM_DETECTED_FACES_SW = "max-num-detected-faces-sw";
		private static final String KEY_MAX_NUM_FOCUS_AREAS = "max-num-focus-areas";
		private static final String KEY_MAX_NUM_METERING_AREAS = "max-num-metering-areas";
		private static final String KEY_MAX_ZOOM = "max-zoom";
		private static final String KEY_METERING_AREAS = "metering-areas";
		private static final String KEY_MIN_EXPOSURE_COMPENSATION = "min-exposure-compensation";
		private static final String KEY_PICTURE_FORMAT = "picture-format";
		private static final String KEY_PICTURE_SIZE = "picture-size";
		private static final String KEY_PREFERRED_PREVIEW_SIZE_FOR_VIDEO = "preferred-preview-size-for-video";
		private static final String KEY_PREVIEW_FORMAT = "preview-format";
		private static final String KEY_PREVIEW_FPS_RANGE = "preview-fps-range";
		private static final String KEY_PREVIEW_FRAME_RATE = "preview-frame-rate";
		private static final String KEY_PREVIEW_SIZE = "preview-size";
		private static final String KEY_RECORDING_HINT = "recording-hint";
		private static final String KEY_ROTATION = "rotation";
		private static final String KEY_SCENE_MODE = "scene-mode";
		private static final String KEY_SMOOTH_ZOOM_SUPPORTED = "smooth-zoom-supported";
		private static final String KEY_VERTICAL_VIEW_ANGLE = "vertical-view-angle";
		private static final String KEY_VIDEO_SIZE = "video-size";
		private static final String KEY_VIDEO_SNAPSHOT_SUPPORTED = "video-snapshot-supported";
		private static final String KEY_VIDEO_STABILIZATION = "video-stabilization";
		private static final String KEY_VIDEO_STABILIZATION_SUPPORTED = "video-stabilization-supported";
		private static final String KEY_WHITE_BALANCE = "whitebalance";
		private static final String KEY_ZOOM = "zoom";
		private static final String KEY_ZOOM_RATIOS = "zoom-ratios";
		private static final String KEY_ZOOM_SUPPORTED = "zoom-supported";
		private static final String PIXEL_FORMAT_BAYER_RGGB = "bayer-rggb";
		private static final String PIXEL_FORMAT_JPEG = "jpeg";
		private static final String PIXEL_FORMAT_RGB565 = "rgb565";
		private static final String PIXEL_FORMAT_YUV420P = "yuv420p";
		private static final String PIXEL_FORMAT_YUV420SP = "yuv420sp";
		private static final String PIXEL_FORMAT_YUV422I = "yuv422i-yuyv";
		private static final String PIXEL_FORMAT_YUV422SP = "yuv422sp";
		public static final int PREVIEW_FPS_MAX_INDEX = 1;
		public static final int PREVIEW_FPS_MIN_INDEX = 0;
		public static final String SCENE_MODE_ACTION = "action";
		public static final String SCENE_MODE_AUTO = "auto";
		public static final String SCENE_MODE_BARCODE = "barcode";
		public static final String SCENE_MODE_BEACH = "beach";
		public static final String SCENE_MODE_CANDLELIGHT = "candlelight";
		public static final String SCENE_MODE_FIREWORKS = "fireworks";
		public static final String SCENE_MODE_LANDSCAPE = "landscape";
		public static final String SCENE_MODE_NIGHT = "night";
		public static final String SCENE_MODE_NIGHT_PORTRAIT = "night-portrait";
		public static final String SCENE_MODE_PARTY = "party";
		public static final String SCENE_MODE_PORTRAIT = "portrait";
		public static final String SCENE_MODE_SNOW = "snow";
		public static final String SCENE_MODE_SPORTS = "sports";
		public static final String SCENE_MODE_STEADYPHOTO = "steadyphoto";
		public static final String SCENE_MODE_SUNSET = "sunset";
		public static final String SCENE_MODE_THEATRE = "theatre";
		private static final String SUPPORTED_VALUES_SUFFIX = "-values";
		private static final String TRUE = "true";
		public static final String WHITE_BALANCE_AUTO = "auto";
		public static final String WHITE_BALANCE_CLOUDY_DAYLIGHT = "cloudy-daylight";
		public static final String WHITE_BALANCE_DAYLIGHT = "daylight";
		public static final String WHITE_BALANCE_FLUORESCENT = "fluorescent";
		public static final String WHITE_BALANCE_INCANDESCENT = "incandescent";
		public static final String WHITE_BALANCE_SHADE = "shade";
		public static final String WHITE_BALANCE_TWILIGHT = "twilight";
		public static final String WHITE_BALANCE_WARM_FLUORESCENT = "warm-fluorescent";
		private HashMap<String, String> mMap;

		public Parameters() {
			super();
			this.mMap = new HashMap<String, String>();
		}

		private String cameraFormatForPixelFormat(final int n) {
			switch (n) {
			default: {
				return null;
			}
			case 16: {
				return "yuv422sp";
			}
			case 17: {
				return "yuv420sp";
			}
			case 20: {
				return "yuv422i-yuyv";
			}
			case 842094169: {
				return "yuv420p";
			}
			case 4: {
				return "rgb565";
			}
			case 256: {
				return "jpeg";
			}
			case 512: {
				return "bayer-rggb";
			}
			}
		}

		private float getFloat(final String s, final float n) {
			try {
				return Float.parseFloat(this.mMap.get(s));
			} catch (NumberFormatException ex) {
				return n;
			}
		}

		private int getInt(final String s, final int n) {
			try {
				return Integer.parseInt(this.mMap.get(s));
			} catch (NumberFormatException ex) {
				return n;
			}
		}

		private int pixelFormatForCameraFormat(final String s) {
			if (s != null) {
				if (s.equals("yuv422sp")) {
					return 16;
				}
				if (s.equals("yuv420sp")) {
					return 17;
				}
				if (s.equals("yuv422i-yuyv")) {
					return 20;
				}
				if (s.equals("yuv420p")) {
					return 842094169;
				}
				if (s.equals("rgb565")) {
					return 4;
				}
				if (s.equals("jpeg")) {
					return 256;
				}
			}
			return 0;
		}

		private void set(final String s, final List<Area> list) {
			if (list == null) {
				this.set(s, "(0,0,0,0,0)");
				return;
			}
			final StringBuilder sb = new StringBuilder();
			for (int i = 0; i < list.size(); ++i) {
				final Area area = list.get(i);
				final Rect rect = area.rect;
				sb.append('(');
				sb.append(rect.left);
				sb.append(',');
				sb.append(rect.top);
				sb.append(',');
				sb.append(rect.right);
				sb.append(',');
				sb.append(rect.bottom);
				sb.append(',');
				sb.append(area.weight);
				sb.append(')');
				if (i != -1 + list.size()) {
					sb.append(',');
				}
			}
			this.set(s, sb.toString());
		}

		private ArrayList<String> split(final String s) {
			ArrayList<String> list;
			if (s == null) {
				list = null;
			} else {
				final StringTokenizer stringTokenizer = new StringTokenizer(s,
						",");
				list = new ArrayList<String>();
				while (stringTokenizer.hasMoreElements()) {
					list.add(stringTokenizer.nextToken());
				}
			}
			return list;
		}

		private ArrayList<Area> splitArea(final String s) {
			ArrayList<Area> list;
			if (s == null || s.charAt(0) != '('
					|| s.charAt(-1 + s.length()) != ')') {
				Log.e("SecCamera-JNI-Java", "Invalid area string=" + s);
				list = null;
			} else {
				list = new ArrayList<Area>();
				int n = 1;
				final int[] array = new int[5];
				int i;
				do {
					i = s.indexOf("),(", n);
					if (i == -1) {
						i = -1 + s.length();
					}
					this.splitInt(s.substring(n, i), array);
					list.add(new Area(new Rect(array[0], array[1], array[2],
							array[3]), array[4]));
					n = i + 3;
				} while (i != -1 + s.length());
				if (list.size() == 0) {
					return null;
				}
				if (list.size() == 1) {
					final Area area = list.get(0);
					final Rect rect = area.rect;
					if (rect.left == 0 && rect.top == 0 && rect.right == 0
							&& rect.bottom == 0 && area.weight == 0) {
						return null;
					}
				}
			}
			return list;
		}

		private void splitFloat(final String s, final float[] array) {
			if (s != null) {
				final StringTokenizer stringTokenizer = new StringTokenizer(s,
						",");
				int n = 0;
				while (stringTokenizer.hasMoreElements()) {
					final String nextToken = stringTokenizer.nextToken();
					final int n2 = n + 1;
					array[n] = Float.parseFloat(nextToken);
					n = n2;
				}
			}
		}

		private ArrayList<Integer> splitInt(final String s) {
			ArrayList<Integer> list;
			if (s == null) {
				list = null;
			} else {
				final StringTokenizer stringTokenizer = new StringTokenizer(s,
						",");
				list = new ArrayList<Integer>();
				while (stringTokenizer.hasMoreElements()) {
					list.add(Integer.parseInt(stringTokenizer.nextToken()));
				}
				if (list.size() == 0) {
					return null;
				}
			}
			return list;
		}

		private void splitInt(final String s, final int[] array) {
			if (s != null) {
				final StringTokenizer stringTokenizer = new StringTokenizer(s,
						",");
				int n = 0;
				while (stringTokenizer.hasMoreElements()) {
					final String nextToken = stringTokenizer.nextToken();
					final int n2 = n + 1;
					array[n] = Integer.parseInt(nextToken);
					n = n2;
				}
			}
		}

		private ArrayList<int[]> splitRange(final String s) {
			ArrayList<int[]> list;
			if (s == null || s.charAt(0) != '('
					|| s.charAt(-1 + s.length()) != ')') {
				Log.e("SecCamera-JNI-Java", "Invalid range list string=" + s);
				list = null;
			} else {
				list = new ArrayList<int[]>();
				int n = 1;
				int i;
				do {
					final int[] array = new int[2];
					i = s.indexOf("),(", n);
					if (i == -1) {
						i = -1 + s.length();
					}
					this.splitInt(s.substring(n, i), array);
					list.add(array);
					n = i + 3;
				} while (i != -1 + s.length());
				if (list.size() == 0) {
					return null;
				}
			}
			return list;
		}

		private ArrayList<Size> splitSize(final String s) {
			ArrayList<Size> list;
			if (s == null) {
				list = null;
			} else {
				final StringTokenizer stringTokenizer = new StringTokenizer(s,
						",");
				list = new ArrayList<Size>();
				while (stringTokenizer.hasMoreElements()) {
					final Size strToSize = this.strToSize(stringTokenizer
							.nextToken());
					if (strToSize == null) {
						continue;
					}
					list.add(strToSize);
				}
				if (list.size() == 0) {
					return null;
				}
			}
			return list;
		}

		private Size strToSize(final String s) {
			if (s == null) {
				return null;
			}
			final int index = s.indexOf(120);
			if (index != -1) {
				return new Size(Integer.parseInt(s.substring(0, index)),
						Integer.parseInt(s.substring(index + 1)));
			}
			Log.e("SecCamera-JNI-Java", "Invalid size parameter string=" + s);
			return null;
		}

		public void dump() {
			Log.e("SecCamera-JNI-Java", "dump: size=" + this.mMap.size());
			for (final String s : this.mMap.keySet()) {
				Log.e("SecCamera-JNI-Java",
						"dump: " + s + "=" + this.mMap.get(s));
			}
		}

		public String flatten() {
			final StringBuilder sb = new StringBuilder();
			for (final String s : this.mMap.keySet()) {
				sb.append(s);
				sb.append("=");
				sb.append(this.mMap.get(s));
				sb.append(";");
			}
			sb.deleteCharAt(-1 + sb.length());
			return sb.toString();
		}

		public String get(final String s) {
			return this.mMap.get(s);
		}

		public String getAntibanding() {
			return this.get("antibanding");
		}

		public boolean getAutoExposureLock() {
			return "true".equals(this.get("auto-exposure-lock"));
		}

		public boolean getAutoWhiteBalanceLock() {
			return "true".equals(this.get("auto-whitebalance-lock"));
		}

		public String getColorEffect() {
			return this.get("effect");
		}

		public int getExposureCompensation() {
			return this.getInt("exposure-compensation", 0);
		}

		public float getExposureCompensationStep() {
			return this.getFloat("exposure-compensation-step", 0.0f);
		}

		public String getFlashMode() {
			return this.get("flash-mode");
		}

		public float getFocalLength() {
			return Float.parseFloat(this.get("focal-length"));
		}

		public List<Area> getFocusAreas() {
			return this.splitArea(this.get("focus-areas"));
		}

		public void getFocusDistances(final float[] array) {
			if (array == null || array.length != 3) {
				throw new IllegalArgumentException(
						"output must be an float array with three elements.");
			}
			this.splitFloat(this.get("focus-distances"), array);
		}

		public String getFocusMode() {
			return this.get("focus-mode");
		}

		public float getHorizontalViewAngle() {
			return Float.parseFloat(this.get("horizontal-view-angle"));
		}

		public int getInt(final String s) {
			return Integer.parseInt(this.mMap.get(s));
		}

		public int getJpegQuality() {
			return this.getInt("jpeg-quality");
		}

		public int getJpegThumbnailQuality() {
			return this.getInt("jpeg-thumbnail-quality");
		}

		public Size getJpegThumbnailSize() {
			return new Size(this.getInt("jpeg-thumbnail-width"),
					this.getInt("jpeg-thumbnail-height"));
		}

		public int getMaxExposureCompensation() {
			return this.getInt("max-exposure-compensation", 0);
		}

		public int getMaxNumDetectedFaces() {
			return this.getInt("max-num-detected-faces-hw", 0);
		}

		public int getMaxNumFocusAreas() {
			return this.getInt("max-num-focus-areas", 0);
		}

		public int getMaxNumMeteringAreas() {
			return this.getInt("max-num-metering-areas", 0);
		}

		public int getMaxZoom() {
			return this.getInt("max-zoom", 0);
		}

		public List<Area> getMeteringAreas() {
			return this.splitArea(this.get("metering-areas"));
		}

		public int getMinExposureCompensation() {
			return this.getInt("min-exposure-compensation", 0);
		}

		public int getPictureFormat() {
			return this.pixelFormatForCameraFormat(this.get("picture-format"));
		}

		public Size getPictureSize() {
			return this.strToSize(this.get("picture-size"));
		}

		public Size getPreferredPreviewSizeForVideo() {
			return this.strToSize(this.get("preferred-preview-size-for-video"));
		}

		public int getPreviewFormat() {
			return this.pixelFormatForCameraFormat(this.get("preview-format"));
		}

		public void getPreviewFpsRange(final int[] array) {
			if (array == null || array.length != 2) {
				throw new IllegalArgumentException(
						"range must be an array with two elements.");
			}
			this.splitInt(this.get("preview-fps-range"), array);
		}

		@Deprecated
		public int getPreviewFrameRate() {
			return this.getInt("preview-frame-rate");
		}

		public Size getPreviewSize() {
			return this.strToSize(this.get("preview-size"));
		}

		public String getSceneMode() {
			return this.get("scene-mode");
		}

		public List<String> getSupportedAntibanding() {
			return this.split(this.get("antibanding-values"));
		}

		public List<String> getSupportedColorEffects() {
			return this.split(this.get("effect-values"));
		}

		public List<String> getSupportedFlashModes() {
			return this.split(this.get("flash-mode-values"));
		}

		public List<String> getSupportedFocusModes() {
			return this.split(this.get("focus-mode-values"));
		}

		public List<Size> getSupportedJpegThumbnailSizes() {
			return this.splitSize(this.get("jpeg-thumbnail-size-values"));
		}

		public List<Integer> getSupportedPictureFormats() {
			final String value = this.get("picture-format-values");
			final ArrayList<Integer> list = new ArrayList<Integer>();
			final Iterator<String> iterator = this.split(value).iterator();
			while (iterator.hasNext()) {
				final int pixelFormatForCameraFormat = this
						.pixelFormatForCameraFormat(iterator.next());
				if (pixelFormatForCameraFormat == 0) {
					continue;
				}
				list.add(pixelFormatForCameraFormat);
			}
			return list;
		}

		public List<Size> getSupportedPictureSizes() {
			return this.splitSize(this.get("picture-size-values"));
		}

		public List<Integer> getSupportedPreviewFormats() {
			final String value = this.get("preview-format-values");
			final ArrayList<Integer> list = new ArrayList<Integer>();
			final Iterator<String> iterator = this.split(value).iterator();
			while (iterator.hasNext()) {
				final int pixelFormatForCameraFormat = this
						.pixelFormatForCameraFormat(iterator.next());
				if (pixelFormatForCameraFormat == 0) {
					continue;
				}
				list.add(pixelFormatForCameraFormat);
			}
			return list;
		}

		public List<int[]> getSupportedPreviewFpsRange() {
			return this.splitRange(this.get("preview-fps-range-values"));
		}

		@Deprecated
		public List<Integer> getSupportedPreviewFrameRates() {
			return this.splitInt(this.get("preview-frame-rate-values"));
		}

		public List<Size> getSupportedPreviewSizes() {
			return this.splitSize(this.get("preview-size-values"));
		}

		public List<String> getSupportedSceneModes() {
			return this.split(this.get("scene-mode-values"));
		}

		public List<Size> getSupportedVideoSizes() {
			return this.splitSize(this.get("video-size-values"));
		}

		public List<String> getSupportedWhiteBalance() {
			return this.split(this.get("whitebalance-values"));
		}

		public float getVerticalViewAngle() {
			return Float.parseFloat(this.get("vertical-view-angle"));
		}

		public boolean getVideoStabilization() {
			return "true".equals(this.get("video-stabilization"));
		}

		public String getWhiteBalance() {
			return this.get("whitebalance");
		}

		public int getZoom() {
			return this.getInt("zoom", 0);
		}

		public List<Integer> getZoomRatios() {
			return this.splitInt(this.get("zoom-ratios"));
		}

		public boolean isAutoExposureLockSupported() {
			return "true".equals(this.get("auto-exposure-lock-supported"));
		}

		public boolean isAutoWhiteBalanceLockSupported() {
			return "true".equals(this.get("auto-whitebalance-lock-supported"));
		}

		public boolean isSmoothZoomSupported() {
			return "true".equals(this.get("smooth-zoom-supported"));
		}

		public boolean isVideoSnapshotSupported() {
			return "true".equals(this.get("video-snapshot-supported"));
		}

		public boolean isVideoStabilizationSupported() {
			return "true".equals(this.get("video-stabilization-supported"));
		}

		public boolean isZoomSupported() {
			return "true".equals(this.get("zoom-supported"));
		}

		public void remove(final String s) {
			this.mMap.remove(s);
		}

		public void removeGpsData() {
			this.remove("gps-latitude");
			this.remove("gps-longitude");
			this.remove("gps-altitude");
			this.remove("gps-timestamp");
			this.remove("gps-processing-method");
		}

		public void set(final String s, final int n) {
			this.mMap.put(s, Integer.toString(n));
		}

		public void set(final String s, final String s2) {
			if (s.indexOf(61) != -1 || s.indexOf(59) != -1) {
				Log.e("SecCamera-JNI-Java", "Key \"" + s
						+ "\" contains invalid character (= or ;)");
				return;
			}
			if (s2.indexOf(61) != -1 || s2.indexOf(59) != -1) {
				Log.e("SecCamera-JNI-Java", "Value \"" + s2
						+ "\" contains invalid character (= or ;)");
				return;
			}
			this.mMap.put(s, s2);
		}

		public void setAntibanding(final String s) {
			this.set("antibanding", s);
		}

		public void setAutoExposureLock(final boolean b) {
			String s;
			if (b) {
				s = "true";
			} else {
				s = "false";
			}
			this.set("auto-exposure-lock", s);
		}

		public void setAutoWhiteBalanceLock(final boolean b) {
			String s;
			if (b) {
				s = "true";
			} else {
				s = "false";
			}
			this.set("auto-whitebalance-lock", s);
		}

		public void setColorEffect(final String s) {
			this.set("effect", s);
		}

		public void setExposureCompensation(final int n) {
			this.set("exposure-compensation", n);
		}

		public void setFlashMode(final String s) {
			this.set("flash-mode", s);
		}

		public void setFocusAreas(final List<Area> list) {
			this.set("focus-areas", list);
		}

		public void setFocusMode(final String s) {
			this.set("focus-mode", s);
		}

		public void setGpsAltitude(final double n) {
			this.set("gps-altitude", Double.toString(n));
		}

		public void setGpsLatitude(final double n) {
			this.set("gps-latitude", Double.toString(n));
		}

		public void setGpsLongitude(final double n) {
			this.set("gps-longitude", Double.toString(n));
		}

		public void setGpsProcessingMethod(final String s) {
			this.set("gps-processing-method", s);
		}

		public void setGpsTimestamp(final long n) {
			this.set("gps-timestamp", Long.toString(n));
		}

		public void setJpegQuality(final int n) {
			this.set("jpeg-quality", n);
		}

		public void setJpegThumbnailQuality(final int n) {
			this.set("jpeg-thumbnail-quality", n);
		}

		public void setJpegThumbnailSize(final int n, final int n2) {
			this.set("jpeg-thumbnail-width", n);
			this.set("jpeg-thumbnail-height", n2);
		}

		public void setMeteringAreas(final List<Area> list) {
			this.set("metering-areas", list);
		}

		public void setPictureFormat(final int n) {
			final String cameraFormatForPixelFormat = this
					.cameraFormatForPixelFormat(n);
			if (cameraFormatForPixelFormat == null) {
				throw new IllegalArgumentException("Invalid pixel_format=" + n);
			}
			this.set("picture-format", cameraFormatForPixelFormat);
		}

		public void setPictureSize(final int n, final int n2) {
			this.set("picture-size",
					Integer.toString(n) + "x" + Integer.toString(n2));
		}

		public void setPreviewFormat(final int n) {
			final String cameraFormatForPixelFormat = this
					.cameraFormatForPixelFormat(n);
			if (cameraFormatForPixelFormat == null) {
				throw new IllegalArgumentException("Invalid pixel_format=" + n);
			}
			this.set("preview-format", cameraFormatForPixelFormat);
		}

		public void setPreviewFpsRange(final int n, final int n2) {
			this.set("preview-fps-range", "" + n + "," + n2);
		}

		@Deprecated
		public void setPreviewFrameRate(final int n) {
			this.set("preview-frame-rate", n);
		}

		public void setPreviewSize(final int n, final int n2) {
			this.set("preview-size",
					Integer.toString(n) + "x" + Integer.toString(n2));
		}

		public void setRecordingHint(final boolean b) {
			String s;
			if (b) {
				s = "true";
			} else {
				s = "false";
			}
			this.set("recording-hint", s);
		}

		public void setRotation(final int n) {
			if (n == 0 || n == 90 || n == 180 || n == 270) {
				this.set("rotation", Integer.toString(n));
				return;
			}
			throw new IllegalArgumentException("Invalid rotation=" + n);
		}

		public void setSceneMode(final String s) {
			this.set("scene-mode", s);
		}

		public void setVideoStabilization(final boolean b) {
			String s;
			if (b) {
				s = "true";
			} else {
				s = "false";
			}
			this.set("video-stabilization", s);
		}

		public void setWhiteBalance(final String s) {
			this.set("whitebalance", s);
			this.set("auto-whitebalance-lock", "false");
		}

		public void setZoom(final int n) {
			this.set("zoom", n);
		}

		public void unflatten(final String s) {
			this.mMap.clear();
			final StringTokenizer stringTokenizer = new StringTokenizer(s, ";");
			while (stringTokenizer.hasMoreElements()) {
				final String nextToken = stringTokenizer.nextToken();
				final int index = nextToken.indexOf(61);
				if (index == -1) {
					continue;
				}
				this.mMap.put(nextToken.substring(0, index),
						nextToken.substring(index + 1));
			}
		}

	}

	public static class Area {
		public Rect rect;
		public int weight;

		public Area(final Rect rect, final int weight) {
			super();
			this.rect = rect;
			this.weight = weight;
		}

		public boolean equals(final Object o) {
			if (o instanceof Area) {
				final Area area = (Area) o;
				if (this.rect == null) {
					if (area.rect != null) {
						return false;
					}
				} else if (!this.rect.equals((Object) area.rect)) {
					return false;
				}
				if (this.weight == area.weight) {
					return true;
				}
			}
			return false;
		}
	}

}