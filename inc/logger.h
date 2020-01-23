#ifndef _LOGGER_H
#define _LOGGER_H

#include <string>
#include <mutex>
#include <cassert>
#include <fstream>
#include "exc.h"

struct logExc_c : public exc_c
{
	enum class errCode_t : std::uint32_t {
		ERROR_EMPTY_FILENAME,
		ERROR_OPEN,
	} m_errCode;

	logExc_c(enum errCode_t code, const std::string &strFile, const std::string &strFunction, const std::string &strErrorDescription = "") noexcept :
			exc_c(strFile, strFunction, strErrorDescription), m_errCode(code)
	{}

	const std::string &Msg() const noexcept override { return strErrorMessages[(int)m_errCode]; }
	void ToStderr() const noexcept override
	{
		std::cerr << "WTF:" << m_strFile << "(" << m_strFunction << "):" << strErrorMessages[(int)m_errCode] << "-" << m_strErrorDescription << std::endl;
	}

	private:
		static std::string strErrorMessages[];
};

class logger_c
{
	std::string m_filename;
	mutable std::mutex m_mutex;

public:
	logger_c(const std::string &filename);
	void Write(const std::string &msg) const;

private:
	inline std::string prefix() const noexcept
	{
		static char data_time[80];
		time_t raw_time;
		struct tm* time_info;

		time(&raw_time);
		time_info = localtime(&raw_time);
		strftime(data_time, sizeof data_time, "[%d-%m-%Y %I:%M:%S] ", time_info);
		return std::string(data_time);
	}
};

#endif // _LOGGER_H
