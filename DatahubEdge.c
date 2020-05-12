#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

#include "DatahubEdge.h"
#include "inc/cJSON.h"

TOPTION_STRUCT option;
struct mosquitto *mosq = NULL;
bool IsConnected = false;

char *_configTopic;
char *_dataTopic;
char *_nodeConnTopic;
char *_deviceConnTopic;
char *_cmdTopic;
char *_ackTopic;
char *_cfgAckTopic;

char *_insertSql = NULL;
char *_querySql = NULL;
char *_deleteSql = NULL;

char *json_ref = NULL;

/* sqllite */
int rows = 0, cols = 0 ;
sqlite3 *db;
char **result;
char *ids = NULL;

/* prototypes */
void* heartbeat_proc(void *secs);
void* recover_proc(void *secs);
void* ovpn_proc();

/* create thread */
pthread_t thread_hbt_id, thread_rcov_id, thread_rcov_ovpn;

/* base64 */
static char encoding_table[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                                'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                                'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                                'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                'w', 'x', 'y', 'z', '0', '1', '2', '3',
                                '4', '5', '6', '7', '8', '9', '+', '/'};
static char *decoding_table = NULL;
static int mod_table[] = {0, 2, 1};

void build_decoding_table() {

    decoding_table = malloc(256);

    for (int i = 0; i < 64; i++)
        decoding_table[(unsigned char) encoding_table[i]] = i;
}

char *base64_encode(const unsigned char *data,
                    size_t input_length,
                    size_t *output_length) {

    *output_length = 4 * ((input_length + 2) / 3);

    char *encoded_data = malloc(*output_length);
    if (encoded_data == NULL) return NULL;

    for (int i = 0, j = 0; i < input_length;) {

        uint32_t octet_a = i < input_length ? (unsigned char)data[i++] : 0;
        uint32_t octet_b = i < input_length ? (unsigned char)data[i++] : 0;
        uint32_t octet_c = i < input_length ? (unsigned char)data[i++] : 0;

        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        encoded_data[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
    }

    for (int i = 0; i < mod_table[input_length % 3]; i++)
        encoded_data[*output_length - 1 - i] = '=';

    return encoded_data;
}

unsigned char *base64_decode(const char *data,
                            size_t input_length,
                            size_t *output_length) {

    if (decoding_table == NULL) {
		build_decoding_table();
	}

    if (input_length % 4 != 0) {
		return NULL;
	}

    *output_length = input_length / 4 * 3;
    if (data[input_length - 1] == '=') (*output_length)--;
    if (data[input_length - 2] == '=') (*output_length)--;

    unsigned char *decoded_data = malloc(*output_length);
    if (decoded_data == NULL) return NULL;

    for (int i = 0, j = 0; i < input_length;) {

        uint32_t sextet_a = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
        uint32_t sextet_b = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
        uint32_t sextet_c = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
        uint32_t sextet_d = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];

        uint32_t triple = (sextet_a << 3 * 6)
        + (sextet_b << 2 * 6)
        + (sextet_c << 1 * 6)
        + (sextet_d << 0 * 6);

        if (j < *output_length) decoded_data[j++] = (triple >> 2 * 8) & 0xFF;
        if (j < *output_length) decoded_data[j++] = (triple >> 1 * 8) & 0xFF;
        if (j < *output_length) decoded_data[j++] = (triple >> 0 * 8) & 0xFF;
    }

    return decoded_data;
}

void base64_cleanup() {
    free(decoding_table);
}

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

static int data_callback(void *data, int argc, char **argv, char **azColName){
	int i;
	for(i=0; i<argc; i++){
		if(strcmp(azColName[i], "id") == 0){
			asprintf(&_deleteSql, DeleteSql, argv[i]);
			sqlite3_exec(db, _deleteSql, 0, 0, NULL);
		}
	}
	return 0;
}

/* event struct */
typedef struct{
	void (*callback)(void);
}TCALLBACK_STRUCT;

typedef struct{
	void (*recieve)(char *cmd, char *val);
}TRECIEVE_STRUCT;

TCALLBACK_STRUCT event_connect;
TCALLBACK_STRUCT event_disconnect;
TRECIEVE_STRUCT event_recieve;

