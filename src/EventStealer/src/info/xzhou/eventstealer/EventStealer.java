package info.xzhou.eventstealer;

import android.os.Bundle;
import android.app.Activity;
import android.util.Log;
import android.view.Menu;

public class EventStealer extends Activity {
	private final static String TAG = "EventReader";

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_event_stealer);
		Log.i(TAG, stringFromJNI());
		//startLoggingThread();
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.event_stealer, menu);
		return true;
	}
	
	public void startLoggingThread(){
		Thread eventLogger = new Thread(new Runnable() {
			@Override
			public void run() {
				// TODO Auto-generated method stub
				Log.i(TAG, "starting");
				startLogging();
			}
		});
		eventLogger.start();
	}
	
	// start native logging, 
	public native int startLogging();
	public native String stringFromJNI();
	
	//private native static int getValue();
    static {
        System.loadLibrary("ReadEvent");
    }
}
