//
// Created by 廖海龙 on 2024/7/24.
//

#ifndef HELLOGL_FFMETADATA_H
#define HELLOGL_FFMETADATA_H

#include <string>
#include <vector>
#include <unordered_map>


struct VideoMetaData
{
    std::string mime; // 视频编码类型
    int width; // 视频宽度
    int height; // 视频高度
    int fps; // 帧率
    int bitrate; // 比特率
    int rotation; // 旋转角度
    long durationUs; // 视频时长，单位：微秒
};

struct AudioMetaData
{
    std::string mime; // 音频编码类型
    int sampleRate; // 采样率
    int channelCount; // 声道数量
    int bitsPerSample; // 位深
    long durationUs; // 音频时长：单位：微秒
};


struct FFMetaData
{
    std::vector<std::shared_ptr<AudioMetaData>> audioTrackers; // 音频轨道信息
    std::vector<std::shared_ptr<VideoMetaData>> videoTrackers; // 视频轨道信息
    std::unordered_map<std::string, std::string> tag; // 封装格式信息
};


#endif //HELLOGL_FFMETADATA_H
