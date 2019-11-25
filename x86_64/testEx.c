#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <dlfcn.h>

#include "WISEPaaS.h"


void edgeAgent_Connected(){
    printf("Connect success\n");
}

void edgeAgent_Disconnected(){
    printf("Disconnected\n");
}

void connect_sample(void (*Connect)(void)){
    Connect();
}

void disconnect_sample(void (*Disconnect)(void)){
    Disconnect();
}

void construct_sample(void (*Constructor)(TOPTION_STRUCT)){

    TOPTION_STRUCT options;

	options.AutoReconnect = true;
	options.ReconnectInterval = 1000;
	options.ScadaId = "c9851920-ca7f-4cfd-964a-1969aef958f6";
	options.Heartbeat = 60;
	options.DataRecover = true;
	options.ConnectType = DCCS; 
    options.Type = Gatway;
	options.UseSecure = false;

    switch (options.ConnectType)
	{
		case 1: // DCCS
			options.DCCS.CredentialKey = "9b03e5524c70b6e4c503173c6553abrh";
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

    Constructor(options);
}

void uploadConfig_sample(int (*UploadConfig)(ActionType, TSCADA_CONFIG_STRUCT)){
    TSCADA_CONFIG_STRUCT config;
    ActionType action = Create;

    config.Id = "c9851920-ca7f-4cfd-964a-1969aef958f6"; 
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

    for (int i = 0; i < device_num; i++){
        for ( int j = 0; j < analog_tag_num; j++ )
        {
            asprintf(&simTagName, "%s_%d", "TagName_ana", j);
            analogTag[j].Name = simTagName;    
            analogTag[j].Description = "description_update";          
            analogTag[j].ReadOnly = false;
            // analogTag[j].ArraySize = 0;
            // analogTag[j].AlarmStatus = false;
            // analogTag[j].SpanHigh = 1000;
            // analogTag[j].SpanLow = 0;
            // analogTag[j].EngineerUnit = "enuit";
            // analogTag[j].IntegerDisplayFormat = 4;
            // analogTag[j].FractionDisplayFormat = 2;
            // analogTag[j].HHPriority = 1;
            // analogTag[j].HHAlarmLimit = 1;
            // analogTag[j].HighPriority = 1;
            // analogTag[j].HighAlarmLimit = 1;
            // analogTag[j].LowPriority = 1;
            // analogTag[j].LowAlarmLimit = 1;
            // analogTag[j].LLPriority = 1;
            // analogTag[j].LLAlarmLimit = 1;
            // analogTag[j].NeedLog = true;
        }    
        for ( int j = 0; j < discrete_tag_num; j++ )
        {
            asprintf(&simTagName, "%s_%d", "TagName_dis", j);
            discreteTag[j].Name = simTagName;
            // discreteTag[j].Description = "description_d";
            // discreteTag[j].ReadOnly = false;
            // discreteTag[j].ArraySize = 0;
            // discreteTag[j].AlarmStatus = false;
            // discreteTag[j].State0 = "0";
            // discreteTag[j].State1 = "1";
            // discreteTag[j].State2 = "";
            // discreteTag[j].State3 = "";
            // discreteTag[j].State4 = "";
            // discreteTag[j].State5 = "";
            // discreteTag[j].State6 = "";
            // discreteTag[j].State7 = "";
            // discreteTag[j].State0AlarmPriority = 0;
            // discreteTag[j].State1AlarmPriority = 0;
            // discreteTag[j].State2AlarmPriority = 0;
            // discreteTag[j].State3AlarmPriority = 0;
            // discreteTag[j].State4AlarmPriority = 0;
            // discreteTag[j].State5AlarmPriority = 0;
            // discreteTag[j].State6AlarmPriority = 0;
            // discreteTag[j].State7AlarmPriority = 0;
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

    UploadConfig(action, config);

    free(device);
    free(analogTag);
    free(discreteTag);
    free(textTag);
}

void sendDeviceStatus_sample(int (*SendDeviceStatus)(TEDGE_DEVICE_STATUS_STRUCT)){

    int device_num = 1;

    char *simDevId = NULL;

    TEDGE_DEVICE_STATUS_STRUCT status;
    status.DeviceNumber = device_num;

    PTDEVICE_LIST_STRUCT dev_list = malloc(device_num * sizeof(struct DEVICE_LIST_STRUCT));
    for (int i = 0; i < device_num; i++){
        asprintf(&simDevId, "%s_%d", "DeviceID", i);
        dev_list[i].Id = simDevId;
        dev_list[i].Status = 1;
    }

    status.DeviceList = dev_list;

    SendDeviceStatus(status);

    free(simDevId);
}

void sendData_sample(int (*SendData)(TEDGE_DATA_STRUCT)){

    int device_num = 1;

    char *simValue = NULL;  
    char *simTagName = NULL; 
    char *simDevId = NULL;
 
    int tag_num = 100;
    TEDGE_DATA_STRUCT data;

    PTEDGE_DEVICE_STRUCT data_device = malloc(device_num * sizeof(struct EDGE_DEVICE_STRUCT));
    PTEDGE_TAG_STRUCT data_tag = malloc(tag_num * sizeof(struct EDGE_TAG_STRUCT));

    int value = 0;
    
    //while(1){
        if(value >=1000){value =0;}
        value ++;
        sleep(3);

        for(int i = 0; i < device_num; i++){
            for ( int j = 0; j < tag_num; j++ ){

                asprintf(&simTagName, "%s_%d", "TagName_ana", j);
                data_tag[j].Name = simTagName;

                asprintf(&simValue, "%d", value);
                data_tag[j].Value = simValue;
            }
            data_device[i].TagNumber = tag_num;
            data_device[i].TagList = data_tag;

            asprintf(&simDevId, "%s_%d", "DeviceID", i);
            data_device[i].Id = simDevId;
        }
        data.DeviceNumber = device_num;
        data.DeviceList = data_device;

        SendData(data);
    //}

    free(simTagName);
    free(simDevId);
    free(simValue);
}

int main(int argc, char *argv[]) {

    void *handle;
    handle = dlopen ("./WISEPaaS.so.1.0.0", RTLD_LAZY);

    void (*SetConnectEvent)();
    void (*SetDisconnectEvent)();
    void (*Constructor)(TOPTION_STRUCT);
    void (*Connect)();
    void (*Disconnect)();
    int (*UploadConfig)();
    int (*SendData)();
    int (*SendDeviceStatus)();

    SetConnectEvent = dlsym(handle, "SetConnectEvent");
    SetDisconnectEvent = dlsym(handle, "SetDisconnectEvent");

    Constructor = dlsym(handle, "Constructor");
    Connect = dlsym(handle, "Connect");
    Disconnect = dlsym(handle, "Disconnect");
    UploadConfig = dlsym(handle, "UploadConfig");
    SendDeviceStatus = dlsym(handle, "SendDeviceStatus");
    SendData = dlsym(handle, "SendData");

    SetConnectEvent(edgeAgent_Connected);
    SetDisconnectEvent(edgeAgent_Disconnected);
    
    construct_sample(Constructor);

    connect_sample(Connect);
/*
    uploadConfig_sample(UploadConfig);

    sendDeviceStatus_sample(SendDeviceStatus);

    sendData_sample(SendData);*/

    dlclose(handle);

    return 0;
}

