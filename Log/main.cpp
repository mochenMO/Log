#include "dbg/recorder.h"
using namespace mochen;
//
//dbg::MemoryRecorder memoryRecorder{};

#include "dbg/log.h"

#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>  // Sleep(1000);

using namespace mochen;

int main()
{
	log::Logger Logger01("Logger01", log::LogLevel::debug, std::make_shared<log::ConsoleLogAppender>());

	Logger01.debug("test debug");
	Logger01.info("test debug%d", 1);
	Logger01.warn("test warn%lf", 1.123213);
	Logger01.error("test error%s", "asdsad");
	Logger01.fatal("test fatal%d %s %lf", 10, "sads", 0.343);

	log::debug("test debug");
	log::info("test debug%d", 1);
	log::warn("test warn%lf", 1.123213);
	log::error("test error%s", "asdsad");
	log::fatal("test fatal%d %s %lf",10,"sads", 0.343);


	

	printf("78678ugu\n");


	return 0;
}




