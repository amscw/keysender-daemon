#include "daemon-tool.h"


std::string daemonToolExc_c::strErrorMessages[] = {
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
		try
		{
			err = daemon->Exec();
		} catch (exc_c &exc) {
			throw;
		}

		if (err != 0)
			throw daemonToolExc_c(daemonToolExc_c::errCode_t::ERROR_EXEC_RUN, __FILE__, __FUNCTION__);
	} else {
		wait(&wstatus);
		std::string str(daemon->Stdout());
		logger->Write(str);
		oss << "process " << daemon->Pid() << " terminated with status: " << "0x" << std::hex << wstatus;
		logger->Write(oss.str());
	}

	if (!WIFEXITED(wstatus))
		throw daemonToolExc_c(daemonToolExc_c::errCode_t::ERROR_EXEC_FAIL, __FILE__, __FUNCTION__);

	return WEXITSTATUS(wstatus);
}

int daemonTool_c::Run()
{
	std::ostringstream oss;

	// prepare signal set
	sigemptyset(&sigset);

	// add signals to signal set
	sigaddset(&sigset, SIGQUIT); 	// сигнал остановки процесса пользователем
	sigaddset(&sigset, SIGINT);		// сигнал для остановки процесса пользователем с терминала
	sigaddset(&sigset, SIGTERM);	// сигнал запроса завершения процесса
	sigaddset(&sigset, SIGCHLD); 	// сигнал посылаемый при изменении статуса дочернего процесса
	sigaddset(&sigset, SIGUSR1);	// пользовательский сигнал который мы будем использовать для обновления конфига
	sigprocmask(SIG_BLOCK, &sigset, NULL);

	// start engine
	while (1)
	{
		try
		{
			sshpass = std::make_unique<sshpass_c>();
			exec(std::move(sshpass));
		} catch (exc_c &exc) {
			logger->Write(exc.ToString());
			break;
		}
		break;
	}
	return 0;
}
