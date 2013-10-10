package com.sec.android.seccamera;

public class Size
{
    public int height;
    public int width;
    
    public Size(final int width, final int height) {
        super();
        this.width = width;
        this.height = height;
    }
    
    public boolean equals(final Object o) {
        if (o instanceof Size) {
            final Size size = (Size)o;
            if (this.width == size.width && this.height == size.height) {
                return true;
            }
        }
        return false;
    }
    
    public int hashCode() {
        return 32713 * this.width + this.height;
    }
}