package com.example.base_thumb;

import androidx.annotation.NonNull;
import androidx.annotation.WorkerThread;

import com.example.base_thumb.util.MediaUtil;
import com.example.base_thumb.util.MetaData;


/**
 * Author: liaohailong
 * Time: 2021/11/15 18:15
 * Describe: 获取视频序列帧的配置属性
 */
public class ThumbAttr {
    /**
     * 本地文件路径
     */
    private final String path;
    /**
     * 期望尺寸 - 宽度
     */
    private final int desireWidth;
    /**
     * 期望尺寸 - 高度
     */
    private final int desireHeight;
    /**
     * 视频截取帧的起始时间，单位：ms
     */
    private long startTimeMs;
    /**
     * 视频截取帧的结束时间，单位：ms
     */
    private long endTimeMs;
    /**
     * 帧数量
     */
    private int count;
    /**
     * true表示精准模式，速度慢，画面精准 false表示高效模式，速度快，可能画面不准
     */
    private boolean exactly;

    /**
     * true表示使用平台侧编解码器 false表示使用ffmpeg
     */
    private boolean mediacodec;

    /**
     * 视频缩略图配置属性
     *
     * @param path         视频文件路径
     * @param desireWidth  期望尺寸 - 宽度
     * @param desireHeight 期望尺寸 - 高度
     * @param startTimeMs  视频截取帧的起始时间，单位：ms
     * @param endTimeMs    视频截取帧的结束时间，单位：ms
     * @param count        帧数量
     * @param exactly      true表示精准模式，速度慢，画面精准 false表示高效模式，速度快，可能画面不准
     * @param mediacodec   true表示使用平台侧编解码器 false表示使用ffmpeg
     */
    private ThumbAttr(@NonNull String path, int desireWidth,
                      int desireHeight, long startTimeMs,
                      long endTimeMs, int count, boolean exactly, boolean mediacodec) {
        this.path = path;
        this.desireWidth = desireWidth;
        this.desireHeight = desireHeight;
        this.startTimeMs = startTimeMs;
        this.endTimeMs = endTimeMs;
        this.count = count;
        this.exactly = exactly;
        this.mediacodec = mediacodec;
    }

    /**
     * @return 本地文件路径
     */
    public String getPath() {
        return path;
    }

    /**
     * @return 期望尺寸 - 宽度
     */
    public int getDesireWidth() {
        return desireWidth;
    }

    /**
     * @return 期望尺寸 - 高度
     */
    public int getDesireHeight() {
        return desireHeight;
    }

    /**
     * @return 视频截取帧的起始时间，单位：ms
     */
    public long getStartTimeMs() {
        return startTimeMs;
    }

    /**
     * @return 视频截取帧的结束时间，单位：ms
     */
    public long getEndTimeMs() {
        return endTimeMs;
    }

    /**
     * @return 帧数量
     */
    public int getCount() {
        return count;
    }

    /**
     * @return true表示精准模式，速度慢，false表示高效模式，速度快，可能画面不准
     */
    public boolean isExactly() {
        return exactly;
    }

    /**
     * @return true表示使用平台侧编解码器 false表示使用ffmpeg
     */
    public boolean isMediacodec() {
        return mediacodec;
    }

    /**
     * @param startTimeMs 视频截取帧的起始时间，单位：ms
     */
    public ThumbAttr setStartTimeMs(long startTimeMs) {
        this.startTimeMs = startTimeMs;
        return this;
    }

    /**
     * @param endTimeMs 视频截取帧的结束时间，单位：ms
     */
    public ThumbAttr setEndTimeMs(long endTimeMs) {
        this.endTimeMs = endTimeMs;
        return this;
    }

    /**
     * @param count 帧数量
     */
    public ThumbAttr setCount(int count) {
        this.count = count;
        return this;
    }

    /**
     * @param exactly true表示精准模式，速度慢，画面精准 false表示高效模式，速度快，可能画面不准
     */
    public ThumbAttr setExactly(boolean exactly) {
        this.exactly = exactly;
        return this;
    }

    /**
     * @param mediacodec true表示使用平台侧编解码器 false表示使用ffmpeg
     */
    public void setMediacodec(boolean mediacodec) {
        this.mediacodec = mediacodec;
    }

    public static final class Builder {
        private final String path;
        private int desireWidth = -1;
        private int desireHeight = -1;
        private long startTimeMs = -1;
        private long endTimeMs = -1;
        private int count = 20;
        private boolean exactly = true;
        private boolean mediacodec;

        /**
         * @param path 本地多媒体文件
         */
        public Builder(@NonNull String path) {
            this.path = path;
        }

        /**
         * @param desireWidth 视频帧期望尺寸 - 宽度
         */
        public Builder setDesireWidth(int desireWidth) {
            this.desireWidth = desireWidth;
            return this;
        }

        /**
         * @param desireHeight 视频帧期望尺寸 - 高度
         */
        public Builder setDesireHeight(int desireHeight) {
            this.desireHeight = desireHeight;
            return this;
        }

        /**
         * @param startTimeMs 视频帧开始截取时间，单位：ms
         */
        public Builder setStartTimeMs(long startTimeMs) {
            this.startTimeMs = startTimeMs;
            return this;
        }

        /**
         * @param endTimeMs 视频帧结束截取时间，单位：ms
         */
        public Builder setEndTimeMs(long endTimeMs) {
            this.endTimeMs = endTimeMs;
            return this;
        }

        /**
         * @param count 视频帧结束截取数量
         */
        public Builder setCount(int count) {
            this.count = count;
            return this;
        }

        /**
         * @param exactly true表示精准模式，速度慢，画面精准 false表示高效模式，速度快，可能画面不准
         */
        public Builder setExactly(boolean exactly) {
            this.exactly = exactly;
            return this;
        }

        /**
         * @param mediacodec true表示使用平台侧编解码器 false表示使用ffmpeg
         */
        public Builder setMediacodec(boolean mediacodec) {
            this.mediacodec = mediacodec;
            return this;
        }

        @WorkerThread
        public ThumbAttr build() {
            MetaData metaData = MediaUtil.getMetaData(path);
            if (metaData == null)
                throw new IllegalStateException("Can not get meta data from " + path);

            // 如果没填写期望尺寸，则按照视频视频输出
            if (desireWidth == -1 || desireHeight == -1) {
                desireWidth = metaData.getDisplayWidth();
                desireHeight = metaData.getDisplayHeight();
            }

            // 如果没有填写取帧区间，则按照视频长度取出
            if (startTimeMs == -1 || endTimeMs == -1) {
                startTimeMs = 0L;
                endTimeMs = metaData.getDurationMs();
            }

            // 参数校验
            if (startTimeMs < 0) {
                error("startTimeMs need greater than zero, current is " + startTimeMs);
            }
            if (endTimeMs > metaData.getVideoDurationUs()) {
                error("endTimeMs need lower than video duration, current is " + endTimeMs);
            }
            if (startTimeMs > endTimeMs) {
                error("endTimeMs is greater than startTimeMs? how could i do?");
            }
            if (count < 1) {
                error("count is lower than one? how could i do?");
            }

            return new ThumbAttr(path, desireWidth, desireHeight, startTimeMs, endTimeMs, count, exactly, mediacodec);
        }

        private void error(String msg) {
            throw new IllegalStateException(msg);
        }
    }
}
