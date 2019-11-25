#ifndef __EDGE_DATA_H__
#define __EDGE_DATA_H__

//#include<time.h>

typedef struct EDGE_TAG_STRUCT {
	char * Name;
	char * Value;
} TEDGE_TAG_STRUCT, *PTEDGE_TAG_STRUCT;

typedef struct EDGE_DEVICE_STRUCT {
	int TagNumber;
	char * Id;
	PTEDGE_TAG_STRUCT TagList;
} TEDGE_DEVICE_STRUCT, *PTEDGE_DEVICE_STRUCT;

typedef struct EDGE_DATA_STRUCT {
	int DeviceNumber;
	//time_t Timestamp;
	PTEDGE_DEVICE_STRUCT DeviceList;
} TEDGE_DATA_STRUCT, *PTEDGE_DATA_STRUCT;

#endif