const char * usage =
"                                                               \n"
"IRCServer:                                                   \n"
"                                                               \n"
"Simple server program used to communicate multiple users       \n"
"                                                               \n"
"To use it in one window type:                                  \n"
"                                                               \n"
"   IRCServer <port>                                          \n"
"                                                               \n"
"Where 1024 < port < 65536.                                     \n"
"                                                               \n"
"In another window type:                                        \n"
"                                                               \n"
"   telnet <host> <port>                                        \n"
"                                                               \n"
"where <host> is the name of the machine where talk-server      \n"
"is running. <port> is the port number you used when you run    \n"
"daytime-server.                                                \n"
"                                                               \n";

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "HashTableVoid.h"
#include "IRCServer.h"
#include <vector>
#include <iostream>
using namespace std;
HashTableVoid Users;
vector<char*> users;
//vector<char*> pass;
void sArray();
int count = 0;
struct Room {
	char *name;
	vector<char*> users;
	vector<char*> messages;
};
int QueueLength = 5;
vector<Room> rooms;
int
IRCServer::open_server_socket(int port) {

	// Set the IP address and port for this server
	struct sockaddr_in serverIPAddress; 
	memset( &serverIPAddress, 0, sizeof(serverIPAddress) );
	serverIPAddress.sin_family = AF_INET;
	serverIPAddress.sin_addr.s_addr = INADDR_ANY;
	serverIPAddress.sin_port = htons((u_short) port);

	// Allocate a socket
	int masterSocket =  socket(PF_INET, SOCK_STREAM, 0);
	if ( masterSocket < 0) {
		perror("socket");
		exit( -1 );
	}

	// Set socket options to reuse port. Otherwise we will
	// have to wait about 2 minutes before reusing the sae port number
	int optval = 1; 
	int err = setsockopt(masterSocket, SOL_SOCKET, SO_REUSEADDR, 
			(char *) &optval, sizeof( int ) );

	// Bind the socket to the IP address and port
	int error = bind( masterSocket,
			(struct sockaddr *)&serverIPAddress,
			sizeof(serverIPAddress) );
	if ( error ) {
		perror("bind");
		exit( -1 );
	}

	// Put socket in listening mode and set the 
	// size of the queue of unprocessed connections
	error = listen( masterSocket, QueueLength);
	if ( error ) {
		perror("listen");
		exit( -1 );
	}

	return masterSocket;
}

	void
IRCServer::runServer(int port)
{
	int masterSocket = open_server_socket(port);

	initialize();

	while ( 1 ) {

		// Accept incoming connections
		struct sockaddr_in clientIPAddress;
		int alen = sizeof( clientIPAddress );
		int slaveSocket = accept( masterSocket,
				(struct sockaddr *)&clientIPAddress,
				(socklen_t*)&alen);

		if ( slaveSocket < 0 ) {
			perror( "accept" );
			exit( -1 );
		}

		// Process request.
		processRequest( slaveSocket );		
	}
}

	int
main( int argc, char ** argv )
{
	// Print usage if not enough arguments
	if ( argc < 2 ) {
		fprintf( stderr, "%s", usage );
		exit( -1 );
	}

	// Get the port from the arguments
	int port = atoi( argv[1] );

	IRCServer ircServer;

	// It will never return
	ircServer.runServer(port);

}

