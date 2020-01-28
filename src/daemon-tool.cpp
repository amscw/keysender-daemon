#include "daemon-tool.h"

std::string daemonToolExc_c::strErrorMessages[] = {
		"can't open file",
		"can't fork to new process",
		"can't exec new process",
		"exec process fail",
};

daemonTool_c::daemonTool_c(const std::string &filename) : cfgFilename(filename), pid(-1)
{
	std::ostringstream oss;

	try
	{
		logger = std::make_unique<logger_c>("daemon.log");
	} catch (exc_c &exc) {
		throw ;
	}

	pid = fork();
	switch (pid)
	{
	case -1:
		throw daemonToolExc_c(daemonToolExc_c::errCode_t::ERROR_FORK, __FILE__, __FUNCTION__);

	case 0:
		// child
		umask(0);
		setsid();

		// std streams no more need
		close(STDIN_FILENO);
		// close(STDOUT_FILENO);
		close(STDERR_FILENO);
		break;

	default:
		// parent
		oss << "created child: " << pid;
		// logger->Write(oss.str());
		TRACE(oss);
	}
}

int daemonTool_c::exec(std::unique_ptr<daemon_c> daemon)
{
	int wstatus;
	std::ostringstream oss;
	int err;

	if (daemon->IsChild())
	{
		// child process
		try
		{
			err = daemon->Exec();
		} catch (exc_c &exc) {
			throw;
		}
		// normally unreachable here. If not - throw exc
		oss << "error: " << err;
		throw daemonToolExc_c(daemonToolExc_c::errCode_t::ERROR_EXEC_RUN, __FILE__, __FUNCTION__, oss.str());
	} else {
		// parent process
		wait(&wstatus);
#if defined(DBG_PROCESS_TRACE)
		std::string str(daemon->Stdout());
		logger->Write(str);
		oss << "process " << daemon->Pid() << " terminated with status: " << "0x" << std::hex << wstatus;
		logger->Write(oss);
#endif // DBG_PROCESS_TRACE
	}

	if (!WIFEXITED(wstatus))
		throw daemonToolExc_c(daemonToolExc_c::errCode_t::ERROR_EXEC_FAIL, __FILE__, __FUNCTION__);

	return WEXITSTATUS(wstatus);
}

void daemonTool_c::savePIDToFile(const std::string &filename)
{
	std::ofstream ofs;

	ofs.open(filename, std::ios::out);
	if (!ofs.is_open())
		throw daemonToolExc_c(daemonToolExc_c::errCode_t::ERROR_OPEN, __FILE__, __FUNCTION__, filename);

	ofs << getpid();
	ofs.close();
}

void daemonTool_c::loadConfigsFromFile(const std::string &filename)
{
	try
	{
		cfgparser = YAML::LoadFile(filename);

		// common settings
		cfg.cmn.ifname = cfgparser["common"]["ifname"].as<std::string>();
		cfg.cmn.ipaddr = cfgparser["common"]["ipaddr"].as<std::string>();
		cfg.cmn.login = cfgparser["common"]["login"].as<std::string>();
		cfg.cmn.password = cfgparser["common"]["password"].as<std::string>();
		cfg.cmn.srcfile = cfgparser["common"]["srcfile"].as<std::string>();
		cfg.cmn.dstdir = cfgparser["common"]["dstdir"].as<std::string>();

		// ping settings
		cfg.ping.count = cfgparser["ping"]["count"].as<int>();
		cfg.ping.timeout = cfgparser["ping"]["timeout"].as<int>();

		// ssh settings
		cfg.sshp.timeout = cfgparser["sshpass"]["timeout"].as<int>();
	} catch (YAML::ParserException &exc) {
		throw;
	}
}

