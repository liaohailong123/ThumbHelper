package com.example.base_thumb.decoder;

import android.annotation.SuppressLint;
import android.media.MediaCodec;
import android.media.MediaExtractor;
import android.media.MediaFormat;
import android.os.Handler;
import android.view.Surface;

import androidx.annotation.NonNull;

import com.example.base_thumb.entity.FrameGroupInfo;
import com.example.base_thumb.entity.FrameInfo;
import com.example.base_thumb.util.MediaUtil;

import java.nio.ByteBuffer;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;

/**
 * create by liaohailong
 * 2024/6/13 21:35
 * desc: use MediaCodec to decode
 */
public class SystemVideoDecoder implements IVideoDecoder {


    private String path;
    private Surface surface;

    private boolean exactly;

    private Callback callback;

    private MediaExtractor extractor;
    private MediaFormat mediaFormat;
    private MediaCodec codec;
    private final LinkedList<FrameGroupInfo> queue = new LinkedList<>();
    private volatile int resumeCount;
    private Handler inputHandler;
    private Handler outputHandler;

    private final Object resumeCountLock = new Object();
    private final Object queueRemoveLock = new Object();

    private final MediaCodec.BufferInfo bufferInfo = new MediaCodec.BufferInfo();
    private final long timeoutUs = 100 * 1000; // 100ms
    private final HashMap<Long, Integer> renderFrameMap = new HashMap<>();

    private volatile boolean released = false;

    @Override
    public void setDataSource(String uri) {
        path = uri;
    }

    @Override
    public void setSurface(@NonNull Surface surface) {
        this.surface = surface;
    }

    @Override
    public void setCallback(Callback callback) {
        this.callback = callback;
    }

    @Override
    public void start(@NonNull List<FrameInfo> frames, boolean exactly) throws Exception {
        prepare();

        resetCodec();

        // 执行任务队列
        this.exactly = exactly;
        List<FrameGroupInfo> tasks = calcQueue(extractor, frames, exactly);
        queue.clear();
        queue.addAll(tasks);
        resumeCount = getFrameCount(queue);
        renderFrameMap.clear();

        inputHandler.post(() -> dequeueInputFrame(extractor));
        dequeueNextFrame();

    }

    private void prepare() throws Exception {
        extractor = new MediaExtractor();
        // 可能文件找不到，这里抛出异常
        try {
            extractor.setDataSource(path);
        } catch (Exception e) {
            extractor.release();
            throw e;
        }

        mediaFormat = MediaUtil.selectVideo(extractor);

        // 多媒体文件中没有视频轨道？抛出异常
        if (mediaFormat == null) {
            extractor.release();
            throw new IllegalArgumentException(path + ", has no video track?");
        }

        inputHandler = MediaUtil.newWorkHandler("frame-decoder-input");
        outputHandler = MediaUtil.newWorkHandler("frame-decoder-output");
    }

    private void resetCodec() throws Exception {
        if (codec == null) {
            // 视频编码类型
            String mimeType = mediaFormat.getString(MediaFormat.KEY_MIME);
            // 创建codec
            codec = MediaCodec.createDecoderByType(mimeType);
        } else {
            codec.reset();
        }
        codec.configure(mediaFormat, surface, null, 0);
        codec.start();
    }

    private List<FrameGroupInfo> calcQueue(@NonNull MediaExtractor extractor,
                                           @NonNull List<FrameInfo> frames,
                                           boolean exactly) {
        LinkedList<FrameGroupInfo> result = new LinkedList<>();

        if (exactly) {
            HashMap<Long, FrameGroupInfo> groupMap = new HashMap<>();
            // 解析出所有截取帧的I帧与截帧信息
            for (FrameInfo frame : frames) {
                extractor.seekTo(frame.getTimestampsUs(), MediaExtractor.SEEK_TO_PREVIOUS_SYNC);
                long sampleTime = extractor.getSampleTime(); // seekTo的肯定是I帧
                FrameGroupInfo fgi = groupMap.get(sampleTime);
                if (fgi == null) {
                    fgi = new FrameGroupInfo(sampleTime, true);
                    groupMap.put(sampleTime, fgi);
                    result.addLast(fgi);
                }
                fgi.getFrames().addLast(frame);
            }
        } else {
            // 非精准模式，全量添加
            for (FrameInfo frame : frames) {
                FrameGroupInfo fgi = new FrameGroupInfo(frame.getTimestampsUs(), false);
                fgi.getFrames().add(frame);
                result.add(fgi);
            }
        }
        return result;
    }

