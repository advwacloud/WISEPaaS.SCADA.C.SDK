#!/bin/bash
# FROM: https://mosquitto.org/man/mosquitto-tls-7.html and
# http://www.steves-internet-guide.com/mosquitto-tls/
set -e

# logging
RESTORE='\033[0m'

RED='\033[00;31m'
GREEN='\033[00;32m'
YELLOW='\033[00;33m'
BLUE='\033[00;34m'
PURPLE='\033[00;35m'
CYAN='\033[00;36m'
LIGHTGRAY='\033[00;37m'

LRED='\033[01;31m'
LGREEN='\033[01;32m'
LYELLOW='\033[01;33m'
LBLUE='\033[01;34m'
LPURPLE='\033[01;35m'
LCYAN='\033[01;36m'
WHITE='\033[01;37m'

REQNUM=0

print_err() {
    echo -e "${RED}ERROR $@ ${RESTORE}"
}

print_succ() {
    echo -e "${GREEN} SUCCES: $@ ${RESTORE}"
}

print_warn() {
    echo -e "${BLUE} WARN: $@ ${RESTORE}"
}


# CA & SRV need to have different params for mosquitto broker to work & to avoid needles asking
SUBJ="-subj "'/C=GB/ST=London/L=London/O='"$((++REQNUM))$1"'/OU=IT_Department/CN=localhost.local'

# gen CA
gen_CA() {
    print_warn "generate CA"
    openssl genrsa -out ./cert/ca.key 4096
    openssl req -x509 -new -nodes -key ./cert/ca.key -sha256 ${DAYS} -out ./cert/ca.crt ${SUBJ}
}

# CLIENT
gen_client_keys() {
    print_warn "Generate a client key"
    openssl genrsa ${PSWD} -out ./cert/client.key 2048 ${SUBJ} 
    print_warn " Generate a certificate signing request to send to the CA"
    openssl req -out ./cert/client.csr -key ./cert/client.key -new ${SUBJ}
    print_warn "Send the CSR to the CA, or sign it with your CA key"
    openssl x509 -req -in ./cert/client.csr -CA ./cert/ca.crt -CAkey ./cert/ca.key  -addtrust clientAuth -CAcreateserial -out ./cert/client.crt ${DAYS}
}

print_help() {
    echo "usage: "
    echo "--CA or --CLI"
    echo "--des3 to use passwd on cers"
    echo "--days 'N' to use expirydate"
}

[ $1 ] || print_help

for a in $@; do 
    case "$a" in
        "--CA")
            gen_CA && print_succ "CA" || print_err "CA failed"
            ;;
        "--CLI")
            gen_client_keys && print_succ "cli" || print_err "client keys failed"
            ;;
        "--pass")
            PSWD="-des3"
            ;;
        "--days")
            DAYS="-days $2"
            shift
            ;;
        -h|--help)
            print_help
            ;;
        *)
            print_help;
            echo "bad param! $a"
            ;;
    esac
done