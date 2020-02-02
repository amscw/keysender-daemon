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
	static const std::size_t BUFSIZE;
	int pid;
	int pipefd[2];
	char *buf;

public:
	daemon_c();
	virtual ~daemon_c();

	inline bool IsChild() const noexcept {return (pid == 0); }
	int Exec();
	const char* Stdout() noexcept;
	inline int Pid() const noexcept { return pid; }

protected:
	std::vector<std::string> args;	// формируется наследником
	char **argv;					// массив строк, заканчивающийся NULL
	void buildArgs(const std::string& cmdline) noexcept;
	virtual std::string buildCmdline() const noexcept = 0;

};

class sshpass_c : public daemon_c
{
public:
	using login_t = std::pair<std::string, std::string>;

private:
	std::string ipaddr;
	login_t login;
	std::string srcFile;
	std::string dstDir;
	int timeout;

public:
	sshpass_c(const std::string &a, const login_t &l, const std::string &src, const std::string &dst, int t);

private:
	std::string buildCmdline() const noexcept override;
};

class ping_c : public daemon_c
{
	std::string interface;
	std::string ipaddr;
	int count;
	int timeout;

public:
	ping_c(const std::string &I, const std::string &a, int c, int t);

private:
	std::string buildCmdline() const noexcept override;
};

class fakeDaemon_c : public daemon_c
{
	// fake params...
	std::string args;

public:
	fakeDaemon_c(const std::string &args) noexcept;

private:
	std::string buildCmdline() const noexcept override;
};

#endif // _DAEMON_H

