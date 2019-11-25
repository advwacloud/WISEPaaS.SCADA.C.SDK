#ifndef __CONST_H__
#define __CONST_H__

const char *ConfigTopic = "/wisepaas/scada/%s/cfg";
const char *DataTopic = "/wisepaas/scada/%s/data";
const char *ScadaConnTopic = "/wisepaas/scada/%s/conn";
const char *DeviceConnTopic = "/wisepaas/scada/%s/%s/conn";
const char *ScadaCmdTopic = "/wisepaas/scada/%s/cmd";
const char *DeviceCmdTopic = "/wisepaas/scada/%s/%s/cmd";
const char *AckTopic = "/wisepaas/scada/%s/ack";
const char *CfgAckTopic = "/wisepaas/scada/%s/cfgack";

const char *CreateSql = "CREATE TABLE Data (id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, message TEXT NOT NULL)";
const char *InsertSql =  "INSERT INTO Data (message) VALUES(%s)";
const char *QuerysSql = "SELECT * FROM Data LIMIT %d;";
const char *DeleteSql = "DELETE FROM Data WHERE id IN %d";

const int hbt_sec = 1;
const int rcov_sec = 2;

const char *WirteValueCommand = "WV";
const char *WriteConfigCommand = "WC";

//#define MAX_TOPIC 100

#endif