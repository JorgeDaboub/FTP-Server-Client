
/*FTP Client*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

/*for getting file size using stat()*/
#include <sys/stat.h>

/*for sendfile()*/
#include <sys/sendfile.h>

/*for O_RDONLY*/
#include <fcntl.h>

int main(int argc, char *argv[])
{
	struct sockaddr_in sin;
	struct stat obj;
	struct hostent *hp;
	char *host;
	int SERVER_PORT;
	int sock;
	int choice;
	char buf[BUFSIZ], command[5], filename[20], *f;
	int k, size, status;
	int len;
	int filehandle;
	int file_size;
	FILE *received_file;
	int remain_data = 0;

	// Parse Command Line Arguments
	if (argc == 3)
	{
		host = argv[1];
		SERVER_PORT = atoi(argv[2]);
	}
	else
	{
		fprintf(stderr, "usage: client host port\n");
		exit(1);
	}

	/* translate host name into peer's IP address */
	hp = gethostbyname(host);
	if (!hp)
	{
		fprintf(stderr, "client: unknown host: %s\n", host);
		exit(1);
	}

	/* build address data structure */
	bzero((char *)&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
	sin.sin_port = htons(SERVER_PORT);

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1)
	{
		printf("socket creation failed");
		exit(1);
	}

	k = connect(sock, (struct sockaddr *)&sin, sizeof(sin));
	if (k == -1)
	{
		printf("Connect Error");
		exit(1);
	}
	int i = 1;

	while (1)
	{
		printf("Enter a choice:\n1- get\n2- put\n3- pwd\n4- ls\n5- cd\n6- quit\n");
		scanf("%d", &choice);
		switch (choice)
		{
		case 1:
			printf("Enter filename to get: ");
			scanf("%s", filename);
			strcpy(buf, "get ");
			strcat(buf, filename);
			printf("Sending TO SERVER %s\n", buf);
			send(sock, buf, 100, 0);

			bzero(buf, sizeof(buf));
			len = recv(sock, buf, BUFSIZ, 0);
			buf[len] = '\0';
			file_size = 19;
			printf("Received Size %d\n", file_size);

			received_file = fopen(filename, "w");
			if (received_file == NULL)
			{
				printf("Failed to open file foo --> \n");

				exit(EXIT_FAILURE);
			}

			remain_data = file_size;
			while ((remain_data > 0) && ((len = recv(sock, buf, BUFSIZ, 0)) > 0))
			{
				printf("%s", buf);
				fwrite(buf, sizeof(char), len, received_file);
				remain_data -= len;
				printf("Receive %d bytes and we hope :- %d bytes\n", len, remain_data);
			};
			close(received_file);
			strcpy(buf, "cat ");
			strcat(buf, filename);
			system(buf);
			break;
			
		case 2:
			printf("Enter filename to put to server: ");
			scanf("%s", filename);
			filehandle = open(filename, O_RDONLY);
			if (filehandle == -1)
			{
				printf("No such file on the local directory\n\n");
				break;
			}
			strcpy(buf, "put ");
			strcat(buf, filename);
			send(sock, buf, 100, 0);
			stat(filename, &obj);
			size = obj.st_size;
			send(sock, &size, sizeof(int), 0);
			sendfile(sock, filehandle, NULL, size);
			recv(sock, &status, sizeof(int), 0);
			if (status)
				printf("File stored successfully\n");
			else
				printf("File failed to be stored to remote machine\n");
			break;
		case 3:
			strcpy(buf, "pwd");
			send(sock, buf, 100, 0);
			recv(sock, buf, 100, 0);
			printf("The path of the remote directory is: %s\n", buf);
			break;
		case 4:
			strcpy(buf, "ls");
			send(sock, buf, 100, 0);
			recv(sock, &size, sizeof(int), 0);
			f = malloc(size);
			recv(sock, f, size, 0);
			filehandle = creat("temp.txt", O_WRONLY);
			write(filehandle, f, size, 0);
			close(filehandle);
			printf("The remote directory listing is as follows:\n");
			system("chmod +rw temp.txt");
			system("cat temp.txt");
			break;
		case 5:
			strcpy(buf, "cd ");
			printf("Enter the path to change the remote directory: ");
			scanf("%s", buf + 3);
			send(sock, buf, 100, 0);
			recv(sock, &status, sizeof(int), 0);
			if (status)
				printf("Remote directory successfully changed\n");
			else
				printf("Remote directory failed to change\n");
			break;
		case 6:
			strcpy(buf, "quit");
			send(sock, buf, 100, 0);
			recv(sock, &status, 100, 0);
			if (status)
			{
				printf("Server closed\nQuitting..\n");
				exit(0);
			}
			printf("Server failed to close connection\n");
		}
	}
	return 0;
}