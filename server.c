#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int	main(int argc, char *argv[])
{

	// make socket
	int	serv_sock;
	int	clnt_sock;

	struct sockaddr_in	serv_addr;
	struct sockaddr_in	clnt_addr;
	socklen_t clnt_addr_size;

	char message[] = "Hello World";

	if (argc != 2)
	{
		printf("arguments count error");
		return (1);
	}

	serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	if (serv_sock == -1)
	{
		fputs("socket() error\n", stderr);
		return (1);
	}

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(atoi(argv[1]));

	// binding socket
	if (bind(serv_sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1)
	{
		fputs("binding error\n", stderr);
		return (1);
	}

	if (listen(serv_sock, 5) == -1)
	{
		fputs("listen error\n", stderr);
		return (1);
	}

	clnt_addr_size = sizeof(clnt_addr);
	clnt_sock = accept(serv_sock, (struct sockaddr *) &clnt_addr, &clnt_addr_size);
	if (clnt_sock == -1)
		fputs("accept error\n", stderr);
	
	write(clnt_sock, message, sizeof(message));
	close(serv_sock);
	close(clnt_sock);
}