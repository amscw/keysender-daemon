//============================================================================
// Name        : keysender-daemon.cpp
// Author      : amscw
// Version     :
// Copyright   : Your copyright notice
// Description :
//============================================================================

#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "tracers.h"

class daemon_c
{
	int m_pid;

public:
	void Run() noexcept;
};

int main() {
	std::ostringstream oss;
	int pid, err;

	// TODO: load config

	// try to create child
	pid = fork();
	switch (pid)
	{
	case -1:
		// error
		oss << "start daemon failed (" << strerror(errno) << ")";
		TRACE(oss);
		return -1;

	case 0:
		// child: start new session
		setsid();
		close (STDIN_FILENO);
		close (STDOUT_FILENO);
		close (STDERR_FILENO);
		// err = monitorProc();
		return err;

	default:
		// parent: exit
		oss << "start daemon! pid:" << pid;
		TRACE(oss);
	}
	return 0;
}
