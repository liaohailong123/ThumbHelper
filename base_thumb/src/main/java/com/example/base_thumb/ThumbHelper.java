package com.example.base_thumb;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.PixelFormat;
import android.graphics.Rect;
import android.graphics.RectF;
import android.graphics.SurfaceTexture;
import android.media.Image;
import android.media.ImageReader;
import android.os.Handler;
import android.os.Looper;
import android.view.Surface;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.WorkerThread;

import com.example.base_thumb.reader.ThumbInfo;
import com.example.base_thumb.reader.ThumbReader;
import com.example.base_thumb.render.GLES2Util;
import com.example.base_thumb.render.HelloEGLCore;
import com.example.base_thumb.render.HelloGLProgram;
import com.example.base_thumb.util.MediaUtil;
import com.example.base_thumb.util.MetaData;
import com.example.base_thumb.util.ThumbUtil;

import java.nio.ByteBuffer;
import java.util.LinkedList;

/**
 * Author: liaohailong
 * Date: 2021/8/14
 * Time: 10:09
 * Description: 视频缩略图解析：MediaCodec硬解 + OpenGL OES转像素 + ImageReader压缩图片尺寸
 **/
public class ThumbHelper implements ImageReader.OnImageAvailableListener, SurfaceTexture.OnFrameAvailableListener {
    @NonNull
    private final Context context;

    @Nullable
    private Callback callback;
    @NonNull
    private final Handler mainHandler;
    @NonNull
    private final Handler workHandler;
    @Nullable
    private Handler imageReaderHandler;
    @Nullable
    private Handler bitmapCreateHandler;
    private volatile Status status = Status.IDLE;


    @NonNull
    private final ThumbAttr attr;
    private int renderIndex = -1;
    private ImageReader imageReader;

    private final HelloGLProgram glProgram;
    private final HelloEGLCore eglCore;
    private SurfaceTexture surfaceTexture;
    private ThumbReader thumbReader;

    private final LinkedList<Long> renderTimeUsList = new LinkedList<>();


    public ThumbHelper(@NonNull Context context, @NonNull ThumbAttr attr) {
        this.context = context;
        this.attr = attr;
        mainHandler = new Handler(Looper.getMainLooper());
        workHandler = MediaUtil.newWorkHandler("thumb-worker");
        glProgram = new HelloGLProgram(HelloGLProgram.Type.SAMPLER_OES);
        eglCore = new HelloEGLCore(context, glProgram);
    }

    @NonNull
    public ThumbAttr getAttr() {
        return attr;
    }

    /**
     * 开始获取缩略图，缩略图根据desireWidth和desireHeight，以CenterCrop模式取视频帧中的内容作为结果
     *
     * @param callback 解析回调
     */
    public void start(@NonNull Callback callback) {
        this.callback = callback;

        Thread thumbWorkThread = workHandler.getLooper().getThread();
        if (Thread.currentThread().equals(thumbWorkThread)) {
            _start();
        } else
            workHandler.post(this::_start);
    }

    /**
     * 开始获取缩略图，缩略图根据desireWidth和desireHeight，以CenterCrop模式取视频帧中的内容作为结果
     */
    @WorkerThread
    private void _start() {
        renderIndex = -1;
        prepare(() -> thumbReader.start(attr));
    }


    private void prepare(@NonNull Runnable complete) {
        if (status.ordinal() > Status.IDLE.ordinal()) {
            // 当前处于已准备好的状态
            if (status == Status.PREPARED) {
                eglCore.post(() -> prepareDecoder(complete));
            }
            return;
        }

        String path = attr.getPath();
        // 解析多媒体文件信息，判断是否可用
        MetaData metaData = MediaUtil.getMetaData(path);
        if (metaData == null) {
            error("Can not get meta data from path: " + path);
            return;
        }
        int desireWidth = attr.getDesireWidth();
        int desireHeight = attr.getDesireHeight();


        // 等比例缩放
        int width = metaData.getDisplayWidth();
        int height = metaData.getDisplayHeight();
        int rotation = metaData.getRotation();
        float scaleW = desireWidth * 1f / width;
        float scaleH = desireHeight * 1f / height;
        float scale = Math.max(scaleW, scaleH);
        int adjustW = (int) (width * scale);
        int adjustH = (int) (height * scale);
        // 尺寸必须偶数
        adjustW = MediaUtil.even(adjustW);
        adjustH = MediaUtil.even(adjustH);

        // 准备OpenGL渲染部分
        imageReader = ImageReader.newInstance(adjustW, adjustH, PixelFormat.RGBA_8888, 3);
        imageReaderHandler = MediaUtil.newWorkHandler("thumb-helper-image-reader");
        imageReader.setOnImageAvailableListener(this, imageReaderHandler);
        bitmapCreateHandler = MediaUtil.newWorkHandler("thumb-helper-bitmap-create");

        // 初始化egl环境
        eglCore.init(new Runnable() {
            @Override
            public void run() {
                eglCore.setSurface(imageReader.getSurface(), () -> {
                    glProgram.setRotation(-rotation); // 视频旋转角度纠正
                    // 画面镜像，这个会后作用于上面的旋转角度，可能会有冲突
//                    glProgram.setMirror(false, false);
                    prepareDecoder(complete);
                });
            }
        });

        // 标记准备就绪，防止多次调用prepare
        status = Status.PREPARING;
    }

    private void error(String msg, Object... obj) {
        String errorMsg = String.format(msg, obj);
        mainHandler.post(() -> {
            if (callback != null) callback.onError(new IllegalStateException(errorMsg));
        });
    }

