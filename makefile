all: openvpn build

openvpn: openssl-link lzo-link openvpn-link WISEPaaS.Datahub.Edge.so.1.0.2

build: WISEPaaS.Datahub.Edge.so.1.0.2
	gcc sample.c -ldl -g -o sample -std=c99

WISEPaaS.Datahub.Edge.so.1.0.2: DatahubEdge.o cJSON.o message.o libmosquitto.so libsqlite3.so libcurl.so.4.4.0
	gcc -shared -Wl,-soname,DatahubEdge.so.1 -o DatahubEdge.so.1.0.2 DatahubEdge.o message.o cJSON.o libmosquitto.so libsqlite3.so libcurl.so.4.4.0

DatahubEdge.o: DatahubEdge.c
	gcc -c -fPIC -o DatahubEdge.o DatahubEdge.c

message.o : message.c cJSON.o
	gcc -c -fPIC -o message.o message.c

cJSON.o: cJSON.c
	gcc -c -fPIC -o cJSON.o cJSON.c

clean:
	rm -f sample *.o *.so.1.0.2 
	rm -rf $(CURDIR)/libovpn/build/*.*

openssl-link:
	tar -xvf $(CURDIR)/libovpn/openssl-1.0.2j.tar.gz
	cd $(CURDIR)/openssl-1.0.2j && ./Configure gcc -static -no-shared --prefix=$(CURDIR)/libovpn/build  
	cd $(CURDIR)/openssl-1.0.2j && $(MAKE) 
	cd $(CURDIR)/openssl-1.0.2j && $(MAKE) install
	rm -rf $(CURDIR)/openssl-1.0.2j

lzo-link:
	tar -xvf $(CURDIR)/libovpn/lzo-2.09.tar.gz
	cd $(CURDIR)/lzo-2.09 && ./configure --prefix=$(CURDIR)/libovpn/build --enable-static --disable-debug   
	cd $(CURDIR)/lzo-2.09 && $(MAKE) 
	cd $(CURDIR)/lzo-2.09 && $(MAKE) install
	rm -rf $(CURDIR)/lzo-2.09

openvpn-link:
	tar -xvf $(CURDIR)/libovpn/openvpn-2.3.12.tar.gz
	cd $(CURDIR)/openvpn-2.3.12 && \
	./configure --prefix=$(CURDIR)/libovpn/build \
	 	--disable-server --enable-static --disable-shared --disable-debug --disable-plugins \
	 	OPENSSL_SSL_LIBS="-L$(CURDIR)/libovpn/build/lib -lssl" \
	 	OPENSSL_SSL_CFLAGS="-I$(CURDIR)/libovpn/build/include" \
	 	OPENSSL_CRYPTO_LIBS="-L$(CURDIR)/libovpn/build/lib -lcrypto" \
	 	OPENSSL_CRYPTO_CFLAGS="-I$(CURDIR)/libovpn/build/include" \
	 	LZO_CFLAGS="-I$(CURDIR)/libovpn/build/include" \
	 	LZO_LIBS="-L$(CURDIR)/libovpn/build/lib -llzo2"&& \
	cd $(CURDIR)/openvpn-2.3.12 && $(MAKE) LIBS="-all-static"
	cd $(CURDIR)/openvpn-2.3.12 && $(MAKE) install
	rm -rf $(CURDIR)/openvpn-2.3.12
	mv $(CURDIR)/libovpn/build/sbin/openvpn $(CURDIR) 

