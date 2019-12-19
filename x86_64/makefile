build: WISEPaaS.so.1.0.0
	gcc sample.c -ldl -g -o sample -std=c99

WISEPaaS.so.1.0.0: WISEPaaS.o cJSON.o message.o libmosquitto.so libsqlite3.so libcurl.so.4.4.0
	gcc -shared -Wl,-soname,WISEPaaS.so.1 -o WISEPaaS.so.1.0.0 WISEPaaS.o message.o cJSON.o libmosquitto.so libsqlite3.so libcurl.so.4.4.0

WISEPaaS.o: WISEPaaS.c
	gcc -c -fPIC -o WISEPaaS.o WISEPaaS.c

message.o : message.c cJSON.o
	gcc -c -fPIC -o message.o message.c

cJSON.o: cJSON.c
	gcc -c -fPIC -o cJSON.o cJSON.c

clean:
	rm -f sample *.o *.so.1.0.0
