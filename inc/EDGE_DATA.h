#ifndef __EDGE_DATA_H__
#define __EDGE_DATA_H__

//#include<time.h>
typedef struct EDGE_ANALOG_ARRAY_TAG_STRUCT {
	int Index;
	double Value;
} TEDGE_ANALOG_ARRAY_TAG_STRUCT, *PTEDG_ANALOG_ARRAY_TAG_STRUCT;

typedef struct EDGE_DISCRETE_ARRAY_TAG_STRUCT {
	int Index;
	unsigned int Value;
} TEDGE_DISCRETE_ARRAY_TAG_STRUCT, *PTEDGE_DISCRETE_ARRAY_TAG_STRUCT;

typedef struct EDGE_TEXT_ARRAY_TAG_STRUCT {
	int Index;
	char * Value;
} TEDGE_TEXT_ARRAY_TAG_STRUCT, *PTEDGE_TEXT_ARRAY_TAG_STRUCT;

typedef struct EDGE_ANALOG_TAG_STRUCT {
	char * Name;
	double Value;
	int ArraySize;
	PTEDG_ANALOG_ARRAY_TAG_STRUCT ArrayList;
} TEDGE_ANALOG_TAG_STRUCT, *PTEDGE_ANALOG_TAG_STRUCT;

typedef struct EDGE_DISCRETE_TAG_STRUCT {
	char * Name;
	unsigned int Value;
	int ArraySize;
	PTEDGE_DISCRETE_ARRAY_TAG_STRUCT ArrayList;
} TEDGE_DISCRETE_TAG_STRUCT, *PTEDGE_DISCRETE_TAG_STRUCT;

typedef struct EDGE_TEXT_TAG_STRUCT {
	char * Name;
	char * Value;
	int ArraySize;
	PTEDGE_TEXT_ARRAY_TAG_STRUCT ArrayList;
} TEDGE_TEXT_TAG_STRUCT, *PTEDGE_TEXT_TAG_STRUCT;

typedef struct EDGE_DEVICE_STRUCT {
	int AnalogTagNumber;
	int DiscreteTagNumber;
	int TextTagNumber;
	char * Id;
	PTEDGE_ANALOG_TAG_STRUCT AnalogTagList;
	PTEDGE_DISCRETE_TAG_STRUCT DiscreteTagList;
	PTEDGE_TEXT_TAG_STRUCT TextTagList;
} TEDGE_DEVICE_STRUCT, *PTEDGE_DEVICE_STRUCT;

typedef struct EDGE_DATA_STRUCT {
	int DeviceNumber;
	//time_t Timestamp;
	PTEDGE_DEVICE_STRUCT DeviceList;
} TEDGE_DATA_STRUCT, *PTEDGE_DATA_STRUCT;

#endif