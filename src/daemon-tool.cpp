#include "daemon-tool.h"


std::string daemonToolExc_c::strErrorMessages[] = {
		"can't open file",
		"can't fork to new process",
		"can't exec new process",
		"exec process fail",
};

daemonTool_c::daemonTool_c() : pid(-1)
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
		std::string str(daemon->Stdout());
		logger->Write(str);
		oss << "process " << daemon->Pid() << " terminated with status: " << "0x" << std::hex << wstatus;
		logger->Write(oss);
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

int daemonTool_c::Run()
{
	std::ostringstream oss;
	int err;

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

	// start engine
	while (1)
	{
		try
		{
			// waiting connection to slave
			ping = std::make_unique<ping_c>("eth0", "192.168.0.2", 3, 2);
			err = exec(std::move(ping));
			oss << "ping exit code: " << err << ", destroyed: " << std::boolalpha << !static_cast<bool>(ping);
			logger->Write(oss);
//			if (err != 0)
//				continue;

			// attempt to pass the keys to slave
			sshpass = std::make_unique<sshpass_c>("192.168.0.2", sshpass_c::login_t("root", "root"), "keys", "/mnt/configs/", 2);
			err = exec(std::move(sshpass));
			oss << "sshpass exit code: " << err << ", destroyed: " << std::boolalpha << !static_cast<bool>(sshpass);
			logger->Write(oss);
//			if (err != 0)
//				continue;
		} catch (exc_c &exc) {
			logger->Write(exc.ToString());
			exit(-1);
		}
		break;
	}
	return 0;
}
