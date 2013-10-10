package com.sec.android.seccamera;

public interface OnPanoramaEventListener {
    void onPanoramaCaptured();
    
    void onPanoramaCapturedNew();
    
    void onPanoramaDirectionChanged(int p0);
    
    void onPanoramaError(int p0);
    
    void onPanoramaProgressStitching(int p0);
    
    void onPanoramaRectChanged(int p0, int p1);
}
