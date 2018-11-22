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

//�ļ����б���Ϣ
typedef struct HLSSegment {
	int32_t index;	//url������
	int32_t data;	//����λ��
	int32_t size;	//���ݳ���
	int32_t finish; //�Ƿ����
	int32_t duration;//ʱ��
}HLSSegment;


//�����ļ�ͷ��Ϣ
typedef struct HLSFileHeader {
	char    meigic[4];
	int32_t segmentCount;
}HLSFileHeader;


enum {
	HLS_CACHE_HEADER_LEN = sizeof(HLSFileHeader),
	HLS_CACHE_SEGMENT_LEN = sizeof(HLSSegment),
};

//hls����
typedef struct HLSCache {
	FILE* file;
	char fileName[MAX_PATH];
	//�ļ�����Ϣ
	int segmentCount;
	HLSSegment* segments;
}HLSCache;

//���ļ�������ļ��������򴴽��ļ�
HLSCache* hls_create(const char* cachePath,const char* url, int segmentCount);
void hls_free(HLSCache* cache);

//дͷ����Ϣ
BOOL hls_write_segment(HLSCache* cache,int pos,const HLSSegment* segment);
void hls_write_flush(HLSCache* cache);


HLSSegment* hls_find_segment(HLSCache* cache, int index);

//��д���ݲ���
int hls_read_data(HLSCache* cache, int index, void* data, int len, int offset);
BOOL hls_write_data(HLSCache* cache,int index,const void* data,int len,int offset);


//����ļ����Ƿ���������������������������أ�������������
BOOL hls_check_segment(HLSCache* cache, int index);



#endif