    private void prepareDecoder(@NonNull Runnable complete) {
        status = Status.PREPARED;

        String path = attr.getPath();
        boolean mediacodec = attr.isMediacodec();

        // 创建OES纹理
        int[] textures = new int[1];
        GLES2Util.generateOESTexture(textures);
        surfaceTexture = new SurfaceTexture(textures[0]);
        surfaceTexture.setOnFrameAvailableListener(this);
        Surface surface = new Surface(surfaceTexture);
        glProgram.setTexture(textures[0]);

        // 释放上个资源
        if (thumbReader != null) {
            thumbReader.release();
            thumbReader = null;
        }
        // 准备开始解码
        thumbReader = new ThumbReader();
        thumbReader.config(path, mediacodec, surface, new ThumbReader.Callback() {
            @Override
            public void onFailure(@NonNull Exception e) {
                surfaceTexture.release();
                error(e.toString());
            }

            @Override
            public void onRender(long timestamps) {
                renderTimeUsList.addLast(timestamps);
            }

            @Override
            public void onComplete() {

            }
        });

        complete.run();
    }

    /**
     * release之后，这个ThumbHelper就不能再start了
     * 重新new一个ThumbHelper来start
     */
    public void release() {
        callback = null;
        workHandler.getLooper().quit();
        if (imageReaderHandler != null) imageReaderHandler.getLooper().quitSafely();
        if (bitmapCreateHandler != null) bitmapCreateHandler.getLooper().quitSafely();
        if (thumbReader != null) thumbReader.release();
        if (surfaceTexture != null) surfaceTexture.release();
        if (imageReader != null) imageReader.close();
        eglCore.release();
        status = Status.RELEASED;
    }

    @Override
    public void onFrameAvailable(SurfaceTexture surfaceTexture) {

        try {
            surfaceTexture.updateTexImage();
        } catch (Exception e) {
            e.printStackTrace();
        }
        eglCore.updateFrame();
    }

    private final Rect src = new Rect();
    private final RectF dst = new RectF();
    private final Rect result = new Rect();
    private final Paint paint = new Paint();

    @Override
    public void onImageAvailable(ImageReader reader) {
//        long startTime = System.currentTimeMillis();
        Image image = reader.acquireLatestImage();
        Long timestampUs = renderTimeUsList.poll();

        int width = image.getWidth();
        int height = image.getHeight();
        final Image.Plane[] planes = image.getPlanes();
        final ByteBuffer buffer = planes[0].getBuffer();
        int pixelStride = planes[0].getPixelStride();
        int rowStride = planes[0].getRowStride();
        int rowPadding = rowStride - pixelStride * width;
        int paddingRight = rowPadding / pixelStride;

        int fakeWidth = width + paddingRight;
        Bitmap inBitmap = Bitmap.createBitmap(fakeWidth, height, Bitmap.Config.ARGB_8888);
        inBitmap.copyPixelsFromBuffer(buffer);

        image.close();

        renderIndex++;
        final int currIndex = renderIndex;
        final long currTimestampsUs = timestampUs == null ? 0 : timestampUs;
        if (bitmapCreateHandler != null)
            bitmapCreateHandler.post(() -> {
                if (Status.RELEASED == status) {
                    inBitmap.recycle();
                    return;
                }
                int desireWidth = attr.getDesireWidth();
                int desireHeight = attr.getDesireHeight();
                int count = attr.getCount();
                // 尺寸矫正
                Bitmap bitmap = Bitmap.createBitmap(desireWidth, desireHeight, Bitmap.Config.ARGB_8888);
                Canvas canvas = new Canvas(bitmap);
                src.set(0, 0, inBitmap.getWidth() - paddingRight, inBitmap.getHeight());
                dst.set(0f, 0f, bitmap.getWidth(), bitmap.getHeight());

                // CenterCrop计算
                ThumbUtil.centerCrop(src, dst, result);
                src.set(result);

                paint.setAntiAlias(true); // 抗锯齿
                paint.setFilterBitmap(true); // 滤波
                canvas.drawColor(Color.BLACK);
                canvas.drawBitmap(inBitmap, src, dst, paint);
                inBitmap.recycle();
                mainHandler.post(() -> {
                    if (callback != null) {
                        callback.onThumbRead(new ThumbInfo(bitmap, currTimestampsUs, currIndex));
                        if (currIndex >= count - 1) callback.onComplete();
                    }
                });
            });

//        long costTime = System.currentTimeMillis() - startTime;
//        Log.i("Victor", "onImageAvailable cost " + costTime + ", count = " + (renderIndex + 1));

        // 通知解析下一帧
        thumbReader.decodeNextFrame();
    }

    /**
     * 当前状态
     */
    private enum Status {
        /**
         * 初次创建，未使用
         */
        IDLE,
        /**
         * 正在初始化中
         */
        PREPARING,
        /**
         * 已经初始化完成，可以使用
         */
        PREPARED,
        /**
         * 已释放
         */
        RELEASED
    }

    public interface Callback {
        /**
         * @param thumbInfo 主线程回调：每一帧解析完成时回调
         */
        void onThumbRead(@NonNull ThumbInfo thumbInfo);

        /**
         * @param exception 主线程回调：解析过程中出错回调
         */
        default void onError(@NonNull Exception exception) {
            // do nothing...
        }

        /**
         * 主线程回调：所有帧解析完毕后回调
         */
        default void onComplete() {
            // do nothing...
        }
    }
}
