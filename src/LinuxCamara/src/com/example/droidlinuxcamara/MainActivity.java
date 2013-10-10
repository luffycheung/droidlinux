package com.example.droidlinuxcamara;

import java.io.FileOutputStream;
import java.io.IOException;

import android.os.Bundle;
import android.app.Activity;
import android.util.Log;
import android.view.Menu;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.graphics.Bitmap;
import android.graphics.Matrix;

public class MainActivity extends Activity {
	private final static String TAG = "MINA_ACTIVITY";
	private Button btnTakePicture;
	private ImageView imageView;

	private Bitmap bmp = null;
	private Bitmap bmpRet = null;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		btnTakePicture = (Button) findViewById(R.id.takePicture);
		imageView = (ImageView) findViewById(R.id.imageView1);
		registerHandler();
	}

	private void registerHandler() {
		btnTakePicture.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				Log.i(TAG, "click get picture");
				startTakingPictureThread();
				// openTestFile();
			}
		});
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

	private void startTakingPictureThread() {
		Thread takePictureThread = new Thread(new Runnable() {

			@Override
			public void run() {
				// TODO Auto-generated method stub
				getPicture();
				bmp = Bitmap.createBitmap(640, 480, Bitmap.Config.ARGB_8888);

				pixeltobmp(bmp);
				// write to bmp file;
				Matrix matrix = new Matrix();
			    matrix.postRotate(270);
			    bmpRet =  Bitmap.createBitmap(bmp, 0, 0, bmp.getWidth(), bmp.getHeight(), matrix, true);
				try {
					FileOutputStream out = new FileOutputStream(
							"/sdcard/test3.png");
					bmpRet.compress(Bitmap.CompressFormat.PNG, 90, out);
					out.close();
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
		});
		takePictureThread.start();
		try {
			takePictureThread.join();
//			Bitmap bmp = BitmapFactory.decodeByteArray(buffer, start, a);
			imageView.setImageBitmap(bmpRet);
		} catch (InterruptedException e) {
		}
	}

	private native void pixeltobmp(Bitmap bmp);

	private native int getPicture();

	private native int getPictureNoMmap();

	private native int openTestFile();

	static {
		System.loadLibrary("access-camera");
	}

}