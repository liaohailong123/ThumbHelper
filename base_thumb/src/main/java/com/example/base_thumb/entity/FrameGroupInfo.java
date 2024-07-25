package com.example.base_thumb.entity;

import java.util.LinkedList;

/**
 * Author: liaohailong
 * Time: 2021/12/2 13:42
 * Describe: 一组依赖同一个I帧解析的视频帧
 */
public class FrameGroupInfo {
    /**
     * I帧时间戳，单位：us
     */
    public final long timestampsUs;

    /**
     * true表示精准模式
     */
    private final boolean exactly;

    private final LinkedList<FrameInfo> frames = new LinkedList<>();

    public FrameGroupInfo(long timestampsUs, boolean exactly) {
        this.timestampsUs = timestampsUs;
        this.exactly = exactly;
    }

    public long getTimestampsUs() {
        return timestampsUs;
    }

    public LinkedList<FrameInfo> getFrames() {
        return frames;
    }

    public boolean isExactly() {
        return exactly;
    }
}
