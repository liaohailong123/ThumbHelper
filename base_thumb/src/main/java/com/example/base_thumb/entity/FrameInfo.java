package com.example.base_thumb.entity;

/**
 * Author: liaohailong
 * Date: 2021/8/14
 * Time: 9:22
 * Description:
 **/
public class FrameInfo {
    private final long timestampsUs;

    public FrameInfo(long timestampsUs) {
        this.timestampsUs = timestampsUs;
    }

    public long getTimestampsUs() {
        return timestampsUs;
    }
}
