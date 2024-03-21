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



// ����Ĭ��Logger ���Զ���Logger
void test01()
{
	log::Logger Logger01("Logger01");

	Logger01.log(log::LogLevel::debug, __FILE__, __LINE__, "test debug");
	Logger01.log(log::LogLevel::info, __FILE__, __LINE__, "test info %d", 1123);
	Logger01.log(log::LogLevel::warn, __FILE__, __LINE__, "test warn %lf", 1.123213);
	Logger01.log(log::LogLevel::error, __FILE__, __LINE__, "test error %s", "asdsad");
	Logger01.log(log::LogLevel::fatal, __FILE__, __LINE__, "test fatal %d %s %lf", 10, "sads", 0.343);

	logger_debug(&Logger01, "test debug");
	logger_info(&Logger01, "test info %d", 1123);
	logger_warn(&Logger01, "test warn %lf", 1.123213);
	logger_error(&Logger01, "test error %s", "asdsad");
	logger_fatal(&Logger01, "test fatal %d %s %lf", 10, "sads", 0.343);

	default_debug("test debug");
	default_info("test info %d", 1123);
	default_warn("test warn %lf", 1.123213);
	default_error("test error %s", "asdsad");
	default_fatal("test fatal %d %s %lf", 10, "sads", 0.343);
}

// �����ļ�д��
void test02()
{
	log::Logger Logger01("Logger01");
	Logger01.addAppender(std::make_shared<log::FileLogAppender>("test02"));

	for (int i = 0; i < 1000; ++i) {
		logger_debug(&Logger01, "test03 %d", i);
	}
}

// ��������Ч��
void test03()
{
	dbg::RunTimeRecorder runTimeRecorder{};

	log::Logger Logger01("Logger01");

	for (int i = 0; i < 1000; ++i) {
		logger_debug(&Logger01, "main => %d", i);
	}

	//for (int i = 0; i < 1000; ++i) {
	//	printf("main => %d\n", i);
	//}

	// ���ۣ������ö��߳��첽������־�����logger_debug �� printf ��ʮ�౶��
}


void threadFunctionConsole(std::string _name)
{
	log::Logger logger(_name);
	for (int i = 0; i < 100; ++i) {
		logger_debug(&logger, "%s => %d", _name.c_str(), i);
	}
}


void threadFunctionFile(std::string _name, std::string _filename)
{
	log::Logger logger(_name);
	std::shared_ptr<log::FileLogAppender> fileLogAppender = std::make_shared<log::FileLogAppender>();
	fileLogAppender->open(_filename);
	logger.addAppender(fileLogAppender);
	for (int i = 0; i < 100; ++i) {
		logger_debug(&logger, "%s => %d", _name.c_str(), i);
	}
}


// ���Զ��߳��µ��̰߳�ȫ��
void test04()
{
	std::thread t01(&threadFunctionConsole, "thread01");
	std::thread t02(&threadFunctionConsole, "thread02");

	std::thread t03(&threadFunctionFile, "thread03","test01");   // �����߳�д��ͬһ���ļ�
	std::thread t04(&threadFunctionFile, "thread04","test01");   // �����߳�д��ͬһ���ļ�

	std::thread t05(&threadFunctionFile, "thread05", "test02");
	std::thread t06(&threadFunctionFile, "thread06", "test03");

	t01.join();
	t02.join();
	t03.join();
	t04.join();
	t05.join();
	t06.join();
}


void threadFunctionConsoleSync(std::string _name, std::string _filename)
{
	log::Logger logger(_name);
	std::shared_ptr<log::FileLogAppender> fileLogAppender = std::make_shared<log::FileLogAppender>();
	fileLogAppender->open(_filename);
	logger.addAppender(fileLogAppender);
	logger.setUpSyncDebug();  // ����ͬ��ģʽ

	for (int i = 0; i < 100; ++i) {
		logger_debug(&logger, "%s => %d", _name.c_str(), i);
		printf("%s => %d\n", _name.c_str(), i);
	}
}


// ����ͬ��ģʽ
void test05()
{
	std::thread t01(&threadFunctionConsoleSync, "thread01", "test01");
	std::thread t02(&threadFunctionConsoleSync, "thread02", "test02");
	std::thread t03(&threadFunctionConsoleSync, "thread03", "test03");  // ֻ����һ����־������������������ĸ���˭���Ŀ�

	t01.join();
	t02.join();
	t03.join();
}


int main()
{
	test01(); // ����Ĭ��Logger ���Զ���Logger
	// test02(); // �����ļ�д��
	// test03(); // ��������Ч��
	// test04(); // ���Զ��߳��µ��̰߳�ȫ��
	// test05(); // ����ͬ��ģʽ


	return 0;
}




