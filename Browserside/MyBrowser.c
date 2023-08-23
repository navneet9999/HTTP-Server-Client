/*
	ASSIGNMENT 4
	GANGARAM ARVIND SUDEWAD
	20CS30017
	KANCHI MOHAN KRISHNA
	20CS10030
*/

// GET http://127.0.0.1/abc.txt:20000
// PUT http://127.0.0.1/:20000 abc.txt
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <poll.h>
#include "time.h"
#include <fcntl.h>

#define DEFAULT_PORT 80
#define TIMEOUT 3000


//function to get the file size
int calc_fsize(char *fname)
{
	
	FILE *f = fopen(fname, "rb");
	if (f == NULL)
	{
		printf("Error opening file");
		exit(1);
	}
	fseek(f, 0L, SEEK_END);
	int fsize = ftell(f);	
	fclose(f);
	return fsize;
}

// function to send the file
void recv_data(int sockfd, char *temp_str)
{
	int i = 0;
	char data[21];									 
	char *store_str = (char *)calloc(20, sizeof(char)); 
	int size = recv(sockfd, store_str, 20, 0);
	while (store_str[size - 1]) 
	{
		for (int i = 0; i < 21; i++)
			data[i] = '\0';
		size += recv(sockfd, data, 20, 0); 
		i += 20;
		store_str = (char *)realloc(store_str, (i + 21) * sizeof(char));
		strcat(store_str, data); 
	}
	strcpy(temp_str, store_str);
}
// function to get time in GMT format
char *gmttime()
{
	time_t rawtime;
	struct tm *info;
	time(&rawtime);
	info = gmtime(&rawtime);
	char *gmt = (char *)calloc(100, sizeof(char));
	strftime(gmt, 100, "%a, %d %b %Y %H:%M:%S %Z", info);
	return gmt;
}

// function to get time 2 days before 
char *gmtbefore2days()
{
	time_t rawtime;
	struct tm *info;
	time(&rawtime);
	putenv("TZ=GMT");
	info = gmtime(&rawtime);
	info->tm_mday -= 2;
	mktime(info);
	char *gmt = (char *)calloc(100, sizeof(char));
	strftime(gmt, 100, "%a, %d %b %Y %H:%M:%S %Z", info);
	return gmt;
}

// function to find the ip address from the url
char *find_ip(char *input_url)
{
	char *ip1, *ip2;

	ip1 = strstr(input_url, "//");
	if (ip1)
	{
		ip1 += 2;
		ip2 = strchr(ip1, '/');
		if (!ip2)
			ip2 = strchr(ip1, ':');
		if (ip2)
			*ip2 = '\0';
		return ip1;
	}
	return NULL;
}

// function to find the port number from the url
int find_port(const char *input_url)
{
	int port_no;
	int len = sscanf(input_url, "http://%*[^:]:%d", &port_no);
	if (len == 1)
		return port_no;
	else
		return DEFAULT_PORT;
}

