#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <string.h>
#include <fcntl.h>

int main(int argc, char *argv[]){

	// HTTP RESPONSE HEADER FOR PNG IMAGE FILES
	char *png_header = "HTTP/1.1 200 OK\r\n"
					   "Content-Type: image/png\r\n\r\n";

	char *ico_header = "HTTP/1.1 200 OK\r\n"
					   "Content-Type: image/x-icon\r\n\r\n";

	int command_port;
	// GETTING PORT_NUM FROM COMMAND LINE ARGUMENT
	switch(argc){
		/* CASE 1: NO ARGUMENT SUPPLIED
		 * PRINT ERROR MESSAGES & REQUEST
		 * PROPER ARGUMENT(S)
		 * TERMINATE PROGRAM */
		case 1:
			printf("[-] NO ARGUMENTS SUPPLIED!\n");
			printf("\tYou must specify a port in range [60000, 60099] to connect to\n");
			exit(1);

		case 2:
		/* CASE 2: CORRECT NUMBER OF ARGUMENTS
		 * CHECK THAT THE ARGUMENT IS A VALID PORT NUMBER
		 * IF NOT VALID, THEN PRINT ERROR MESSAGES & REQUEST
		 * FOR VALID PORT NUMBER, EXIT PROGRAM
		 * IF VALID, ACCEPT PORT AND CONTINUE */
			command_port = atoi(argv[1]);  //atoi() converts the string arg to an int
			if(command_port > 60099 || command_port < 60000){
				printf("[-] INVALID PORT NUMBER!\n");
				printf("\tUse port number in range [60000, 60099]\n");
				exit(1);
			}
			printf("[+] PORT ACCEPTED. . .\n");
			break;

		/* DEFAULT: TOO MANY ARGUMENTS SUPPLIED
		 * PRINT ERROR MESSAGES AND REQUEST
		 * CORRECT NUMBER OF ARGS
		 * TERMINATE PROGRAM */
		default:
			printf("[-] TOO MANY ARGUMENTS\n");
			printf("\tExpecting 1 argument:\n");
			printf("\tUse port number in range [60000, 60099]\n");
			exit(1);
	}

	// STORING VALIDATED PORT NUMBER IN PORT_NUM
	const int PORT_NUM = command_port;

	/* CREATING SOCKET
	 * AF_INET     : IPv4 ADDRESS FAMILY
	 * SOCK_STREAM : USING TCP CONNECTION
	 * 0           : DEFAULT PROTOCOLS */
	int server_socket = socket(AF_INET, SOCK_STREAM, 0);

	// POPULATING AN ADDRESS STRUCTURE
	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(PORT_NUM);   //htons() converts int to network byte order
	server_address.sin_addr.s_addr = INADDR_ANY; // INADDR_ANY connects to all IP addresses for local machine


	/* BINDING SOCKET
	 * (struct sockaddr*) &server_address : CASTING SOCKADDR_IN TO SOCKADDR FOR FUCTION
	 * IF bind() RETURNS 0, BINDING FAILED, PRINT ERROR AND EXIT */
	if( bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0){
		printf("[-] FAILED TO BIND TO PORT\n");
		printf("\tTry using a different port number\n");
		exit(1);
	}
	printf("[+] SUCCESSFULLY BOUND:\n");
	printf("\tBound to port:%d\n", PORT_NUM);

	/* LISTENING FOR CONNECTIONS
	 * IF listen() RETURNS 0, SOCKET IS SUCCESSFULLY LISTENING
	 * EXIT PROGRAM
	 * IF listen() RETURNS -1, SOCKET FAILED TO LISTEN
	 * CONTINUE*/
	if( listen(server_socket, 5) != 0){
		printf("[-] LISTENING FOR CONNECTIONS FAILED\n");
		exit(1);
	}

	// BEGIN ACCEPTING REQUESTS AND PROCESSING THEM
	while(1){
		char *token;
		char *action, *path, *end;
		char buff[1024];
		// ACCEPT A CONNECTION
		int client_socket = accept(server_socket, NULL, NULL);

		/* IF FAILED TO CONNECT, PRINT ERROR
		 * AND GO BACK TO LISTENING */
		if(client_socket == -1){
			printf("[-] ERROR ACCEPTING CONNECTION\n");
			continue;
		}

		// GET REQUEST INFO
		read(client_socket, buff, 1024);
		token = strtok(buff, " ");
		action = token;
		token = strtok(NULL, " ");
		path = token;
		token = strtok(NULL, "\r\n");
		end = token;

		printf("[+] CONNECTION ACCEPTED\n");
		printf("\tREQUEST: %s %s %s\n", action, path, end);

		// FORKING CHILD PROCESS TO HANDLE REQUEST
		int pid = fork();
		if(pid == 0){/*child process*/
			close(server_socket);
			int htmlFile;


			//send(client_socket, message, strlen(message), 0);
			if(strcmp(action, "GET") != 0){
				htmlFile = open("400error.html", O_RDONLY);
				sendfile(client_socket, htmlFile, NULL, 1024);
				printf("\t[+] SENT: 400error.html\n");
			}
			else if(strcmp(path, "/") == 0 || strcmp(path, "/index.html") == 0){
				htmlFile = open("index.html", O_RDONLY);
				sendfile(client_socket, htmlFile, NULL, 1024);
				printf("\t[+] SENT: index.html\n");
			}
			else if(strcmp(path, "/favicon.ico") == 0){
				htmlFile = open("favicon.ico", O_RDONLY);
				send(client_socket, ico_header, strlen(ico_header), 0);
				sendfile(client_socket, htmlFile, NULL, 1024);
				printf("\t[+] SENT: favicon.ico\n");
			}
			else if(strcmp(path, "/bearded_bot.html") == 0){
				htmlFile = open("bearded_bot.html", O_RDONLY);
				sendfile(client_socket, htmlFile, NULL, 1024);
				printf("\t[+] SENT: bearded_bot.html\n");
			}
			else if(strcmp(path, "/bot.png") == 0){
				htmlFile = open("bot.png", O_RDONLY);
				send(client_socket, png_header, strlen(png_header), 0);
				sendfile(client_socket, htmlFile, NULL, 500000);
				printf("\t[+] SENT: bot.png\n");
			}
			else{
				htmlFile = open("404error.html", O_RDONLY);
				sendfile(client_socket, htmlFile, NULL, 1024);
				printf("\t[+] SENT: 404error.html\n");
			}
			printf("\t[+] CLOSING COMMUNICATION\n");
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
