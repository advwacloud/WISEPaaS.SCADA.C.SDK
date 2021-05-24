#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#include <stdlib.h>

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <sys/time.h>
#include "inc/cJSON.h"
#include "inc/message.h"

#define TS_BUF_LEN 256

char ts[TS_BUF_LEN] = {0};

const char *ConfigSaveFile = "config.json";

cJSON * pPrevious = NULL;

void InitPrevious(){
    pPrevious = cJSON_CreateObject();
    // previous = malloc(PRE_BUF_LEN * sizeof(struct PREVIOUS_DATA));
}

char * _getTime(){
    time_t rawtime = time(NULL);
        
    struct tm *ptm = gmtime(&rawtime);
    struct timeval tv;

    char buffer [80];
            
    strftime(buffer, 80, "%Y-%m-%dT%H:%M:%S", ptm);
    
    gettimeofday(&tv, NULL);
    int milli = tv.tv_usec/1000; // micro sec
    
    sprintf(ts, "%s.%03dZ", buffer, milli);
    
    return ts; 
}

bool ParseConnectJson(bool secure, char *pMsg, char **pHost, char **pUser, char **pPwd, int *pPort)
{
    cJSON * root = NULL;
    cJSON * cred = NULL;
    cJSON * protocols = NULL;
    cJSON * mqttssl = NULL;

    cJSON * host = NULL;

    cJSON * username = NULL;
    cJSON * password = NULL;
    cJSON * port = NULL;

    cJSON * msg = NULL;
        
    root = cJSON_Parse(pMsg);     
    if (!root) 
    {
        printf("Error before: [%s]\n", cJSON_GetErrorPtr());      
        cJSON_Delete(root);   
        return false;
    }
    else
    {
        msg = cJSON_GetObjectItem(root, "message");

        if(cJSON_Print(msg) != NULL){
            printf("get dccs error: %s\n", cJSON_Print(msg));  
            cJSON_Delete(root);
            return false;
        }
        else{

            cred = cJSON_GetObjectItem(root, "credential");

            host = cJSON_GetObjectItem(root, "serviceHost"); 

            asprintf(pHost,"%s",host->valuestring);

            protocols = cJSON_GetObjectItem(cred, "protocols");   

            if(secure){
              mqttssl = cJSON_GetObjectItem(protocols, "mqtt+ssl"); 
            }
            else{
              mqttssl = cJSON_GetObjectItem(protocols, "mqtt");    
            }

            username = cJSON_GetObjectItem(mqttssl, "username"); 
            password = cJSON_GetObjectItem(mqttssl, "password");
            port = cJSON_GetObjectItem(mqttssl, "port");

            asprintf(pUser,"%s",username->valuestring);
            asprintf(pPwd,"%s",password->valuestring);
            *pPort = port->valueint;
            
            cJSON_Delete(root);
            return true;
        }
    }

    // cJSON_Delete(root);
}

void ParseEventJson(char *pMsg, char **pCmd, char **pVal)
{
    cJSON *root = cJSON_Parse(pMsg);     
    if (!root) 
    {
        printf("Error before: [%s]\n",cJSON_GetErrorPtr());
    }
    else
    {
        cJSON *d = cJSON_GetObjectItem(root, "d");

        cJSON *cmd = cJSON_GetObjectItem(d, "Cmd");

        if(cmd != NULL){
            asprintf(pCmd,"%s",cmd->valuestring);
        }
        cJSON *val = cJSON_GetObjectItem(d, "Val");
        if(val != NULL){
            asprintf(pVal,"%s",cJSON_PrintUnformatted(val));      
        }
    }
    
    cJSON_Delete(root);
}

char * LastWillMessage()
{
    cJSON * pJsonRoot = NULL;
    char * ts = _getTime();

    pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot){
        return NULL;
    }

    cJSON * pSubJson = NULL;
    pSubJson = cJSON_CreateObject();
    if(NULL == pSubJson){
        cJSON_Delete(pJsonRoot);
        return NULL;
    }
    cJSON_AddStringToObject(pSubJson, "UeD", "1");
    cJSON_AddItemToObject(pJsonRoot, "d", pSubJson);
    cJSON_AddStringToObject(pJsonRoot, "ts", ts);

    char * p = cJSON_PrintUnformatted(pJsonRoot);
    if(NULL == p)
    {
        //convert json list to string faild, exit
        //because sub json pSubJson han been add to pJsonRoot, so just delete pJsonRoot, if you also delete pSubJson, it will coredump, and error is : double free
        cJSON_Delete(pJsonRoot);
        return NULL;
    }
    cJSON_Delete(pJsonRoot);

    return p;
}

char * DisconnectMessage()
{
    cJSON * pJsonRoot = NULL;
    char * ts = _getTime();

    pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot){
        return NULL;
    }

    cJSON * pSubJson = NULL;
    pSubJson = cJSON_CreateObject();
    if(NULL == pSubJson)
    {
        // create object faild, exit
        cJSON_Delete(pJsonRoot);
        return NULL;
    }
    cJSON_AddStringToObject(pSubJson, "DsC", "1");
    cJSON_AddItemToObject(pJsonRoot, "d", pSubJson);
    cJSON_AddStringToObject(pJsonRoot, "ts", ts);

    char * p = cJSON_PrintUnformatted(pJsonRoot);
    if(NULL == p)
    {
        cJSON_Delete(pJsonRoot);
        return NULL;
    }
    cJSON_Delete(pJsonRoot);

    return p;
}