// FUNCTION TO CREATE THE HTTP REoui[QUEST GIVEN THE URL, COMMAND AND FILE NAME
char *get_request(const char *input_url, char *input_cmd, char *fname)
{
	char part1[100];
	char part2[100];
	sscanf(input_url, "http://%[^/]/%s", part1, part2);
	int port_no = find_port(input_url);
	char *clie_request = (char *)calloc(1000, sizeof(char));
	char *pntr = strstr(part2, ":");
	if (pntr != NULL)
	{
		*pntr = '\0';
	}

	if (strcmp(input_cmd, "GET") == 0)
	{
		strcat(clie_request, "GET /");
		strcat(clie_request, part2);
	}
	if (strcmp(input_cmd, "PUT") == 0)
	{
		strcat(clie_request, "PUT /");
		strcat(clie_request, fname);
	}

	strcat(clie_request, " HTTP/1.1\r\n");
	strcat(clie_request, "Host: ");
	strcat(clie_request, part1);
	char port[10];
	sprintf(port, "%d", port_no);
	strcat(clie_request, ":");
	strcat(clie_request, port);
	strcat(clie_request, "\r\n");
	strcat(clie_request, "Connection: close\r\n");

	char *gmt = gmttime();
	char *gmt2 = gmtbefore2days();

	strcat(clie_request, "Date: ");
	strcat(clie_request, gmt);
	strcat(clie_request, "\r\n");

	if (strcmp(input_cmd, "GET") == 0)
	{

		if (strstr(part2, ".html") != NULL)
			strcat(clie_request, "Accept: text/html\r\n");
		else if (strstr(part2, ".jpg") != NULL)
			strcat(clie_request, "Accept: image/jpeg\r\n");
		else if (strstr(part2, ".pdf") != NULL)
			strcat(clie_request, "Accept: application/pdf\r\n");
		else
			strcat(clie_request, "Accept: text/*\r\n");

		strcat(clie_request, "Accept-Language: en-Us\r\n");
		strcat(clie_request, "If-Modified-Since: ");
		strcat(clie_request, gmt2);
		strcat(clie_request, "\r\n");
	}
	if (strcmp(input_cmd, "PUT") == 0)
	{
		int fsize = calc_fsize(fname);
		strcat(clie_request, "Content-Language: en-Us\r\n");
		strcat(clie_request, "Content-Length: ");
		char length[10];
		sprintf(length, "%d", fsize);
		strcat(clie_request, length);
		strcat(clie_request, "\r\n");
		if (strstr(fname, ".html") != NULL)
			strcat(clie_request, "Content-Type: text/html\r\n");
		else if (strstr(fname, ".jpg") != NULL)
			strcat(clie_request, "Content-Type: image/jpeg\r\n");
		else if (strstr(fname, ".pdf") != NULL)
			strcat(clie_request, "Content-Type: application/pdf\r\n");
		else
			strcat(clie_request, "Content-Type: text/*\r\n");
		strcat(clie_request, "\r\n");
	}

	return clie_request;
}

