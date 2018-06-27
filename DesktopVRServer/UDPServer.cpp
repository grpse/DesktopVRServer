#include <cassert>
#include "UDPServer.h"
#include <SDL.h>
#include <SDL_net.h>

int UDPServer::WSInitializerCount = 0;
#define SYSTEM_INIT_ERROR -1
#define SEND_ERROR 0

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
	localInit(localport);
}

UDPServer::UDPServer()
{
	localInit(0);
}


UDPServer::~UDPServer()
{
	SDLNet_UDP_Close(mSocket);
	UDPServer::Finalize();
}


bool UDPServer::bind(int remotePort)
{	
	mRemotePort = remotePort;
	SDLNet_ResolveHost(&mIPaddress, NULL, mRemotePort);
	// Bind address to the first free channel
	mChannel = SDLNet_UDP_Bind(mSocket, -1, &mIPaddress);
	if (mChannel == -1) {
		printf("SDLNet_UDP_Bind: %s\n", SDLNet_GetError());
		// do something because we failed to bind
		return false;
	}
	return true;
}

bool UDPServer::broadcast(byte * buffer, size_t size)
{
	setupPacketSize(size);
	// send a packet using a UDPsocket, using the packet's channel as the channel
	if (mPeers.size() > 0)
	{
		int numsent = SDLNet_UDP_Send(mSocket, mChannel, mPacket);
		if (numsent == SEND_ERROR) {
			printf("SDLNet_UDP_Send: %s\n", SDLNet_GetError());
			// do something because we failed to send
			// this may just be because no addresses are bound to the channel...
			return false;
		}

		return true;
	}
	
	return true;
}



void UDPServer::onReceive(onReceiveFromClientCallback callback)
{
	std::unique_lock<std::mutex> guard(mReceiveMutex);
	mOnReceiveCallbacks.push_back(callback);
	guard.unlock();
}

void UDPServer::localInit(int localport)
{
	UDPServer::Init();
	mChannel = -1;
	mPacket = NULL;
	mSocket = SDLNet_UDP_Open(localport);
	mLocalPort = localport;
	mReceiveThread = std::thread(&UDPServer::receivingThread, this);
}

void UDPServer::setupPacketSize(size_t size)
{
	if (mPacket == NULL)
	{
		mPacket = SDLNet_AllocPacket(size);
	}
	else
	{
		if (mPacket->maxlen < size)
		{
			int allocatedSize = SDLNet_ResizePacket(mPacket, size);
			if (allocatedSize < size)
			{
				printf("SDLNet_ResizePacket: %s\n", SDLNet_GetError());
			}
		}
	}
}

void UDPServer::receivingThread()
{
	// Allocate memory for incoming package
	UDPpacket *packet = SDLNet_AllocPacket(1024);
	
	while (true)
	{
		int receiveResponse = 0;
		{
			
			do
			{
				std::lock_guard<std::mutex> lock_do_while_scope(mReceiveMutex);
				receiveResponse = SDLNet_UDP_Recv(mSocket, packet);
				
				// Received data successfully
				if (receiveResponse == 1)
				{					
					//TODO: Do something with peer data
					mPeers.push_back(packet->address);
				}

			} while (receiveResponse > 0);
		}
	}
}