package com.sec.android.seccamera;

public interface OnBurstShotEventListener {
    void onBurstShotCapturingProgressed(int p0, int p1);
    
    void onBurstShotCapturingStopped(int p0);
    
    void onBurstShotSavingCompleted();
}