void initdb(){
	if (sqlite3_open_v2("recover.db3", &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL)) {
		return;
	}
	sqlite3_exec(db, CreateSql, 0, 0, NULL);
	//sqlite3_free_table(result);
	//sqlite3_close(db);
}

int read_last_config(){
	const char *ConfigSaveFile = "config.json";
	FILE *file;

	file = fopen(ConfigSaveFile,"rb");

	if (file == NULL) {
		fprintf(stderr, "\nCant read the last configuration\n");
		return 1;
    }

	fseek(file, 0, SEEK_END);
	long len = ftell(file);
	fseek(file, 0, SEEK_SET);

	free(json_ref);
	json_ref = (char*) malloc(len+1);

	fread(json_ref, 1, len, file);

	fclose(file);

	return 0;
}

struct string {
  	char *ptr;
  	size_t len;
};

void curlInitString(struct string *s) {
	s->len = 0;
	s->ptr = malloc(s->len+1);
	if (s->ptr == NULL) {
		fprintf(stderr, "malloc() failed\n");
		exit(0);
	}
	s->ptr[0] = '\0';
}

size_t curlWriteFunc(void *ptr, size_t size, size_t nmemb, struct string *s){
	size_t new_len = s->len + size*nmemb;
	s->ptr = realloc(s->ptr, new_len+1);
	if (s->ptr == NULL) {
		fprintf(stderr, "realloc() failed\n");
		exit(0);
	}
	memcpy(s->ptr+s->len, ptr, size*nmemb);
	s->ptr[new_len] = '\0';
	s->len = new_len;

	return size*nmemb;
}

void getCredentialFromDCCS(){
	
	CURL *curl;
	CURLcode res;

	char *url;
	asprintf(&url, "%sv1/serviceCredentials/%s", option.DCCS.APIUrl, option.DCCS.CredentialKey);

	curl = curl_easy_init();
	if(curl) {

		struct string s;
		curlInitString(&s);

		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteFunc);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
		res = curl_easy_perform(curl);

		ParseConnectJson(option.UseSecure, s.ptr, &option.MQTT.HostName, &option.MQTT.Username, &option.MQTT.Password, &option.MQTT.Port);
	
		free(s.ptr);

		/* always cleanup */
		curl_easy_cleanup(curl);
	}
	free(url);
}

void message_callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message){
	
	char *cmd = NULL;
	char *val = NULL;

	if (message->payloadlen){
		//printf("message: %s %s\n", message->topic, message->payload);
		if(strcmp(message->topic, _cmdTopic) == 0){
			ParseEventJson(message->payload, &cmd, &val);
			event_recieve.recieve(cmd, val);
		}
	}
	else{
		printf("message: %s (null)\n", message->topic);
	}

	free(cmd);
	free(val);
	fflush(stdout);
}

void connect_callback(struct mosquitto *mosq, void *userdata, int result){
	if (!result){

		IsConnected = true;

		event_connect.callback();

		mosquitto_subscribe(mosq, NULL, _cmdTopic, 1);
		mosquitto_subscribe(mosq, NULL, _ackTopic, 1);
	}
	else{
		IsConnected = false;

		fprintf(stderr, "Connect failed\n");
	}
}

void disconnect_callback(struct mosquitto *mosq, void *userdata, int result){

	IsConnected = false;
	
	ConnectType cType = DCCS;
	if ( option.ConnectType == cType ){
		getCredentialFromDCCS();
	}
	
	event_disconnect.callback();
}

void log_callback(struct mosquitto *mosq, void *userdata, int level, const char *str){
	printf("%s\n", str);
}

