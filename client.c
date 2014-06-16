#include "chat.h"

/* To run the program as client you need to state the port, hostname and client username Ex: ./client -p 8888 localhost username */

int main(int argc, char *argv[]){

	/* Variables=======================*/
	int i = 0;
	int j = 0;
	int k = 0;
	int aux = 0;
	char msg[MSG_SIZE];
	char kb_msg[MSG_SIZE];
	char username[20];
	char auxname[20];
	int result;
	int port;
	fd_set clientfds, testfds;
	int sockfd;
	char hostname[MSG_SIZE];
	struct hostent *hostinfo;
	struct sockaddr_in address;
	int clientid;
	int fd;

	/* Client==================================================*/
	if (argc == 5){
		if (!strcmp("-p", argv[1])){
			if (argc != 5){
				printf("Invalid parameters.\nUsage: client [-p PORT] HOSTNAME USERNAME\n");
				exit(0);
			}
			else{
				sscanf(argv[2], "%i", &port);
				strcpy(hostname, argv[3]);
				strcpy(username, argv[4]);
			}
		}
		else{
			port = MYPORT;
			strcpy(hostname, argv[1]);
			strcpy(username, argv[4]);
		}
		printf("\n========Client program starting (enter \"/quit\" to stop): \n");
		fflush(stdout);

		/* Create a socket for the client */
		sockfd = socket(AF_INET, SOCK_STREAM, 0);

		/* Name the socket, as agreed with the server */
		hostinfo = gethostbyname(hostname);  /* look for host's name */
		address.sin_addr = *(struct in_addr *)*hostinfo->h_addr_list;
		address.sin_family = AF_INET;
		address.sin_port = htons(port);

		/* Connect the socket to the server's socket */
		if (connect(sockfd, (struct sockaddr *)&address, sizeof(address)) < 0){
			perror("connecting");
			exit(1);
		}

		fflush(stdout);

		FD_ZERO(&clientfds);
		FD_SET(sockfd, &clientfds);
		FD_SET(0, &clientfds);//stdin

		/* Send username to server */
		write(sockfd, username, strlen(username));

		/* Now wait for messages from the server */
		while (1) {
			testfds = clientfds;
			select(FD_SETSIZE, &testfds, NULL, NULL, NULL);

			for (fd = 0; fd < FD_SETSIZE; fd++){
				if (FD_ISSET(fd, &testfds)){
					if (fd == sockfd){   /* Accept data from open socket */

						//read data from open socket
						result = read(sockfd, msg, MSG_SIZE);
						msg[result] = '\0';  /* Terminate string with null */
						printf("%s", msg + 1);
						fflush(stdout);

						if (msg[0] == 'X') {
							close(sockfd);
							exit(0);
						}
					}
					else if (fd == 0){ /* Process keyboard activiy */

						fgets(kb_msg, MSG_SIZE + 1, stdin);
						if (strcmp(kb_msg, "/quit\n") == 0) {
							sprintf(msg, "XClient is shutting down.\n");
							write(sockfd, msg, strlen(msg));
							close(sockfd); //close the current client
							exit(0); //end program
						}

						else {
							sprintf(msg, "M%s", kb_msg);
							write(sockfd, msg, strlen(msg));
						}//else
					}//elif
				}//if
			}//for
		}//while
	}// end client code
}//main