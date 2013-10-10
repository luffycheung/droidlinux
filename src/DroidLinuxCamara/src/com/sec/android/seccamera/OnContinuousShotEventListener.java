package com.sec.android.seccamera;

public interface OnContinuousShotEventListener {
    void onContinuousShotCapturingProgressed(int p0, int p1);
    
    void onContinuousShotCapturingStopped(int p0);
    
    void onContinuousShotSavingCompleted();
}
