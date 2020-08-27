#ifndef __strutil__h__
#define __strutil__h__

#pragma once

#include <stdint.h>

#include <string>
#include <vector>

using namespace std;


namespace util {
	namespace str {

		std::vector<std::string> string_split(const std::string &strSource, const char &chSplitChar);

		std::string string_concat(const std::vector<std::string> &strv, const std::string &c = " ");

		std::string string_trim(const std::string &str, const char c = ' ');

		std::string& replace_all(std::string& str, const std::string& old_value, const std::string& new_value);

		uint64_t str_to_UINT64(const std::string &strNumber);

		int32_t str_to_INT32(const std::string &strNumber);

		const std::string bytesToStr(const std::vector<char>& bytes, std::string& str, unsigned	maxLen = 0);
		const std::string bytesToStr(const std::vector<char>& bytes, unsigned maxLen = 0);

		const void strToBytes(const std::string& str, std::vector<char>& bytes);


		std::string getRawString(std::string const& s);
		// sample: toHexString(pDataBuffer, dataLen, " ")
		std::string toHexString(const char* buf, int len, const std::string& tok = " ");
		unsigned string2int(const std::string& str);


		//no MultiByteToWideChar but std::wstring_convert or std::codecvt_utf8, support VC and Clang
		//string UnicodeToUtf8(const wstring& wstr);
	}
}


#endif
