#!/usr/bin/env python3
# -*- coding: utf-8 -*-

####################################################################################################

### Note: http.server is not recommended for production, it only implements basic security. ###

from http.server import HTTPServer, SimpleHTTPRequestHandler
import ssl
import signal
import os

####################################################################################################

port = 443
server_path_dir = os.path.join(os.path.dirname(os.path.realpath(__file__)), "server")
certs_path_dir = os.path.join(os.path.dirname(os.path.realpath(__file__)), "certs")
server_cert_file = os.path.join(certs_path_dir, "cert.pem")
server_key_file = os.path.join(certs_path_dir, "key.pem")

####################################################################################################

### Termination signals handler for program process ###
def signal_handler(signal, frame):
    '''Termination signals (SIGINT, SIGTERM) handler for program process'''
    print("\nProgram close successfully.\n")
    os._exit(0)


### Signals attachment ###
signal.signal(signal.SIGTERM, signal_handler) # SIGTERM (kill pid) to signal_handler
signal.signal(signal.SIGINT, signal_handler) # SIGINT (Ctrl+C) to signal_handler

####################################################################################################

### Main Function ###

def main():
    '''Main Function'''
    # Change to Server subdirectory
    print("\nScript started.\n")
    os.chdir(server_path_dir)
    # Create the Server
    print("Launching HTTPS Server:")
    print("    Server Port - {}".format(port))
    print("    Server path - {}".format(server_path_dir))
    print("    Private key file - {}".format(server_key_file))
    print("    Certificate file - {}\n".format(server_cert_file))
    httpd = HTTPServer(("", port), SimpleHTTPRequestHandler)
    httpd.socket = ssl.wrap_socket(httpd.socket, server_side=True, certfile=server_cert_file, 
	                               keyfile=server_key_file, ssl_version=ssl.PROTOCOL_TLS)
    print("Server running at https://localhost:{}".format(str(port)))
    # Run the Server
    httpd.serve_forever()


####################################################################################################

if __name__ == '__main__':
    main()

### End Of Code ###