void Constructor(TOPTION_STRUCT query) {

	option = query;
	bool clean_session = true;

	/* create topic */
	char *_nodeCmdTopic;
	char *_deviceCmdTopic;

	asprintf(&_nodeCmdTopic, NodeCmdTopic, option.NodeId);
	asprintf(&_deviceCmdTopic, DeviceCmdTopic, option.NodeId, option.DeviceId);
	asprintf(&_configTopic, ConfigTopic, option.NodeId);
	asprintf(&_dataTopic, DataTopic, option.NodeId);
	asprintf(&_nodeConnTopic, NodeConnTopic, option.NodeId);
	asprintf(&_deviceConnTopic, NodeConnTopic, option.NodeId);
	asprintf(&_ackTopic, AckTopic, option.NodeId);
	asprintf(&_cfgAckTopic, CfgAckTopic, option.NodeId);
	if(option.Type == 0){
		asprintf(&_cmdTopic,"%s",_nodeCmdTopic);
	}
	else{
		asprintf(&_cmdTopic, "%s", _deviceCmdTopic);	
	}

	mosquitto_lib_init();

	/* Create a new mosquitto client instance. */
	mosq = mosquitto_new(NULL, clean_session, NULL); 
	if (!mosq){
		fprintf(stderr, "Error: new mosq failed.\n");
	}

	size_t length = strlen(LastWillMessage());
	mosquitto_will_set(mosq, _nodeConnTopic, length, LastWillMessage(), 1, true); //?
	if ( option.UseSecure ){
		mosquitto_tls_insecure_set(mosq, true);
	}                 	
	mosquitto_connect_callback_set(mosq, connect_callback);
	mosquitto_disconnect_callback_set(mosq, disconnect_callback);
	mosquitto_message_callback_set(mosq, message_callback);

    int rc = 0;
	if(option.Heartbeat >= 0){
		rc = pthread_create(&thread_hbt_id, NULL, heartbeat_proc, (void*)(size_t) option.Heartbeat);
		if(rc){
			printf("=== Error Creating heartbeat thread\n");
		} 
	}

	rc = pthread_create(&thread_rcov_id, NULL, recover_proc, (void*)(size_t) rcov_sec);
    if(rc){
        printf("=== Error Creating recover thread\n");
    } 	

    rc = pthread_create(&thread_rcov_ovpn, NULL, ovpn_proc, (void*)(size_t) rcov_sec);
    if(rc){
        printf("=== Error Creating openvpn thread\n");
    } 	

	build_decoding_table();
	initdb();
	read_last_config();

	InitPrevious();
	

	//previous = malloc(PREVIOUD_BUF_LEN * szieof(struct EDGE_PREVIOUS_DATA));
}

void SetConnectEvent(void (*callback)(void)){
	event_connect.callback = callback;
}

void SetDisconnectEvent(void (*callback)(void)){
	event_disconnect.callback = callback;
}

void SetMessageReceived(void (*callback)(char *cmd, char *val) ){
	event_recieve.recieve = callback;
}

void Connect() {

	ConnectType cType = DCCS;
	if ( option.ConnectType == cType ){
		getCredentialFromDCCS();
	}

	mosquitto_username_pw_set(mosq, option.MQTT.Username, option.MQTT.Password);
	mosquitto_connect(mosq, option.MQTT.HostName, option.MQTT.Port, option.Heartbeat);
	mosquitto_loop_start(mosq); 
	// if (mosquitto_connect_async(mosq, option.MQTT.HostName, option.MQTT.Port, option.Heartbeat)){
	// 	fprintf(stderr, "Unable to connect.\n");
	// 	return 1;
	// }
}

void Disconnect(){

	EdgeType eType = Gatway;

	char * payload = DisconnectMessage();	

	int rc = mosquitto_publish(
		mosq, // a valid mosquitto instance.
		NULL,
		(option.Type == eType ) ? _nodeConnTopic : _deviceConnTopic, // topic
		strlen(payload), // payloadlen
		payload, // payload
		1,
		false);

	free(payload);

	if (mosquitto_disconnect(mosq)){
		fprintf(stderr, "Unable to disconnect.\n");
	}
	mosquitto_loop_stop(mosq, true);
}

int UploadConfig(ActionType action, TNODE_CONFIG_STRUCT config){
	
	char *payload = NULL;
	bool result = false;
	switch ( action )
	{
		case 0: // Create
			result = ConvertCreateOrUpdateConfig( 1, config, &payload, option.Heartbeat );
			SaveConfig(1, config);
			//printf("%s\n",payload);
			break;
		case 1: // Update
			result = ConvertCreateOrUpdateConfig( 2, config, &payload, option.Heartbeat );
			SaveConfig(2, config);
			break;
		case 2: // Delete
			result = ConvertDeleteConfig( 3, config, &payload );
			break;
	}

	//printf("%s\n",payload);
	int rc = mosquitto_publish(
		mosq, // a valid mosquitto instance.
		NULL,
		_configTopic, // topic
		strlen(payload), // payloadlen
		payload, // payload
		1,
		false);

	free(payload);

	if (rc) {
		fprintf(stderr, "Can't publish config to Mosquitto server\n");
		return (1);
	} else{
		read_last_config(); // update the last config
	}
	return 0;
}

