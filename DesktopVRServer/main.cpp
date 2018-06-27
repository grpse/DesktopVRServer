#include <windows.h>

#include "ScreenCapture.h"
#include "UDPServer.h"

int main(int argc, char **argv)
{
	UDPServer server(9000);
	ScreenCapture capture(3);

	byte* screenData;
	size_t screenDataSize;

	capture.capture(screenData, screenDataSize);
	
	//server.bind(9001);

	while(true)
		server.broadcast(screenData, screenDataSize);
}