    private int getFrameCount(List<FrameGroupInfo> tasks) {
        int count = 0;
        for (FrameGroupInfo task : tasks) {
            count += task.getFrames().size();
        }
        return count;
    }

    private int acquireInputBuffer() {
        while (true) {
            int inputBufferIndex = codec.dequeueInputBuffer(timeoutUs);
            if (inputBufferIndex >= 0) return inputBufferIndex;
        }
    }

    @SuppressLint("WrongConstant")
    private void dequeueInputFrame(@NonNull MediaExtractor extractor) {
        // 队列任务没有了，就可以停了
        if (isFrameInfoEmpty()) {
            if (!released) {
                try {
                    int inputBufferIndex = acquireInputBuffer();
                    // sampleData 小于等于0，表示数据都取完了
                    // 小于0，说明读完了
                    codec.queueInputBuffer(inputBufferIndex,
                            0,
                            0,
                            0L,
                            MediaCodec.BUFFER_FLAG_END_OF_STREAM);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
            return;
        }
        try {
            FrameGroupInfo groupInfo = queue.poll();
            while (groupInfo != null) {

                // 基于本I帧，所有视频帧任务
                LinkedList<FrameInfo> frames = groupInfo.getFrames();
                // true表示精准模式，先切换到前一个I帧，然后逐帧解析
                boolean exactly = groupInfo.isExactly();
                // 标记位：第一次执行一组解析任务，需要先seekTo到I帧
                boolean seekToIframe = true;

                // 循环解析视频帧，复用一个I帧，避免回溯解析
                FrameInfo currentFrame = frames.poll();
                while (currentFrame != null) {
                    int inputBufferIndex = acquireInputBuffer();
                    ByteBuffer inputBuffer = codec.getInputBuffer(inputBufferIndex);
                    if (seekToIframe) {
                        seekToIframe = false;
                        long timestampsUs = groupInfo.getTimestampsUs(); // 这个时间戳是I帧
                        extractor.seekTo(timestampsUs, MediaExtractor.SEEK_TO_PREVIOUS_SYNC);
                    } else {
                        // 解析下一帧
                        extractor.advance();
                    }

                    // 此视频数据对应的展示时间，单位：us
                    long sampleTime = extractor.getSampleTime();
                    // 抽出视频轨道数据的量
                    int sampleData = extractor.readSampleData(inputBuffer, 0);

                    long targetTime = currentFrame.getTimestampsUs();
                    // 截帧时间超过了目标时间，即可使用
                    // 非精准模式也可直接使用
                    if (targetTime <= sampleTime || !exactly) {
                        // 时间正确，记录渲染
                        addRenderFrame(sampleTime);
                        frames.remove(currentFrame);
                        // 正常数据，送去解码
                        codec.queueInputBuffer(inputBufferIndex, 0, sampleData, sampleTime, 0);
                    } else {
                        // 正常数据，送去解码
                        codec.queueInputBuffer(inputBufferIndex, 0, sampleData, sampleTime, 0);
                        // 时间戳为达到目标，继续advance
                        while (true) {
                            extractor.advance(); // 逐帧解析
                            inputBufferIndex = acquireInputBuffer();
                            inputBuffer = codec.getInputBuffer(inputBufferIndex);
                            sampleData = extractor.readSampleData(inputBuffer, 0);
                            sampleTime = extractor.getSampleTime();
                            boolean done = false;
                            if (targetTime <= sampleTime) {
                                // 找到了
                                // 时间正确，记录渲染
                                addRenderFrame(sampleTime);
                                frames.remove(currentFrame);
                                done = true;
                            }
                            // 正常数据，送去解码
                            codec.queueInputBuffer(inputBufferIndex, 0, sampleData, sampleTime, 0);
                            if (done) break;
                        }
                    }
                    // 继续下一个任务
                    currentFrame = frames.poll();
                }
                // 继续下一组任务
                groupInfo = queue.poll();
            }

        } catch (Exception e) {
            e.printStackTrace();
        }
        inputHandler.post(() -> dequeueInputFrame(extractor));
    }

    private void addRenderFrame(long timestamps) {
        synchronized (renderFrameMap) {
            Integer count = renderFrameMap.get(timestamps);
            if (count == null) count = 0;
            renderFrameMap.put(timestamps, count + 1);
        }
    }

    private synchronized boolean canRenderFrame(long timestamps) {
        synchronized (renderFrameMap) {
            if (!exactly) return true;
            Integer unRenderCount = renderFrameMap.get(timestamps);
            if (unRenderCount == null) return false;
            return unRenderCount > 0;
        }
    }

    private synchronized void signRenderFrame(long timestamps) {
        synchronized (renderFrameMap) {
            Integer count = renderFrameMap.get(timestamps);
            if (count != null) {
                renderFrameMap.put(timestamps, count - 1);
            }
        }
    }

    private void decreaseResumeCount() {
        synchronized (resumeCountLock) {
            resumeCount--;
        }
    }

    private void removeFrameInfo(@NonNull FrameInfo frameInfo) {
        synchronized (queueRemoveLock) {
            queue.remove(frameInfo);
        }
    }

    private boolean isFrameInfoEmpty() {
        synchronized (queueRemoveLock) {
            return queue.isEmpty();
        }
    }

    @Override
    public void dequeueNextFrame() {
        outputHandler.post(() -> readOutput(codec, bufferInfo, timeoutUs));
    }

    private void readOutput(@NonNull MediaCodec codec,
                            @NonNull MediaCodec.BufferInfo bufferInfo,
                            long timeoutUs) {
        if (released) return;

        // 获取解码好的数据 status 有可能是buffer index 也有可能是状态flag
        int status = codec.dequeueOutputBuffer(bufferInfo, timeoutUs);
        switch (status) {
            // 继续
            case MediaCodec.INFO_TRY_AGAIN_LATER:
                // 开始进行解码
            case MediaCodec.INFO_OUTPUT_FORMAT_CHANGED:
                // 正在解码
            case MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED:
                dequeueNextFrame();
                break;
            default:
                // 判断当前编解码器的状态 - 是否已经解码完毕
                boolean done = false;
                if ((bufferInfo.flags & MediaCodec.BUFFER_FLAG_END_OF_STREAM) != 0) {
                    done = true;
                }

                long presentationTimeUs = bufferInfo.presentationTimeUs;
                // 解码内容送显
                boolean doRender = (bufferInfo.size != 0) && canRenderFrame(presentationTimeUs);
                if (doRender) {
                    signRenderFrame(presentationTimeUs);
                    callback.onRender(presentationTimeUs);
                }
                codec.releaseOutputBuffer(status, doRender);
                // 异步解码，计数判断是否解析完最后
                if (doRender) {
                    decreaseResumeCount();
                    if (resumeCount <= 0) callback.onComplete();
                } else {
                    if (!done) dequeueNextFrame();
                }
                break;
        }
    }

    private final Runnable releaseCodedRun = () -> {
        released = true;
        try {
            codec.stop();
            codec.release();
        } catch (Exception e) {
            e.printStackTrace();
        }
    };

    @Override
    public void release() {
        try {
            extractor.release();
        } catch (Exception ex) {
            ex.printStackTrace();
        }
        if (inputHandler != null) {
            inputHandler.getLooper().quitSafely();
        }
        if (outputHandler != null) {
            outputHandler.postAtFrontOfQueue(releaseCodedRun);
            outputHandler.getLooper().quitSafely();
        }
    }
}
