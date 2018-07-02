#include <cassert>
#include "UDPServer.h"

int UDPServer::WSInitializerCount = 0;
#define SYSTEM_INIT_ERROR -1
#define SEND_ERROR 0
#define MAX_PACKET_SIZE 1024
WSADATA wsaData;

void UDPServer::Init()
{
	if (UDPServer::WSInitializerCount == 0)
	{
		int windowsSocketVersion = MAKEWORD(2, 2);
		printf("Windows Socket Version: %u.%u\n", windowsSocketVersion, windowsSocketVersion >> 2);
		WSAStartup(windowsSocketVersion, &wsaData);
	}

	UDPServer::WSInitializerCount += 1;
}

void UDPServer::Finalize()
{
	UDPServer::WSInitializerCount -= 1;

	if (UDPServer::WSInitializerCount == 0)
	{
		WSACleanup();
	}
}

UDPServer::UDPServer(const char* ip, int localport)
{
	UDPServer::Init();	
	struct ip_mreq mreq; // for receiving from multicast
	char message[50];

	/* set up socket */
	mSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (mSocket < 0) {
		perror("socket");
		exit(1);
	}
	memset((void *)&mServerAddress, NULL, sizeof(struct sockaddr_in));
	mServerAddress.sin_family = AF_INET;
	mServerAddress.sin_addr.s_addr = ip ? inet_addr(ip) : htonl(INADDR_ANY);
	mServerAddress.sin_port = htons(localport);
	memset(&(mServerAddress.sin_zero), 0, 8);
	addrlen = sizeof(mServerAddress);
	
	mPort = localport;
}

UDPServer::UDPServer(int localport)
{
	UDPServer::Init();
	memset((void *)&mServerAddress, NULL, sizeof(struct sockaddr_in));
	/* set up socket */
	mSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (mSocket < 0) {
		perror("socket");
		exit(1);
	}
	mServerAddress.sin_family = AF_INET;
	mServerAddress.sin_port = htons(localport);
	mServerAddress.sin_addr.s_addr = INADDR_ANY;
	memset(&(mServerAddress.sin_zero), 0, 8);
}

UDPServer::~UDPServer()
{
	closesocket(mSocket);
	UDPServer::Finalize();
}

bool UDPServer::multicast(byte * buffer, size_t size, size_t& sent)
{
	if (size > MAX_PACKET_SIZE)
	{
		byte* bufferPtr = buffer;
		int countBytesSent = 0;
		int totalSent = 0;

		do {
			countBytesSent = sendto(mSocket, (const char*)bufferPtr, MAX_PACKET_SIZE, 0, (struct sockaddr *) &mServerAddress, sizeof(struct sockaddr_in));
			printf("Sent chunk: %d\n", countBytesSent);
			bufferPtr = (bufferPtr + countBytesSent);
			totalSent += countBytesSent;

		} while (countBytesSent > 0 && totalSent < size);

		sent = totalSent;
		return totalSent >= 0;
	}
	else {
		int countBytesSent = sendto(mSocket, (const char*)buffer, size, 0, (struct sockaddr *) &mServerAddress, sizeof(struct sockaddr_in));
		sent = countBytesSent;
		return countBytesSent >= 0;
	}

	// return (sent = sendto(mSocket, (const char*)buffer, size, 0, (struct sockaddr *) &mServerAddress, sizeof(struct sockaddr_in))) < 0;
}

void UDPServer::onReceive(onReceiveFromClientCallback callback)
{
	std::unique_lock<std::mutex> guard(mReceiveMutex);
	mOnReceiveCallbacks.push_back(callback);
	guard.unlock();
}

void UDPServer::startReceiver()
{
	mReceiveThread = std::thread(std::bind(&UDPServer::receivingThread, this));
}

void UDPServer::bindServer()
{
	int bindResponse = bind(mSocket, (struct sockaddr *)&mServerAddress, sizeof(struct sockaddr_in));	
	if (bindResponse < 0) 
	{
		perror("ERROR on binding");
		closesocket(mSocket);
	}
}

void UDPServer::broadcastMessage(byte * bytes, size_t length)
{
	size_t peer_struct_size = sizeof(struct sockaddr_in);
	struct sockaddr* peer_ptr = NULL;

	for (auto peerKeyValue : mPeers)
	{
		struct sockaddr* peer_ptr = (struct sockaddr*) &peerKeyValue.second;
		unsigned long amountOfBytes = length;

		int sentBytesOfBytesThatShouldSend = sendto(mSocket, (const char*)&amountOfBytes, sizeof(unsigned long), 0, peer_ptr, peer_struct_size);
		if (length > MAX_PACKET_SIZE)
		{
			byte* bufferPtr = bytes;
			int countBytesSent = 0;
			int totalSent = 0;
			do {
				countBytesSent = sendto(mSocket, (const char*)bufferPtr, MAX_PACKET_SIZE, 0, peer_ptr, peer_struct_size);
				bufferPtr = (bufferPtr + countBytesSent);
				totalSent += countBytesSent;
			} while (countBytesSent > 0 && totalSent < length);

			if (countBytesSent < 0) {
				removePeer(peerKeyValue.first);
			}
		}
		else {
			int countBytesSent = sendto(mSocket, (const char*)bytes, length, 0, peer_ptr, peer_struct_size);
		}
	}
}

bool UDPServer::isOpen()
{
	return true;
}

void UDPServer::receivingThread()
{
	struct sockaddr_in peerConnection;
	int sockAddrStructSize = sizeof(peerConnection);
	struct sockaddr* peerConnectionPtr = (struct sockaddr*)&peerConnection;

	const size_t bufferSize = 1024;
	byte buffer[bufferSize];

	while (isOpen())
	{
		{	/// Locked by scope guard
			std::lock_guard<std::mutex> scopeGuard(mReceiveMutex);
			int bytesReceived = recvfrom(mSocket, (char*) buffer, bufferSize, 0, peerConnectionPtr, &sockAddrStructSize);
			ULONG peerAddress = peerConnection.sin_addr.s_addr;
			u_short peerPort = peerConnection.sin_port;
			auto peerKey = std::make_pair(peerAddress, peerPort);
			auto peerIterator = mPeers.find(peerKey);
			if (bytesReceived > 0 && peerIterator == mPeers.end())
			{
				printf("Received: %s\n", buffer);
				printf("Adding Peer\n");
				// received some data
				// TODO: receive data from socket to control mouse pointer
				// store peer address
				mPeers[peerKey] = peerConnection;
			}
		}
	}
}

void UDPServer::removePeer(std::pair<ULONG, u_short> peerKey)
{
	auto peerIter = mPeers.find(peerKey);
	if (peerIter != mPeers.end())
		mPeers.erase(peerIter);
}