void HearbeatMessage(char **payload){
    cJSON * pJsonRoot = NULL;
    char * ts = _getTime();

    pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot){
        //return NULL;
    }

    cJSON * pSubJson = NULL;
    pSubJson = cJSON_CreateObject();
    if(NULL == pSubJson){
        cJSON_Delete(pJsonRoot);
    }
    cJSON_AddStringToObject(pSubJson, "Hbt", "1");
    cJSON_AddItemToObject(pJsonRoot, "d", pSubJson);
    cJSON_AddStringToObject(pJsonRoot, "ts", ts);

    *payload = cJSON_PrintUnformatted(pJsonRoot);

    cJSON_Delete(pJsonRoot);
}

int DeviceStatusMessage(TEDGE_DEVICE_STATUS_STRUCT data, char **payload){

    cJSON * pJsonRoot = NULL;
    char * ts = _getTime();

    pJsonRoot = cJSON_CreateObject();

    cJSON * subJson_d = NULL;                        
    subJson_d = cJSON_CreateObject();

    cJSON * subJson_status = NULL;
    subJson_status = cJSON_CreateObject();

    if(data.DeviceList != NULL){

        for(int idev = 0; idev< data.DeviceNumber; idev++){
            cJSON_AddNumberToObject(subJson_status, data.DeviceList[idev].Id, data.DeviceList[idev].Status);
        }
    }

    cJSON_AddItemToObject(subJson_d, "Dev", subJson_status);
    cJSON_AddItemToObject(pJsonRoot, "d", subJson_d);
    cJSON_AddStringToObject(pJsonRoot, "ts", ts);

    //cJSON_Print cJSON_PrintUnformatted
    *payload = cJSON_PrintUnformatted(pJsonRoot);

    cJSON_Delete(pJsonRoot);

    return 0;
}

