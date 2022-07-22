# Decode MP3 Info (MP3音频信息获取)

> Id3v2.3.0 parsing, and audio duration and other information acquisition
>
> ID3V2.3.0 解析, 以及 音频时长等信息获取



#### struct info(结构体信息)

```c
#pragma pack(1)
struct mp3_info
{
    char *path;           /* 文件路径(mallo) */
    uint16_t time;        /* 音频时长(单位:S) */
    uint16_t bitRate;     /* 音频比特率 */
    uint16_t sampleRate;  /* 音频采样率 */
    uint8_t channel : 4;  /* 音频通道 */
    uint8_t img_type : 4; /* 歌曲图片类型(2 JPG,1 PNG,0 NULL) */
    uint32_t size;        /* 文件总大小Byte */
    uint8_t *title;       /* 歌曲名(mallo) */
    uint8_t *artist;      /* 歌手(mallo) */
    uint8_t *album;       /* 歌曲专辑名(mallo) */
    uint8_t *img_data;    /* 歌曲图片数据(mallo) */
    uint32_t img_size;    /* 歌曲图片大小 */
};
#pragma pack()
typedef struct mp3_info *mp3_info_t;
```



#### API Interface(API接口)

```c

// typedef struct mp3_info mp3_info_s;
typedef struct mp3_info *mp3_info_t;
/* free mp3 info struct pointer 释放MP3信息结构体 */
void mp3_info_free(mp3_info_t info);
/* Get mp3 info , success : return mp3 info struct pointer 获取 MP3信息, 成功返回结构体指针 */
mp3_info_t mp3_get_info(char *path, uint8_t utf8_max_len);  
/**
 *  Performance tests, parses/prints the specified number of times
 *  性能测试，解析/打印指定次数
 */
void mp3_get_info_test(char *path, uint8_t max_txt_length, uint32_t print_info_number, uint32_t repeat_number);
```



#### lg(例子):

```c
#include "mp3_info.h"

int main(int args, char **argv)
{
	printf("struct size : %ld\n", sizeof(struct mp3_info));
	mp3_get_info_test("./2.mp3", 15, 1, 0);
}

/* or see mp3_get_info_test function */
```

