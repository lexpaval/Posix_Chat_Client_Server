#include "chat.h"

/* To run the program as server you can state the port (optional) Ex: ./server -p 8888 */

/* Clients database */
typedef struct users_db_t{
	char username[20];
	int sockfd;
}users_db;

/* Function that compares names - qsort function */
int compare(const void *a, const void *b){
	const users_db *a_node = a;
	const users_db *b_node = b;
	
	return strcmp(a_node->username, b_node->username);
}

/* Function that correctly clears a string */
void cls(char string[]){
	int i = 0;
	int aux = strlen(string);
	
	for (i = 0; i < aux; i++){
		string[i] = '\0';
	}
}

/* Function that handles the exit of a client */
void exitClient(int fd, fd_set *readfds, users_db usersDB[], int *num_clients){
	int i;

	close(fd);
	FD_CLR(fd, readfds); //clear the leaving client from the set
	for (i = 0; i < (*num_clients) - 1; i++)
	if (usersDB[i].sockfd == fd)
		break;
	for (; i < (*num_clients) - 1; i++)
		(usersDB[i]) = (usersDB[i + 1]);
	(*num_clients)--;
	//now sort our remaining list
	qsort(usersDB, *(num_clients), sizeof(users_db), compare);

	//now show the remaining users to the server
	printf("=======Total users connected: \n");
	for (i = 0; i < *(num_clients); i++){
		printf(" Username: %s \n", usersDB[i].username);
	}
}