int daemonTool_c::Run()
{
	std::ostringstream oss;
	int err;
	struct timespec ts = {0, 0};	// s, ns

	// prepare signal set
	sigemptyset(&sigset);

	// add signals to signal set
	sigaddset(&sigset, SIGQUIT); 	// сигнал остановки процесса пользователем
	sigaddset(&sigset, SIGINT);		// сигнал для остановки процесса пользователем с терминала
	sigaddset(&sigset, SIGTERM);	// сигнал запроса завершения процесса
	sigaddset(&sigset, SIGCHLD); 	// сигнал посылаемый при изменении статуса дочернего процесса
	sigaddset(&sigset, SIGUSR1);	// пользовательский сигнал который мы будем использовать для обновления конфига
	sigprocmask(SIG_BLOCK, &sigset, NULL);

	// save PID
	try {
		savePIDToFile(pidFilename);
	} catch (exc_c &exc) {
		logger->Write(exc.ToString());
		exit(-1);
	}
	oss << "pid saved to file \"" << pidFilename << "\"";
	logger->Write(oss);

	// load configuration
	try {
		loadConfigsFromFile(cfgFilename);
	} catch (YAML::ParserException &exc) {
		logger->Write(exc.msg);
		exit(-1);
	}
	oss << "configuration successfully loaded from \"" << cfgFilename << "\"";
	logger->Write(oss);

	// create hw level
	try {
		hw = std::make_unique<hw_c>(this->cfg.cmn.ifname);
	} catch (exc_c &exc) {
		logger->Write(exc.ToString());
		exit(-1);
	}
	oss << "hardware abstraction level initialized";
	logger->Write(oss);

	// start engine
	while (1)
	{
		// TODO: process the signals
		err = sigtimedwait(&sigset, &siginfo, &ts);
		if (err > 0)
		{
			oss << "received the signal: " << err << "/";
			switch (siginfo.si_signo)
			{
				case SIGQUIT:
					oss << "SIGQUIT";
					exit(0);
					break;

				case SIGINT:
					oss << "SIGINT";
					break;

				case SIGTERM:
					oss << "SIGTERM";
					exit(0);
					break;

				case SIGCHLD:
					oss << "SIGCHLD";
					break;

				case SIGUSR1:
					oss << "SIGUSR1";
					exit(0);
					break;

				default:
					oss << "unknown";
			}
#if defined(DBG_SIGNALS)
			logger->Write(oss);
#else
			oss.str("");
			oss.clear();
#endif
		}

		try
		{
			// wait for linkup
			if (!hw->IsOnline())
				continue;
			oss << "link ok! try ping...";
			logger->Write(oss);

			// waiting connection to slave
			ping = std::make_unique<ping_c>(
					cfg.cmn.ifname,
					cfg.cmn.ipaddr,
					cfg.ping.count,
					cfg.ping.timeout);
			err = exec(std::move(ping));
#if defined(DBG_PROCESS_TRACE)
			oss << "ping exit code: " << err << ", destroyed: " << std::boolalpha << !static_cast<bool>(ping);
			logger->Write(oss);
#endif
			if (err == 0)
			{
				oss << "ping ok! try ssh...";
				logger->Write(oss);
			} else continue;

			// attempt to pass the keys to slave
			sshpass = std::make_unique<sshpass_c>(
					cfg.cmn.ipaddr,
					sshpass_c::login_t(cfg.cmn.login, cfg.cmn.password),
					cfg.cmn.srcfile,
					cfg.cmn.dstdir,
					cfg.sshp.timeout);
			err = exec(std::move(sshpass));
#if defined(DBG_PROCESS_TRACE)
			oss << "sshpass exit code: " << err << ", destroyed: " << std::boolalpha << !static_cast<bool>(sshpass);
			logger->Write(oss);
#endif
			if (err == 0)
			{
				oss << "keys fired! Exit";
				logger->Write(oss);
				break;
			} else continue;
		} catch (exc_c &exc) {
			logger->Write(exc.ToString());
			exit(-1);
		}
	}
	return 0;
}
