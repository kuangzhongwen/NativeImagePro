LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_CFLAGS := -Wall -DANDROID -DHAVE_MALLOC_H -DHAVE_PTHREAD -DWEBP_USE_THREAD -finline-functions -ffast-math -ffunction-sections -fdata-sections -O0
LOCAL_C_INCLUDES += ./jni/libwebp/src
LOCAL_ARM_MODE := arm
LOCAL_STATIC_LIBRARIES := cpufeatures
LOCAL_MODULE := webp

ifneq ($(findstring armeabi-v7a, $(TARGET_ARCH_ABI)),)
  NEON := c.neon
else
  NEON := c
endif

LOCAL_SRC_FILES := \
./libwebp/dec/alpha.c \
./libwebp/dec/buffer.c \
./libwebp/dec/frame.c \
./libwebp/dec/idec.c \
./libwebp/dec/io.c \
./libwebp/dec/quant.c \
./libwebp/dec/tree.c \
./libwebp/dec/vp8.c \
./libwebp/dec/vp8l.c \
./libwebp/dec/webp.c \
./libwebp/dsp/alpha_processing.c \
./libwebp/dsp/alpha_processing_sse2.c \
./libwebp/dsp/cpu.c \
./libwebp/dsp/dec.c \
./libwebp/dsp/dec_clip_tables.c \
./libwebp/dsp/dec_mips32.c \
./libwebp/dsp/dec_neon.$(NEON) \
./libwebp/dsp/dec_sse2.c \
./libwebp/dsp/enc.c \
./libwebp/dsp/enc_avx2.c \
./libwebp/dsp/enc_mips32.c \
./libwebp/dsp/enc_neon.$(NEON) \
./libwebp/dsp/enc_sse2.c \
./libwebp/dsp/lossless.c \
./libwebp/dsp/lossless_mips32.c \
./libwebp/dsp/lossless_neon.$(NEON) \
./libwebp/dsp/lossless_sse2.c \
./libwebp/dsp/upsampling.c \
./libwebp/dsp/upsampling_neon.$(NEON) \
./libwebp/dsp/upsampling_sse2.c \
./libwebp/dsp/yuv.c \
./libwebp/dsp/yuv_mips32.c \
./libwebp/dsp/yuv_sse2.c \
./libwebp/enc/alpha.c \
./libwebp/enc/analysis.c \
./libwebp/enc/backward_references.c \
./libwebp/enc/config.c \
./libwebp/enc/cost.c \
./libwebp/enc/filter.c \
./libwebp/enc/frame.c \
./libwebp/enc/histogram.c \
./libwebp/enc/iterator.c \
./libwebp/enc/picture.c \
./libwebp/enc/picture_csp.c \
./libwebp/enc/picture_psnr.c \
./libwebp/enc/picture_rescale.c \
./libwebp/enc/picture_tools.c \
./libwebp/enc/quant.c \
./libwebp/enc/syntax.c \
./libwebp/enc/token.c \
./libwebp/enc/tree.c \
./libwebp/enc/vp8l.c \
./libwebp/enc/webpenc.c \
./libwebp/utils/bit_reader.c \
./libwebp/utils/bit_writer.c \
./libwebp/utils/color_cache.c \
./libwebp/utils/filters.c \
./libwebp/utils/huffman.c \
./libwebp/utils/huffman_encode.c \
./libwebp/utils/quant_levels.c \
./libwebp/utils/quant_levels_dec.c \
./libwebp/utils/random.c \
./libwebp/utils/rescaler.c \
./libwebp/utils/thread.c \
./libwebp/utils/utils.c \

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_PRELINK_MODULE := false

LOCAL_MODULE 	:= native_image
LOCAL_CFLAGS 	:= -w -std=c11 -Os -DNULL=0 -DSOCKLEN_T=socklen_t -DLOCALE_NOT_USED -D_LARGEFILE_SOURCE=1 -D_FILE_OFFSET_BITS=64
LOCAL_CPPFLAGS 	:= -DBSD=1 -ffast-math -Os -funroll-loops -std=c++11
LOCAL_LDLIBS 	:= -ljnigraphics -llog -lz -latomic -lOpenSLES
LOCAL_STATIC_LIBRARIES := webp

LOCAL_SRC_FILES     += \
./utils.c \
./image.c \

include $(BUILD_SHARED_LIBRARY)
$(call import-module,android/cpufeatures)