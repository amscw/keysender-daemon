#ifndef _DAEMON_H
#define _DAEMON_H

#include "exc.h"
#include <vector>

struct daemonExc_c : public exc_c
{
	enum class errCode_t : std::uint32_t {
		ERROR_FORK,
		ERROR_IS_NOT_CHILD,
		ERROR_NO_ARGV,
	} m_errCode;

	daemonExc_c(enum errCode_t code, const std::string &strFile, const std::string &strFunction, const std::string &strErrorDescription = "") noexcept :
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


class daemon_c
{
	int pid;
	int pipefd[2];
	char *buf;
	static const std::size_t BUFSIZE;

public:
	daemon_c();
	virtual ~daemon_c();

	inline bool IsChild() const noexcept {return (pid == 0); }
	int Exec();
	const char* Stdout() noexcept;
	inline int Pid() const noexcept { return pid; }

protected:
	char **argv;	// массив строк, заканчивающийся NULL

};

class sshpass_c : public daemon_c
{
	std::vector<std::string> args;

public:
	sshpass_c();
	virtual ~sshpass_c();
};

#endif // _DAEMON_H
