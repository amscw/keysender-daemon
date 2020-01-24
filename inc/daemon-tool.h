#ifndef _DAEMON_TOOL
#define _DAEMON_TOOL

#include <memory>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include "logger.h"
#include "tracers.h"
#include "exc.h"
#include "daemon.h"

struct daemonToolExc_c : public exc_c
{
	enum class errCode_t : std::uint32_t {
		ERROR_FORK,
		ERROR_EXEC_RUN,
		ERROR_EXEC_FAIL,
	} m_errCode;

	daemonToolExc_c(enum errCode_t code, const std::string &strFile, const std::string &strFunction, const std::string &strErrorDescription = "") noexcept :
			exc_c(strFile, strFunction, strErrorDescription), m_errCode(code)
	{}

	const std::string &Msg() const noexcept override { return strErrorMessages[(int)m_errCode]; }
	void ToStderr() const noexcept override
	{
		std::cerr << "WTF:" << m_strFile << "(" << m_strFunction << "):" << strErrorMessages[(int)m_errCode] << "-" << m_strErrorDescription << std::endl;
	}

	std::string ToString() noexcept override
	{
		oss.str("");
		oss.clear();
		oss << "WTF:" << m_strFile << "(" << m_strFunction << "):" << strErrorMessages[(int)m_errCode] << "-" << m_strErrorDescription;
		return oss.str();
	}

	private:
		static std::string strErrorMessages[];
		std::ostringstream oss;
};


class daemonTool_c
{
	std::unique_ptr<logger_c>logger = nullptr;
	std::unique_ptr<daemon_c> sshpass = nullptr;

	int pid;
	sigset_t sigset;
	siginfo_t siginfo;


public:
	daemonTool_c();
	inline bool IsChild() const noexcept {return (pid == 0); }
	int Run();

private:
	int exec(std::unique_ptr<daemon_c> daemon);
};

#endif // _DAEMON_TOOL
