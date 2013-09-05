package info.xzhou.droidlinux;

import java.io.IOException;
import java.io.BufferedReader;
import java.io.InputStreamReader;

import android.os.Bundle;
import android.app.Activity;
import android.util.Log;
import android.view.Menu;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;

public class MainActivity extends Activity {
	
	static final String TAG = "SCREEN_CAP";
	Button capButton;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		capButton = (Button)findViewById(R.id.cap);
		register_event();
	}

	private void register_event(){
		capButton.setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				exec_screencap();
			}
		});
	}
	
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}
	
	public String exec_screencap(){
		try{
			Process process = Runtime.getRuntime().exec("/system/bin/screencap");
			InputStreamReader reader = new InputStreamReader(process.getInputStream());
	    	BufferedReader bufferedReader = new BufferedReader(reader);
	    	int numRead;
	    	char[] buffer = new char[5000];
	    	StringBuffer commandOutput = new StringBuffer();
	    	while ((numRead = bufferedReader.read(buffer)) > 0) {
	    		commandOutput.append(buffer, 0, numRead);
	        }
	        bufferedReader.close();
	        Log.e(TAG, commandOutput.toString());
	        process.waitFor();
		    return commandOutput.toString();
		}catch (IOException e){
			//do nothing
			Log.e(TAG, e.toString());
		} catch (InterruptedException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		return "";
	}

}