//
// Commands:
//   Commands are started y the client.
//
//   Request: ADD-USER <USER> <PASSWD>\r\n
//   Answer: OK\r\n or DENIED\r\n
//
//   REQUEST: GET-ALL-USERS <USER> <PASSWD>\r\n
//   Answer: USER1\r\n
//            USER2\r\n
//            ...
//            \r\n
//
//   REQUEST: CREATE-ROOM <USER> <PASSWD> <ROOM>\r\n
//   Answer: OK\n or DENIED\r\n
//
//   Request: LIST-ROOMS <USER> <PASSWD>\r\n
//   Answer: room1\r\n
//           room2\r\n
//           ...
//           \r\n
//
//   Request: ENTER-ROOM <USER> <PASSWD> <ROOM>\r\n
//   Answer: OK\n or DENIED\r\n
//
//   Request: LEAVE-ROOM <USER> <PASSWD>\r\n
//   Answer: OK\n or DENIED\r\n
//
//   Request: SEND-MESSAGE <USER> <PASSWD> <MESSAGE> <ROOM>\n
//   Answer: OK\n or DENIED\n
//
//   Request: GET-MESSAGES <USER> <PASSWD> <LAST-MESSAGE-NUM> <ROOM>\r\n
//   Answer: MSGNUM1 USER1 MESSAGE1\r\n
//           MSGNUM2 USER2 MESSAGE2\r\n
//           MSGNUM3 USER2 MESSAGE2\r\n
//           ...\r\n
//           \r\n
//
//    REQUEST: GET-USERS-IN-ROOM <USER> <PASSWD> <ROOM>\r\n
//    Answer: USER1\r\n
//            USER2\r\n
//            ...
//            \r\n
//

	void
IRCServer::processRequest( int fd )
{
	// Buffer used to store the comand received from the client
	const int MaxCommandLine = 1024;
	char commandLine[ MaxCommandLine + 1 ];
	int commandLineLength = 0;
	int n;

	// Currently character read
	unsigned char prevChar = 0;
	unsigned char newChar = 0;

	//
	// The client should send COMMAND-LINE\n
	// Read the name of the client character by character until a
	// \n is found.
	//

	// Read character by character until a \n is found or the command string is full.
	while ( commandLineLength < MaxCommandLine &&
			read( fd, &newChar, 1) > 0 ) {

		if (newChar == '\n' && prevChar == '\r') {
			break;
		}

		commandLine[ commandLineLength ] = newChar;
		commandLineLength++;

		prevChar = newChar;
	}

	// Add null character at the end of the string
	// Eliminate last \r
	commandLineLength--;
	commandLine[ commandLineLength ] = 0;

	printf("RECEIVED: %s\n", commandLine);

	//printf("The commandLine has the following format:\n");
	//printf("COMMAND <user> <password> <arguments>. See below.\n");
	//printf("You need to separate the commandLine into those components\n");
	//printf("For now, command, user, and password are hardwired.\n");
	char ch[5][1000];
	for(int i = 0; i<5; i++) {
		for (int j = 0; j < 1000; j++) {
			ch[i][j] = 0;
		}
	}
	char *p = commandLine;
	//printf("abcd\n");
	int i = 0, j = 0;
	while(*p != '\0') {
		if(*p != ' ' || i == 4)
			ch[i][j++] = *p;
		if(*p == ' ' && i < 4) {
			ch[i][j] = '\0';
			i++;
			j = 0;
		}
		p++;
	}
	//printf("%s%s\n",ch[0],ch[1]);
	const char * command = ch[0];
	const char * user = ch[1];
	const char * password = ch[2];
	const char * args = ch[3];
	const char * args1 = ch[4];
	//memset(ch, NULL, sizeof(ch[0][0] * 10 * 100));

	printf("command=%s\n", command);
	printf("user=%s\n", user);
	printf( "password=%s\n", password );
	printf("args=%s\n", args);
	printf("args1=%s\n", args1);
	if (!strcmp(command, "ADD-USER")) {
		addUser(fd, user, password, args);
	}
	else if (!strcmp(command, "ENTER-ROOM")) {
		enterRoom(fd, user, password, args);
	}
	else if (!strcmp(command, "LEAVE-ROOM")) {
		leaveRoom(fd, user, password, args);
	}
	else if (!strcmp(command, "SEND-MESSAGE")) {
		sendMessage(fd, user, password, args, args1);
	}
	else if (!strcmp(command, "GET-MESSAGES")) {
		getMessages(fd, user, password, args, args1);
	}
	else if (!strcmp(command, "GET-USERS-IN-ROOM")) {
		getUsersInRoom(fd, user, password, args);
	}
	else if (!strcmp(command, "GET-ALL-USERS")) {
		getAllUsers(fd, user, password, args);
	}
	else if (!strcmp(command, "CREATE-ROOM")) {
		createRoom(fd, user, password, args);
	}
	else if (!strcmp(command, "LIST-ROOMS")) {
		listRooms(fd, user, password, args);
	}
	else {
		const char * msg =  "UNKNOWN COMMAND\r\n";
		write(fd, msg, strlen(msg));
	}

	// Send OK answer
	//const char * msg =  "OK\n";
	//write(fd, msg, strlen(msg));

	close(fd);	
}

	void
