// Kanchi Mohan Krishna - 20CS10030
// Gangaram Arvind Sudewad - 20CS30017

// Compile by giving port number as command line argument 

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "time.h"
#include <sys/stat.h>
#define D_PORT 80

char *gmttime(){
    time_t rawtime;
    struct tm *info;
    time(&rawtime);
    info = gmtime(&rawtime);
    char *gmt = (char *)calloc(100, sizeof(char));
    strftime(gmt, 100, "%a, %d %b %Y %H:%M:%S %Z", info);
    return gmt;
}

char *expiry(){
    time_t rawtime;
    struct tm *info;
    time(&rawtime);
    putenv("TZ=gmt");
    info = gmtime(&rawtime);
    info->tm_mday += 3;
    mktime(info);
    char *gmt = (char *)calloc(100, sizeof(char));
    strftime(gmt, 100, "%a, %d %b %Y %H:%M:%S %Z", info);
    return gmt;
}

char *last_changed(char *filename){
	struct stat sr;
	if (stat(filename, &sr) == -1){
		printf("ERROR : Can't get file information for %s\n", filename);
		return NULL;
	}
	struct tm *tm = gmtime(&sr.st_mtime);
	char *d = (char *)calloc(35, sizeof(char));
	strftime(d, 35, "%a, %d %b %Y %H:%M:%S GMT", tm);
	return d;
}

void changedate(int *dd, int *mm, int *yy, char *GMT)
{
	char day[3], month[4], year[5];
	for (int i = 4; i < 7; i++){
		day[i - 4] = GMT[i];
	}
	day[3] = '\0';
	for (int i = 8; i < 11; i++){
		month[i - 8] = GMT[i];
	}
	month[3] = '\0';
	for (int i = 12; i < 16; i++){
		year[i - 12] = GMT[i];
	}
	year[4] = '\0';
	*dd = atoi(day);
	if (strcmp(month, "Jan") == 0)
		*mm = 1;
	if (strcmp(month, "Feb") == 0)
		*mm = 2;
	if (strcmp(month, "Mar") == 0)
		*mm = 3;
	if (strcmp(month, "Apr") == 0)
		*mm = 4;
	if (strcmp(month, "May") == 0)
		*mm = 5;
	if (strcmp(month, "Jun") == 0)
		*mm = 6;
	if (strcmp(month, "Jul") == 0)
		*mm = 7;
	if (strcmp(month, "Aug") == 0)
		*mm = 8;
	if (strcmp(month, "Sep") == 0)
		*mm = 9;
	if (strcmp(month, "Oct") == 0)
		*mm = 10;
	if (strcmp(month, "Nov") == 0)
		*mm = 11;
	if (strcmp(month, "Dec") == 0)
		*mm = 12;
	*yy = atoi(year);
}

