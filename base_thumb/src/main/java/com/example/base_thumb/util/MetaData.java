package com.example.base_thumb.util;

import android.text.TextUtils;

/**
 * Author: liaohailong
 * Date: 2021/8/14
 * Time: 8:28
 * Description: 多媒体文件基本信息
 **/
public class MetaData {
    /**
     * 源文件路径
     */
    private final String uri;
    /**
     * 视频编码类型
     */
    private final String videoMime;
    /**
     * 视频宽度
     */
    private final int width;
    /**
     * 视频高度
     */
    private final int height;
    /**
     * 视频总时长，单位：us
     */
    private final long videoDurationUs;
    /**
     * 旋转角度
     */
    private final int rotation;
    /**
     * 视频比特率 bit/sec
     */
    private final int bitrate;
    /**
     * 视频帧率
     */
    private final int frameRate;
    /**
     * 音频编码类型
     */
    private final String audioMime;
    /**
     * 音频采样率 每一秒采多少声音（赫兹）
     */
    private final int sampleRate;
    /**
     * 声道数量
     */
    private final int channelCount;

    /**
     * 位深
     */
    private final int bitsPerSample;

    /**
     * 音频总时长，单位：us
     */
    private final long audioDurationUs;

    public MetaData(String uri, String videoMime, int width, int height, long videoDurationUs, int rotation, int bitrate,
                    int frameRate, String audioMime, int sampleRate, int channelCount, int bitsPerSample, long audioDurationUs) {
        this.uri = uri;
        this.videoMime = videoMime;
        this.width = width;
        this.height = height;
        this.videoDurationUs = videoDurationUs;
        this.rotation = rotation;
        this.bitrate = bitrate;
        this.frameRate = frameRate;
        this.audioMime = audioMime;
        this.sampleRate = sampleRate;
        this.channelCount = channelCount;
        this.bitsPerSample = bitsPerSample;
        this.audioDurationUs = audioDurationUs;
    }

    public String getUri() {
        return uri;
    }

    public String getVideoMime() {
        return videoMime;
    }

    public int getWidth() {
        return width;
    }

    public int getHeight() {
        return height;
    }

    public int getDisplayWidth() {
        return rotation % 180 == 0 ? getWidth() : getHeight();
    }

    public int getDisplayHeight() {
        return rotation % 180 == 0 ? getHeight() : getWidth();
    }

    public long getVideoDurationUs() {
        return videoDurationUs;
    }

    public int getRotation() {
        return rotation;
    }

    public int getBitrate() {
        return bitrate;
    }

    public int getFrameRate() {
        return frameRate;
    }

    public String getAudioMime() {
        return audioMime;
    }

    public int getSampleRate() {
        return sampleRate;
    }

    public int getChannelCount() {
        return channelCount;
    }

    public long getAudioDurationUs() {
        return audioDurationUs;
    }

    public long getDurationUs() {
        return Math.max(videoDurationUs, audioDurationUs);
    }

    public long getDurationMs() {
        return getDurationUs() / 1000;
    }

    public boolean hasVideo() {
        return !TextUtils.isEmpty(videoMime);
    }

    public boolean hasAudio() {
        return !TextUtils.isEmpty(audioMime);
    }
}
