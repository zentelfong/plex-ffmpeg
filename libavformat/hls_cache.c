#include "hls_cache.h"
#include <string.h>
#include <sys/stat.h>
#include "avformat.h"

typedef  uint32_t  u4;   /* unsigned 4-byte type */
typedef  uint8_t  u1;   /* unsigned 1-byte type */
#define BF_SZ 1024

void hls_hash_str(register const u1 *k, u4 length, u4 initval, char out[64]);

BOOL hls_create_file(const char* fileName,int32_t size) {
	FILE* file = fopen(fileName,"wb");
	if (file)
	{
		char buff[BF_SZ];
		int32_t i = size;
		int32_t bytes_written = 0;
		memset(buff, 0, sizeof(buff));
		for (; i > BF_SZ; i -= bytes_written) {
			bytes_written = fwrite(buff, BF_SZ,1, file);
			if (bytes_written == -1)
			{
				fclose(file);
				return FALSE;
			}
		}
		for (; i > 0; i -= bytes_written) {
			bytes_written = fwrite(buff, BF_SZ,1, file);
			if (bytes_written == -1)
			{
				fclose(file);
				return FALSE;
			}
		}
		fclose(file);
		return TRUE;
	}
	else
		return FALSE;
}

BOOL hls_file_exist(const char* fileName)
{
	struct stat st;
	if (stat(fileName, &st) == 0)
	{
		return TRUE;
	}
	else
		return FALSE;
}


char srtlastchar(const char* str) {
	int len = strlen(str);
	if (len)
		return str[len - 1];
	else
		return '\0';
}

HLSCache* hls_create(const char* cachePath, const char* url, int segmentCount)
{
	FILE* file=NULL;
	char path[MAX_PATH] = {0};
	char name[128];
	HLSSegment* segments=NULL;
	HLSCache*   cache=NULL;
	hls_hash_str(url, strlen(url),0xF4C44B5D, name);//计算文件名称

	if(cachePath)
		strcpy(path, cachePath);
	if (!srtlastchar(path) == '/')
	{
		strcat(path, '/');
	}
	strcat(path, name);
	
	if (hls_file_exist(path))
	{
		file = fopen(path, "rb+");

		HLSFileHeader fileHeader;
		fread(&fileHeader, sizeof(fileHeader), 1, file);
		if (strcmp(fileHeader.meigic,"hls")!=0 || segmentCount!= fileHeader.segmentCount)
		{
			goto Error;
		}

		segments = av_malloc(HLS_CACHE_SEGMENT_LEN* segmentCount);
		memset(segments, 0, HLS_CACHE_SEGMENT_LEN* segmentCount);
		fread(segments, HLS_CACHE_SEGMENT_LEN, segmentCount, file);
	}
	else
	{
		file = fopen(path, "wb+");
		if (!file)
			goto Error;

		HLSFileHeader fileHeader;
		strcpy(fileHeader.meigic,"hls");
		fileHeader.segmentCount = segmentCount;

		fwrite(&fileHeader, sizeof(fileHeader), 1, file);

		//创建segment
		segments = av_malloc(HLS_CACHE_SEGMENT_LEN* segmentCount);
		memset(segments,0, HLS_CACHE_SEGMENT_LEN* segmentCount);

		//写空白segment头
		fwrite(segments, HLS_CACHE_SEGMENT_LEN, segmentCount, file);

		//保存到文件
		fflush(file);
	}
	
	cache = av_malloc(sizeof(HLSCache));
	memset(cache, 0, sizeof(HLSCache));
	cache->segmentCount = segmentCount;
	cache->segments = segments;
	cache->file = file;
	strcpy(cache->fileName,path);
	return cache;

Error:
	if (file)
		fclose(file);
	if (segments)
		av_free(segments);

	if (cache)
		av_free(cache);
	return NULL;
}



void hls_free(HLSCache* cache)
{
	if (cache)
	{
		av_free(cache->segments);
		fclose(cache->file);
		av_free(cache);
	}
}


BOOL hls_write_segment(HLSCache* cache,int pos, const HLSSegment* segment)
{
	if (pos > cache->segmentCount || pos<0)
		return FALSE;

	int offset = segment->index;
	if (!cache->segments[pos].data)
	{
		cache->segments[pos] = *segment;
		fseek(cache->file, HLS_CACHE_HEADER_LEN + pos* HLS_CACHE_SEGMENT_LEN, SEEK_SET);
		return fwrite(segment, sizeof(HLSSegment), 1, cache->file) == sizeof(HLSSegment);
	}
	else
		return TRUE;//无需重写
}

void hls_write_flush(HLSCache* cache)
{
	fflush(cache->file);
}

HLSSegment* hls_find_segment(HLSCache* cache, int index)
{
	int i;
	for (i = 0; i < cache->segmentCount; ++i)
	{
		if (cache->segments[i].index == index)
			return &cache->segments[i];
	}
	return NULL;
}

