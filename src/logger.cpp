#include "logger.h"

std::string logExc_c::strErrorMessages[] = {
	"empty filename",
	"can't open logfile"
};

logger_c::logger_c(const std::string &filename) : m_filename(filename)
{
	if (m_filename.empty())
		throw logExc_c(logExc_c::errCode_t::ERROR_EMPTY_FILENAME, __FILE__, __FUNCTION__);
}

void logger_c::Write(const std::string &msg) const
{
	std::ofstream ofs;
	std::lock_guard<std::mutex> guard(m_mutex);
	std::string logline;

	ofs.open(m_filename, std::ios::out | std::ios::app);
	if (!ofs.is_open())
		throw logExc_c(logExc_c::errCode_t::ERROR_OPEN, __FILE__, __FUNCTION__);

	logline = prefix();
	logline.append(msg);
	ofs.write(logline.c_str(), logline.size());
	ofs.close();
}
