#include "mp3_info.h"

#pragma pack(1)
/* ID3基础帧 */
typedef struct id3_frame_base
{
	char id[4];
	uint32_t len;
	uint16_t flag;
} id3_frame_base_s, *id3_frame_base_t;

/* ID3 T开头的帧结构 */
typedef struct id3_frame
{
	char id[4];
	uint32_t len;
	uint16_t flag;
	uint8_t encode;
	uint16_t utfx;
	uint8_t data[];
} id3_frame_s, *id3_frame_t;
#pragma pack()

/* UTF-16大端转小端 */
void ID3_UTF16BE(uint8_t *databuf, uint16_t tag_size)
{
	uint8_t temp2 = 0;
	uint8_t temp;
	for (temp = 0; temp < tag_size; temp += 2)
	{
		temp2 = databuf[temp + 1];
		databuf[temp + 1] = databuf[temp];
		databuf[temp] = temp2;
	}
}
/* UTF-16编码转UTF-8编码 */
uint8_t UTF16ToUTF8(uint16_t *utf16_p, uint16_t u16_len, uint8_t *utf8, uint16_t len)
{
	if (!utf8 || !utf16_p)
		return 0;
	uint16_t utf16;
	uint16_t w_index = 0;
	for (uint16_t i = 0; i < u16_len; i++)
	{
		utf16 = utf16_p[i];
		if (!utf16)
			break;
		if (utf16 <= 0xD7FF || utf16 >= 0xE000)
		{/* Start Of UTF-8 encoding */
			if (utf16 < 0x80)
			{ // Encoding using 1 byte , 0xxxxxxx
				if (len == w_index)
					return 0;
				utf8[w_index++] = utf16;
			}
			else if (utf16 < 0x800)
			{ // Encoding using 2 bytes , 110xxxxx 10xxxxxx
				if (len == w_index)
					return 0;
				utf8[w_index++] = 192 | (utf16 >> 6);
				utf8[w_index++] = 128 | (utf16 & 63);
			}
			else
			{ // Encoding using 3 bytes , 1110xxxx 10xxxxxx 10xxxxxx
				if (utf16 < 0xD800 || utf16 > 0xDFFF)
				{
					if (len == w_index)
						return 0;
					utf8[w_index++] = 224 | (utf16 >> 12);
					utf8[w_index++] = 128 | ((utf16 >> 6) & 63);
					utf8[w_index++] = 128 | (utf16 & 63);
				}
			}
		}
		else
			return 0;
	}
	return 1;
}

