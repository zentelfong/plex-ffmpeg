#ifndef AVFORMAT_HLS_CACHE_H
#define AVFORMAT_HLS_CACHE_H
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define MAX_PATH 1024

#ifndef BOOL
#define BOOL int
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

//文件块列表信息
typedef struct HLSSegment {
	int32_t index;	//url的索引
	int32_t data;	//数据位置
	int32_t size;	//数据长度
	int32_t finish; //是否完成
	int32_t duration;//时长
}HLSSegment;


//缓存文件头信息
typedef struct HLSFileHeader {
	char    meigic[4];
	int32_t segmentCount;
}HLSFileHeader;


enum {
	HLS_CACHE_HEADER_LEN = sizeof(HLSFileHeader),
	HLS_CACHE_SEGMENT_LEN = sizeof(HLSSegment),
};

//hls缓存
typedef struct HLSCache {
	FILE* file;
	char fileName[MAX_PATH];
	//文件块信息
	int segmentCount;
	HLSSegment* segments;
}HLSCache;

//打开文件，如果文件不存在则创建文件
HLSCache* hls_create(const char* cachePath,const char* url, int segmentCount);
void hls_free(HLSCache* cache);

//写头部信息
BOOL hls_write_segment(HLSCache* cache,int pos,const HLSSegment* segment);
void hls_write_flush(HLSCache* cache);


HLSSegment* hls_find_segment(HLSCache* cache, int index);

//读写数据部分
int hls_read_data(HLSCache* cache, int index, void* data, int len, int offset);
BOOL hls_write_data(HLSCache* cache,int index,const void* data,int len,int offset);


//检测文件块是否完整，如果不完整则需重新下载，否则无需下载
BOOL hls_check_segment(HLSCache* cache, int index);



#endif
