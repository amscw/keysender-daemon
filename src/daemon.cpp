#include <unistd.h>
#include <string.h>
#include "daemon.h"

std::string daemonExc_c::strErrorMessages[] = {
		"can't fork to new process",
		"this process is not child",
		"no arguments for exec()",
};

const std::size_t daemon_c::BUFSIZE = 2048;

daemon_c::daemon_c() : pid(-1), buf(new char [BUFSIZE]), argv(nullptr)
{
	// создать анонимный канал
	pipe(pipefd);
	memset(buf, 0, BUFSIZE);

	// создать дочерний процесс
	pid = fork();
	switch(pid)
	{
	case -1:
		throw daemonExc_c(daemonExc_c::errCode_t::ERROR_FORK, __FILE__, __FUNCTION__);

	case 0:
		// child
		close(pipefd[0]);					// unused in
		dup2(pipefd[1], 1);					// stdout to pipe out
		// exec ...
		break;

	default:
		close(pipefd[1]);					// unused out
	}
}

daemon_c::~daemon_c()
{
	if (buf != nullptr)
		delete [] buf;

	if (argv != nullptr)
		delete [] argv;
}

int daemon_c::Exec()
{
	int err;

	if (pid != 0)
		throw daemonExc_c(daemonExc_c::errCode_t::ERROR_IS_NOT_CHILD, __FILE__, __FUNCTION__);

	if (argv == nullptr)
		throw daemonExc_c(daemonExc_c::errCode_t::ERROR_NO_ARGV, __FILE__, __FUNCTION__);

	err = execvp(const_cast<const char*>(argv[0]), argv);
	close(pipefd[1]);
	return err;
}

const char* daemon_c::Stdout() noexcept
{
	read(pipefd[0], buf, BUFSIZE);
	close(pipefd[0]);
	return buf;
}

void daemon_c::buildArgs(const std::string &cmdline) noexcept
{
	size_t pos, nextpos;
	int i;
#if defined(DBG_DAEMON_ARGS)
	std::ostringstream oss;
#endif

	// create args
	for (pos = 0, nextpos = cmdline.find(' ', pos); nextpos != std::string::npos; nextpos = cmdline.find(' ', pos))
	{
		args.emplace_back(cmdline.substr(pos, nextpos - pos));
		pos = nextpos+1;
	}
	if (pos < cmdline.length())
		args.emplace_back(cmdline.substr(pos, cmdline.length() - pos));

	// create argv
	// TODO: use smart pointer!
	argv = new char* [args.size() + 1];

	for (i = 0; i < static_cast<int>(args.size()); i++)
	{
		argv[i] = const_cast<char*>(args.at(i).c_str());
#if defined(DBG_DAEMON_ARGS)
		oss << argv[i];
		TRACE(oss);
#endif
	}
	argv[i] = nullptr;
}

sshpass_c::sshpass_c(const std::string &a, const login_t &l, const std::string &src, const std::string &dst, int t) :
		ipaddr(a), login(l), srcFile(src), dstDir(dst), timeout(t)
{
	buildArgs(std::move(buildCmdline()));
}

std::string sshpass_c::buildCmdline() const noexcept
{
	std::ostringstream cmdline;

	// TODO: validate parameters
	// ...

	// create cmd line
	// "sshpass -p root scp /home/alex-m/keys root@192.168.0.2:/mnt/configs";
	cmdline << "sshpass" << " -p " << login.second << " " <<
		"scp" << " -o ConnectTimeout=" << timeout << " " << srcFile << " " << login.first << "@" << ipaddr << ":" << dstDir;

	return cmdline.str();
}

ping_c::ping_c(const std::string &I, const std::string &a, int c, int t) :
		interface(I), ipaddr(a), count(c), timeout(t)
{
	buildArgs(std::move(buildCmdline()));
}

std::string ping_c::buildCmdline() const noexcept
{
	std::ostringstream cmdline;

	// TODO: validate parameters
	// ...

	// create cmd line
	cmdline << "ping" << " -I " << interface << " -W " << timeout << " -c " << count << " " << ipaddr;

	return  cmdline.str();
}
