#include "ScreenCapture.h"

ScreenCapture::ScreenCapture(int bitmapBitCount)
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

ScreenCapture::~ScreenCapture()
{
	delete screenImageData;
	ReleaseDC(NULL, hdc);
	DeleteDC(hDest);
}

void ScreenCapture::capture()
{
	BitBlt(hDest, 0, 0, width, height, hdc, 0, 0, SRCCOPY);
	GetDIBits(hDest, hbDesktop, 0, height, screenImageData, (BITMAPINFO *)&bi, DIB_RGB_COLORS);
}

void ScreenCapture::capture(byte*& outImageData, size_t& outScreenImageDataSize)
{
	BitBlt(hDest, 0, 0, width, height, hdc, 0, 0, SRCCOPY);
	GetDIBits(hDest, hbDesktop, 0, height, screenImageData, (BITMAPINFO *)&bi, DIB_RGB_COLORS);
	outImageData = screenImageData;
	outScreenImageDataSize = screenImageDataSize;
}