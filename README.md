FFmpeg README
=============

该版本的FFmpeg支持安卓下使用mediacodec硬件编码h264视频。 

### 支持mediacodec编译

1. 先编译mediacodec，该库允许在Android 4.x版本下使用mediacodec ndk接口。进入mediacodec目录下，运行ndk-build即可编译为libmediacodec.a。

2. ffmpeg configure添加编译选项如下：

```
 --enable-jni
 --enable-mediacodec
 --enable-mediacodecndk
 --enable-encoder=h264_mediacodecndk
 --enable-decoder=h264_mediacodecndk
```

建议编译为静态库，编译完成后再合成为一个动态库。

3. 将ffmpeg编译出的静态库以及libmediacodec.a文件一起编译为ffmpeg.so即可。

### 使用mediacodec进行编解码

编码时使用 avcodec_find_encoder_by_name("h264_mediacodecndk");替换h264编码器即可。

解码时使用avcodec_find_decoder_by_name("h264_mediacodecndk");替换h264解码器即可。



注意：

安卓4.x下运行需要将mediacodec/libs下的.so文件拷贝到项目工程下。