int SendDataMessage(TEDGE_DATA_STRUCT data, char **payload, char **json_ref){

    //printf("\n[Debug] input reference:\n  %s\n", *json_ref);
    bool data_exist = false;

    // config ref
    cJSON * pJsonRef = NULL;
    pJsonRef = cJSON_Parse(*json_ref);
    cJSON * subJsonRef_d = cJSON_GetObjectItem(pJsonRef, "d");
    cJSON * subJsonRef_dev = NULL;
    cJSON * subJsonRef_tag = NULL;
    cJSON * subJsonRef_info = NULL;

    // previous data 
    cJSON * subJsonPre_dev = NULL;
    cJSON * subJsonPre_tag = NULL;
    cJSON * subJsonPre_array = NULL;  
    cJSON * subJsonPre_normal = NULL;     
    cJSON * subJsonPre_info = NULL;    

    // original used 
    cJSON * pJsonRoot = NULL;
    char * ts = _getTime();

    pJsonRoot = cJSON_CreateObject();

    cJSON * subJson_d = NULL;                        
    subJson_d = cJSON_CreateObject();

    cJSON * subJson_node_dev = NULL;
    cJSON * subJson_node_array = NULL;

    if(data.DeviceList != NULL){

        bool match_config = false;      // used for reading reference configuration file
        bool match_previous_dev = false;    
        bool match_previous_tag = false;
        bool ana_exist = false; 
        for(int idev = 0; idev< data.DeviceNumber; idev++){

            bool valid = false;

            subJson_node_dev = cJSON_CreateObject();

            subJsonRef_dev = cJSON_GetObjectItem(subJsonRef_d, data.DeviceList[idev].Id);
            if(subJsonRef_dev == NULL){
                //printf("\n[Debug](config) miss device\n");
                match_config = false;
            } else{
                //printf("\n[Debug](config) match device\n");
                match_config = true;
            }

            subJsonPre_dev = cJSON_GetObjectItem(pPrevious, data.DeviceList[idev].Id);
            if(subJsonPre_dev == NULL){
                //printf("\n[Debug](previous) miss device\n");
                subJsonPre_dev = cJSON_CreateObject();
                match_previous_dev = false;
            } else{
                //printf("\n[Debug](previous) match device\n");
                match_previous_dev = true;
            }

            // analog tag
            for(int itag = 0; itag< data.DeviceList[idev].AnalogTagNumber; itag++){

                ana_exist = true;
                int span_high, span_low;
                double last_data, deadband;

                if(match_config){
                    subJsonRef_tag = cJSON_GetObjectItem(subJsonRef_dev, data.DeviceList[idev].AnalogTagList[itag].Name);
                    if(subJsonRef_tag == NULL){
                        //printf("\n[Debug](config) miss tag\n");
                        match_config = false;
                    } else{
                        //printf("\n[Debug](config) match tag\n");

                        subJsonRef_info = cJSON_GetObjectItem(subJsonRef_tag, "SH");
                        span_high = subJsonRef_info->valueint;

                        subJsonRef_info = cJSON_GetObjectItem(subJsonRef_tag, "SL");
                        span_low = subJsonRef_info->valueint;

                        subJsonRef_info = cJSON_GetObjectItem(subJsonRef_tag, "Deadband");
                        deadband = subJsonRef_info->valueint;

                        if(deadband == 0){
                            match_config = false;
                        } else{
                            match_config = true;
                        }
                    }
                }

                if(match_previous_dev){ // ?
                    subJsonPre_tag = cJSON_GetObjectItem(subJsonPre_dev, data.DeviceList[idev].AnalogTagList[itag].Name);
                    if(subJsonPre_tag == NULL){
                        match_previous_tag = false;
                    } else{
                        match_previous_tag = true;
                    }
                }

		        if(data.DeviceList[idev].AnalogTagList[itag].Name  
					&& data.DeviceList[idev].AnalogTagList[itag].ArrayList
					&& data.DeviceList[idev].AnalogTagList[itag].ArraySize){    // array case
                    // ?

                    subJson_node_array= cJSON_CreateObject();
                    subJsonPre_array= cJSON_CreateObject();

                    for(int iarray = 0; iarray < data.DeviceList[idev].AnalogTagList[itag].ArraySize; iarray ++){

                        char *idx = NULL;
                        asprintf(&idx, "%d", data.DeviceList[idev].AnalogTagList[itag].ArrayList[iarray].Index);   

                        if(match_previous_tag && match_config){

                            subJsonPre_info = cJSON_GetObjectItem(subJsonPre_tag, idx);
                            last_data = subJsonPre_info->valuedouble;                    
                            double range = (fabs(data.DeviceList[idev].AnalogTagList[itag].ArrayList[iarray].Value - last_data ))/(span_high - span_low);                           
                            double threshold = (deadband)/100;

                            if(range >= threshold){

                                valid = true;

                                cJSON_AddNumberToObject(subJson_node_array, idx, data.DeviceList[idev].AnalogTagList[itag].ArrayList[iarray].Value);   
                                cJSON_AddNumberToObject(subJsonPre_array, idx, data.DeviceList[idev].AnalogTagList[itag].ArrayList[iarray].Value);
                            } 
                        } else{
                            // send 1st data or no deadband case
                            valid = true;

                            cJSON_AddNumberToObject(subJson_node_array, idx, data.DeviceList[idev].AnalogTagList[itag].ArrayList[iarray].Value);  
                            cJSON_AddNumberToObject(subJsonPre_array, idx, data.DeviceList[idev].AnalogTagList[itag].ArrayList[iarray].Value);                 
                        }
                        free(idx);
                    }
                    if(valid){
                        
                        data_exist = true;
                        if(match_previous_tag){
                            cJSON_ReplaceItemInObject(subJsonPre_dev, data.DeviceList[idev].AnalogTagList[itag].Name, subJsonPre_array);
                        } else{
                            cJSON_AddItemToObject(subJsonPre_dev, data.DeviceList[idev].AnalogTagList[itag].Name, subJsonPre_array);
                        }
                        cJSON_AddItemToObject(subJson_node_dev, data.DeviceList[idev].AnalogTagList[itag].Name, subJson_node_array);
                    }
                } 
				else if(data.DeviceList[idev].AnalogTagList[itag].Name){ // normal case

                    subJsonPre_array= cJSON_CreateObject();

                    char *idx = "0";
                    if(match_previous_tag && match_config){

                        subJsonPre_info = cJSON_GetObjectItem(subJsonPre_tag, idx);
                        last_data = subJsonPre_info->valuedouble;
                        double range = (fabs(data.DeviceList[idev].AnalogTagList[itag].Value - last_data ))/(span_high - span_low);                       
                        double threshold = (deadband)/100;

                        if(range >= threshold){
                            valid = true;

                            cJSON_AddNumberToObject(subJson_node_dev, data.DeviceList[idev].AnalogTagList[itag].Name, data.DeviceList[idev].AnalogTagList[itag].Value);
                            cJSON_AddNumberToObject(subJsonPre_array, idx, data.DeviceList[idev].AnalogTagList[itag].Value);  

                        }
                    } else{
                        // send 1st data or no deadband case
                        valid = true;

                        cJSON_AddNumberToObject(subJson_node_dev, data.DeviceList[idev].AnalogTagList[itag].Name, data.DeviceList[idev].AnalogTagList[itag].Value);
                        cJSON_AddNumberToObject(subJsonPre_array, idx, data.DeviceList[idev].AnalogTagList[itag].Value);                
                    }

                    if(valid){
                        data_exist = true;
                        if(match_previous_tag){
                            cJSON_ReplaceItemInObject(subJsonPre_dev, data.DeviceList[idev].AnalogTagList[itag].Name, subJsonPre_array);
                        } else{
                            cJSON_AddItemToObject(subJsonPre_dev, data.DeviceList[idev].AnalogTagList[itag].Name, subJsonPre_array);
                        }
                        cJSON_AddNumberToObject(subJson_node_dev, data.DeviceList[idev].AnalogTagList[itag].Name, data.DeviceList[idev].AnalogTagList[itag].Value);
                    }

                    // cJSON_AddNumberToObject(subJson_node_dev, data.DeviceList[idev].AnalogTagList[itag].Name, data.DeviceList[idev].AnalogTagList[itag].Value);
                }
            }
            // discrete tag           
            for(int itag = 0; itag< data.DeviceList[idev].DiscreteTagNumber; itag++){
                if(data.DeviceList[idev].DiscreteTagList[itag].Name 
					&& data.DeviceList[idev].DiscreteTagList[itag].ArrayList
					&& data.DeviceList[idev].DiscreteTagList[itag].ArraySize){

                    subJson_node_array= cJSON_CreateObject();
                    for(int iarray = 0; iarray < data.DeviceList[idev].DiscreteTagList[itag].ArraySize; iarray ++){
                        data_exist = true;
                        char *idx = NULL;
                        asprintf(&idx, "%d", data.DeviceList[idev].DiscreteTagList[itag].ArrayList[iarray].Index);
                        cJSON_AddNumberToObject(subJson_node_array, idx, data.DeviceList[idev].DiscreteTagList[itag].ArrayList[iarray].Value);
                        free(idx);
                    }
                    cJSON_AddItemToObject(subJson_node_dev, data.DeviceList[idev].DiscreteTagList[itag].Name, subJson_node_array);
                }   
				else if(data.DeviceList[idev].DiscreteTagList[itag].Name){
                    data_exist = true;
                    cJSON_AddNumberToObject(subJson_node_dev, data.DeviceList[idev].DiscreteTagList[itag].Name, data.DeviceList[idev].DiscreteTagList[itag].Value);
                }				
            }
            // text tag
            for(int itag = 0; itag< data.DeviceList[idev].TextTagNumber; itag++){
                if(data.DeviceList[idev].TextTagList[itag].Name 
					&& data.DeviceList[idev].TextTagList[itag].ArrayList
					&& data.DeviceList[idev].TextTagList[itag].ArraySize){
 
                    subJson_node_array= cJSON_CreateObject();
                    for(int iarray = 0; iarray < data.DeviceList[idev].TextTagList[itag].ArraySize; iarray ++){
                        data_exist = true;
                        char *idx = NULL;
                        asprintf(&idx, "%d", data.DeviceList[idev].TextTagList[itag].ArrayList[iarray].Index);
                        cJSON_AddStringToObject(subJson_node_array, idx, data.DeviceList[idev].TextTagList[itag].ArrayList[iarray].Value);
                        free(idx);
                    }
                    cJSON_AddItemToObject(subJson_node_dev, data.DeviceList[idev].TextTagList[itag].Name, subJson_node_array);
                }    
				else if(data.DeviceList[idev].TextTagList[itag].Name && data.DeviceList[idev].TextTagList[itag].Value){
                    data_exist = true;
                    cJSON_AddStringToObject(subJson_node_dev, data.DeviceList[idev].TextTagList[itag].Name, data.DeviceList[idev].TextTagList[itag].Value);
                }				
            }
            
            if(ana_exist){
                if(valid){
                    if(match_previous_dev){
                        // printf("\n[Debug](previous) replace device\n");
                        cJSON_ReplaceItemInObject(pPrevious, data.DeviceList[idev].Id, subJsonPre_dev);
                    }else{
                        // printf("\n[Debug](previous) new device\n");
                        cJSON_AddItemToObject(pPrevious, data.DeviceList[idev].Id, subJsonPre_dev);
                    }
                }
            } 
            cJSON_AddItemToObject(subJson_d, data.DeviceList[idev].Id, subJson_node_dev);
        }
    }

    cJSON_AddItemToObject(pJsonRoot, "d", subJson_d);
    if(data.Time){
        
        char * tld = strrchr(data.Time, '.'); 
        struct tm ltm;
        time_t epoch;
        if ( strptime(data.Time, "%Y-%m-%d %H:%M:%S", &ltm) != NULL ){
          epoch = mktime(&ltm);
          
           gmtime_r(&epoch,&ltm);
    
          char buffer [80];    
          strftime(buffer, 80, "%Y-%m-%dT%H:%M:%S", &ltm);   
          sprintf(ts, "%s%s", buffer, tld);    
          cJSON_AddStringToObject(pJsonRoot, "ts", ts);
        } else {
          printf("fail to parse timestamp\n");
        }
    }else{
        cJSON_AddStringToObject(pJsonRoot, "ts", ts);
    }

    *payload = cJSON_PrintUnformatted(pJsonRoot);
    //char * cJsonRoot = cJSON_PrintUnformatted(pJsonRoot);
    //printf("\n[JsonRoot] %s\n", cJsonRoot); 

    cJSON_Delete(pJsonRoot);
    cJSON_Delete(pJsonRef);

    return data_exist;
}

