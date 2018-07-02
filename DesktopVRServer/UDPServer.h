#pragma once
#include "Defines.h"
#include <cstdio>
#include <cctype>
#include <functional>
#include <thread>
#include <vector>
#include <mutex>
#include <winsock.h>
#include <tuple>
#include <map>

typedef std::function<void(byte* buffer, size_t bufferLength)> onReceiveFromClientCallback;

class UDPServer
{
public:
	UDPServer(const char* ip, int localport);
	UDPServer(int localport);
	~UDPServer();

	bool multicast(byte * buffer, size_t size, size_t& sent);
	void onReceive(onReceiveFromClientCallback callback);
	void startReceiver();
	void bindServer();
	void broadcastMessage(byte* bytes, size_t length);
	bool isOpen();

private:

	static int WSInitializerCount;

	static void Init();
	static void Finalize();
	struct sockaddr_in mServerAddress;
	int addrlen;

	SOCKET mSocket;
	int mPort;

	std::vector<onReceiveFromClientCallback> mOnReceiveCallbacks;

	std::map<std::pair<ULONG, u_short>, struct sockaddr_in> mPeers;
	std::mutex mReceiveMutex;
	std::thread mReceiveThread;

	void receivingThread();
	void removePeer(std::pair<ULONG, u_short> peerKey);
};