int SendDeviceStatus(TEDGE_DEVICE_STATUS_STRUCT data){

	char *payload = NULL;
	bool result = false;

	result = DeviceStatusMessage( data, &payload );

	//printf("%s\n",payload);
	if(IsConnected){

		int rc = mosquitto_publish(
			mosq, // a valid mosquitto instance.
			NULL,
			_nodeConnTopic, // topic
			strlen(payload), // payloadlen
			payload, // payload
			1,
			false);

		if (rc) {
			fprintf(stderr, "Can't publish data to Mosquitto server\n");

			free(payload);
			return (1);
		}
	}

	free(payload);
	return 0;
}

int SendData(TEDGE_DATA_STRUCT data){

	char *payload = NULL;
	bool valid = false;
	valid = SendDataMessage( data, &payload, &json_ref);

	if(valid){
		if(IsConnected){
			int rc = mosquitto_publish(
				mosq, // a valid mosquitto instance.
				NULL,
				_dataTopic, // topic
				strlen(payload), // payloadlen
				payload, // payload
				1,
				false);

			if (rc) {
				fprintf(stderr, "Can't publish data to Mosquitto server\n");
				return (1);
			}
		}
		else{
			//char *trans = NULL; // base64 test
			size_t size = 0;
			//trans = base64_encode(payload, strlen(payload), &size);
			asprintf(&_insertSql, InsertSql, payload);
			int result = sqlite3_exec(db, _insertSql, 0, 0, NULL);
			//free(trans);
		}
	}

	free(payload);
	return 0;
}

/* start the timing in hbt thread */
void* heartbeat_proc(void *secs)
{
	int seconds = (int)(size_t) secs;
	EdgeType eType = Gatway;

	char *payload = NULL;

	while(option.Heartbeat > 0){

		if(IsConnected){

			HearbeatMessage( &payload );

			nsleep(seconds*1000);
			//sleep(seconds);

			int rc = mosquitto_publish(
				mosq, // a valid mosquitto instance.
				NULL,
				(option.Type == eType ) ? _nodeConnTopic : _deviceConnTopic, // topic
				strlen(payload), // payloadlen
				payload, // payload
				1,
				false);
		}
	}

	free(payload);
    pthread_exit(NULL);
}

/* start the timing in rcov thread */
void* recover_proc(void *secs)
{
	int seconds = (int)(size_t) secs;

	while(option.DataRecover){

		if(IsConnected){
			nsleep(seconds*1000);

			asprintf(&_querySql, QuerysSql, rcov_limit);
			sqlite3_get_table(db , _querySql, &result , &rows, &cols, NULL);
			//char *trans = NULL; // base64 test
			//size_t size = 0;

			for (int i=0; i<rows; i++) {
				if(strcmp(result[i*cols+1], "message") != 0){
					//trans = base64_decode(result[i*cols+1],strlen(result[i*cols+1])/3*4, &size);
					int rc = mosquitto_publish(
						mosq, // a valid mosquitto instance.
						NULL,
						_dataTopic, // topic
						strlen(result[i*cols+1]), // payloadlens
						result[i*cols+1], // payload
						1,
						false);

					if (rc) {
						fprintf(stderr, "Can't publish data to Mosquitto server\n");
					}
					printf("\n");

					sqlite3_exec(db, _querySql, data_callback, 0, NULL); // delete id
				}
			}
			//free(trans);
		}
	}
    pthread_exit(NULL);
}

void* ovpn_proc(void *secs)
{
	// printf("Check OpenVPN\n");
	if(strlen(option.OvpnPath) != 0 ){

		char *cmdbuf;

		asprintf(&cmdbuf, "./openvpn %s", option.OvpnPath);
		system(cmdbuf);

		free(cmdbuf);
	}
    pthread_exit(NULL);
}