int ConvertCreateOrUpdateConfig(int action, TNODE_CONFIG_STRUCT config, char **payload, int hbt){

    cJSON * pJsonRoot = NULL;
    char * ts = _getTime();

    pJsonRoot = cJSON_CreateObject();

    cJSON * subJson_d = NULL;                        
    subJson_d = cJSON_CreateObject();

    cJSON * subJson_node = NULL;
    subJson_node = cJSON_CreateObject();

    cJSON * subJson_node_id = NULL;
    subJson_node_id = cJSON_CreateObject();

    cJSON * subJson_node_dev = NULL;
    subJson_node_dev = cJSON_CreateObject();

    cJSON * subJson_node_dev_name = NULL;
    cJSON * subJson_node_dev_name_tag = NULL;
    cJSON * subJson_tag_ana = NULL;
    cJSON * subJson_tag_dis = NULL;
    cJSON * subJson_tag_txt = NULL;

    if(config.DeviceList != NULL){
        
        for(int idev = 0; idev< config.DeviceNumber; idev++){

            subJson_node_dev_name = cJSON_CreateObject();
            subJson_node_dev_name_tag = cJSON_CreateObject();

            for(int iana = 0; iana< config.DeviceList[idev].AnalogNumber; iana++){

                subJson_tag_ana = cJSON_CreateObject();

                if(config.DeviceList[idev].AnalogTagList[iana].Description){
                    cJSON_AddStringToObject(subJson_tag_ana, "Desc", config.DeviceList[idev].AnalogTagList[iana].Description);
                }

                cJSON_AddNumberToObject(subJson_tag_ana, "Type", 1);

                //if(config.DeviceList[idev].AnalogTagList[iana].ReadOnly){
                cJSON_AddNumberToObject(subJson_tag_ana, "Ro", config.DeviceList[idev].AnalogTagList[iana].ReadOnly);
                //}

                if(config.DeviceList[idev].AnalogTagList[iana].ArraySize){
                    cJSON_AddNumberToObject(subJson_tag_ana, "Ary", config.DeviceList[idev].AnalogTagList[iana].ArraySize); 
                }

                //if(config.DeviceList[idev].AnalogTagList[iana].AlarmStatus){
                cJSON_AddNumberToObject(subJson_tag_ana, "Alm", config.DeviceList[idev].AnalogTagList[iana].AlarmStatus);
                //}

                //if(config.DeviceList[idev].AnalogTagList[iana].NeedLog){
                cJSON_AddNumberToObject(subJson_tag_ana, "Log", config.DeviceList[idev].AnalogTagList[iana].NeedLog);
                //}

                if(config.DeviceList[idev].AnalogTagList[iana].SpanHigh){
                    cJSON_AddNumberToObject(subJson_tag_ana, "SH", config.DeviceList[idev].AnalogTagList[iana].SpanHigh);
                }

                if(config.DeviceList[idev].AnalogTagList[iana].SpanLow){
                    cJSON_AddNumberToObject(subJson_tag_ana, "SL", config.DeviceList[idev].AnalogTagList[iana].SpanLow);
                }

                if(config.DeviceList[idev].AnalogTagList[iana].EngineerUnit){
                    cJSON_AddStringToObject(subJson_tag_ana, "EU", config.DeviceList[idev].AnalogTagList[iana].EngineerUnit);
                }

                if(config.DeviceList[idev].AnalogTagList[iana].IntegerDisplayFormat){
                    cJSON_AddNumberToObject(subJson_tag_ana, "IDF", config.DeviceList[idev].AnalogTagList[iana].IntegerDisplayFormat);
                }

                if(config.DeviceList[idev].AnalogTagList[iana].FractionDisplayFormat){
                    cJSON_AddNumberToObject(subJson_tag_ana, "FDF", config.DeviceList[idev].AnalogTagList[iana].FractionDisplayFormat);
                }

                if(config.DeviceList[idev].AnalogTagList[iana].HighPriority){
                    cJSON_AddNumberToObject(subJson_tag_ana, "HHP", config.DeviceList[idev].AnalogTagList[iana].HighPriority);
                }

                if(config.DeviceList[idev].AnalogTagList[iana].HHAlarmLimit){
                    cJSON_AddNumberToObject(subJson_tag_ana, "HHA", config.DeviceList[idev].AnalogTagList[iana].HHAlarmLimit);
                }

                if(config.DeviceList[idev].AnalogTagList[iana].HighPriority){
                    cJSON_AddNumberToObject(subJson_tag_ana, "HiP", config.DeviceList[idev].AnalogTagList[iana].HighPriority);
                }

                if(config.DeviceList[idev].AnalogTagList[iana].HighAlarmLimit){
                    cJSON_AddNumberToObject(subJson_tag_ana, "HiA", config.DeviceList[idev].AnalogTagList[iana].HighAlarmLimit);
                }

                if(config.DeviceList[idev].AnalogTagList[iana].LowPriority){
                    cJSON_AddNumberToObject(subJson_tag_ana, "LoP", config.DeviceList[idev].AnalogTagList[iana].LowPriority);
                }

                if(config.DeviceList[idev].AnalogTagList[iana].LowPriority){
                    cJSON_AddNumberToObject(subJson_tag_ana, "LoA", config.DeviceList[idev].AnalogTagList[iana].LowPriority);
                }

                if(config.DeviceList[idev].AnalogTagList[iana].LLPriority){
                    cJSON_AddNumberToObject(subJson_tag_ana, "LLP", config.DeviceList[idev].AnalogTagList[iana].LLPriority);
                }

                if(config.DeviceList[idev].AnalogTagList[iana].LLAlarmLimit){
                    cJSON_AddNumberToObject(subJson_tag_ana, "LLA", config.DeviceList[idev].AnalogTagList[iana].LLAlarmLimit);
                }

                if(config.DeviceList[idev].AnalogTagList[iana].Name){
                    cJSON_AddItemToObject(subJson_node_dev_name_tag, config.DeviceList[idev].AnalogTagList[iana].Name, subJson_tag_ana);
                }  
            }
            for(int idis = 0; idis< config.DeviceList[idev].DiscreteNumber; idis++){

                subJson_tag_dis = cJSON_CreateObject();

                if(config.DeviceList[idev].DiscreteTagList[idis].Description){
                    cJSON_AddStringToObject(subJson_tag_dis, "Desc", config.DeviceList[idev].DiscreteTagList[idis].Description);
                }

                cJSON_AddNumberToObject(subJson_tag_dis, "Type", 2);

                //if(config.DeviceList[idev].DiscreteTagList[idis].ReadOnly){
                cJSON_AddNumberToObject(subJson_tag_dis, "Ro", config.DeviceList[idev].DiscreteTagList[idis].ReadOnly);
                //}

                if(config.DeviceList[idev].DiscreteTagList[idis].ArraySize){
                cJSON_AddNumberToObject(subJson_tag_dis, "Ary", config.DeviceList[idev].DiscreteTagList[idis].ArraySize); 
                }

                //if(config.DeviceList[idev].DiscreteTagList[idis].AlarmStatus){
                cJSON_AddNumberToObject(subJson_tag_dis, "Alm", config.DeviceList[idev].DiscreteTagList[idis].AlarmStatus);
                //}

                //if(config.DeviceList[idev].DiscreteTagList[idis].NeedLog){
                cJSON_AddNumberToObject(subJson_tag_dis, "Log", config.DeviceList[idev].DiscreteTagList[idis].NeedLog);
                //}
                
                if(config.DeviceList[idev].DiscreteTagList[idis].State0){
                    cJSON_AddStringToObject(subJson_tag_dis, "S0", config.DeviceList[idev].DiscreteTagList[idis].State0);
                }
                if(config.DeviceList[idev].DiscreteTagList[idis].State1){
                    cJSON_AddStringToObject(subJson_tag_dis, "S1", config.DeviceList[idev].DiscreteTagList[idis].State1);
                }
                if(config.DeviceList[idev].DiscreteTagList[idis].State2){
                    cJSON_AddStringToObject(subJson_tag_dis, "S2", config.DeviceList[idev].DiscreteTagList[idis].State2);
                }
                if(config.DeviceList[idev].DiscreteTagList[idis].State3){
                    cJSON_AddStringToObject(subJson_tag_dis, "S3", config.DeviceList[idev].DiscreteTagList[idis].State3);
                }
                if(config.DeviceList[idev].DiscreteTagList[idis].State4){
                    cJSON_AddStringToObject(subJson_tag_dis, "S4", config.DeviceList[idev].DiscreteTagList[idis].State4);
                }
                if(config.DeviceList[idev].DiscreteTagList[idis].State5){
                    cJSON_AddStringToObject(subJson_tag_dis, "S5", config.DeviceList[idev].DiscreteTagList[idis].State5);
                }
                if(config.DeviceList[idev].DiscreteTagList[idis].State6){
                    cJSON_AddStringToObject(subJson_tag_dis, "S6", config.DeviceList[idev].DiscreteTagList[idis].State6);
                }
                if(config.DeviceList[idev].DiscreteTagList[idis].State7){
                    cJSON_AddStringToObject(subJson_tag_dis, "S7", config.DeviceList[idev].DiscreteTagList[idis].State7);
                }
                if(config.DeviceList[idev].DiscreteTagList[idis].State0AlarmPriority){
                    cJSON_AddNumberToObject(subJson_tag_dis, "S0P", config.DeviceList[idev].DiscreteTagList[idis].State0AlarmPriority);
                }

                if(config.DeviceList[idev].DiscreteTagList[idis].State1AlarmPriority){    
                    cJSON_AddNumberToObject(subJson_tag_dis, "S1P", config.DeviceList[idev].DiscreteTagList[idis].State1AlarmPriority);
                }

                if(config.DeviceList[idev].DiscreteTagList[idis].State2AlarmPriority){    
                    cJSON_AddNumberToObject(subJson_tag_dis, "S2P", config.DeviceList[idev].DiscreteTagList[idis].State2AlarmPriority);
                }

                if(config.DeviceList[idev].DiscreteTagList[idis].State3AlarmPriority){    
                    cJSON_AddNumberToObject(subJson_tag_dis, "S3P", config.DeviceList[idev].DiscreteTagList[idis].State3AlarmPriority);
                }

                if(config.DeviceList[idev].DiscreteTagList[idis].State4AlarmPriority){    
                    cJSON_AddNumberToObject(subJson_tag_dis, "S4P", config.DeviceList[idev].DiscreteTagList[idis].State4AlarmPriority);
                }

                if(config.DeviceList[idev].DiscreteTagList[idis].State5AlarmPriority){    
                    cJSON_AddNumberToObject(subJson_tag_dis, "S5P", config.DeviceList[idev].DiscreteTagList[idis].State5AlarmPriority);
                }

                if(config.DeviceList[idev].DiscreteTagList[idis].State6AlarmPriority){    
                    cJSON_AddNumberToObject(subJson_tag_dis, "S6P", config.DeviceList[idev].DiscreteTagList[idis].State6AlarmPriority);
                }

                if(config.DeviceList[idev].DiscreteTagList[idis].State7AlarmPriority){    
                    cJSON_AddNumberToObject(subJson_tag_dis, "S7P", config.DeviceList[idev].DiscreteTagList[idis].State7AlarmPriority);
                }

                if(config.DeviceList[idev].DiscreteTagList[idis].Name){
                    cJSON_AddItemToObject(subJson_node_dev_name_tag, config.DeviceList[idev].DiscreteTagList[idis].Name, subJson_tag_dis);  
                }
            }
            for(int itxt = 0; itxt< config.DeviceList[idev].TextNumber; itxt++){

                subJson_tag_txt = cJSON_CreateObject();

                if(config.DeviceList[idev].TextTagList[itxt].Description){
                    cJSON_AddStringToObject(subJson_tag_txt, "Desc", config.DeviceList[idev].TextTagList[itxt].Description);
                }
                cJSON_AddNumberToObject(subJson_tag_txt, "Type", 3);

                //if(config.DeviceList[idev].TextTagList[itxt].ReadOnly){
                cJSON_AddNumberToObject(subJson_tag_txt, "Ro", config.DeviceList[idev].TextTagList[itxt].ReadOnly);
                //}

                if(config.DeviceList[idev].TextTagList[itxt].ArraySize){
                cJSON_AddNumberToObject(subJson_tag_txt, "Ary", config.DeviceList[idev].TextTagList[itxt].ArraySize); 
                }

                //if(config.DeviceList[idev].TextTagList[itxt].AlarmStatus){
                cJSON_AddNumberToObject(subJson_tag_txt, "Alm", config.DeviceList[idev].TextTagList[itxt].AlarmStatus);
                //}

                //if(config.DeviceList[idev].TextTagList[itxt].NeedLog){
                cJSON_AddNumberToObject(subJson_tag_txt, "Log", config.DeviceList[idev].TextTagList[itxt].NeedLog);
                //}

                if(config.DeviceList[idev].TextTagList[itxt].Name){
                    cJSON_AddItemToObject(subJson_node_dev_name_tag, config.DeviceList[idev].TextTagList[itxt].Name, subJson_tag_txt);  
                }
            }

            cJSON_AddStringToObject(subJson_node_dev_name, "Name", config.DeviceList[idev].Name);
            cJSON_AddStringToObject(subJson_node_dev_name, "Type", config.DeviceList[idev].Type);
            cJSON_AddStringToObject(subJson_node_dev_name, "Desc", config.DeviceList[idev].Description); 
            cJSON_AddNumberToObject(subJson_node_dev_name, "PNbr", config.DeviceList[idev].ComPortNumber);
            cJSON_AddStringToObject(subJson_node_dev_name, "RP", config.DeviceList[idev].RetentionPolicyName);

            cJSON_AddItemToObject(subJson_node_dev, config.DeviceList[idev].Id, subJson_node_dev_name);  
            cJSON_AddItemToObject(subJson_node_dev_name, "Tag", subJson_node_dev_name_tag);       
        }
        //cJSON_Delete(subJson_node_dev_name);
        cJSON_AddItemToObject(subJson_node_id, "Device", subJson_node_dev);
    }

    if(config.Name){
        cJSON_AddStringToObject(subJson_node_id, "Name", config.Name);
    }
    if(config.Description){
        cJSON_AddStringToObject(subJson_node_id, "Desc", config.Description);
    }

    cJSON_AddNumberToObject(subJson_node_id, "Type", config.Type);
    cJSON_AddNumberToObject(subJson_node_id, "Hbt", hbt);

    // subJson_node
    cJSON_AddItemToObject(subJson_node, config.Id, subJson_node_id);

    // subJson_d
    cJSON_AddNumberToObject(subJson_d, "Action", action);

    cJSON_AddItemToObject(subJson_d, "Scada", subJson_node);

    // pJsonRoot
    cJSON_AddItemToObject(pJsonRoot, "d", subJson_d);
    cJSON_AddStringToObject(pJsonRoot, "ts", ts);

    // cJSON_Print cJSON_PrintUnformatted
    *payload = cJSON_PrintUnformatted(pJsonRoot);

    cJSON_Delete(pJsonRoot);

    return 0;
}

