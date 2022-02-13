#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <string.h>
#include <fcntl.h>

int main(){

	const int PORT_NUM = 60079;
	int server_socket = socket(AF_INET, SOCK_STREAM, 0);

	char *png_header = "HTTP/1.1 200 OK\r\n"
					   "Content-Type: image/png\r\n\r\n";

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

		//PRACTICE PARSING
		char *token;
		char buff[1024];
		read(client_socket, buff, 1024);
		token = strtok(buff, " ");
		token = strtok(NULL, " ");
		printf("Page Requested:%s\n", token);

		int pid = fork();
		if(pid == 0){/*child process*/
			close(server_socket);
			int htmlFile;

			//send(client_socket, message, strlen(message), 0);
			if(strcmp(token, "/") == 0){
				htmlFile = open("index.html", O_RDONLY);
				sendfile(client_socket, htmlFile, NULL, 1024);
			}
			else if(strcmp(token, "/favicon.ico") == 0){
				htmlFile = open("index.html", O_RDONLY);
				sendfile(client_socket, htmlFile, NULL, 1024);
			}
			else if(strcmp(token, "/contact.html") == 0){
				htmlFile = open("bot.png", O_RDONLY);
				send(client_socket, png_header, strlen(png_header), 0);
				sendfile(client_socket, htmlFile, NULL, 500000);
			}
			close(htmlFile);
			close(client_socket);
			break;
		}
		else{
			close(client_socket);
		}
	}

	return 0;
}
