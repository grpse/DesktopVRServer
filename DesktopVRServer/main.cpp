#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <windows.h>
#include <winsock.h>
#include <ctime>
#include <cstdlib>
#include "ScreenCapture.h"
#include "UDPServer.h"
#include <fstream>
#include <string>

#define EXAMPLE_PORT 6000
#define EXAMPLE_GROUP "239.0.0.1"

void SavePPM(std::string& filepath, unsigned char* bytes, size_t bytesLength)
{
	std::ofstream file(filepath);

	file << "P6" << std::endl;
	file << "# Comment" << std::endl;
	file << "1366 768" << std::endl;
	file << "255" << std::endl;

	for (size_t i = 0; i < bytesLength; i++)
	{
		file << std::to_string(bytes[i]) << std::endl;
	}

	file.close();
}

int main(int argc, char **argv)
{
	UDPServer server(EXAMPLE_PORT);
	ScreenCapture capture(4);

	byte* screenData;
	size_t screenDataSize;
	size_t sentAmount = 0;
	int fileIndex = 0;

	server.bindServer();
	server.startReceiver();

	while (server.isOpen())
	{
		capture.capture();
		screenData = capture.getData();
		screenDataSize = capture.getDataLength();
		auto bitmap = capture.getBitmapHeader();
		std::cout << "BITMAP   SIZE: " << bitmap.biSizeImage << std::endl;
		std::cout << "BITMAP  WIDTH: " << bitmap.biWidth << std::endl;
		std::cout << "BITMAP HEIGHT: " << bitmap.biHeight << std::endl;
		for (size_t i = 0; i < bitmap.biSizeImage; i++)
		{
			std::cout << std::to_string(screenData[i]) << std::endl;
		}

		server.broadcastMessage(screenData, screenDataSize);
		//server.broadcastMessage((byte*)"HAAAA", 5);
		Sleep(500);
	}
	

	return 0;
}