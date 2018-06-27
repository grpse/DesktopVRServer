#include <windows.h>
#include <iostream>
#include <chrono>
#include <vector>
#include <cassert>
#include "ScreenCapture.h"
#include "UDPServer.h"


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

	byte* screendata;
	size_t screendatasize;


	for (int TestIndex = 0; TestIndex < NumberOfTests; TestIndex++)
	{
		double CountSamples = 0;
		for (int SampleIndex = 0; SampleIndex < NumberOfSamples; SampleIndex++)
		{
			startTime = high_resolution_clock::now();

			screenCapture.capture(screendata, screendatasize);


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