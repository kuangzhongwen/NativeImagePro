package kzw.io.nativeimagepro;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Build;
import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.ImageView;
import java.io.File;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;

public class MainActivity extends AppCompatActivity {

    private static final String TAG = "MainActivity";

    private ImageView imageView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        imageView = (ImageView) findViewById(R.id.image);
        findViewById(R.id.blur_btn).setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                blurImage();
            }
        });
        findViewById(R.id.webp_btn).setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                loadWebp();
            }
        });
    }

    private void blurImage() {
        new Thread(new Runnable() {
            @Override
            public void run() {
                final Bitmap bitmap = BitmapFactory.decodeResource(getResources(), R.drawable.test);
                BitmapFactory.Options opts = new BitmapFactory.Options();
                opts.inSampleSize = 1;
                if (Build.VERSION.SDK_INT < 21) {
                    opts.inPurgeable = true;
                }
                long start = System.currentTimeMillis();
                NativeImageUtilities.blurBitmap(bitmap, 50, opts.inPurgeable ? 0 : 1, bitmap.getWidth(), bitmap.getHeight(),
                    bitmap.getRowBytes());
                Log.i(TAG, "blur image cost = " + (System.currentTimeMillis() - start));
                imageView.post(new Runnable() {
                    @Override
                    public void run() {
                        imageView.setImageBitmap(bitmap);
                    }
                });
            }
        }).start();
    }

    private void loadWebp() {
        new Thread(new Runnable() {
            @Override
            public void run() {
                try {
                    // 先得获取 sd 卡权限
                    File cacheFile = new File(Environment.getExternalStorageDirectory() + "/test.jpg");
                    RandomAccessFile file = new RandomAccessFile(cacheFile, "r");
                    ByteBuffer buffer = file.getChannel().map(FileChannel.MapMode.READ_ONLY, 0, cacheFile.length());

                    BitmapFactory.Options bmOptions = new BitmapFactory.Options();
                    bmOptions.inJustDecodeBounds = true;
                    NativeImageUtilities.loadWebpImage(null, buffer, buffer.limit(), bmOptions, true);
                    final Bitmap image = Bitmap.createBitmap(bmOptions.outWidth, bmOptions.outHeight, Bitmap.Config.ARGB_8888);
                    NativeImageUtilities.loadWebpImage(image, buffer, buffer.limit(), null, true);
                    imageView.post(new Runnable() {
                        @Override
                        public void run() {
                            imageView.setImageBitmap(image);
                        }
                    });
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        });
    }
}
