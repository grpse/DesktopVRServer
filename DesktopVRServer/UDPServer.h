#pragma once
#include "Defines.h"
#include <stdio.h>
#include <functional>
#include <thread>
#include <vector>
#include <mutex>
#include <SDL_net.h>

typedef std::function<void(UDPsocket peersocket, byte* buffer, size_t bufferLength)> onReceiveFromClientCallback;

class UDPServer
{
public:
	UDPServer(int localport);
	UDPServer();
	~UDPServer();

	bool bind(int remotePort);
	bool broadcast(byte * buffer, size_t size);

	void onReceive(onReceiveFromClientCallback callback);

private:

	static int WSInitializerCount;

	static void Init();
	static void Finalize();


	UDPsocket mSocket;
	IPaddress mIPaddress;
	UDPpacket *mPacket;
	int mChannel;
	int mLocalPort;
	int mRemotePort;

	std::vector<onReceiveFromClientCallback> mOnReceiveCallbacks;

	std::vector<IPaddress> mPeers;
	std::mutex mReceiveMutex;
	std::thread mReceiveThread;

	void localInit(int localport);
	void setupPacketSize(size_t size);
	void receivingThread();
};

