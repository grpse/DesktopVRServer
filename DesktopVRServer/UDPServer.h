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
	~UDPServer();

	bool broadcast(byte * buffer, size_t size);
	bool bindbroadcast(const char* ip, int port, int localport);
	void onReceive(onReceiveFromClientCallback callback);

private:

	static int WSInitializerCount;

	static void Init();
	static void Finalize();

	UDPsocket mSocket;
	int mLocalPort;

	std::vector<onReceiveFromClientCallback> mOnReceiveCallbacks;

	std::vector<struct SocketAddressIn> mPeersAddr;
	std::mutex mReceiveMutex;
	std::thread mReceiveThread;

	void receivingThread();
};