int ConvertDeleteConfig(int action, TNODE_CONFIG_STRUCT config, char **payload){

    cJSON * pJsonRoot = NULL;
    char * ts = _getTime();

    pJsonRoot = cJSON_CreateObject();

    cJSON * subJson_d = NULL;                        
    subJson_d = cJSON_CreateObject();

    cJSON * subJson_node = NULL;
    subJson_node = cJSON_CreateObject();

    cJSON * subJson_node_id = NULL;
    subJson_node_id = cJSON_CreateObject();

    cJSON * subJson_node_dev = NULL;
    subJson_node_dev = cJSON_CreateObject();

    cJSON * subJson_node_dev_name = NULL;

    cJSON * subJson_node_dev_name_tag = NULL;

    cJSON * subJson_tag_ana = NULL;
    cJSON * subJson_tag_dis = NULL;
    cJSON * subJson_tag_txt = NULL;

    if(config.DeviceList != NULL){
        
        for(int idev = 0; idev< config.DeviceNumber; idev++){

            subJson_node_dev_name = cJSON_CreateObject();
            subJson_node_dev_name_tag = cJSON_CreateObject();

            for(int iana = 0; iana< config.DeviceList[idev].AnalogNumber; iana++){

                subJson_tag_ana = cJSON_CreateObject();

                if(config.DeviceList[idev].AnalogTagList[iana].Name){
                    cJSON_AddItemToObject(subJson_node_dev_name_tag, config.DeviceList[idev].AnalogTagList[iana].Name, subJson_tag_ana);
                }  
            }
            for(int idis = 0; idis< config.DeviceList[idev].DiscreteNumber; idis++){

                subJson_tag_dis = cJSON_CreateObject();

                if(config.DeviceList[idev].DiscreteTagList[idis].Name){
                    cJSON_AddItemToObject(subJson_node_dev_name_tag, config.DeviceList[idev].DiscreteTagList[idis].Name, subJson_tag_dis);  
                }
            }
            for(int itxt = 0; itxt< config.DeviceList[idev].TextNumber; itxt++){

                subJson_tag_txt = cJSON_CreateObject();

                if(config.DeviceList[idev].TextTagList[itxt].Name){
                    cJSON_AddItemToObject(subJson_node_dev_name_tag, config.DeviceList[idev].TextTagList[itxt].Name, subJson_tag_txt);  
                }
            }

            cJSON_AddStringToObject(subJson_node_dev_name, "Name", config.DeviceList[idev].Name);

            cJSON_AddItemToObject(subJson_node_dev, config.DeviceList[idev].Id, subJson_node_dev_name);  
            cJSON_AddItemToObject(subJson_node_dev_name, "Tag", subJson_node_dev_name_tag);       
        }
        //cJSON_Delete(subJson_node_dev_name);
        cJSON_AddItemToObject(subJson_node_id, "Device", subJson_node_dev);
    }

    if(config.Name){
        cJSON_AddStringToObject(subJson_node_id, "Name", config.Name);
    }

    // subJson_node
    cJSON_AddItemToObject(subJson_node, config.Id, subJson_node_id);

    // subJson_d
    cJSON_AddNumberToObject(subJson_d, "Action", action);

    cJSON_AddItemToObject(subJson_d, "Scada", subJson_node);

    // pJsonRoot
    cJSON_AddItemToObject(pJsonRoot, "d", subJson_d);
    cJSON_AddStringToObject(pJsonRoot, "ts", ts);

    // cJSON_Print cJSON_PrintUnformatted
    *payload = cJSON_PrintUnformatted(pJsonRoot);

    cJSON_Delete(pJsonRoot);

    return 0;
}

