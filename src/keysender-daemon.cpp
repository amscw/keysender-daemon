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
#include "daemon-tool.h"

static std::unique_ptr<daemonTool_c>(daemonTool);

int main() {
	std::ostringstream oss;

	try
	{
		daemonTool = std::make_unique<daemonTool_c>();
		if (daemonTool->IsChild())
		{
			daemonTool->Run();
		} else {
			oss << "daemon running. Exit";
			TRACE(oss);
		}
	} catch (exc_c &exc) {
		// fail to run daemon
		exc.ToStderr();
	}

	return 0;
}