IRCServer::initialize()
{
	// Open password file
	int count = 0;
	char *user, *pass;
	char *c = (char*) malloc(100*sizeof(char));
	char *d = (char*) malloc(100*sizeof(char));
	FILE *f = fopen("password.txt", "r");
	if(f!=NULL){
		while(fscanf(f, "%s", c) == 1) {
			//count++;
			//printf("abhiga\n");
			//user = c;
			user = strtok(c,"|");
			pass = strtok(NULL, "|");
			//fscanf(f,"%s",d);
			//pass = d;
			//fscanf(f,"%s",c);
			//printf("%s|%s\n",user,pass);
			//printf("%d%d\n",strcmp(user,"agaurav"),strcmp(pass,"purdue"));
			Users.insertItem(strdup(user),(void*) strdup(pass));
		}
	}
	// Initialize users in room
	free(c);
	free(d);
	// Initalize message list
	//vector<Room> rooms;
}

bool
IRCServer::checkPassword(int fd, const char * user, const char * password) {
	// Here check the password
	void *pass; //= (char*) malloc(100*sizeof(char));
	if(Users.find(user, &pass)){
		//printf("%s - %s\n", (char*)pass, password);
		if(strcmp(password,(char*)pass)==0){
			return true;
		}
		else return false;
	}
	else 
		return false;
}

	void
IRCServer::addUser(int fd, const char * user, const char * password, const char * args)
{
	// Here add a new user. For now always return OK.
	char *store = (char*) malloc(100*sizeof(char));
	FILE *file = fopen("password.txt", "a");
	//store = strdup(user);
	//memcpy(store, user, 100*sizeof(char));
	strcpy(store,user);
	strcat(store, "|");
	strcat(store, password);
	//strcat(store, "\n");
	//write(fd,store, strlen(store));
	void * s;
	//printf("----%s----\n",store); 
	if(!Users.find(user, &s)) {
		fprintf(file, "%s\n", store);
		//printf("%s\n",store);
		Users.insertItem(strdup(user), (void*)strdup(password));
		const char * msg =  "OK\r\n";
		write(fd, msg, strlen(msg));
	}
	else {
		const char * msg = "DENIED\r\n";
		write(fd,msg,strlen(msg));
	}
	//fprintf(file, "%s\n", store);
	fclose(file);
	free(store);
}

	void
IRCServer::enterRoom(int fd, const char * user, const char * password, const char * room)
{	int pos = 0;
	bool check = false;
	if(checkPassword(fd, user, password)) {
		for (int i = 0; i < rooms.size(); i++) {
			if(strcmp(rooms[i].name, room) == 0) {
				pos = i;
				check = true;
				break;
			}

		}
		if(check) { 
			for(int i = 0; i < rooms[pos].users.size(); i++) {
				if(strcmp(rooms[pos].users[i],user) == 0) {
					const char * msg = "DENIED\r\n";
					write(fd,msg,strlen(msg));
					return;
				}
			}
			rooms[pos].users.push_back(strdup(user));
			const char * msg = "OK\r\n";
			write(fd,msg,strlen(msg));

		}
		else {
			const char * msg = "DENIED\r\n";
			write(fd,msg,strlen(msg));
		}
	}
	else {
		const char * msg = "ERROR (Wrong password)\n";
		write(fd,msg,strlen(msg));
	}
}
	void
