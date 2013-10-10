package com.sec.android.seccamera;

public interface OnHDRShotEventListener {
    void onHDRShotAllProgressCompleted(boolean p0);
    
    void onHDRShotResultCompleted(boolean p0);
    
    void onHDRShotResultProgress(int p0);
    
    void onHDRShotResultStarted();
}
