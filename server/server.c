
/*FTP server*/
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/*for getting file size using stat()*/
#include <sys/stat.h>

/*for sendfile()*/
#include <sys/sendfile.h>

/*for O_RDONLY*/
#include <fcntl.h>

#define MAX_PENDING 5
#define MAX_LINE 4096

int main(int argc, char *argv[])
{
	struct sockaddr_in sin, client_addr;
	char buf[BUFSIZ];
	int addr_len;
	int s, new_s, line_c;
	FILE *fd;
	int SERVER_PORT;
	const void *opt;
	struct stat obj;
	off_t offset;
	int remain_data;
	char command[5], filename[20];
	int k, i, size, len, c;
	int filehandle;
	int sent_bytes = 0;

	// Parse Command Line Arguments
	if (argc == 2)
	{
		SERVER_PORT = atoi(argv[1]);
	}
	else
	{
		fprintf(stderr, "usage: server port\n");
		exit(1);
	}

	// Build Struct
	bzero((char *)&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(SERVER_PORT);
	sin.sin_addr.s_addr = INADDR_ANY;

	// Create the Socket
	s = socket(PF_INET, SOCK_STREAM, 0);
	if (s < 0)
	{
		printf("Socket creation failed");
		exit(1);
	}

	// Set up the socket
	if ((setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(int))) < 0)
	{
		printf("Error Setting Socket");
		exit(1);
	}

	// Bind socket
	if ((bind(s, (struct sockaddr *)&sin, sizeof(sin))) < 0)
	{
		printf("Binding error");
		exit(1);
	}

	// Set Listening
	if ((listen(s, MAX_PENDING)) < 0)
	{
		printf("Listen failed");
		exit(1);
	}

	printf("Waiting for connections on port %d\n", SERVER_PORT);

	addr_len = sizeof(client_addr);
	i = 1;
	while (1)
	{
		new_s = accept(s, (struct sockaddr *)&client_addr, &addr_len);

		recv(new_s, buf, 100, 0);
		sscanf(buf, "%s", command);
		if (!strcmp(command, "ls"))
		{
			system("ls > tempServer.txt");
			i = 0;
			stat("tempServer.txt", &obj);
			size = obj.st_size;
			send(new_s, &size, sizeof(int), 0);
			filehandle = open("tempServer.txt", O_RDONLY);
			sendfile(new_s, filehandle, NULL, size);
		}
		else if (!strcmp(command, "get"))
		{
			sscanf(buf, "%s%s", filename, filename);
			stat(filename, &obj);
			filehandle = open(filename, O_RDONLY);
			size = obj.st_size;
			if (filehandle == -1)
				size = 0;

			printf("Sending size %d\n", size);
			if (send(new_s, &size, sizeof(int), 0) < 0)
			{
				printf("Error on sending greetings");

				exit(EXIT_FAILURE);
			}
			if (size)
			{
				offset = 0;
				sent_bytes = 0;
				remain_data = obj.st_size;
				/* Sending file data */
				while (((sent_bytes = sendfile(new_s, filehandle, &offset, BUFSIZ)) > 0) && (remain_data > 0))
				{
					fprintf(stdout, "1. Server sent %d bytes from file's data, offset is now : %d and remaining data = %d\n", sent_bytes, offset, remain_data);
					remain_data -= sent_bytes;
					fprintf(stdout, "2. Server sent %d bytes from file's data, offset is now : %d and remaining data = %d\n", sent_bytes, offset, remain_data);
				}
			}
		}
		else if (!strcmp(command, "put"))
		{
			int c = 0, len;
			char *f;
			sscanf(buf + strlen(command), "%s", filename);
			recv(new_s, &size, sizeof(int), 0);
			i = 1;
			while (1)
			{
				filehandle = open(filename, O_CREAT | O_EXCL | O_WRONLY, 0666);
				if (filehandle == -1)
				{
					sprintf(filename + strlen(filename), "%d", i);
				}
				else
					break;
			}
			f = malloc(size);
			recv(new_s, f, size, 0);
			c = write(filehandle, f, size);
			close(filehandle);
			send(new_s, &c, sizeof(int), 0);
		}
		else if (!strcmp(command, "pwd"))
		{
			system("pwd> tempServer.txt");
			i = 0;
			FILE *f = fopen("tempServer.txt", "r");
			while (!feof(f))
				buf[i++] = fgetc(f);
			buf[i - 1] = '\0';
			fclose(f);
			send(new_s, buf, 100, 0);
		}
		else if (!strcmp(command, "cd"))
		{
			if (chdir(buf + 3) == 0)
				c = 1;
			else
				c = 0;
			send(new_s, &c, sizeof(int), 0);
		}

		else if (!strcmp(command, "bye") || !strcmp(command, "quit"))
		{
			printf("FTP server quitting..\n");
			i = 1;
			send(new_s, &i, sizeof(int), 0);
			exit(0);
		}
	}
	return 0;
}