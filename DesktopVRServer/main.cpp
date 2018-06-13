#include <windows.h>
#include <iostream>
#include <chrono>
#include <vector>
#include <cassert>

using namespace std;

typedef unsigned char byte;

class ScreenCapture
{
public:
	ScreenCapture(int bitmapBitCount)
	{
		assert(bitmapBitCount >= 3 || bitmapBitCount <= 4);
		height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
		width = GetSystemMetrics(SM_CXVIRTUALSCREEN);

		screenImageData = createMatrixForDesktopSize(bitmapBitCount);

		hdc = GetDC(NULL);
		hDest = CreateCompatibleDC(hdc);
		hbDesktop = CreateCompatibleBitmap(hdc, width, height);

		// describe screen bitmap
		bi.biSize = sizeof(BITMAPINFOHEADER);
		bi.biWidth = width;
		bi.biHeight = -height;
		bi.biPlanes = 1;
		bi.biBitCount = bitmapBitCount * 8;
		bi.biCompression = BI_RGB;
		bi.biSizeImage = 0;
		bi.biXPelsPerMeter = 0;
		bi.biYPelsPerMeter = 0;
		bi.biClrUsed = 0;
		bi.biClrImportant = 0;
	}

	void capture()
	{
		BitBlt(hDest, 0, 0, width, height, hdc, 0, 0, SRCCOPY);
		GetDIBits(hDest, hbDesktop, 0, height, screenImageData, (BITMAPINFO *)&bi, DIB_RGB_COLORS);  //copy from hwindowCompatibleDC to hbwindow
	}

	inline byte* getData()
	{
		return screenImageData;
	}

	~ScreenCapture()
	{
		ReleaseDC(NULL, hdc);
		DeleteDC(hDest);
	}

private:
	BITMAPINFOHEADER  bi;
	HDC hdc, hDest;
	HBITMAP hbDesktop;
	int width, height;
	byte* screenImageData;

	inline byte* createMatrixForDesktopSize(int bitmapBitsCount)
	{
		return new byte[height * width * sizeof(byte) * bitmapBitsCount];
	}
};



int main(int argc, char **argv)
{
	const int bitmapBits = 3;
	ScreenCapture screenCapture(bitmapBits);

	using namespace std::chrono;

	high_resolution_clock::time_point startTime = high_resolution_clock::now();
	high_resolution_clock::time_point endTime = high_resolution_clock::now();

	const int NumberOfSamples = 100;
	const int NumberOfTests = 10;

	std::vector<double> timings;
	timings.resize(NumberOfTests);

	double lowerTime = MAXINT;
	double higherTime = MININT;

	const double HigherValueLimit = 0.017;
	int howManyHigher = 0;

	for (int TestIndex = 0; TestIndex < NumberOfTests; TestIndex++)
	{
		double CountSamples = 0;
		for (int SampleIndex = 0; SampleIndex < NumberOfSamples; SampleIndex++)
		{
			startTime = high_resolution_clock::now();

			screenCapture.capture();

			endTime = high_resolution_clock::now();

			const double time_span = duration_cast<duration<double>>(endTime - startTime).count();

			if (time_span < lowerTime)
				lowerTime = time_span;
			if (time_span > higherTime)
				higherTime = time_span;
			if (time_span > HigherValueLimit)
				howManyHigher++;

			CountSamples += time_span;
		}

		timings[TestIndex] = (CountSamples / NumberOfSamples);
	}

	for (auto& timing : timings)
	{
		std::cout << "Avg Sample time: " << timing << std::endl;
	}

	std::cout << "Number of frames: " << NumberOfSamples * NumberOfTests << std::endl;
	std::cout << "How many were higher than (" << HigherValueLimit << ") s :" << howManyHigher << std::endl;
	std::cout << "Max time: " << higherTime << std::endl;
	std::cout << "Min time: " << lowerTime << std::endl;

	std::cin.get();
}