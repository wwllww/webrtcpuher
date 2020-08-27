#ifndef RTCLOGWRITE_H
#define RTCLOGWRITE_H

#include <sstream>
#include <time.h>
using namespace std;

void writelogFunc(const std::string& data);
std::string getCurrentUTCTime();

#define LOG_BASE_INFO __FILE__ << ": " << getCurrentUTCTime().c_str() << "|" << __LINE__ << "|" << __FUNCTION__ << "; "
#define LOG__(level, d) { stringstream ss; ss << level << " " << LOG_BASE_INFO << d << endl; writelogFunc(ss.str()); }
#define LOGGER_(level, d) { LOG__(level,d) }


#define LOG_DEBUG(d) 	LOGGER_("DEBUG", d)
#define LOG_INFO(d)		LOGGER_("INFO", d)
#define LOG_WARN(d)		LOGGER_("WARN", d)
#define LOG_ERROR(d)	LOGGER_("ERROR", d)
#define LOG_FATAL(d)	LOGGER_("FATAL", d)


#define LOGER  LOG_INFO("")

#endif