/* Main function, duh */
int main(int argc, char *argv[]){

	/*Variables=======================*/
	int i = 0;
	int j = 0;
	int k = 0;
	char msg[MSG_SIZE];
	char kb_msg[MSG_SIZE];
	char aux_msg[MSG_SIZE];
	int aux = 0;
	char auxname[20];
	int result;
	int port;
	int num_clients = 0;
	int max_clients = 0;
	unsigned long lines = 0;
	int server_sockfd, client_sockfd;
	struct sockaddr_in server_address;
	int addresslen = sizeof(struct sockaddr_in);
	int fd;
	int file_fd;
	fd_set readfds, testfds, clientfds;
	users_db usersDB[MAX_CLIENTS];
	time_t t = NULL;
	struct tm tm;
	char c;

	/*Server==================================================*/
	if (argc == 1 || argc == 3){
		if (argc == 3){
			if (!strcmp("-p", argv[1])){
				sscanf(argv[2], "%i", &port);
			}
			else{
				printf("Invalid parameter.\nUsage: server [-p PORT]\n");
				exit(0);
			}
		}
		else port = MYPORT;

		/* Open or create log file */
		file_fd = open(MYPATH, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IXUSR);

		printf("\n========Server program starting at ip %s (enter \"/quit\" to stop): \n", MYIP);

		t = time(NULL);
		tm = *localtime(&t);
		sprintf(msg, "=======Session started at ip: %s and date: %d-%d-%d %d:%d:%d\n", MYIP, tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec);
		write(file_fd, msg, strlen(msg));
		cls(msg);
		fflush(stdout);

		/* Create and name a socket for the server */
		server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
		server_address.sin_family = AF_INET;
		server_address.sin_addr.s_addr = inet_addr (MYIP);
		server_address.sin_port = htons(port);
		bind(server_sockfd, (struct sockaddr *)&server_address, addresslen);

		/* Create a connection queue and initialize a file descriptor set */
		listen(server_sockfd, 1);
		FD_ZERO(&readfds);
		FD_SET(server_sockfd, &readfds);
		FD_SET(0, &readfds);  /* Add keyboard to file descriptor set */

		/*  Now wait for clients and requests */
		while (1) {
			testfds = readfds;
			select(FD_SETSIZE, &testfds, NULL, NULL, NULL);

			/* If there is activity, find which descriptor it's on using FD_ISSET */
			for (fd = 0; fd < FD_SETSIZE; fd++) {
				if (FD_ISSET(fd, &testfds)) {

					if (fd == server_sockfd) { /* Accept a new connection request */
						client_sockfd = accept(server_sockfd, NULL, NULL);

						if (num_clients < MAX_CLIENTS) {
							FD_SET(client_sockfd, &readfds);
							//Client ID
							usersDB[num_clients].sockfd = client_sockfd;
							//Clear the string if other users have previously been connected
							cls(msg);
							//Get username from client
							result = read(client_sockfd, msg, MSG_SIZE);
							if (result == -1) perror("read():");

							strcpy(usersDB[num_clients].username, msg);

							t = time(NULL);
							tm = *localtime(&t);

							printf("=======Client %s at socket %d joined\n", usersDB[num_clients].username, usersDB[num_clients].sockfd);
							sprintf(kb_msg, "========Client %s at socket %d joined\n", usersDB[num_clients].username, usersDB[num_clients].sockfd);
							
							//Send to all clients that a new client has joined
							for (i = 0; i < num_clients; i++){
								write(usersDB[i].sockfd, kb_msg, strlen(kb_msg));
							}

							//Put the event in the log with time
							sprintf(kb_msg, "%d:%d:%d:=======Client %s at socket %d joined\n", tm.tm_hour, tm.tm_min, tm.tm_sec,
								usersDB[num_clients].username, usersDB[num_clients].sockfd);
							write(file_fd, kb_msg, strlen(kb_msg));

							//Now increment our num_clients
							num_clients++;
							
							//Maximum connected clients this session
							if (num_clients > max_clients)
								max_clients++;

							//Sort clients database alphabetically
							qsort(usersDB, num_clients, sizeof(users_db), compare);

							//Now print all users to the server
							printf("=======Total users connected: \n");
							for (i = 0; i < num_clients; i++){
								printf(" Username: %s \n", usersDB[i].username);
							}
							cls(msg);
							cls(kb_msg);
						}
						else {
							sprintf(msg, "X=======Sorry, too many clients.  Try again later.\n");
							write(client_sockfd, msg, strlen(msg));
							close(client_sockfd);
						}
					}
					else if (fd == 0)  {  /* Process keyboard activity */
						fgets(kb_msg, MSG_SIZE + 1, stdin);
						if (strcmp(kb_msg, "/quit\n") == 0) {
							t = time(NULL);
							tm = *localtime(&t);
							sprintf(msg, "X=======Server shut down at %d-%d-%d %d:%d:%d\n", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec);
							write(file_fd, msg + 1, strlen(msg + 1));
							for (i = 0; i < num_clients; i++) {
								write(usersDB[i].sockfd, msg, strlen(msg));
								close(usersDB[i].sockfd);
							}
							close(server_sockfd);
							close(file_fd);
							exit(0);
						}
						else {
							t = time(NULL);
							tm = *localtime(&t);
							lines++;
							sprintf(msg, "MServer: %s", kb_msg);
							for (i = 0; i < num_clients; i++)
								write(usersDB[i].sockfd, msg, strlen(msg));
							sprintf(msg, "%d:%d:%d: Server: %s", tm.tm_hour, tm.tm_min, tm.tm_sec, kb_msg);
							write(file_fd, msg, strlen(msg));
							cls(msg);
							cls(kb_msg);
						}
					}
					else if (fd) {  /* Process Client specific activity */
						//read data from open socket
						cls(msg);
						result = read(fd, msg, MSG_SIZE);

						if (result == -1) perror("read()");
						else if (result > 0){

							/* Check for server related commands requested by the client */
							/* Show all the online users */
							if (strstr(msg, "/users") != NULL) {
								sprintf(msg, "========Total users connected: \n");
								write(fd, msg, strlen(msg));
								cls(msg);
								for (i = 0; i < num_clients; i++){
									sprintf(msg, " Username: %s\n", usersDB[i].username);
									write(fd, msg, strlen(msg));
									cls(msg);
								}
								cls(msg);
							}
							/* Change the username to the client that requested it */
							else if (strstr(msg, "/name") != NULL) {
								//get time of event
								t = time(NULL);
								tm = *localtime(&t);

								aux = strlen(msg);
								strcpy(auxname, &msg[7]);
								auxname[strlen(auxname) - 1] = '\0';
								cls(msg);

								//make the change in the DB
								for (i = 0; i < num_clients; i++){
									if (usersDB[i].sockfd == fd){
										sprintf(msg, "========Username %s changed name to %s \n", usersDB[i].username, auxname);
										strcpy(usersDB[i].username, auxname);
									}
								}

								//show everyone the change
								printf("%s", msg + 1);
								sprintf(kb_msg, "%d:%d:%d:%s", tm.tm_hour, tm.tm_min, tm.tm_sec, msg);
								write(file_fd, kb_msg, strlen(kb_msg));
								for (i = 0; i < num_clients; i++){
									write(usersDB[i].sockfd, msg, strlen(msg));
								}

								//sort the DB after the change
								qsort(usersDB, num_clients, sizeof(users_db), compare);
								cls(msg);
								cls(kb_msg);
							}
							/* Show server's time */
							else if (strstr(msg, "/time") != NULL) {
								t = time(NULL);
								sprintf(msg, "========Server local time: %s", asctime(localtime(&t)));
								write(fd, msg, strlen(msg));
								cls(msg);
							}
							/* Show all the commands supported by the server */
							else if (strstr(msg, "/help") != NULL) {
								sprintf(msg, "========Supported commands are: /help, /name NEWNAME, /users,");
								write(fd, msg, strlen(msg));
								cls(msg);
								sprintf(msg, " /time, /lines, /maxclients, /send @USERNAME MESSAGE @@USERNAME-N MESSAGE, /quit \n");
								write(fd, msg, strlen(msg));
								cls(msg);
							}
							/* Show maximum connected users this session */
							else if (strstr(msg, "/maxclients") != NULL) {
								t = time(NULL);
								sprintf(msg, "========Maximum connected users: %d \n", max_clients);
								write(fd, msg, strlen(msg));
								cls(msg);
							}
							/* Show number of messages sent this session */
							else if (strstr(msg, "/lines") != NULL) {
								t = time(NULL);
								sprintf(msg, "========Messages sent this session: %d \n", lines);
								write(fd, msg, strlen(msg));
								cls(msg);
							}
							/* Send message to one or more clients */
							else if (strstr(msg, "/send") != NULL) {
								for (i = 0; i < strlen(msg); i++){
									cls(kb_msg);
									cls(aux_msg);
									c = msg[i];
									if (c == '@'){
										for (j = 0; j < strlen(msg); j++){
											aux_msg[j] = c;
											i++;
											c = msg[i];
											if (c == '@') break;
											else if (c == '\n') break;
										}
										for (k = 0; k < num_clients; k++){
											if (strstr(aux_msg, usersDB[k].username) != NULL){
												for (j = 0; j < num_clients; j++){
													if (usersDB[j].sockfd == fd){
														sprintf(kb_msg, "M%s: %s\n", usersDB[j].username, aux_msg);
													}
												}
												write(usersDB[k].sockfd, kb_msg, strlen(kb_msg));
											}
										}
										cls(kb_msg);
										cls(aux_msg);
									}
								}
								cls(msg);
							}
							/* Send a client's message globally */
							else {
								//clear stuff just in case
								cls(kb_msg);
							
								//get time of message
								t = time(NULL);
								tm = *localtime(&t);
								lines++;

								//concatenate the client's username with the client's message
								for (i = 0; i < num_clients; i++){
									if (usersDB[i].sockfd == fd){
										sprintf(kb_msg, " %s: ", usersDB[i].username);
										break;
									}
								}
								msg[result] = '\0';
								strcat(kb_msg, msg + 1);

								//print to other clients
								for (i = 0; i < num_clients; i++){
									if (usersDB[i].sockfd != fd)  //dont write msg to same client
										write(usersDB[i].sockfd, kb_msg, strlen(kb_msg));
								}
								//print to server
								printf("%s", kb_msg + 1);

								//print to log
								cls(aux_msg);
								sprintf(aux_msg, "%d:%d:%d:%s", tm.tm_hour, tm.tm_min, tm.tm_sec, kb_msg);
								write(file_fd, aux_msg, strlen(aux_msg));

								//exit client
								if (msg[0] == 'X'){
									exitClient(fd, &readfds, usersDB, &num_clients);
								}
								cls(msg);
								cls(aux_msg);
								cls(kb_msg);
							}
						}
					}
					else {  /* A client is leaving */
						exitClient(fd, &readfds, usersDB, &num_clients);
					}//if
				}//if
			}//for
		}//while
	}//end Server code

}//main