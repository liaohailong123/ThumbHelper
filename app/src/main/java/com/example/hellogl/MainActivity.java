package com.example.hellogl;

import android.app.Activity;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.ParcelFileDescriptor;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import com.example.base_thumb.ThumbAttr;
import com.example.base_thumb.ThumbHelper;
import com.example.base_thumb.reader.ThumbInfo;
import com.example.base_thumb.util.MediaUtil;
import com.example.base_thumb.util.MetaData;
import com.example.hellogl.databinding.ActivityMainBinding;

import java.io.Closeable;
import java.io.File;
import java.io.FileDescriptor;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;

public class MainActivity extends AppCompatActivity {
    private static final int REQUEST_VIDEO = 0;
    private static final String KEY_MEDIACODEC = "KEY_MEDIACODEC";

    private ActivityMainBinding binding;
    private final CustomAdapter listAdapter = new CustomAdapter();
    private final Handler mainHandler = new Handler(Looper.getMainLooper());


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        // 平台解码切换
        boolean mediacodec = SpUtil.getBoolean(KEY_MEDIACODEC, true);
        updateBtnDecode(mediacodec);
        binding.btnDecode.setOnClickListener(v -> {
            boolean tag = (boolean) binding.btnDecode.getTag();
            tag = !tag;
            updateBtnDecode(tag);
        });

        // 精准模式切换
        binding.btnExact.setTag(true);
        binding.btnExact.setOnClickListener(v -> {
            boolean tag = (boolean) binding.btnExact.getTag();
            tag = !tag;
            binding.btnExact.setTag(tag);
            binding.btnExact.setText(tag ? "精准模式：开" : "精准模式：关");
        });

        // 选择手机上的视频文件
        binding.btnFind.setOnClickListener(v -> {
            Intent intent = new Intent();
            intent.setAction(Intent.ACTION_PICK);
            intent.setType("video/*");
            startActivityForResult(intent, REQUEST_VIDEO);
        });

        // 显示视频序列帧的列表
        LinearLayoutManager lm = new LinearLayoutManager(this, RecyclerView.HORIZONTAL, false);
        binding.lvThumb.setLayoutManager(lm);
        binding.lvThumb.setAdapter(listAdapter);

        // 点击重新获取视频帧
        binding.btnAgain.setOnClickListener(v -> getThumbAgain());
    }

    private void updateBtnDecode(boolean tag) {
        binding.btnDecode.setTag(tag);
        binding.btnDecode.setText(tag ? "平台解码：开" : "平台解码：关");
        SpUtil.put(KEY_MEDIACODEC, tag);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, @Nullable Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (Activity.RESULT_OK != resultCode) {
            return;
        }

        if (requestCode == REQUEST_VIDEO && data != null) {
            Uri uri = data.getData();
            if (uri == null) return;
            File videoFile = generateVideoFile();
            if (copyResult(uri, videoFile)) {
                // 复制成功，开始解码
                getThumb(videoFile.getAbsolutePath());
            }
        }
    }

    private File generateVideoFile() {
        File dir = getExternalCacheDir();
        String fileName = "video_" + System.currentTimeMillis();
        return new File(dir, fileName);
    }

    private Boolean copyResult(Uri uri, File dstFile) {
        boolean storage = ContentKits.isStorage(uri);
        String path = storage ? uri.getPath() : ContentKits.getPath(this, uri);
        if (path == null) path = "";
        if (!new File(path).exists()) {
            mainHandler.post(new Runnable() {
                @Override
                public void run() {
                    Toast.makeText(MainActivity.this, "原文件已被删除，请选择其他图片", Toast.LENGTH_SHORT).show();
                }
            });
            return false;
        }

        FileInputStream fis = null;
        FileOutputStream fos = null;
        try {
            if (storage) {
                fis = new FileInputStream(new File(path));
            } else {
                ParcelFileDescriptor pfd = getContentResolver().openFileDescriptor(uri, "r");
                FileDescriptor fd = pfd.getFileDescriptor();
                fis = new FileInputStream(fd);
            }
            fos = new FileOutputStream(dstFile);
            byte[] byteArray = new byte[1024 * 8];
            int len = 0;
            len = fis.read(byteArray, 0, byteArray.length);
            while (len != -1) {
                fos.write(byteArray, 0, len);
                len = fis.read(byteArray, 0, byteArray.length);
            }
            return true;
        } catch (Exception e) {
            e.printStackTrace();
            closeIO(fis);
            closeIO(fos);
            return false;
        }
    }

    private void closeIO(Closeable closeable) {
        if (closeable == null) return;
        try {
            closeable.close();
        } catch (Exception ex) {
            ex.printStackTrace();
        }
    }


    private ThumbHelper helper;
    private boolean exactly = true;
    private boolean mediacodec = true;
    private String filePath = "";
    private long startTimeMs = 0;
    private long endTimeMs = 0;
    private int thumbCount = 100;
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
    private long startTs = 0L;

    private void getThumb(String filePath) {
        this.filePath = filePath;
        listAdapter.clear();
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

        helper = new ThumbHelper(this, attr); // 传入attr配置属性
        startTs = System.currentTimeMillis(); // 记录一下起始时间，callback里面计算总耗时
        helper.start(callback); // 开始截取视频帧
        binding.tvDesc.setText("");
    }


    private void getThumbAgain() {
        // 文件出问题了
        if (!new File(filePath).exists()) {
            Toast.makeText(this, "请先获取视频", Toast.LENGTH_LONG).show();
            return;
        }
        ThumbAttr attr = helper.getAttr();
        exactly = (boolean) binding.btnExact.getTag();
        mediacodec = (boolean) binding.btnDecode.getTag();
        attr.setExactly(exactly);
        attr.setMediacodec(mediacodec);
        listAdapter.clear();
        startTs = System.currentTimeMillis();
        helper.start(callback);
        binding.tvDesc.setText("");
    }

    @Override
    protected void onDestroy() {
        if (helper != null) {
            helper.release();
            helper = null;
        }
        super.onDestroy();
    }

    private static final class CustomAdapter extends RecyclerView.Adapter<CustomAdapter.CustomViewHolder> {
        private final ArrayList<ThumbInfo> data = new ArrayList<>();

        public void addItem(ThumbInfo thumbInfo) {
            data.add(thumbInfo);
            notifyItemInserted(data.size() - 1);
        }

        public void clear() {
            data.clear();
            notifyDataSetChanged();
        }

        @Override
        public long getItemId(int position) {
            return data.get(position).hashCode();
        }

        @NonNull
        @Override
        public CustomViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
            View view = LayoutInflater.from(parent.getContext()).inflate(R.layout.layout_thumb_item, parent, false);
            return new CustomViewHolder(view);
        }

        @Override
        public void onBindViewHolder(@NonNull CustomViewHolder holder, int position) {
            holder.ivThumb.setImageBitmap(data.get(position).getFrame());
            SimpleDateFormat format = new SimpleDateFormat("mm:ss:SS");
            Date date = new Date();
            date.setTime(data.get(position).getTimestampsMs());

            String time = format.format(date);
            holder.tvNumber.setText("" + (position + 1) + "\n" + time);
        }

        @Override
        public int getItemCount() {
            return data.size();
        }

        public static final class CustomViewHolder extends RecyclerView.ViewHolder {
            public final TextView tvNumber;
            public final ImageView ivThumb;

            public CustomViewHolder(@NonNull View itemView) {
                super(itemView);

                tvNumber = itemView.findViewById(R.id.tv_number);
                ivThumb = itemView.findViewById(R.id.iv_thumb);
            }
        }
    }

}