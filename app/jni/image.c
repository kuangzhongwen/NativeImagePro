#include <jni.h>
#include <stdio.h>
#include <setjmp.h>
#include <android/bitmap.h>
#include <libwebp/webp/decode.h>
#include <libwebp/webp/encode.h>
#include "utils.h"
#include "image.h"

jclass jclass_NullPointerException;
jclass jclass_RuntimeException;

jclass jclass_Options;
jfieldID jclass_Options_inJustDecodeBounds;
jfieldID jclass_Options_outHeight;
jfieldID jclass_Options_outWidth;

const uint32_t PGPhotoEnhanceHistogramBins = 256;
const uint32_t PGPhotoEnhanceSegments = 4;

static inline uint64_t get_colors (const uint8_t *p) {
    return p[0] + (p[1] << 16) + ((uint64_t)p[2] << 32);
}

static void fastBlurMore(int imageWidth, int imageHeight, int imageStride, void *pixels, int radius) {
    uint8_t *pix = (uint8_t *)pixels;
    const int w = imageWidth;
    const int h = imageHeight;
    const int stride = imageStride;
    const int r1 = radius + 1;
    const int div = radius * 2 + 1;
    
    if (radius > 15 || div >= w || div >= h || w * h > 150 * 150 || imageStride > imageWidth * 4) {
        return;
    }
    
    uint64_t *rgb = malloc(imageWidth * imageHeight * sizeof(uint64_t));
    if (rgb == NULL) {
        return;
    }
    
    int x, y, i;
    
    int yw = 0;
    const int we = w - r1;
    for (y = 0; y < h; y++) {
        uint64_t cur = get_colors (&pix[yw]);
        uint64_t rgballsum = -radius * cur;
        uint64_t rgbsum = cur * ((r1 * (r1 + 1)) >> 1);
        
        for (i = 1; i <= radius; i++) {
            uint64_t cur = get_colors (&pix[yw + i * 4]);
            rgbsum += cur * (r1 - i);
            rgballsum += cur;
        }
        
        x = 0;
        
    #define update(start, middle, end) \
            rgb[y * w + x] = (rgbsum >> 6) & 0x00FF00FF00FF00FF; \
            rgballsum += get_colors (&pix[yw + (start) * 4]) - 2 * get_colors (&pix[yw + (middle) * 4]) + get_colors (&pix[yw + (end) * 4]); \
            rgbsum += rgballsum; \
            x++; \

        while (x < r1) {
            update (0, x, x + r1);
        }
        while (x < we) {
            update (x - r1, x, x + r1);
        }
        while (x < w) {
            update (x - r1, x, w - 1);
        }
    #undef update
        
        yw += stride;
    }
    
    const int he = h - r1;
    for (x = 0; x < w; x++) {
        uint64_t rgballsum = -radius * rgb[x];
        uint64_t rgbsum = rgb[x] * ((r1 * (r1 + 1)) >> 1);
        for (i = 1; i <= radius; i++) {
            rgbsum += rgb[i * w + x] * (r1 - i);
            rgballsum += rgb[i * w + x];
        }
        
        y = 0;
        int yi = x * 4;
        
    #define update(start, middle, end) \
            int64_t res = rgbsum >> 6; \
            pix[yi] = res; \
            pix[yi + 1] = res >> 16; \
            pix[yi + 2] = res >> 32; \
            rgballsum += rgb[x + (start) * w] - 2 * rgb[x + (middle) * w] + rgb[x + (end) * w]; \
            rgbsum += rgballsum; \
            y++; \
            yi += stride;
        
        while (y < r1) {
            update (0, y, y + r1);
        }
        while (y < he) {
            update (y - r1, y, y + r1);
        }
        while (y < h) {
            update (y - r1, y, h - 1);
        }
    #undef update
    }
}

static void fastBlur(int imageWidth, int imageHeight, int imageStride, void *pixels, int radius) {
    uint8_t *pix = (uint8_t *)pixels;
    if (pix == NULL) {
        return;
    }
    const int w = imageWidth;
    const int h = imageHeight;
    const int stride = imageStride;
    const int r1 = radius + 1;
    const int div = radius * 2 + 1;
    int shift;
    if (radius == 1) {
        shift = 2;
    } else if (radius == 3) {
        shift = 4;
    } else if (radius == 7) {
        shift = 6;
    } else if (radius == 15) {
        shift = 8;
    } else {
        return;
    }
    
    if (radius > 15 || div >= w || div >= h || w * h > 150 * 150 || imageStride > imageWidth * 4) {
        return;
    }
    
    uint64_t *rgb = malloc(imageWidth * imageHeight * sizeof(uint64_t));
    if (rgb == NULL) {
        return;
    }
    
    int x, y, i;
    
    int yw = 0;
    const int we = w - r1;
    for (y = 0; y < h; y++) {
        uint64_t cur = get_colors (&pix[yw]);
        uint64_t rgballsum = -radius * cur;
        uint64_t rgbsum = cur * ((r1 * (r1 + 1)) >> 1);
        
        for (i = 1; i <= radius; i++) {
            uint64_t cur = get_colors (&pix[yw + i * 4]);
            rgbsum += cur * (r1 - i);
            rgballsum += cur;
        }
        
        x = 0;
        
        #define update(start, middle, end)  \
                rgb[y * w + x] = (rgbsum >> shift) & 0x00FF00FF00FF00FFLL; \
                rgballsum += get_colors (&pix[yw + (start) * 4]) - 2 * get_colors (&pix[yw + (middle) * 4]) + get_colors (&pix[yw + (end) * 4]); \
                rgbsum += rgballsum;        \
                x++;                        \

        while (x < r1) {
            update (0, x, x + r1);
        }
        while (x < we) {
            update (x - r1, x, x + r1);
        }
        while (x < w) {
            update (x - r1, x, w - 1);
        }
        
        #undef update
        
        yw += stride;
    }
    
    const int he = h - r1;
    for (x = 0; x < w; x++) {
        uint64_t rgballsum = -radius * rgb[x];
        uint64_t rgbsum = rgb[x] * ((r1 * (r1 + 1)) >> 1);
        for (i = 1; i <= radius; i++) {
            rgbsum += rgb[i * w + x] * (r1 - i);
            rgballsum += rgb[i * w + x];
        }
        
        y = 0;
        int yi = x * 4;
        
        #define update(start, middle, end)  \
                int64_t res = rgbsum >> shift;   \
                pix[yi] = res;              \
                pix[yi + 1] = res >> 16;    \
                pix[yi + 2] = res >> 32;    \
                rgballsum += rgb[x + (start) * w] - 2 * rgb[x + (middle) * w] + rgb[x + (end) * w]; \
                rgbsum += rgballsum;        \
                y++;                        \
                yi += stride;
        
        while (y < r1) {
            update (0, y, y + r1);
        }
        while (y < he) {
            update (y - r1, y, y + r1);
        }
        while (y < h) {
            update (y - r1, y, h - 1);
        }
        #undef update
    }
    
    free(rgb);
}

