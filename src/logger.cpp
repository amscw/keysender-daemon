#include "logger.h"

std::string logExc_c::strErrorMessages[] = {
	"empty filename",
	"can't open logfile"
};

logger_c::logger_c(const std::string &filename) : m_filename(filename)
{
	if (m_filename.empty())
		throw logExc_c(logExc_c::errCode_t::ERROR_EMPTY_FILENAME, __FILE__, __FUNCTION__);

	std::ofstream ofs(filename);
	if (!ofs.is_open())
		throw logExc_c(logExc_c::errCode_t::ERROR_OPEN, __FILE__, __FUNCTION__);
	ofs << __FILE__ << "(" << __FUNCTION__ << ")" << " initialized ok" << std::endl;
	ofs.close();
}

void logger_c::Write(const std::string &msg) const
{
	std::ofstream ofs;
	std::lock_guard<std::mutex> guard(m_mutex);

	ofs.open(m_filename, std::ios::out | std::ios::app);
	if (!ofs.is_open())
		throw logExc_c(logExc_c::errCode_t::ERROR_OPEN, __FILE__, __FUNCTION__);

	// ofs.write(logline.c_str(), logline.size());
	ofs << prefix() << msg << std::endl;
	ofs.close();
}

void logger_c::Write(std::ostringstream &oss) const
{
	std::ofstream ofs;
	std::lock_guard<std::mutex> guard(m_mutex);

	ofs.open(m_filename, std::ios::out | std::ios::app);
	if (!ofs.is_open())
		throw logExc_c(logExc_c::errCode_t::ERROR_OPEN, __FILE__, __FUNCTION__);

	// ofs.write(logline.c_str(), logline.size());
	ofs << prefix() << oss.str() << std::endl;
	ofs.close();
	oss.clear();
	oss.str("");
}