IRCServer::leaveRoom(int fd, const char * user, const char * password, const char * room)
{	
	bool check = false;
	int pos = 0;
	int upos = 0;
	bool exist = false;
	if(checkPassword(fd,user,password)) {
		for (int i = 0; i < rooms.size(); i++) {
			if(strcmp(rooms[i].name, room) == 0) {
				pos = i;
				check = true;
				break;
			}
		}
		if(check) {
			for (int i = 0; i < rooms[pos].users.size(); i++) {
				if (strcmp(rooms[pos].users[i], user) == 0) {
					upos = i;
					exist = true;
					break;
				}
			}
		}
		if(exist) {
			rooms[pos].users.erase(rooms[pos].users.begin() + upos);
			const char * msg = "OK\r\n";
			write(fd,msg,strlen(msg));
			return;
		}
		else {
			const char * msg = "ERROR (No user in room)\r\n";
			write(fd,msg,strlen(msg));
			return;
		}
	}
	else {
		 const char * msg = "ERROR (Wrong password)\r\n";
		 write(fd,msg,strlen(msg));
		 return;
	}
	const char * msg = "DENIED\r\n";
	write(fd,msg,strlen(msg));


}

	void
IRCServer::sendMessage(int fd, const char * user, const char * password, const char * room, const char * mess)
{	
	char *temp = (char*) malloc((sizeof(user) + sizeof(mess) + 100)*sizeof(char));
	bool check = false;
	bool exist = false;
	int pos = 0;
	if (checkPassword(fd, user, password)) {
		for (int i = 0; i < rooms.size(); i++) {
			if (strcmp(rooms[i].name, room) == 0) {
				check = true;
				pos = i;
				break;
			}
		}
		if(check) {
			for (int i = 0; i < rooms[pos].users.size(); i++) {
				if(strcmp(rooms[pos].users[i], user) == 0) {
					exist = true;
					break;
				}
			}}
		else {
			const char * msg = "ERROR (user not in room)\r\n";
			write(fd,msg,strlen(msg));
			return;
		}
		if (exist) {
			count++;
			if(rooms[pos].messages.size()==100)
				rooms[pos].messages.erase(rooms[pos].users.begin());
			strcpy(temp,user);
			strcat(temp," ");
			strcat(temp, mess);
			rooms[pos].messages.push_back(strdup(temp));
			const char * msg = "OK\r\n";
			write(fd,msg,strlen(msg));
			free(temp);
			return;
		}
	}
	else {
	                 const char * msg = "ERROR (Wrong password)\r\n";
	                 write(fd,msg,strlen(msg));
			 free(temp);
			 return;
	     }
	const char * msg = "DENIED\r\n";
	write(fd,msg,strlen(msg));
	

}

	void
IRCServer::getMessages(int fd, const char * user, const char * password, const char * n, const char * room)
{ 
	int pos = 0;
	bool check = false;
	int num = atoi(n);
	char number[4];
	if(checkPassword(fd, user, password)) {
		for (int i = 0; i < rooms.size(); i++) {
			if (strcmp(rooms[i].name, room) == 0) {
				check = true;
				pos = i;
				break;
			}
		}
		if (check) {
			for (int i = num + 1; i < count; i++) {
				sprintf(number, "%d", i);
				write(fd, number, strlen(number));
				write(fd, " ", strlen(" "));
				write(fd, rooms[pos].messages[i], strlen(rooms[pos].messages[i]));
				write(fd, "\r\n", strlen("\r\n"));
			}
			write(fd, "\r\n", strlen("\r\n"));
			return;
		}
	}
	else {
	                  const char * msg = "ERROR (Wrong password)\r\n";
	                  write(fd,msg,strlen(msg));
			  return;
	}
	const char * msg = "DENIED\r\n";
	write(fd,msg,strlen(msg));
}

	void
