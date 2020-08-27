#include <algorithm>
#include <string.h>
#include <sstream>

#include <locale>
#include <codecvt>

#include "strutil.h"

namespace util {
	namespace str
	{

#define STR_TO_NUM(type) \
    std::stringstream ss; \
    ss << strNumber; \
    type num = 0; \
    ss >> num; \
    return num; \

		std::vector<std::string> string_split(const std::string &strSource, const char &chSplitChar)
		{
			std::vector<std::string> ret;
			std::string::size_type last = 0;
			std::string::size_type index = strSource.find(chSplitChar, last);
			while (index != std::string::npos)
			{
				ret.push_back(strSource.substr(last, index - last));
				last = index + 1;
				index = strSource.find(chSplitChar, last);
			}
			if (index - last > 0)
			{
				ret.push_back(strSource.substr(last, index - last));
			}

			return ret;
		}

		std::vector<std::string> split(const std::string &s, char delim) {
			std::stringstream ss(s);
			std::string item;
			std::vector<std::string> elems;
			while (std::getline(ss, item, delim)) {
				elems.push_back(item);
			}
			return elems;
		}

		std::string string_concat(const std::vector<std::string> &strv, const std::string &c)
		{
			std::string ret;
			std::vector<std::string>::const_iterator cit = strv.begin();

			for (; cit != strv.end(); cit++)
			{
				ret += *cit;
				ret += c;
			}
			if (ret.size() > 0)
				ret.erase(ret.end() - 1);

			return ret;
		}

		std::string string_trim(const std::string &str, const char c)
		{
			std::string::size_type pos = str.find_first_not_of(c);
			if (pos == std::string::npos)
			{
				return str;
			}
			std::string::size_type pos2 = str.find_last_not_of(c);
			if (pos2 != std::string::npos)
			{
				return str.substr(pos, pos2 - pos + 1);
			}
			return str.substr(pos);
		}

		std::string& replace_all(std::string& str, const std::string& old_value, const std::string& new_value)
		{
			while (true)
			{
				size_t pos = 0;
				if ((pos = str.find(old_value, 0)) != std::string::npos)
					str.replace(pos, old_value.length(), new_value);
				else break;
			}
			return str;
		}


		uint64_t str_to_UINT64(const std::string &strNumber)
		{
			STR_TO_NUM(uint64_t);
		}

		int32_t str_to_INT32(const std::string &strNumber)
		{
			STR_TO_NUM(int32_t);
		}


		const std::string bytesToStr(const std::vector<char>& bytes, std::string& str, unsigned	maxLen)
		{
			unsigned len = (maxLen == 0) ? bytes.size() : std::min(maxLen, (unsigned)bytes.size());
			str.resize(len);
			memcpy(&str[0], &bytes[0], len);
			return str;
		}
		const std::string bytesToStr(const std::vector<char>& bytes, unsigned maxLen)
		{
			std::string tempStr;
			return bytesToStr(bytes, tempStr, maxLen);
		}

		const void strToBytes(const std::string& str, std::vector<char>& bytes)
		{
			bytes.resize(str.size());
			memcpy(&bytes[0], &str[0], str.size());
		}

		std::string getRawString(std::string const& s)
		{
			std::ostringstream out;
			out << '\"';
			out << std::hex;
			for (std::string::const_iterator it = s.begin(); it != s.end(); ++it)
			{
				// AND 0xFF will remove the leading "ff" in the output,
				// So that we could get "\xab" instead of "\xffab"
				out << " " << (static_cast<short>(*it) & 0xff);
			}
			out << '\"';
			return out.str();
		}

		std::string toHexString(const char* buf, int len, const std::string& token)
		{
			std::string output;
			output.reserve(len * (2 + token.size()));
			char temp[8];
			for (int i = 0; i < len; ++i)
			{
				sprintf(temp, "%.2x", (uint8_t)buf[i]);
				output.append(temp, 2);
				output.append(token);
			}

			return output;
		}
		unsigned string2int(const std::string& str)
		{
			unsigned num = 0;
			for (unsigned i = 0; i < str.size(); ++i) {
				num += (unsigned)str[i];
			}
			return num;
		}		
	}
}
