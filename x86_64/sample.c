#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <dlfcn.h>
#include <string.h>

#include "WISEPaaS.h"

int nsleep(long miliseconds)
{
   struct timespec req, rem;

   if(miliseconds > 999)
   {   
        req.tv_sec = (int)(miliseconds / 1000);                            /* Must be Non-Negative */
        req.tv_nsec = (miliseconds - ((long)req.tv_sec * 1000)) * 1000000; /* Must be in range of 0 to 999999999 */
   }   
   else
   {   
        req.tv_sec = 0;                         /* Must be Non-Negative */
        req.tv_nsec = miliseconds * 1000000;    /* Must be in range of 0 to 999999999 */
   }   
   return nanosleep(&req , &rem);
}

bool IsConnected = false;

void edgeAgent_Connected(){
    printf("Connect success\n");
    IsConnected = true;
}

void edgeAgent_Disconnected(){
    printf("Disconnected\n");
    IsConnected = false;
}

void edgeAgent_Recieve(char *cmd, char *val){

    if(strcmp(cmd, WirteValueCommand) == 0){
        printf("write value: %s\n", val);
    }
    else if(strcmp(cmd, WriteConfigCommand) == 0){
        printf("write config: %s\n", val);
    }
}

int main(int argc, char *argv[]) {

/*  load library */
    void (*SetConnectEvent)();
    void (*SetDisconnectEvent)();
    void (*SetMessageReceived)();
    void (*Constructor)(TOPTION_STRUCT);
    void (*Connect)();
    void (*Disconnect)();
    int (*UploadConfig)(ActionType, TSCADA_CONFIG_STRUCT);
    int (*SendData)(TEDGE_DATA_STRUCT);
    int (*SendDeviceStatus)(TEDGE_DEVICE_STATUS_STRUCT);

    char *error;

    void *handle;
    handle = dlopen ("./WISEPaaS.so.1.0.0", RTLD_LAZY);

    if (!handle) {
        fputs (dlerror(), stderr);
        exit(1);
    }

    SetConnectEvent = dlsym(handle, "SetConnectEvent");
    SetDisconnectEvent = dlsym(handle, "SetDisconnectEvent");
    SetMessageReceived = dlsym(handle, "SetMessageReceived");

    Constructor = dlsym(handle, "Constructor");
    Connect = dlsym(handle, "Connect");
    Disconnect = dlsym(handle, "Disconnect");
    UploadConfig = dlsym(handle, "UploadConfig");
    SendData = dlsym(handle, "SendData");
    SendDeviceStatus = dlsym(handle, "SendDeviceStatus");

    if ((error = dlerror()) != NULL)  {
        fputs(error, stderr);
        exit(1);
    }

/*  Set Event */
    SetConnectEvent(edgeAgent_Connected);
    SetDisconnectEvent(edgeAgent_Disconnected);
    SetMessageReceived(edgeAgent_Recieve);

/*  Set Construct */
	TOPTION_STRUCT options;
	options.AutoReconnect = true;
	options.ReconnectInterval = 1000;
	options.ScadaId = "1cd2fd22-9eb6-4c6e-9c59-73c4dd4088ad"; // your scada Id
	options.Heartbeat = 60;
	options.DataRecover = true;
	options.ConnectType = DCCS; 
    options.Type = Gatway;
	options.UseSecure = false;

    switch (options.ConnectType)
	{
		case 1: // DCCS
			options.DCCS.CredentialKey = "442a41baee49f2c845155ffa9668d52w"; // your CredentialKey
			options.DCCS.APIUrl = "https://api-dccs.wise-paas.com/"; 
			break;

		case 0: // MQTT
			options.MQTT.HostName = "";
			options.MQTT.Port = 1883;
			options.MQTT.Username = "";
			options.MQTT.Password = "";
			options.MQTT.ProtocolType = TCP;
			break;
	}

/*  Set Config */
    TSCADA_CONFIG_STRUCT config;
    ActionType action = Create;

    config.Id = options.ScadaId; 
    config.Description = "description";
    config.Name = "test_scada_01";
    config.PrimaryIP = NULL;
    config.BackupIP= NULL;
    config.PrimaryPort = 1883;
    config.BackupPort = 1883;
    config.Type = 1;

    int device_num = 1, analog_tag_num = 10, discrete_tag_num = 10, text_tag_num = 10;

    PTDEVICE_CONFIG_STRUCT device = malloc(device_num * sizeof(struct DEVICE_CONFIG_STRUCT));
    PTANALOG_TAG_CONFIG analogTag = malloc(analog_tag_num * sizeof(struct ANALOG_TAG_CONFIG));
    PTDISCRETE_TAG_CONFIG discreteTag = malloc(discrete_tag_num * sizeof(struct DISCRETE_TAG_CONFIG));
    PTTEXT_TAG_CONFIG textTag = malloc(text_tag_num * sizeof(struct TEXT_TAG_CONFIG));
    
    char *simTagName = NULL;
    char *simDevId = NULL;
    char *simDevName = NULL;
    char *simValue = NULL;

    bool arraySample = true;
    int array_size = 3; 
    
    for (int i = 0; i < device_num; i++){
        for ( int j = 0; j < analog_tag_num; j++ )
        {
            asprintf(&simTagName, "%s_%d", "TagName_ana", j);
            analogTag[j].Name = simTagName;    
            analogTag[j].Description = "description_update";          
            analogTag[j].ReadOnly = false;

            // analogTag[j].ArraySize = array_size;   /* used for array tag */

            // analogTag[j].AlarmStatus = false;
            // analogTag[j].SpanHigh = 1000;
            // analogTag[j].SpanLow = 0;
            // analogTag[j].EngineerUnit = "enuit";
            // analogTag[j].IntegerDisplayFormat = 4;
            // analogTag[j].FractionDisplayFormat = 2;
        }    
        for ( int j = 0; j < discrete_tag_num; j++ )
        {
            asprintf(&simTagName, "%s_%d", "TagName_dis", j);
            discreteTag[j].Name = simTagName;

            // discreteTag[j].ArraySize = 0;    /* used for array tag */

            // discreteTag[j].Description = "description_d";
            // discreteTag[j].ReadOnly = false;
            // discreteTag[j].AlarmStatus = false;
            // discreteTag[j].State0 = "0";
            // discreteTag[j].State1 = "1";
            // discreteTag[j].State2 = "";
            // discreteTag[j].State3 = "";
            // discreteTag[j].State4 = "";
            // discreteTag[j].State5 = "";
            // discreteTag[j].State6 = "";
            // discreteTag[j].State7 = "";
        }
        for ( int j = 0; j < text_tag_num; j++ )
        {
            asprintf(&simTagName, "%s_%d", "TagName_txt", j);
            textTag[j].Name = simTagName;
            textTag[j].Description = "description_t";
            textTag[j].ReadOnly = false;
            textTag[j].ArraySize = 0;
            textTag[j].AlarmStatus = false;
        }
        
        asprintf(&simDevId, "%s_%d", "DeviceID", i);
        device[i].Id = simDevId;

        asprintf(&simDevName, "%s_%d", "DeviceName", i);
        device[i].Name = simDevName;
        device[i].Type = "DType";
        device[i].Description = "DDESC";
        device[i].IP = "127.0.0.1";
        device[i].Port = 1;

        device[i].AnalogNumber = analog_tag_num;
        device[i].DiscreteNumber = discrete_tag_num;
        device[i].TextNumber = text_tag_num;

        device[i].AnalogTagList = analogTag;
        device[i].DiscreteTagList = discreteTag;
        device[i].TextTagList = textTag;
    }

    config.DeviceNumber = device_num;
    config.DeviceList = device;   

/* Set Device Status */
    TEDGE_DEVICE_STATUS_STRUCT status;
    status.DeviceNumber = device_num;

    PTDEVICE_LIST_STRUCT dev_list = malloc(device_num * sizeof(struct DEVICE_LIST_STRUCT));
    for (int i = 0; i < device_num; i++){
        asprintf(&simDevId, "%s_%d", "DeviceID", i);
        dev_list[i].Id = simDevId;
        dev_list[i].Status = 1;
    }

    status.DeviceList = dev_list;

/* Set Data Content */
    int tag_num = 100;
    TEDGE_DATA_STRUCT data;

    PTEDGE_DEVICE_STRUCT data_device = malloc(device_num * sizeof(struct EDGE_DEVICE_STRUCT));
    PTEDGE_TAG_STRUCT data_tag = malloc(tag_num * sizeof(struct EDGE_TAG_STRUCT));
    PTEDGE_ARRAY_TAG_STRUCT data_array_tag = malloc(3 * sizeof(struct EDGE_ARRAY_TAG_STRUCT));

/* Use SDK API */
    Constructor(options);
	Connect();

    nsleep(2000);
    
    UploadConfig(action, config);
    SendDeviceStatus(status);
 
    int value = 0; 

    while(1){

        if( value >= 1000 ){ value =0; } // simulation values 0 ~ 1000
        value ++;
        
        nsleep(1000); // send simulation per sec

        for(int i = 0; i < device_num; i++){
            for ( int j = 0; j < tag_num; j++ ){

                asprintf(&simTagName, "%s_%d", "TagName_ana", j);
                data_tag[j].Name = simTagName;
        
		        asprintf(&simValue, "%d", value);
	            data_tag[j].Value = simValue;

                /* array tag data */
                //    for(int k = 0; k< array_size; k++){
                //       asprintf(&simValue, "%d", value);
                //       data_array_tag[k].Index = k;
                //           data_array_tag[k].Value = simValue;
                //        }
                //    data_tag[j].ArraySize = array_size;
                //    data_tag[j].ArrayList = data_array_tag;
            }
            data_device[i].TagNumber = tag_num;
            data_device[i].TagList = data_tag;

            asprintf(&simDevId, "%s_%d", "DeviceID", i);
            data_device[i].Id = simDevId;
        }
        data.DeviceNumber = device_num;
        data.DeviceList = data_device;

        SendData(data);
    }
    
/* release */
    free(device);
    free(analogTag);
    free(discreteTag);
    free(textTag);

    free(simTagName);
    free(simDevId);
    free(simDevName);
    free(simValue);

    dlclose(handle);

    return 0;
}

