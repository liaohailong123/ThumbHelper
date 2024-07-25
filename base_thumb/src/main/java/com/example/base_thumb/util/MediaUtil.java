package com.example.base_thumb.util;

import android.media.MediaCodecList;
import android.media.MediaExtractor;
import android.media.MediaFormat;
import android.media.MediaMetadataRetriever;
import android.os.Build;
import android.os.Handler;
import android.os.HandlerThread;

import androidx.annotation.NonNull;
import androidx.annotation.RequiresApi;
import androidx.annotation.WorkerThread;

import java.util.HashMap;
import java.util.Map;

/**
 * Author: liaohailong
 * Date: 2021/8/14
 * Time: 8:26
 * Description: 多媒体文件工具类
 **/
public class MediaUtil {
    private static final MediaCodecList sMCL = new MediaCodecList(MediaCodecList.REGULAR_CODECS);

    /**
     * @param value 找最近的上一个偶数
     * @return 偶数
     */
    public static int even(int value) {
        return value % 2 == 0 ? value : value - 1;
    }

    /**
     * 简单的创建一个工作线程的handler
     *
     * @param name HandlerThread 的名字
     * @return A Work-Handler
     */
    public static Handler newWorkHandler(@NonNull String name) {
        HandlerThread thread = new HandlerThread(name);
        thread.start();
        return new Handler(thread.getLooper());
    }

    /**
     * @param extractor 拆封装器选中视频轨道
     * @return MediaFormat轨道信息，返回null，表示未找到视频轨道
     */
    public static MediaFormat selectVideo(@NonNull MediaExtractor extractor) {
        int trackCount = extractor.getTrackCount();
        for (int i = 0; i < trackCount; i++) {
            MediaFormat trackFormat = extractor.getTrackFormat(i);
            String mime = trackFormat.getString(MediaFormat.KEY_MIME);
            if (mime != null && mime.contains("video")) {
                extractor.selectTrack(i);
                return trackFormat;
            }
        }
        return null;
    }

    /**
     * @param extractor 拆封装器选中音频轨道
     * @return MediaFormat轨道信息，返回null，表示未找到音频轨道
     */
    public static MediaFormat selectAudio(@NonNull MediaExtractor extractor) {
        int trackCount = extractor.getTrackCount();
        for (int i = 0; i < trackCount; i++) {
            MediaFormat trackFormat = extractor.getTrackFormat(i);
            String mime = trackFormat.getString(MediaFormat.KEY_MIME);
            if (mime != null && mime.contains("audio")) {
                extractor.selectTrack(i);
                return trackFormat;
            }
        }
        return null;
    }

    /**
     * @param filePath 多媒体文件
     * @return true表示本地可以解码该文件中的视频轨道
     */
    @WorkerThread
    public static boolean isVideoDecodeSupport(String filePath) {
        MediaExtractor extractor = new MediaExtractor();
        try {
            extractor.setDataSource(filePath);
            MediaFormat mediaFormat = selectVideo(extractor);
            if (mediaFormat == null) return false;
            String decoder = sMCL.findDecoderForFormat(mediaFormat);
            return decoder != null;
        } catch (Exception e) {
            return false;
        }
    }

    /**
     * @param filePath 多媒体文件
     * @return true表示本地可以解码该文件中的音频轨道
     */
    @WorkerThread
    public static boolean isAudioDecodeSupport(String filePath) {
        MediaExtractor extractor = new MediaExtractor();
        try {
            extractor.setDataSource(filePath);
            MediaFormat mediaFormat = selectAudio(extractor);
            if (mediaFormat == null) return false;
            String decoder = sMCL.findDecoderForFormat(mediaFormat);
            return decoder != null;
        } catch (Exception e) {
            return false;
        }
    }

    private static final Map<String, MetaData> sMetaDataCache = new HashMap<>();


    @WorkerThread
    public static MetaData getMetaData(String filePath) {
        return getMetaData(filePath, true);
    }

    @WorkerThread
    public static MetaData getMetaData(String filePath, boolean ffmpeg) {
        if (sMetaDataCache.get(filePath) != null) {
            return sMetaDataCache.get(filePath);
        }

        try {
            MetaData metaData;
            if (ffmpeg) {
                metaData = new FFMediaHelper().getMetaData(filePath);
            } else {
                metaData = getMetaDataByAndroid(filePath);
            }

            sMetaDataCache.put(filePath, metaData);
            return metaData;
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }
    }

