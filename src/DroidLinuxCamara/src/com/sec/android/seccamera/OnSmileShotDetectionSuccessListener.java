package com.sec.android.seccamera;

public interface OnSmileShotDetectionSuccessListener {
    void onSmileShotDetectionSuccess();
    
    void onSmileShotFaceRectChanged(byte[] p0);
    
    void onSmileShotSmileRectChanged(byte[] p0);
}
