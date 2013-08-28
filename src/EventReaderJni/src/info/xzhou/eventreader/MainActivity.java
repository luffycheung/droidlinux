package info.xzhou.eventreader;

import android.os.Bundle;
import android.app.Activity;
import android.util.Log;
import android.view.Menu;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

public class MainActivity extends Activity {
	private final static String TAG = "EventReaderMainActivity";
	
	private Button btn;
	private TextView tv; 

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		
		btn = (Button) findViewById(R.id.button1);
		tv = (TextView) findViewById(R.id.textView1);
		tv.setText(stringFromJNI());
		registerEvent();
	}
	
	private void registerEvent(){
		btn.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				Log.i(TAG, "button clicked");
				startLoggingThread();
			}
		});
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

	public void startLoggingThread(){
		Thread eventLogger = new Thread(new Runnable() {
			@Override
			public void run() {
				// TODO Auto-generated method stub
				Log.i(TAG, "starting thread");
				startLogging();
			}
		});
		eventLogger.start();
	}
	
	
    public native String stringFromJNI();
    public native int startLogging();
    
    static {
    	Log.i(TAG, "loading lib");
        System.loadLibrary("ReadEvent");
        Log.i(TAG, "loading lib complete");
    }
	

}
