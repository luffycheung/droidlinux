package com.sec.android.seccamera;

public interface OnActionShotEventListener {
	void onActionShotAcquisitionProgress(int p0);

	void onActionShotCaptured();

	void onActionShotCreatingResultCompleted(boolean p0);

	void onActionShotCreatingResultProgress(int p0);

	void onActionShotCreatingResultStarted();

	void onActionShotRectChanged(byte[] p0);
}