int hls_read_data(HLSCache* cache, int index, void* data, int len, int offset)
{
	if (!cache)
		return FALSE;

	int readLen = -1;
	HLSSegment* seg = hls_find_segment(cache, index);
	if (seg && seg->data)
	{
		fseek(cache->file, seg->data + offset, SEEK_SET);
		readLen = fread(data, 1, len, cache->file);
	}
	return readLen;
}


BOOL hls_write_data(HLSCache* cache, int index, const void* data, int len, int offset)
{
	if (!cache)
		return FALSE;

	HLSSegment* seg = hls_find_segment(cache,index);
	if (seg)
	{
		if (len == AVERROR_EOF)
		{
			seg->size = offset;
			seg->finish = TRUE;
			//下载该文件块完成
			fseek(cache->file, HLS_CACHE_HEADER_LEN+ HLS_CACHE_SEGMENT_LEN* index,SEEK_SET);
			fwrite(seg, HLS_CACHE_SEGMENT_LEN,1,cache->file);
			fflush(cache->file);
		}
		else
		{
			if (seg->data == 0)
			{
				fseek(cache->file, 0, SEEK_END);
				seg->data = ftell(cache->file);
				seg->finish = FALSE;
			}
			else
			{

			}

			fwrite(data, len, 1, cache->file);
		}
	}
}


BOOL hls_check_segment(HLSCache* cache, int index)
{
	if (!cache)
		return FALSE;

	HLSSegment* seg = hls_find_segment(cache, index);
	if (seg && seg->finish)
	{
		return TRUE;
	}
	else
		return FALSE;
}















#define _T(c) c

char hCharArray[] = {
	_T('A'),_T('B'),_T('C'),_T('D'),_T('E'),_T('F'),_T('G'),_T('H'),_T('I'),_T('J'),_T('K'),_T('L'),_T('M'),_T('N'),_T('O'),_T('P'),_T('Q'),_T('R'),_T('S'),_T('T'),_T('U'),_T('V'),_T('W'),_T('X'),_T('Y'),_T('Z'),
	_T('0'),_T('1'),_T('2'),_T('3'),_T('4'),_T('5'),_T('6'),_T('7'),_T('8'),_T('9')
};

/* The mixing step */
#define mix(a,b,c) \
	{ \
	a=a-b;  a=a-c;  a=a^(c>>13); \
	b=b-c;  b=b-a;  b=b^(a<<8);  \
	c=c-a;  c=c-b;  c=c^(b>>13); \
	a=a-b;  a=a-c;  a=a^(c>>12); \
	b=b-c;  b=b-a;  b=b^(a<<16); \
	c=c-a;  c=c-b;  c=c^(b>>5);  \
	a=a-b;  a=a-c;  a=a^(c>>3);  \
	b=b-c;  b=b-a;  b=b^(a<<10); \
	c=c-a;  c=c-b;  c=c^(b>>15); \
	}


void hls_hash_str(register const u1 *k, u4 length, u4 initval, char out[64])
{
	register u4 a, b, c;  /* the internal state */
	u4          len;    /* how many key bytes still need mixing */

						/* Set up the internal state */
	len = length;
	a = b = 0x9e3779b9;  /* the golden ratio; an arbitrary value */
	c = initval;         /* variable initialization of internal state */

						 /*---------------------------------------- handle most of the key */
	while (len >= 12)
	{
		a = a + (k[0] + ((u4)k[1] << 8) + ((u4)k[2] << 16) + ((u4)k[3] << 24));
		b = b + (k[4] + ((u4)k[5] << 8) + ((u4)k[6] << 16) + ((u4)k[7] << 24));
		c = c + (k[8] + ((u4)k[9] << 8) + ((u4)k[10] << 16) + ((u4)k[11] << 24));
		mix(a, b, c);
		k = k + 12; len = len - 12;
	}

	/*------------------------------------- handle the last 11 bytes */
	c = c + length;
	switch (len)              /* all the case statements fall through */
	{
	case 11: c = c + ((u4)k[10] << 24);
	case 10: c = c + ((u4)k[9] << 16);
	case 9: c = c + ((u4)k[8] << 8);
		/* the first byte of c is reserved for the length */
	case 8: b = b + ((u4)k[7] << 24);
	case 7: b = b + ((u4)k[6] << 16);
	case 6: b = b + ((u4)k[5] << 8);
	case 5: b = b + k[4];
	case 4: a = a + ((u4)k[3] << 24);
	case 3: a = a + ((u4)k[2] << 16);
	case 2: a = a + ((u4)k[1] << 8);
	case 1: a = a + k[0];
		/* case 0: nothing left to add */
	}

	memset(out, 0,sizeof(out));
	int pos = 0;

	//转换abc为字符串
	while (a>0)
	{
		out[pos++] = hCharArray[a % 36];
		a = a / 36;
	}

	while (b>0)
	{
		out[pos++] = hCharArray[b % 36];
		b = b / 36;
	}

	while (c>0)
	{
		out[pos++] = hCharArray[c % 36];
		c = c / 36;
	}
	out[pos] = _T('\0');
}