JNIEXPORT void Java_kzw_io_nativeimagepro_NativeImageUtilities_blurBitmap(JNIEnv *env, jclass class, jobject bitmap, int radius, int unpin, int width, int height, int stride) {
    if (!bitmap) {
        return;
    }
    
    if (!width || !height || !stride) {
        return;
    }
    
    void *pixels = 0;
    if (AndroidBitmap_lockPixels(env, bitmap, &pixels) < 0) {
        return;
    }
    if (radius <= 3) {
        fastBlur(width, height, stride, pixels, radius);
    } else {
        fastBlurMore(width, height, stride, pixels, radius);
    }
    if (unpin) {
        AndroidBitmap_unlockPixels(env, bitmap);
    }
}

JNIEXPORT int Java_kzw_io_nativeimagepro_NativeImageUtilities_pinBitmap(JNIEnv *env, jclass class, jobject bitmap) {
    if (bitmap == NULL) {
        return 0;
    }
    unsigned char *pixels;
    return AndroidBitmap_lockPixels(env, bitmap, &pixels) >= 0 ? 1 : 0;
}

JNIEXPORT void Java_kzw_io_nativeimagepro_NativeImageUtilities_unpinBitmap(JNIEnv *env, jclass class, jobject bitmap) {
    if (bitmap == NULL) {
        return;
    }
    AndroidBitmap_unlockPixels(env, bitmap);
}

JNIEXPORT jboolean Java_kzw_io_nativeimagepro_NativeImageUtilities_loadWebpImage(JNIEnv *env, jclass class, jobject outputBitmap, jobject buffer, jint len, jobject options, jboolean unpin) {
    if (!buffer) {
        (*env)->ThrowNew(env, jclass_NullPointerException, "Input buffer can not be null");
        return 0;
    }
    
    jbyte *inputBuffer = (*env)->GetDirectBufferAddress(env, buffer);
    
    int bitmapWidth = 0;
    int bitmapHeight = 0;
    if (!WebPGetInfo((uint8_t*)inputBuffer, len, &bitmapWidth, &bitmapHeight)) {
        (*env)->ThrowNew(env, jclass_RuntimeException, "Invalid WebP format");
        return 0;
    }
    
    if (options && (*env)->GetBooleanField(env, options, jclass_Options_inJustDecodeBounds) == JNI_TRUE) {
        (*env)->SetIntField(env, options, jclass_Options_outWidth, bitmapWidth);
        (*env)->SetIntField(env, options, jclass_Options_outHeight, bitmapHeight);
        return 1;
    }
    
    if (!outputBitmap) {
        (*env)->ThrowNew(env, jclass_NullPointerException, "output bitmap can not be null");
        return 0;
    }
    
    AndroidBitmapInfo bitmapInfo;
    if (AndroidBitmap_getInfo(env, outputBitmap, &bitmapInfo) != ANDROID_BITMAP_RESUT_SUCCESS) {
        (*env)->ThrowNew(env, jclass_RuntimeException, "Failed to get Bitmap information");
        return 0;
    }
    
    void *bitmapPixels = 0;
    if (AndroidBitmap_lockPixels(env, outputBitmap, &bitmapPixels) != ANDROID_BITMAP_RESUT_SUCCESS) {
        (*env)->ThrowNew(env, jclass_RuntimeException, "Failed to lock Bitmap pixels");
        return 0;
    }
    
    if (!WebPDecodeRGBAInto((uint8_t*)inputBuffer, len, (uint8_t*)bitmapPixels, bitmapInfo.height * bitmapInfo.stride, bitmapInfo.stride)) {
        AndroidBitmap_unlockPixels(env, outputBitmap);
        (*env)->ThrowNew(env, jclass_RuntimeException, "Failed to decode webp image");
        return 0;
    }
    
    if (unpin && AndroidBitmap_unlockPixels(env, outputBitmap) != ANDROID_BITMAP_RESUT_SUCCESS) {
        (*env)->ThrowNew(env, jclass_RuntimeException, "Failed to unlock Bitmap pixels");
        return 0;
    }
    
    return 1;
}
