package com.sec.android.seccamera;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;

import com.example.droidlinuxcamara.R;

import android.os.Bundle;
import android.app.Activity;
import android.util.Log;
import android.view.Menu;
import android.view.View;
import android.widget.Button;
import android.content.Context;
import android.view.SurfaceView;
import android.view.View.OnClickListener;
import android.view.View.OnLongClickListener;
import android.view.ViewGroup.LayoutParams;
import android.view.Window;
import android.view.WindowManager;
import android.widget.FrameLayout;

public class MainActivity extends Activity {
	public static final int MEDIA_TYPE_IMAGE = 1;

	private final static String TAG = "MINA_ACTIVITY";
	private Button btnTakePicture;
	private Button btnCreateFile;
	private SecCamera mCamera;
	Preview preview;
	Button buttonClick;
	String fileName;
	Activity act;
	Context ctx;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		ctx = this;
		act = this;
		requestWindowFeature(Window.FEATURE_NO_TITLE);
		getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);

		setContentView(R.layout.activity_main);

		preview = new Preview(this, (SurfaceView)findViewById(R.id.surfaceView));
		preview.setLayoutParams(new LayoutParams(LayoutParams.FILL_PARENT, LayoutParams.FILL_PARENT));
		((FrameLayout) findViewById(R.id.preview)).addView(preview);
		preview.setKeepScreenOn(true);

		buttonClick = (Button) findViewById(R.id.buttonClick);

		buttonClick.setOnClickListener(new OnClickListener() {
			public void onClick(View v) {
				//				preview.camera.takePicture(shutterCallback, rawCallback, jpegCallback);
				mCamera.takePicture(shutterCallback, rawCallback, jpegCallback);
			}
		});

		buttonClick.setOnLongClickListener(new OnLongClickListener(){
			@Override
			public boolean onLongClick(View arg0) {
				mCamera.autoFocus(new AutoFocusCallback(){
					@Override
					public void onAutoFocus(int arg0, SecCamera arg1) {
						//camera.takePicture(shutterCallback, rawCallback, jpegCallback);
					}
				});
				return true;
			}
		});
//		setContentView(R.layout.activity_main);
//		btnTakePicture = (Button) findViewById(R.id.takePicture);
//		btnCreateFile = (Button) findViewById(R.id.createFile);
//		registerHandler();
	}
	


	@Override
	protected void onResume() {
		super.onResume();
		//      preview.camera = Camera.open();
		mCamera = mCamera.open();
		mCamera.startPreview();
		preview.setCamera(mCamera);
	}

	@Override
	protected void onPause() {
		if(mCamera != null) {
			mCamera.stopPreview();
			preview.setCamera(null);
			mCamera.release();
			mCamera = null;
		}
		super.onPause();
	}

	private void resetCam() {
		mCamera.startPreview();
		preview.setCamera(mCamera);
	}

	ShutterCallback shutterCallback = new ShutterCallback() {
		public void onShutter() {
			// Log.d(TAG, "onShutter'd");
		}
	};
	
	PictureCallback rawCallback = new PictureCallback() {
		public void onPictureTaken(byte[] data, SecCamera camera) {
			// Log.d(TAG, "onPictureTaken - raw");
		}
	};

	static {
		System.loadLibrary("access-camera");
		System.loadLibrary("seccamera_jni");
	}

	private PictureCallback mPicture = new PictureCallback() {

		@Override
		public void onPictureTaken(byte[] data, SecCamera camera) {
/*
			File pictureFile = getOutputMediaFile(MEDIA_TYPE_IMAGE);
			if (pictureFile == null) {
				Log.d(TAG,
						"Error creating media file, check storage permissions: "
								+ e.getMessage());
				return;
			}

			try {
				FileOutputStream fos = new FileOutputStream(pictureFile);
				fos.write(data);
				fos.close();
			} catch (FileNotFoundException e) {
				Log.d(TAG, "File not found: " + e.getMessage());
			} catch (IOException e) {
				Log.d(TAG, "Error accessing file: " + e.getMessage());
			}*/
		}
	};
	PictureCallback jpegCallback = new PictureCallback() {
		public void onPictureTaken(byte[] data, SecCamera camera) {
			FileOutputStream outStream = null;
			try {
				// Write to SD Card
				String fileName = String.format("/sdcard/Camtest/%d.jpg",
						System.currentTimeMillis());
				outStream = new FileOutputStream(fileName);
				outStream.write(data);
				outStream.close();
				Log.d(TAG, "onPictureTaken - wrote bytes: " + data.length);

				resetCam();

			} catch (FileNotFoundException e) {
				e.printStackTrace();
			} catch (IOException e) {
				e.printStackTrace();
			} finally {
			}
			Log.d(TAG, "onPictureTaken - jpeg");
		}
	};
}
