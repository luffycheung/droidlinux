package com.example.droidlinuxcamara;

import android.os.Bundle;
import android.app.Activity;
import android.util.Log;
import android.view.Menu;
import android.view.View;
import android.widget.Button;

public class MainActivity extends Activity {
	private final static String TAG = "MINA_ACTIVITY";
	private Button btnTakePicture;
	private Button btnCreateFile;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		btnTakePicture = (Button)findViewById(R.id.takePicture);
		btnCreateFile = (Button)findViewById(R.id.createFile);
		registerHandler();
	}
	
	private void registerHandler(){
		btnTakePicture.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				Log.i(TAG, "click get picture");
				startTakingPictureThread();
				//openTestFile();
			}
		});
		
		btnCreateFile.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				openTestFile();
			}
		});
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}
	
	private void startTakingPictureThread(){
		Thread takePictureThread = new Thread(new Runnable() {
			
			@Override
			public void run() {
				// TODO Auto-generated method stub
				int i = getPicture();
				Log.i(TAG, "return from getPicture() " + i);
			}
		});
		takePictureThread.start();
	}

	private native int getPicture();
	private native int getPictureNoMmap();
	private native int openTestFile();
	
    static {
        System.loadLibrary("access-camera");
    }
    
}
