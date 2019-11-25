#ifndef __EDGE_DEVICE_STATUS_H__
#define __EDGE_DEVICE_STATUS_H__

typedef enum {
	Offline, Online
} Status;

typedef struct DEVICE_LIST_STRUCT {
	char * Id;
    Status Status;
} TDEVICE_LIST_STRUCT, *PTDEVICE_LIST_STRUCT;

typedef struct EDGE_DEVICE_STATUS_STRUCT {
	int DeviceNumber;
	PTDEVICE_LIST_STRUCT DeviceList;
} TEDGE_DEVICE_STATUS_STRUCT, *PTEDGE_DEVICE_STATUS_STRUCT;

#endif