    /**
     * @param filePath 使用 Android系统 自带的 Media框架 获取多媒体文件信息
     * @return MetaData如果为null表示解析不出来
     */
    private static MetaData getMetaDataByAndroid(String filePath) {
        MediaExtractor extractor = null;
        try {
            extractor = new MediaExtractor();
            extractor.setDataSource(filePath);
            int trackCount = extractor.getTrackCount();
            // 找出并使用video轨道数据
            MediaFormat videoFormat = null;
            int videoTrackIndex = -1;
            MediaFormat audioFormat = null;
            int audioTrackIndex = -1;
            for (int i = 0; i < trackCount; i++) {
                MediaFormat trackFormat = extractor.getTrackFormat(i);
                String mime = trackFormat.getString(MediaFormat.KEY_MIME);
                if (mime != null && mime.contains("video")) {
                    if (videoFormat == null) {
                        videoFormat = trackFormat;
                        videoTrackIndex = i;
                    }
                } else if (mime != null && mime.contains("audio")) {
                    if (audioFormat == null) {
                        audioFormat = trackFormat;
                        audioTrackIndex = i;
                    }
                }
            }

            int formatWidth = -1; // 视频宽度
            int formatHeight = -1; // 视频高度
            String videoMimeType = ""; // 视频编码类型
            long videoDurationUs = -1; // 视频时长，单位：us
            int bitrate = -1; // bit/sec
            int frameRate = -1; // 帧率
            int rotation = 0; // 旋转角度

            if (videoFormat != null) {
                extractor.selectTrack(videoTrackIndex);
                formatWidth = videoFormat.getInteger(MediaFormat.KEY_WIDTH);
                formatHeight = videoFormat.getInteger(MediaFormat.KEY_HEIGHT);
                videoMimeType = videoFormat.getString(MediaFormat.KEY_MIME);
                videoDurationUs = videoFormat.getLong(MediaFormat.KEY_DURATION);
                // oppo r9m Android 5.1 解析内容未包含frame rate
                if (videoFormat.containsKey(MediaFormat.KEY_FRAME_RATE))
                    frameRate = videoFormat.getInteger(MediaFormat.KEY_FRAME_RATE);
                // 某些低版本手机拿不到比特率
                if (videoFormat.containsKey(MediaFormat.KEY_BIT_RATE))
                    bitrate = videoFormat.getInteger(MediaFormat.KEY_BIT_RATE);
                // Android 6.0 以下需要用MediaMetadataRetriever拿旋转角度
                boolean findRotation = false;
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
                    if (videoFormat.containsKey(MediaFormat.KEY_ROTATION)) {
                        findRotation = true;
                        rotation = videoFormat.getInteger(MediaFormat.KEY_ROTATION);
                    }
                }
                if (!findRotation && Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
                    rotation = getRotation(filePath);
                }

                extractor.unselectTrack(videoTrackIndex);
            }

            String audioMimeType = ""; // 音频编码格式
            int sampleRate = -1; // 音频采样率
            int channelCount = -1; // 声道个数
            long audioDurationUs = -1; // 视频时长，单位：us

            if (audioFormat != null) {
                extractor.selectTrack(audioTrackIndex);
                audioMimeType = audioFormat.getString(MediaFormat.KEY_MIME);
                sampleRate = audioFormat.getInteger(MediaFormat.KEY_SAMPLE_RATE);
                channelCount = audioFormat.getInteger(MediaFormat.KEY_CHANNEL_COUNT);
                audioDurationUs = audioFormat.getLong(MediaFormat.KEY_DURATION);
                extractor.unselectTrack(audioTrackIndex);
            }
            return new MetaData(filePath, videoMimeType, formatWidth, formatHeight,
                    videoDurationUs, rotation, bitrate, frameRate,
                    audioMimeType, sampleRate, channelCount, -1, audioDurationUs);
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        } finally {
            if (extractor != null) {
                extractor.release();
            }
        }
    }

    @RequiresApi(api = Build.VERSION_CODES.Q)
    @WorkerThread
    private static int getRotation(String path) {
        try (MediaMetadataRetriever retriever = new MediaMetadataRetriever()) {
            retriever.setDataSource(path);
            String videoRotation = retriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_VIDEO_ROTATION);
            if (videoRotation != null) {
                return Integer.parseInt(videoRotation);
            }
            retriever.release();
            return 0;
        } catch (Exception e) {
            e.printStackTrace();
            return 0;
        }
    }
}
