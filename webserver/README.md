### Description
This directory contains the ESP32 HTTPS Webserver with his certificate and private key, that going to be uploaded as binary into ESP Flash .rodata section.

Note: Server is send through HTML files and images are encoded and hardcoded inside HTML code.


### To create custom certifies
1 - Create a 2048 bits AES256 encrypted key (Set password passphrase, in this example "pass1234"):
```
openssl genrsa -aes256 -out esp_key.pem 2048
```

2 - Create a certificate sign request file (Remember that "Common Name" must be same as server IP adress, in this case, the ESP32 that will run the webserver and has the IP 192.168.4.1):
```
openssl req -key esp_key.pem -new -sha256 -out esp_csr.pem

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
Common Name (e.g. server FQDN or YOUR name) []:192.168.4.1
Email Address []:
```

3 - Self-Sign the certificate for 10 years (3650 days):
```
openssl x509 -req -days 3650 -in esp_csr.pem -signkey esp_key.pem -out esp_cert.pem
```

### Notes
SSL cert file is uploaded to ESP because it was specificied using -DCOMPONENT_EMBED_TXTFILES build flag inside platformio.init file:
```
build_flags = -DCOMPONENT_EMBED_TXTFILES=httpsserver/certs/esp_cert.pem:httpsserver/certs/esp_key.pem
```

To access cert binary data from ESP program code, use:
```
extern const uint8_t server_cert_pem_start[] asm("_binary_httpsserver_certs_esp_cert_pem_start");
extern const uint8_t server_cert_pem_end[] asm("_binary_httpsserver_certs_esp_cert_pem_end");
extern const uint8_t server_key_pem_start[] asm("_binary_httpsserver_certs_esp_key_pem_start");
extern const uint8_t server_key_pem_end[] asm("_binary_httpsserver_certs_esp_key_pem_end");
```
