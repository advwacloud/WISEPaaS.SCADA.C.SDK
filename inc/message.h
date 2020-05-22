#ifndef __MSG_H__
#define __MSG_H__

#include "EDGE_CONFIG.h"
#include "EDGE_DATA.h"
#include "EDGE_DEVICE_STATUS.h"

char * LastWillMessage();

char * DisconnectMessage();

void HearbeatMessage(char **payload);

bool ParseConnectJson(bool secure, char *pMsg, char **host, char **user, char **pwd, int *port);

void ParseEventJson(char *pMsg, char **pCmd, char **pVal);

int DeviceStatusMessage(TEDGE_DEVICE_STATUS_STRUCT data, char **payload);

int SendDataMessage(TEDGE_DATA_STRUCT data, char **payload, char **json_ref);

int ConvertCreateOrUpdateConfig(int action, TNODE_CONFIG_STRUCT config, char **payload, int hbt);

int ConvertDeleteConfig(int action, TNODE_CONFIG_STRUCT config, char **payload);

int SaveConfig(int action, TNODE_CONFIG_STRUCT config);

void InitPrevious();

#endif