/* ID3 帧 ID 检测 */
uint8_t id3_frame_check_id(id3_frame_t frame, char *id)
{
	if (frame && id)
	{
		return !memcmp(frame->id, id, 4);
	}
	return 0;
}
/* ID3 获取此帧(id3_frame_base_s)数据长度 */
uint32_t id3_frame_get_len(id3_frame_t frame)
{
	if (frame)
	{
		uint8_t *array = (uint8_t *)&frame->len;
		return (array[0] << 24 | array[1] << 16 | array[2] << 8 | array[3]);
	}
	return 0;
}
/* 解析ID3帧文本内容 */
uint32_t id3_frame_content_to_utf8(id3_frame_t frame, uint8_t *to, uint16_t len)
{
	if (frame && to)
	{
		uint32_t tag_size = id3_frame_get_len(frame);
		if (frame->encode == 0 || frame->encode == 3)/* 1字节编码 ASCII / ISO-8859-1 */
		{
			if(tag_size > len)
				tag_size = len;
			tag_size -= 1;
			if (id3_frame_check_id(frame, "COMM"))
				memcpy(to, &frame->data[2], tag_size-4);
			else
				memcpy(to, &frame->utfx, tag_size);
			return tag_size;
		}
		else if (frame->encode == 1 && (frame->utfx == 0xFEFF || frame->utfx == 0xFFFE))/* 2字节编码 UTF-16 大/小端格式 */
		{
			tag_size -= 3;
			if (frame->utfx == 0xFFFE)/* 2字节编码 UTF-16 大端格式 */
				ID3_UTF16BE((uint8_t *)frame->data, tag_size);
			UTF16ToUTF8((uint16_t *)frame->data, tag_size / 2, to, len);
			return tag_size / 2;
		}
	}
	return 0;
}
/* ID3帧处理接口 */
uint32_t id3_process(id3_frame_t frame, char *id, uint8_t *to, uint16_t len)
{
	if (!frame || !id || !to)
		return 0;
	// if (strcmp(frame->id, id) == 0)
	if ((char *)frame == id || id3_frame_check_id(frame, id))
	{
		return id3_frame_content_to_utf8(frame, to, len);
	}
	return 0;
}
/* 获取音频文件的比特率 */
int GetBitRate(uint8_t bRateIndex, int LayerDescript, int Version)
{
    static const int BitrateTable[6][15] =
        {
            {-1, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448},
            {-1, 32, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384},
            {-1, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320},
            {-1, 32, 64, 96, 128, 160, 192, 2324, 256, 288, 320, 352, 384, 416, 448},
            {-1, 32, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384},
            {-1, 8, 16, 24, 32, 64, 80, 56, 64, 128, 160, 112, 128, 256, 320}}; // kbps (-1) means :free
    uint8_t j = 0;
    if (Version == 3 && LayerDescript == 3)
        j = 0;
    else if (Version == 3 && LayerDescript == 2)
        j = 1;
    else if (Version == 3 && LayerDescript == 1)
        j = 2;
    else if (Version == 2 && LayerDescript == 1)
        j = 3;
    else if (Version == 2 && LayerDescript == 2)
        j = 4;
    else if (Version == 2 && LayerDescript == 3)
        j = 5;
    return BitrateTable[j][bRateIndex];
}
/* 完整释放 MP3信息 结构体 */
void mp3_info_free(mp3_info_t info)
{
	if (!info)
		return ;
	if (info->path)
		free(info->path);
	if (info->title)
		free(info->title);
	if (info->artist)
		free(info->artist);
	if (info->album)
		free(info->album);
	if (info->img_data)
		free(info->img_data);
	free(info);
}
/* 获取 MP3文件信息及音频信息 */
mp3_info_t mp3_get_info(char *path, uint8_t utf8_max_len)
{
	if (!path)
		return 0;
	uint8_t *p;
	FILE *fd = 0;
	uint32_t tag_size = 0, decode_len;
	uint8_t *databuf = (uint8_t *)malloc(13 + 3 * utf8_max_len)/* 需要释放 */;
	mp3_info_t info = (mp3_info_t)malloc(sizeof(struct mp3_info));
	id3_frame_t frame = (id3_frame_t)databuf;
	if (!databuf || !info)
		goto MP3_INFO_DECODE_MALLOC_ERR;
	/* 清空容器 */
	memset(databuf, 0, 13 + 3 * utf8_max_len);
	memset(info, 0, sizeof(struct mp3_info));
	/* 尝试打开文件 */
	fd = fopen(path, "r");
	if (!fd)
		goto MP3_INFO_DECODE_OPEN_ERR;
	info->path = malloc(strlen(path))/* 获取信息失败后需要释放 */;
	if (!info->path)
		goto MP3_INFO_DECODE_MALLOC_ERR;
	strcpy(info->path, path);		/* 拷贝路径信息 */

	if (fread(databuf, 1, 10, fd) < 1) //读出MP3文件的 ID3信息头
		goto MP3_INFO_DECODE_READ_ERR;

	/* 开头10个字节数据格式, 'I' 'D' '3' %d(version) %d(version) %d(flag:0b ABCD 0000) MSB 4Byte */
	if ((databuf[0] == 'I' || databuf[0] == 'i') && (databuf[1] == 'D' || databuf[1] == 'd') && databuf[2] == '3')
	{
		//计算帧头大小
		tag_size = databuf[6] << 21 | databuf[7] << 14 | databuf[8] << 7 | databuf[9];
		while(fread(databuf, 1, 10, fd))
		{
			decode_len = id3_frame_get_len(frame);/* decode_len 包括结束符 */
			if (ftell(fd) - 10 >= tag_size || !decode_len)
				break;
			if (databuf[0] == 'T' && (databuf[1] == 'A' || (databuf[1] == 'P' && id3_frame_check_id(frame, "TPE1")) || (databuf[1] == 'I'&& id3_frame_check_id(frame, "TIT2"))))
			{
				/* 歌手: TPE1 */
				/* 歌曲名: TIT2 */
				/* 专辑名 : TALB */
				if (decode_len / 2 > utf8_max_len)
					decode_len = utf8_max_len * 2 + 2;/* 字长度 + 一个结束符 */
				if (fread(&databuf[10], 1, decode_len, fd) < decode_len) // 读取内容
					goto MP3_INFO_DECODE_READ_ERR;
				p = malloc(decode_len * 3 / 2);/* 申请内容存储空间 */
				if (!p)
					goto MP3_INFO_DECODE_MALLOC_ERR;
				/* 解析内容 */
				memset(p, 0 , decode_len * 3 / 2);
				decode_len = id3_process(frame, frame->id, p, decode_len * 3 / 2);
				if(decode_len)
				{
					/* 绑定到信息结构体 */
					if (databuf[1] == 'P')
						info->artist = p;
					else if (databuf[1] == 'I')
						info->title = p;
					else
						info->album = p;
				}
				else
				{
					/* 解析失败, 释放所有内存 */
					free(p);
					goto MP3_INFO_DECODE_FRAME_ERR;
				}
			}
			else if (databuf[0] == 'A' && databuf[1] == 'P')
			{
				/* 歌曲图片 : APIC */
				if (decode_len > 100)
				{
					/* 偏移14字节, 直接获取图片数据 */
					fseek(fd, 14, SEEK_CUR);
					p = malloc(decode_len - 14);
					if (!p)
						goto MP3_INFO_DECODE_MALLOC_ERR;
					if (fread(p, 1, decode_len - 14, fd) < decode_len - 14) /* 读取专辑照片 */
						goto MP3_INFO_DECODE_READ_ERR;
					info->img_size = decode_len - 14;
					info->img_data = p;
					/* JPEG图片开头固定4字节 */
					if (p[0] == 0xff && p[1] == 0xd8 && p[2] == 0xff && p[3] == 0xe0)
						info->img_type = 2; // JPEG
					else
					/* PNG图片开头固定4字节 */
					if (p[0] == 0x89 && p[1] == 0x50 && p[2] == 0x4e && p[3] == 0x47)
						info->img_type = 1; // PNG
					else
					{
						info->img_data = 0;
						info->img_size = 0;
						free(p);
					}
				}
			}
			else/* 移动到下一帧 */
				fseek(fd, decode_len, SEEK_CUR);
		}
	}

	/* 获取歌曲总时间 */
	const int SamplingrateTable[3][3] = {{44100, 22050, 11025}, {48000, 24000, 120000}, {32000, 16000, 8000}};
	uint8_t LayerDescript, bRateIndex, bSampleRate, bPadding, Version;
	int FrameCount = 0, FrameSize, cBuffer_size, i;
	fseek(fd, 0, SEEK_END);
	info->size = ftell(fd);/* 获取文件总大小 */
	fseek(fd, 10 + tag_size, SEEK_SET);
	while ((ftell(fd) + 4) < info->size)
	{
		/* 基本无用, 因为只运行一次 */
		if (ftell(fd) + 13 + 3 * utf8_max_len <= info->size)
            cBuffer_size = 13 + 3 * utf8_max_len;
        else
            cBuffer_size = info->size - ftell(fd);
		memset(databuf, 0, cBuffer_size);
		fread(databuf, 1, cBuffer_size, fd);
        for (i = 0; i < (cBuffer_size - 4); i++)
		{
			/* 只进入一次, 此处是别人的算法, 仅精简了一下 */
			/* https://blog.csdn.net/fengruoying93/article/details/114681189 */
			LayerDescript = (databuf[i + 1] & 0x06) >> 1;
			bRateIndex = databuf[i + 2] / 0x10;
			bSampleRate = (databuf[i + 2] & 0xA) >> 2;
			if (databuf[i] == 0xFF && databuf[i+1] > 0xE0 && bRateIndex != 0xF && LayerDescript != 0x0 && bSampleRate < 0x3)
			{
				Version = (databuf[i+1] & 0x18) >> 3;
				bPadding = (databuf[i + 1] & 0x2) >> 1;
                info->bitRate = GetBitRate(bRateIndex, LayerDescript, Version);
				info->channel = (databuf[i + 3] & 0xC0) >> 6 == 3? 1 : 2;
                if (bRateIndex != 0)
				{
					switch(Version)
					{
						case 0:
							info->sampleRate = SamplingrateTable[bSampleRate][2];
							Version = 72;
							break;
						case 2:
							info->sampleRate = SamplingrateTable[bSampleRate][1];
							Version = 72;
							break;
						case 3:
							info->sampleRate = SamplingrateTable[bSampleRate][0];
							Version = 144;
							break;
						default:
							Version = 0;
							break;
					}
					FrameSize = ((Version * info->bitRate * 1000) / info->sampleRate) + bPadding;
					FrameCount = (int)((info->size - (ftell(fd) - cBuffer_size + i)) / FrameSize);
					info->time = (int)(FrameCount * 0.026); /* 每帧的播放时间：无论帧长是多少，每帧的播放时间都是26ms */
				}
				else
                    printf("This a Free Rate MP3 File!\n");/* 不是很懂这是啥意思? */
                cBuffer_size = -1;
                break;
			}
		}
        if (cBuffer_size == (-1))
            break;
	}
	fclose(fd);/* 收尾操作 */
	free(databuf);
	fd = 0;
	databuf = 0;
	return info;
MP3_INFO_DECODE_FRAME_ERR:
	printf("mp3_get_info : Failed to Decode Frame.\n");
	goto MP3_INFO_DECODE_ERR;
MP3_INFO_DECODE_OPEN_ERR:
	printf("mp3_get_info : Failed to open file.\n");
	goto MP3_INFO_DECODE_ERR;
MP3_INFO_DECODE_READ_ERR:
	printf("mp3_get_info : Failed to read file.\n");
	goto MP3_INFO_DECODE_ERR;
MP3_INFO_DECODE_MALLOC_ERR:
	printf("mp3_get_info : Failed to malloc buffer.\n");
MP3_INFO_DECODE_ERR:
	if (info)
		mp3_info_free(info);
	if (fd)
		fclose(fd);
	if (databuf)
		free(databuf);
	fd = 0;
	info = 0;
	databuf = 0;
	return 0;
}

void mp3_get_info_test(char *path, uint8_t max_txt_length , uint32_t print_info_number, uint32_t repeat_number)
{
	if(!path)
		return ;
	mp3_info_t info;
	do
	{
		info = mp3_get_info(path, max_txt_length);
		if (info && print_info_number)
		{
			printf("Path : %s\n", info->path);
			printf("Size : %dKB\n", info->size / 1024);
			printf("Time : %d:%d\n", info->time/ 60, info->time % 60);
			printf("Title : %s\n", info->title);
			printf("Album : %s\n", info->album);
			printf("Artist : %s\n", info->artist);
			printf("Channel : %d\n", info->channel);
			printf("Bit Rate : %d Kbit\n", info->bitRate);
			printf("sample Rate : %d HZ\n", info->sampleRate);
			printf("Image Type : %d\n", info->img_type);
			printf("Image Size : %dKB\n", info->img_size / 1024);
			printf("Image Pointer : %p\n", info->img_data);
			print_info_number--;
		}
		mp3_info_free(info);
	} while (repeat_number--);
}

// int main(int args, char **argv)
// {
// 	printf("struct size : %ld\n", sizeof(struct mp3_info));
// 	mp3_get_info_test("./2.mp3", 15, 1, 0);
// }