IRCServer::getUsersInRoom(int fd, const char * user, const char * password, const char * room)
{	bool check = false;
	int pos = 0;
	char *temp = (char *)malloc(100*sizeof(char));
	users.clear();
	if(checkPassword(fd, user, password)) {
		for(int i = 0; i < rooms.size(); i++) {
			if(strcmp(rooms[i].name, room) == 0) {
				check = true;
				pos = i;
				break;
			}
		}
		if(check) {
			for (int i = 0; i < rooms[pos].users.size(); i++) {
				users.push_back(strdup(rooms[pos].users[i]));
			}
			sArray();
			for (int i = 0; i < users.size(); i++) {
			        write(fd,users[i],strlen(users[i]));

				//write(fd, rooms[pos].users[i],strlen(rooms[pos].users[i]));
				write(fd, "\r\n", strlen("\r\n"));
			}
		}
		else {
			const char * msg = "DENIED\r\n";
			write(fd,msg,strlen(msg));
		}
	}
	else {
		const char * msg = "ERROR (Wrong Password)\r\n";
		write(fd,msg,strlen(msg));
		return;
	}
	write(fd,"\r\n",strlen("\r\n"));
	free(temp);
}

	void
IRCServer::getAllUsers(int fd, const char * user, const char * password,const  char * args)
{	
	const char * ch;
	char *ptr;
	void* pass;
	char *msg;
	//checkPassword(fd, user, password);
	users.clear();
	if(checkPassword(fd, user, password)) {
		HashTableVoidIterator iterator(&Users);
		while(iterator.next(ch,pass)) {
			//printf("%s\n", ch);
			//strcpy(ptr,ch);
			//strcat(ptr, " \r\n");
			//strcat(msg, ch);
			//ptr = strdup(ch);
			users.push_back(strdup(ch));
		}
			sArray();
		for (int i = 0; i < users.size(); i++) {
			write(fd,users[i],strlen(users[i]));
			write(fd,"\r\n",strlen("\r\n"));
		}
	}
	else {
		const char * msg = "ERROR (Wrong Password)\r\n";
		write(fd,msg,strlen(msg));
		return;
	}
	write(fd,"\r\n",strlen("\r\n"));

}

void
IRCServer::createRoom(int fd, const char * user, const char * password, const char * room) {
	bool check = false;
	if(checkPassword(fd, user, password)) {
		for(int i = 0; i < rooms.size(); i++) {
			if(strcmp(rooms[i].name, room)==0)
				check = true;
		}
		if(check) {
			const char * msg = "DENIED\r\n";
			write(fd,msg,strlen(msg));
		}
		else {
			struct Room nr;
			nr.name = strdup(room);
			rooms.push_back(nr);
			const char * msg = "OK\r\n";
			write(fd, msg, strlen(msg));
		}

	}
	else {
		const char * msg = "ERROR (Wrong Password)\r\n";
		write(fd,msg,strlen(msg));
	}
}

void
IRCServer::listRooms(int fd, const char * user, const char * password, const char * args) {
	if(checkPassword(fd, user, password)) {
		for (int i = 0; i < rooms.size(); i++) {
			write(fd, rooms[i].name, strlen(rooms[i].name));
			write(fd, "\r\n", strlen("\r\n"));
		}
	}
	else {
		const char * msg = "ERROR (Wrong Password)\r\n";
		write(fd,msg,strlen(msg));
	}


}
void sArray() {
char * temp = (char*) malloc(100*sizeof(char));
	for (int i = 0; i< users.size(); i++) {
		for (int j = 0; j < users.size() - 1; j++) {
			if(strcmp(users[j], users[j+1]) > 0) {
				strcpy(temp, users[j]);
				strcpy(users[j],users[j+1]);
				strcpy(users[j+1], temp);
			}
		}
	}
free(temp);
}
