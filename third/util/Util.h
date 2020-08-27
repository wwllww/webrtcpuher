#pragma once
	
#include <chrono>

using namespace std;

namespace util {
	namespace time {
		static uint64_t getCurMsTime()
		{
			return chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
		}
	}
}