int calcdays(int dd1, int mm1, int yy1, int dd2, int mm2, int yy2)
{
	int days1 = 0, days2 = 0, days = 0;
	int monthdays[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	int i;
	for (i = 0; i < mm1 - 1; i++){
		days1 += monthdays[i];
	}
	days1 += dd1;
	for (i = 0; i < mm2 - 1; i++){
		days2 += monthdays[i];
	}
	days2 += dd2;
	if (yy1 % 4 == 0){
		days1 += 1;
	}
	if (yy2 % 4 == 0){
		days2 += 1;
	}
	days = days2 - days1;
	return days;
}

int d_diff(char *d1, char *d2){
	int d, m, y;
	int dEx, mEx, yEx;
	changedate(&d, &m, &y, d1);
	changedate(&dEx, &mEx, &yEx, d2);
	int days = calcdays(d, m, y, dEx, mEx, yEx);
	days = abs(days);
	return days;
}

void recv_mul(int sockfd, char *buf){
	int i = 0;
	char temp[21];									 
	char *dt_buffer = (char *)calloc(20, sizeof(char)); 
	int r = recv(sockfd, dt_buffer, 20, 0);
	while (dt_buffer[r - 1]) // if the end of expression is not reached
	{
		for (int i = 0; i < 21; i++)
			temp[i] = '\0';
		r += recv(sockfd, temp, 20, 0);
		i += 20;
		dt_buffer = (char *)realloc(dt_buffer, (i + 21) * sizeof(char));
		strcat(dt_buffer, temp);
	}
	strcpy(buf, dt_buffer);
	free(dt_buffer);
}

void getnext(char *buf, char *key, char *value){
    char *temp = strstr(buf, key);
    temp += strlen(key);
    int i = 0;
    while (temp[i] != '\n'){
        value[i] = temp[i];
        i++;
    }
    value[i] = '\0';
}

char *put_res(char *buf, char *file_name){
	char *type = (char *)malloc(20 * sizeof(char));
    getnext(buf, "Content-Type: ", type);

	char *file_name1 = (char *)calloc(50, sizeof(char));
	strcpy(file_name1, file_name);
	if (strcmp(type, "text/html\r") == 0){
		strcat(file_name1, ".html");
	}
	else if (strcmp(type, "image/jpeg\r") == 0){
		strcat(file_name1, ".jpeg");
	}
	else if (strcmp(type, "application/pdf\r") == 0){
		strcat(file_name1, ".pdf");
	}
	else{
		strcat(file_name1, ".txt");
	}
	free(type);

	int c_len = strlen(buf);
	// int c_len = strlen(content);
	char *data = (char *)calloc(1000, sizeof(char));
	strcat(data, "HTTP/1.1 200 OK\r\n");
	strcat(data, "Content-Type: ");
	strcat(data, file_name1);
	strcat(data, "\r\n");
	strcat(data, "Content-Language: en-us\r\n");
	strcat(data, "Content-Length: ");
	char *c_len1 = (char *)malloc(10 * sizeof(char));
	sprintf(c_len1, "%d", c_len);
	strcat(data, c_len1);
	strcat(data, "\r\n");
	strcat(data, "Connection: close\r\n");
	return data;
}

char *gen_response(long int len, char *file_name, char *since_modified){
	char *GMT = gmttime();
	char *data = (char *)calloc(1000, sizeof(char));
	strcat(data, "HTTP/1.1 200 OK\r\n");
	strcat(data, "Date: ");
	strcat(data, GMT);
	strcat(data, "\r\n");
	strcat(data, "Expires: ");
	strcat(data, expiry());
	strcat(data, "\r\n");
	strcat(data, "Content-Language: en-us\r\n");
	strcat(data, "Content-Length: ");
	char *content_length = (char *)calloc(10, sizeof(char));
	sprintf(content_length, "%ld", len);
	strcat(data, content_length);
	strcat(data, "\r\n");
	free(content_length);

	strcat(data, "Content-Type: ");
	char *extension = (char *)calloc(10, sizeof(char));
	sscanf(file_name, "%*[^.].%s", extension);
	if (strcmp(extension, "html") == 0){
		strcat(data, "text/html");
	}
	else if (strcmp(extension, "pdf") == 0){
		strcat(data, "application/pdf");
	}
	else if (strcmp(extension, "jpg") == 0){
		strcat(data, "image/jpeg");
	}
	else{
		strcat(data, "text/*");
	}
	free(extension);
	strcat(data, "\r\n");
	strcat(data, "Last-Modified: ");
	strcat(data, last_changed(file_name));
	strcat(data, "\r\n");
	strcat(data, "\r\n");
	if (d_diff(since_modified, last_changed(file_name)) >= 0){
		return data;
	}
	else{
		return "HTTP/1.1 400 Bad Request\r\n";
	}
}

int main(int argc, char const *argv[])
{
	int PORT;
	if (argc != 2)
	{
		PORT = D_PORT;
	}
	else{
		PORT = atoi(argv[1]);
	}

	int sockfd, newsockfd;
	struct sockaddr_in serv_addr, cli_addr;
	socklen_t clilen;

	memset((char *)&serv_addr, '\0', sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(PORT);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd < 0){
		perror("Error opening socket");
		exit(1);
	}
	if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
		perror("Error on binding");
		exit(1);
	}
	listen(sockfd, 5);

	while (1)
	{
		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
		if (newsockfd < 0)
		{
			perror("Error on accept");
			exit(1);
		}

		if (fork() == 0)
		{
			close(sockfd);
			char buffer[1024];
			memset(buffer, '\0', 1024);
			recv_mul(newsockfd, buffer);
			printf("Request: %s\n", buffer);

			if (strncmp(buffer, "GET", 3) == 0)
			{
				char *since_modified = NULL;
				char *t = strtok(buffer, "\r\n");
				while (t != NULL)
				{
					if (strstr(t, "If-Modified-Since: ") != NULL)
					{
						since_modified = strstr(t, ": ") + 2;
						break;
					}
					t = strtok(NULL, "\r\n");
				}

				char *file_name = (char *)calloc(100, sizeof(char));
				sscanf(buffer, "GET %s", file_name);
				file_name = file_name + 1;
				FILE *fp = fopen(file_name, "rb");
				if (fp != NULL)
				{
					printf("File opened successfully\n");
					char ch;
					long int i = 0;
					while ((ch = fgetc(fp)) != EOF)
					{
						i = i + 2;
					}
		
					char *response = gen_response(i, file_name, since_modified);
					printf("%s\n", response);
					send(newsockfd, response, strlen(response) + 1, 0);
					char buf[20];
					memset(buf, '\0', 20);
					recv(newsockfd, buf, 17, 0);
					printf("Response: %s\n\n", buf);
					memset(buf, '\0', 1024);

					char *extension = (char *)calloc(10, sizeof(char));
					sscanf(file_name, "%*[^.].%s", extension);

					// if the file is a jpg or pdf
					if (strcmp(extension, "jpg") == 0 || strcmp(extension, "pdf") == 0)
					{
						int *d_buff = (int *)calloc(20, sizeof(int));
						FILE *fp1 = fopen(file_name, "rb");
						
						int n;
						while ((n = fread(d_buff, sizeof(int), 20, fp1)) > 0)
						{
							send(newsockfd, d_buff, n * sizeof(int), 0);
						}
						fclose(fp1);
					}
					else
					{
						char *d_buff = (char *)calloc(20, sizeof(char));
						FILE *fp1 = fopen(file_name, "r");
						int n;
						while ((n = fread(d_buff, sizeof(char), 20, fp1)) > 0)
						{
							send(newsockfd, d_buff, n * sizeof(char), 0);
						}

						fclose(fp1);
					}

					// (AccessLog.txt) which records every client accesses. 
					int Cli_port = ntohs(cli_addr.sin_port);
					char *URL = (char *)calloc(100, sizeof(char));
					sscanf(buffer, "GET %s", URL);
					char *Cli_IP = (char *)calloc(100, sizeof(char));
					Cli_IP = inet_ntoa(cli_addr.sin_addr);
					char *req = "GET";
					time_t t = time(NULL);
					struct tm tm = *localtime(&t);
					char *date = (char *)calloc(100, sizeof(char));
					sprintf(date, "%d/%d/%d", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900);
					char *time = (char *)calloc(100, sizeof(char));
					sprintf(time, "%d:%d:%d", tm.tm_hour, tm.tm_min, tm.tm_sec);

					FILE *fp1 = fopen("AccessLog.txt", "a");
					fprintf(fp1, "<%s>:<%s>:<%s>:<%d>:<%s>:</%s>\n", date, time, Cli_IP, Cli_port, req, file_name);
					fclose(fp1);
					close(newsockfd);
				}
				// if the file does not exist
				else
				{
					char *GMT = gmttime();
					char *response = (char *)calloc(1000, sizeof(char));
					sprintf(response, "HTTP/1.1 404 Not Found\r\n Date: %s\r\n Content-Length: 0\r\n Content-Type: text/html\r\n\r\n", GMT);
					printf("%s\n",response);
					send(newsockfd, response, strlen(response) + 1, 0);
					free(response);
					free(GMT);
				}
			}

			if (strncmp(buffer, "PUT", 3) == 0)
			{
				char *file_name = (char *)calloc(100, sizeof(char));
				sscanf(buffer, "PUT %s", file_name);
				file_name = file_name + 1;

				// remove extension from file name
				char *no_ext = (char *)calloc(100, sizeof(char));
				sscanf(file_name, "%[^.]", no_ext);

				char *response = put_res(buffer, no_ext);
				printf("Response is %s\n", response);

				send(newsockfd, "RECEIVED", 10, 0);
				
				char *extension = (char *)calloc(10, sizeof(char));
				sscanf(file_name, "%*[^.].%s", extension);
				// if the extension is jpg or pdf

				if (strcmp(extension, "jpg") == 0 || strcmp(extension, "pdf") == 0){
					FILE *fp1 = fopen(file_name, "wb");
					int n;
					int d_buff[20];
					memset(d_buff, 0, sizeof(d_buff));
					while ((n = recv(newsockfd, d_buff, 20, 0)) > 0)
					{
						fwrite(d_buff, 1, n, fp1);
						// printf("\nReceived string: %d\n", n);
						memset(d_buff, 0, sizeof(d_buff));
					}
					fclose(fp1);
				}
				else{
					FILE *fp1 = fopen(file_name, "w");
					int n;
					char d_buff[20];
					memset(d_buff, '\0', sizeof(d_buff));
					while ((n = recv(newsockfd, d_buff, 20, 0)) > 0)
					{
						fwrite(d_buff, 1, n, fp1);
						// printf("\nReceived string: %d\n", n);
						memset(d_buff, '\0', sizeof(d_buff));
					}
					fclose(fp1);
				}
				
				int Cli_port = ntohs(cli_addr.sin_port);
				char *URL = (char *)calloc(100, sizeof(char));
				sscanf(buffer, "PUT %s", URL);
				char *Cli_IP = (char *)calloc(100, sizeof(char));
				Cli_IP = inet_ntoa(cli_addr.sin_addr);
				char *req = "PUT";
				time_t t = time(NULL);
				struct tm tm = *localtime(&t);
				char *date = (char *)calloc(100, sizeof(char));
				sprintf(date, "%d/%d/%d", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900);
				char *time = (char *)calloc(100, sizeof(char));
				sprintf(time, "%d:%d:%d", tm.tm_hour, tm.tm_min, tm.tm_sec);

				// (AccessLog.txt) which records every client accesses.
				FILE *fp = fopen("AccessLog.txt", "a");
				fprintf(fp, "<%s>:<%s>:<%s>:<%d>:<%s>:<%s>\n", date, time, Cli_IP, Cli_port, req, URL);
				fclose(fp);
				free(response);
				close(newsockfd);
			}
				close(newsockfd);
				exit(0);
		}
		close(newsockfd);
	}
}