# ThumbHelper
    1，获取多媒体文件中的一连串视频缩略图，使用Android系统原生的Media框架和FFmpeg，可自主切换。
    2，MediaUtil.getMetaData() 函数，可获取到多媒体文件的音视频基本信息

精准模式：画面渲染跟播放器渲染的内容一致。
非精准模式：画面渲染使用最近的I帧
平台解码：MediaCodec
非平台解码：FFmpeg

<img src="https://github.com/liaohailong123/ThumbHelper/blob/main/images/example01.gif" alt="科学上网 或 Github 加速器" width="224px">

使用方式：将base_thumb模块生成aar包，直接拷到自己的工程中依赖即可，minSdk=21

在Activity中调用以下代码，调起图片裁剪界面
```
        MetaData metaData = MediaUtil.getMetaData(filePath);

        int w = metaData.getDisplayWidth() / 2;
        int h = metaData.getDisplayHeight() / 2;
        float ratio = w * 1.0f / h; // 宽高比
        int viewHeight = 200 * 3; // 200dp
        thumbCount = 50; // 100张缩略图，平均取视频帧

        int desireWidth = (int) (viewHeight * ratio);
        int desireHeight = viewHeight;
        startTimeMs = 0L;
        endTimeMs = metaData.getDurationMs();
        exactly = (boolean) binding.btnExact.getTag();
        mediacodec = (boolean) binding.btnDecode.getTag();

        ThumbAttr attr = new ThumbAttr.Builder(filePath) // 本地多媒体封装文件路径
        .setDesireWidth(desireWidth)    // 视频帧宽度
        .setDesireHeight(desireHeight)  // 视频帧高度
        .setStartTimeMs(startTimeMs)    // 截取视频帧起始时间，单位：ms
        .setEndTimeMs(endTimeMs)        // 截取视频帧结束时间，单位：ms
        .setCount(thumbCount)           // 截取多少张视频帧
        .setExactly(exactly)            // true表示精准截取，速度慢，画面内容准，false表示主帧截取，速度快，画面内容不一定准
        .setMediacodec(mediacodec)      // true表示使用平台侧编解码器 false表示使用ffmpeg
        .build();

        ThumbHelper helper = new ThumbHelper(this, attr); // 传入attr配置属性
        long startTs = System.currentTimeMillis(); // 记录一下起始时间，callback里面计算总耗时
        helper.start(callback); // 开始截取视频帧
```

在start之后，就能够在Callback获取所有的缩略图了，ThumbInfo中持有Bitmap
```
    private final ThumbHelper.Callback callback = new ThumbHelper.Callback() {
        @Override
        public void onThumbRead(@NonNull ThumbInfo thumbInfo) {
            listAdapter.addItem(thumbInfo);
        }

        @Override
        public void onError(@NonNull Exception exception) {
            ThumbHelper.Callback.super.onError(exception);
            binding.tvDesc.setText("ThumbHelper error: " + exception);
        }

        @Override
        public void onComplete() {
            ThumbHelper.Callback.super.onComplete();
            long cost = System.currentTimeMillis() - startTs;
            String string = "ThumbHelper: " +
                    thumbCount + " complete! cost:" +
                    cost + "ms, exactly:" +
                    exactly;
            binding.tvDesc.setText(string);
        }
    };
```