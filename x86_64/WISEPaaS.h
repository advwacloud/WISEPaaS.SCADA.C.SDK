#ifndef _ADD_INT_H_
#define _ADD_INT_H_

#include "inc/EDGE_INC.h"
#include "inc/curl/curl.h"

void SetConnectEvent(void (*callback)(void));

void SetDisconnectEvent(void (*callback)(void));

void SetMessageReceived(void (*callback)(char *cmd, char *val));

void Constructor(TOPTION_STRUCT option);

void Connect();

void Disconnect();

int UploadConfig(ActionType action, TSCADA_CONFIG_STRUCT edgeConfig);
    
int SendData(TEDGE_DATA_STRUCT data);

int SendDeviceStatus(TEDGE_DEVICE_STATUS_STRUCT data);

#endif