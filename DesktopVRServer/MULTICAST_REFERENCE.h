#pragma once

#include <windows.h>
#include <iostream>
#include <winsock.h>
#include <windows.h>
#include <time.h>
#include <stdlib.h>
#include "ScreenCapture.h"
#include "UDPServer.h"
#define EXAMPLE_PORT 6000
#define EXAMPLE_GROUP "239.0.0.1"

void MULTICAST_REFERENCE(int argc, char** argv)
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	struct sockaddr_in addr;
	int addrlen, sock, cnt;
	struct ip_mreq mreq;
	char message[50];

	/* set up socket */
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		perror("socket");
		exit(1);
	}
	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(EXAMPLE_PORT);
	addrlen = sizeof(addr);

	if (argc == 1) {
		/* send */
		addr.sin_addr.s_addr = inet_addr(EXAMPLE_GROUP);
		while (1) {
			time_t t = time(0);
			sprintf_s(message, "time is %-24.24s", ctime(&t));
			printf("sending: %s\n", message);
			cnt = sendto(sock, (const char*)message, sizeof(message), 0, (struct sockaddr *) &addr, addrlen);
			printf("BYTES SENT: %d\n", cnt);
			if (cnt < 0) {
				perror("sendto");
				exit(1);
			}


			Sleep(1000);
		}
	}
	else {

		/* receive */
		if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
			perror("bind");
			exit(1);
		}
		mreq.imr_multiaddr.s_addr = inet_addr(EXAMPLE_GROUP);
		mreq.imr_interface.s_addr = htonl(INADDR_ANY);
		int optionSetReturn = setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char*)&mreq, sizeof(mreq));

		if (optionSetReturn != 0) {
			perror("setsockopt mreq");
			printf("LAST ERROR RETURN: %d\n", optionSetReturn);
			printf("LAST ERROR: %d\n", WSAGetLastError());
			exit(1);
		}
		while (1) {
			cnt = recvfrom(sock, message, sizeof(message), 0,
				(struct sockaddr *) &addr, &addrlen);
			if (cnt < 0) {
				perror("recvfrom");
				exit(1);
			}
			else if (cnt == 0) {
				break;
			}
			printf("%s: message = \"%s\"\n", inet_ntoa(addr.sin_addr), message);
		}
	}

	closesocket(sock);

	WSACleanup();
}