int main()
{
	char *prompt = "MyOwnBrowser> ";
	char terminal_input[100]; // to store the command entered by the user

	while (1)
	{
		for (int i = 0; i < 100; i++)
			terminal_input[i] = '\0';
		printf("%s", prompt);
		scanf("%[^\n]s", terminal_input);
		scanf("%*c");
		char *url_stored = (char *)calloc(1, sizeof(char));
		char *input_cmd = strtok(terminal_input, " ");
		char *temp_cmd = (char *)calloc(strlen(input_cmd) + 1, sizeof(char));
		strcpy(temp_cmd, input_cmd);

		while ((input_cmd = strtok(NULL, " ")) != NULL) 
		{
			url_stored = (char *)realloc(url_stored, strlen(url_stored) + strlen(input_cmd) + 2);
			strcat(url_stored, input_cmd);
			strcat(url_stored, " ");
		}
		url_stored[strlen(url_stored) - 1] = '\0';
		char *fname;
		fname = strtok(url_stored, " ");
		fname = strtok(NULL, " ");
		char *temp_url = (char *)calloc(strlen(url_stored), sizeof(char));
		strcpy(temp_url, url_stored);
		char *ip_addr = find_ip(temp_url);
		int port_no = find_port(url_stored);

		if (strcmp(temp_cmd, "QUIT") == 0)
			break;

		else if (strcmp(temp_cmd, "GET") == 0)
		{
			int sockfd;	
			char temp_str[1000]; 
			struct sockaddr_in server_addr;
			sockfd = socket(AF_INET, SOCK_STREAM, 0);
			if (sockfd < 0)
			{
				perror("Socket not created\n");
				exit(0);
			}
			bzero(&server_addr, sizeof(server_addr)); 
			server_addr.sin_family = AF_INET;		
			server_addr.sin_port = htons(port_no);		 
			inet_aton(ip_addr, &server_addr.sin_addr);
			if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
			{ 
				perror("Connection failed\n");
				exit(0);
			}

			char *clie_request = get_request(url_stored, temp_cmd, fname);
			printf("\n%s\n", clie_request);
			send(sockfd, clie_request, strlen(clie_request) + 1, 0);
			// for using poll()
			struct pollfd fdset[1];
			int ret;
			fdset[0].fd = sockfd;
			fdset[0].events = POLLIN;
			ret = poll(fdset, 1, TIMEOUT);
			if (ret == 0)
			{
				printf("Time exceeded!! Timeout\n");
				continue;
			}
			else if (ret < 0)
			{
				perror("Error int polling \n");
				exit(1);
			}
			else
			{
				memset(temp_str, 0, sizeof(temp_str));
				recv_data(sockfd, temp_str);
				printf("%s\n", temp_str);
				send(sockfd, "Response Received", 17, 0);
				char *fname = (char *)calloc(100, sizeof(char));
				sscanf(clie_request, "GET %s", fname);
				fname = fname + 1;
				char *f_extn = (char *)calloc(10, sizeof(char));
				sscanf(fname, "%*[^.].%s", f_extn);
				if (strcmp(f_extn, "pdf") == 0 || strcmp(f_extn, "jpg") == 0)
				{
					FILE *fp = fopen(fname, "wb");
					int n;
					int data[20];
					memset(data, 0, sizeof(data));
					while ((n = recv(sockfd, data, 20, 0)) > 0)
					{
						fwrite(data, 1, n, fp);
						memset(data, 0, sizeof(data));
					}
					fclose(fp);
				}
				else
				{
					FILE *fp = fopen(fname, "w");
					int n;
					char data[20];
					memset(data, '\0', sizeof(data));
					while ((n = recv(sockfd, data, 20, 0)) > 0)
					{
						fwrite(data, 1, n, fp);
						memset(data, '\0', sizeof(data));
					}
					fclose(fp);
				}
			}

			close(sockfd);
		}
		else if (strcmp(temp_cmd, "PUT") == 0)
		{
			int sockfd;		
			char temp_str[1000]; 
			struct sockaddr_in server_addr;
			sockfd = socket(AF_INET, SOCK_STREAM, 0); 
			if (sockfd < 0)
			{
				perror("Socket not created\n");
				exit(0);
			}
			bzero(&server_addr, sizeof(server_addr)); 
			server_addr.sin_family = AF_INET;		 
			server_addr.sin_port = htons(port_no);		 
			inet_aton(ip_addr, &server_addr.sin_addr);
			if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
			{ 
				perror("Connection failed\n");
				exit(0);
			}
			char *clie_request = get_request(url_stored, temp_cmd, fname);

			printf("\n%s\n", clie_request);
			send(sockfd, clie_request, strlen(clie_request) + 1, 0);

			// Used for polling
			struct pollfd fdset[1];
			int ret;
			fdset[0].fd = sockfd;
			fdset[0].events = POLLIN;
			ret = poll(fdset, 1, TIMEOUT);
			if (ret == 0)
			{
				printf("Time exceeded!! Timeout\n");
				continue;
			}
			else if (ret < 0)
			{
				perror("Error in polling\n");
				exit(1);
			}
			else
			{
				char data[10];
				recv(sockfd, data, 10, 0);
				printf("%s\n", data);
				// printf("Enter the file name: %s\n", fname);
				char *f_extn = (char *)calloc(10, sizeof(char));
				sscanf(fname, "%*[^.].%s", f_extn);

				if (f_extn == "pdf" || f_extn == "jpg")
				{
					FILE *fp = fopen(fname, "rb");
					int n;
					int buffer[20];
					memset(buffer, 0, sizeof(buffer));
					while ((n = fread(buffer, sizeof(int), 20, fp)) > 0)
					{
						send(sockfd, buffer, n * sizeof(int), 0);
						memset(buffer, 0, sizeof(buffer));
					}
					fclose(fp);
				}

				else
				{
					FILE *fp = fopen(fname, "r");
					int n;
					char buffer[20];
					memset(buffer, '\0', sizeof(buffer));
					while ((n = fread(buffer, sizeof(char), 20, fp)) > 0)
					{
						send(sockfd, buffer, n * sizeof(char), 0);
						memset(buffer, '\0', sizeof(buffer));
					}
					fclose(fp);
				}

				printf("File sent\n");
			}

			// RECIEVE THE RESPONSE FROM THE SERVER
			// memset(temp_str, '\0', sizeof(temp_str));
			// recv_data(sockfd, temp_str);
			// // recv(sockfd, temp_str, 1000, 0);
			// printf("Response from server: ");
			// printf("%s\n", temp_str);
			close(sockfd);
		}
		else
		{
			printf("Command not found!! Try again\n ");
		}
	}
	return 0;
}