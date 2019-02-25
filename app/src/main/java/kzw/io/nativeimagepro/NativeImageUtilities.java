package kzw.io.nativeimagepro;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import java.nio.ByteBuffer;

public class NativeImageUtilities {

    static {
        System.loadLibrary("native_image");
    }

    /**
     * jni 层调用 AndroidBitmap_lockPixels 拿到像素的缓存地址
     */
    public native static int pinBitmap(Bitmap bitmap);

    /**
     * jni 层调用 AndroidBitmap_unlockPixels
     */
    public native static void unpinBitmap(Bitmap bitmap);

    /**
     * 对图形进行高斯模糊处理
     */
    public native static void blurBitmap(Object bitmap, int radius, int unpin, int width, int height, int stride);

    /**
     * 加载 webp 图片，将 ByteBuffer 转换成 webp
     */
    public native static boolean loadWebpImage(Bitmap bitmap, ByteBuffer buffer, int len, BitmapFactory.Options options, boolean unpin);
}
