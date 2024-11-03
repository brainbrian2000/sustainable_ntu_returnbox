#!/bin/bash

# 檢查是否安裝了 openssl
if ! command -v openssl &> /dev/null
then
    echo "openssl 未安裝，請先安裝它：sudo apt-get install openssl"
    exit
fi

# 設定檔案名稱
CA_KEY="ca.key"
CA_CERT="ca.crt"
SERVER_KEY="server.key"
SERVER_CSR="server.csr"
SERVER_CERT="server.crt"

# 生成 CA 私鑰
openssl genrsa -out $CA_KEY 2048

# 創建自簽名的 CA 憑證
openssl req -x509 -new -nodes -key $CA_KEY -sha256 -days 1024 -out $CA_CERT -subj "/C=TW/ST=Taiwan/L=Taipei/O=NTU/OU=CC/CN=ntu_peap_CA"

# 生成伺服器私鑰
openssl genrsa -out $SERVER_KEY 2048

# 創建伺服器的 CSR（Certificate Signing Request）
openssl req -new -key $SERVER_KEY -out $SERVER_CSR -subj "/C=TW/ST=Taiwan/L=Taipei/O=NTU/OU=CC/CN=ntu_peap_server"

# 使用 CA 簽署伺服器的 CSR，生成伺服器憑證
openssl x509 -req -in $SERVER_CSR -CA $CA_CERT -CAkey $CA_KEY -CAcreateserial -out $SERVER_CERT -days 500 -sha256

echo "憑證生成完成："
echo "CA 憑證: $CA_CERT"
echo "伺服器憑證: $SERVER_CERT"
xxd -i ca.pem ca_pem.h