#include <windows.h>
#include <cassert>
#include "Defines.h"


class ScreenCapture
{
public:
	ScreenCapture(int bitmapBitCount);
	~ScreenCapture();

	void capture();
	void capture(byte*& outImageData, size_t& screenImageDataSize);

	inline byte* getData()
	{
		return screenImageData;
	}

	inline size_t getDataLength()
	{
		return screenImageDataSize;
	}

	inline const BITMAPINFOHEADER& getBitmapHeader()
	{
		return bi;
	}

private:
	BITMAPINFOHEADER  bi;
	HDC hdc, hDest;
	HBITMAP hbDesktop;
	int width, height;
	byte* screenImageData;
	size_t screenImageDataSize;

	inline byte* createMatrixForDesktopSize(int bitmapBitsCount)
	{
		screenImageDataSize = height * width * sizeof(byte) * bitmapBitsCount;
		return new byte[screenImageDataSize];
	}
};