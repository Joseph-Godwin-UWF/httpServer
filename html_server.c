#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <string.h>
#include <fcntl.h>

int main(){

	const int PORT_NUM = 60044;

	int server_socket = socket(AF_INET, SOCK_STREAM, 0);

	char *message = "HTTP/1.1 200 OK\r\n"
					"Content-Type: text/html\r\n"
					"\r\n"
					"<!DOCTYPE html"
					"<html"
					"<body>"
					"<p>Henlo</p>"
					"</body>"
					"</html>";

	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(PORT_NUM);
	server_address.sin_addr.s_addr = INADDR_ANY;

	int bind_status = bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address));
	if(bind_status < 0){
		perror("[-] Failed to bind to port\n");
		exit(1);
	}
	printf("[+] Binding successful\n");

	listen(server_socket, 5);

	while(1){
		int client_socket = accept(server_socket, NULL, NULL);
		printf("[+] Connection accepted\n");

		int pid = fork();
		if(pid == 0){/*child process*/
			close(server_socket);
			send(client_socket, message, strlen(message), 0);
			close(client_socket);
			break;
		}
		else{
			close(client_socket);
		}
	}

	return 0;
}