int SaveConfig(int action, TNODE_CONFIG_STRUCT config){

    cJSON * pJsonRoot = NULL;

    pJsonRoot = cJSON_CreateObject();

    cJSON * subJson_node_dev = NULL;
    subJson_node_dev = cJSON_CreateObject();

    cJSON * subJson_node_dev_name = NULL;
    cJSON * subJson_node_dev_name_tag = NULL;
    cJSON * subJson_tag_ana = NULL;
    cJSON * subJson_tag_dis = NULL;
    cJSON * subJson_tag_txt = NULL;

    FILE *fptr;

    fptr = fopen(ConfigSaveFile,"w");

    if(config.DeviceList != NULL){
        
        for(int idev = 0; idev< config.DeviceNumber; idev++){

            subJson_node_dev_name = cJSON_CreateObject();
            subJson_node_dev_name_tag = cJSON_CreateObject();

            for(int iana = 0; iana< config.DeviceList[idev].AnalogNumber; iana++){

                subJson_tag_ana = cJSON_CreateObject();

                if(config.DeviceList[idev].AnalogTagList[iana].Deadband){
                    cJSON_AddNumberToObject(subJson_tag_ana, "Deadband", config.DeviceList[idev].AnalogTagList[iana].Deadband); 
                } else{
                    cJSON_AddNumberToObject(subJson_tag_ana, "Deadband", 0); 
                }

                if(config.DeviceList[idev].AnalogTagList[iana].FractionDisplayFormat){
                    cJSON_AddNumberToObject(subJson_tag_ana, "FDF", config.DeviceList[idev].AnalogTagList[iana].FractionDisplayFormat);
                }

                if(config.DeviceList[idev].AnalogTagList[iana].SpanHigh){
                    cJSON_AddNumberToObject(subJson_tag_ana, "SH", config.DeviceList[idev].AnalogTagList[iana].SpanHigh);
                } else{
                    cJSON_AddNumberToObject(subJson_tag_ana, "SH", 1000);
                }

                if(config.DeviceList[idev].AnalogTagList[iana].SpanLow){
                    cJSON_AddNumberToObject(subJson_tag_ana, "SL", config.DeviceList[idev].AnalogTagList[iana].SpanLow);
                } else{
                    cJSON_AddNumberToObject(subJson_tag_ana, "SL", 0);
                }

                //cJSON_AddItemToObject(subJson_tag_ana, "VAL", NULL);

                if(config.DeviceList[idev].AnalogTagList[iana].Name){
                    cJSON_AddItemToObject(subJson_node_dev_name, config.DeviceList[idev].AnalogTagList[iana].Name, subJson_tag_ana);
                }  
            }
            cJSON_AddItemToObject(subJson_node_dev, config.DeviceList[idev].Id, subJson_node_dev_name);       
        }
        cJSON_AddItemToObject(pJsonRoot, "d", subJson_node_dev);
    }

    if(fptr == NULL)
    {
        printf("Can not read config file");            
    } else{
        fprintf(fptr,"%s", cJSON_PrintUnformatted(pJsonRoot));
        fclose(fptr);
    }

    cJSON_Delete(pJsonRoot);

    return 0;
}

