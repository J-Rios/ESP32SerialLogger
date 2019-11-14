### Description
This directory contains a Python HTTPS Webserver with his certificate, that going to be uploaded as binary into ESP Flash .rodata section.


### To create custom certifies
1 - Create a 2048 bits AES256 encrypted key (Set password passphrase, in this example "pass1234"):
```
openssl genrsa -aes256 -out ota_key.pem 2048
```

2 - Create a certificate sign request file (Remember that "Common Name" must be same as server IP adress, in this case, the machine that will run the webserver is iside my local network and has a static IP of 192.168.0.20):
```
openssl req -key ota_key.pem -new -sha256 -out ota_csr.pem

Enter pass phrase for key.pem:pass1234

You are about to be asked to enter information that will be incorporated into your certificate request.
What you are about to enter is what is called a Distinguished Name or a DN.
There are quite a few fields but you can leave some blank
For some fields there will be a default value, If you enter '.', the field will be left blank.
-----
Country Name (2 letter code) [AU]:ES
State or Province Name (full name) [Some-State]:Malaga
Locality Name (eg, city) []:Malaga
Organization Name (eg, company) [Internet Widgits Pty Ltd]:Open Source
Organizational Unit Name (eg, section) []:
Common Name (e.g. server FQDN or YOUR name) []:192.168.0.20
Email Address []:
```

3 - Self-Sign the certificate for 10 years (3650 days):
```
openssl x509 -req -days 3650 -in ota_csr.pem -signkey ota_key.pem -out ota_cert.pem
```


### To start Python HTTPS server
```
python3 ./https_server.py
```


### Notes
SSL cert file is uploaded to ESP because it was specificied using -DCOMPONENT_EMBED_TXTFILES build flag inside platformio.init file:
```
build_flags = -DCOMPONENT_EMBED_TXTFILES=otawebserver/certs/ota_cert.pem
```

To access cert binary data from ESP program code, use:
```
extern const uint8_t server_cert_pem_start[] asm("_binary_otawebserver_certs_ota_cert_pem_start");
extern const uint8_t server_cert_pem_end[] asm("_binary_otawebserver_certs_ota_cert_pem_end");
```
