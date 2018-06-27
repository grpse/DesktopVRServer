#include <cassert>
#include "UDPServer.h"
#include <SDL.h>
#include <SDL_net.h>

int UDPServer::WSInitializerCount = 0;
#define SYSTEM_INIT_ERROR -1

void UDPServer::Init()
{
	if (UDPServer::WSInitializerCount == 0)
	{
		if (SDL_Init(0) == SYSTEM_INIT_ERROR)
		{
			printf("Error init SDL\n");
		}

		if (SDLNet_Init() == SYSTEM_INIT_ERROR)
		{
			printf("Error init SDL Net\n");
		}
	}

	UDPServer::WSInitializerCount += 1;
}

void UDPServer::Finalize()
{
	UDPServer::WSInitializerCount -= 1;

	if (UDPServer::WSInitializerCount == 0)
	{
		SDLNet_Quit();
		SDL_Quit();
	}
}

UDPServer::UDPServer(int localport)
{
	UDPServer::Init();
	mSocket = SDLNet_UDP_Open(localport);
	mLocalPort = localport;
	mReceiveThread = std::thread(&UDPServer::receivingThread, this);
}


UDPServer::~UDPServer()
{
	SDLNet_UDP_Close(mSocket);
	UDPServer::Finalize();
}


bool UDPServer::broadcast(byte * buffer, size_t size)
{
	return false;
}

bool UDPServer::bindbroadcast(const char* ip, int port, int localport)
{
	enableBroadCast();
	mBroadcastPeer.sin_family = AF_INET;
	mBroadcastPeer.sin_port = htons(broadcastport);
	mBroadcastPeer.sin_addr.s_addr = inet_addr(broadcastip);

	//Prepare the sockaddr_in structure
	mServerAddr.sin_family = AF_INET;
	mServerAddr.sin_addr.s_addr = INADDR_ANY;
	mServerAddr.sin_port = htons(localport);

	//Bind
	struct sockaddr* serverAddrCasted = reinterpret_cast<struct sockaddr*>(&mServerAddr);
	if (bind(mSocket, serverAddrCasted, sizeof(mServerAddr)) == SOCKET_ERROR)
	{
		printf("Bind failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	puts("Bind done");

	return true;
}

void UDPServer::onReceive(onReceiveFromClientCallback callback)
{
	std::unique_lock<std::mutex> guard(mReceiveMutex);
	mOnReceiveCallbacks.push_back(callback);
	guard.unlock();
}

void UDPServer::receivingThread()
{
	//try to receive some data, this is a blocking call
	long receivedLength = 0;
	std::vector<byte> bufferReceivedData;
	const size_t bufferLength = 1024;
	char bufferSlice[bufferLength];
	struct sockaddr_in peer;
	int peerStructureSize = sizeof(peer);
	bufferReceivedData.resize(bufferLength);
	int currentBufferDataSize = bufferLength;

	while (true)
	{
		int receivedTotalLength = 0;
		int receivedCurrentIndex = 0;
		while ((receivedLength = recvfrom(mSocket, bufferSlice, bufferLength, 0, (struct sockaddr *) &peer, &peerStructureSize)) > 0)
		{
			// update buffer needed size
			receivedTotalLength += receivedLength;

			// verify if need to update buffer length
			if (receivedTotalLength > currentBufferDataSize)
			{
				bufferReceivedData.resize(receivedTotalLength);
				printf("Buffer data resized to %d byte\n", receivedTotalLength);
			}

			// copy buffer slice to buffer data
			memcpy(&bufferReceivedData[receivedCurrentIndex], bufferSlice, receivedLength);

			// update next index of buffer data to apply buffer slice
			receivedCurrentIndex += receivedLength;
		}

		// if some error occured don't receive the message
		if (receivedLength == SOCKET_ERROR)
		{
			printf("Socket error on receiving\n");
		}
		else
		{
			// add to peers list
			mPeersAddr.push_back(peer);

			// all callbacks receive data received
			for (auto receiveCallback : mOnReceiveCallbacks)
			{
				receiveCallback(peer, bufferReceivedData.data(), bufferReceivedData.size());
			}
		}
	}
}