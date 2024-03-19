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
	log::Logger Logger01("Logger01");

	Logger01.log(log::LogLevel::debug, __FILE__, __LINE__, "test debug");
	Logger01.log(log::LogLevel::info, __FILE__, __LINE__, "test debug%d", 1);
	Logger01.log(log::LogLevel::warn, __FILE__, __LINE__, "test warn%lf", 1.123213);
	Logger01.log(log::LogLevel::error, __FILE__, __LINE__, "test error%s", "asdsad");
	Logger01.log(log::LogLevel::fatal, __FILE__, __LINE__, "test fatal%d %s %lf", 10, "sads", 0.343);

	logger_debug(Logger01, "test fatal%d %s %lf", 10, "sads", 0.343);

	default_debug("test debug");
	default_info("test debug%d", 1);
	default_warn("test warn%lf", 1.123213);
	default_error("test error%s", "asdsad");
	default_fatal("test fatal%d %s %lf",10,"sads", 0.343);

	
	

	printf("78678ugu\n");


	return 0;
}




