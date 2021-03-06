#include <windows.h>
#include <winsock.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>

#define BUF_SIZE 1024

void SavePPM(std::string& filepath, unsigned char* bytes, size_t bytesLength);

int main(int argc, char *argv[])
{
	if (argc < 2) {
		printf("usage: %s host port\n", argv[0]);
		return -1;
	}

	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	char* host = argv[1];
	int port = atoi(argv[2]);


	int sockfd, numbytes;
	struct sockaddr_in serv_addr;
	struct hostent *server;

	unsigned char buffer[BUF_SIZE] = "DATA!";

	server = gethostbyname(host);

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
	memset(&(serv_addr.sin_zero), 0, 8);

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("ERROR opening socket");
		closesocket(sockfd);
		return errno;
	}

	//
	int serv_addr_len = sizeof(struct sockaddr_in);
	struct sockaddr* serv_addr_ptr = (struct sockaddr*) &serv_addr;

	bool canReceive = sendto(sockfd, (const char*)buffer, BUF_SIZE, 0, serv_addr_ptr, serv_addr_len) > 0;
	unsigned long shouldReceive = 0;

	int frameCount = 0;
	std::vector<unsigned char> screenData;

	while (canReceive) {

		// clear buffer and send byte array
		numbytes = recvfrom(sockfd, (char*)&shouldReceive, sizeof(unsigned long), 0, serv_addr_ptr, &serv_addr_len);
		// printf("SHOULD RECEIVE: %lld\n", shouldReceive);
		if (numbytes > 0)
		{
			std::cout << "??? " << numbytes << std::endl;
			std::cout << "SHOULD RECEIVE: " << shouldReceive << std::endl;

			if (screenData.max_size())
			{
				screenData.resize(shouldReceive);
			}

			unsigned long totalReceived = 0;
			do
			{
				numbytes = recvfrom(sockfd, (char*)(screenData.data() + totalReceived), BUF_SIZE, 0, serv_addr_ptr, &serv_addr_len);
				totalReceived += numbytes;

			} while (totalReceived < shouldReceive);
			printf("RECEIVED: %d\n", totalReceived);

			std::string filepath = std::string("frame_") + std::to_string(frameCount++) + ".ppm";

			// SavePPM(filepath, screenData.data(), shouldReceive);
		}


		// HERE WE CAN SEND SOME THING
	}

	closesocket(sockfd);
	WSACleanup();

	return 0;
}

void SavePPM(std::string& filepath, unsigned char* bytes, size_t bytesLength)
{
	std::ofstream file(filepath);

	file << "P6" << std::endl;
	file << "# Comment" << std::endl;
	file << "1366 768" << std::endl;
	file << "255" << std::endl;

	for (size_t i = 0; i < bytesLength; i++)
	{
		file << bytes[i] << std::endl;
	}

	file.close();
}