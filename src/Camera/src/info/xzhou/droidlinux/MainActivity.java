package info.xzhou.droidlinux;

import android.os.Bundle;
import android.app.Activity;
import android.hardware.Camera;
import android.view.Menu;
import android.widget.Button;


public class MainActivity extends Activity {
	
	Camera mCamera;
	
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		
		
	}

	
	
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

}
