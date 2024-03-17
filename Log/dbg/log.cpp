#include "log.h"

using namespace mochen::log;


// =============================================================================================================
// class LogAppender

LogAppender::LogAppender(LogLevel _level = LogLevel::debug) : m_level(_level)
{

}

LogLevel LogAppender::get_level()
{
	return m_level;
}
           
void LogAppender::set_level(LogLevel _level)
{
	m_level = _level;
}



// =============================================================================================================
// class ConsoleLogAppender

ConsoleLogAppender::ConsoleLogAppender(LogLevel _level)
{
	m_level = _level;
}

void ConsoleLogAppender::log(const char* _message)
{
	printf("%s", _message);
}



// =============================================================================================================
// class FileLogAppender

FileLogAppender::FileLogAppender()
{
	m_level = LogLevel::debug;
	m_filename = nullptr;
	m_fp = nullptr;
}


FileLogAppender::FileLogAppender(const std::string& _filename, LogLevel _level = LogLevel::debug)
{
	m_level = _level;
	m_filename = (char*)malloc(_filename.size() + 1);      // +1�����β��\0
	strcpy(m_filename, _filename.c_str());

	if ((m_fp = fopen(m_filename, "a")) == nullptr) {
		throw std::logic_error("filded to open the file");    // std::logic_error�쳣�����л��Զ������ļ������кŵȵ�����Ϣ
	}
}


FileLogAppender::~FileLogAppender()
{
	clear();
}


FileLogAppender::FileLogAppender(const FileLogAppender& _value)
{
	m_level = _value.m_level;

	m_filename = (char*)malloc(strlen(_value.m_filename) + 1);      // +1�����β��\0
	strcpy(m_filename, _value.m_filename);

	if ((m_fp = fopen(m_filename, "a")) == nullptr) {
		throw std::logic_error("filded to open the file");    // std::logic_error�쳣�����л��Զ������ļ������кŵȵ�����Ϣ
	}
}


FileLogAppender::FileLogAppender(FileLogAppender&& _value) noexcept
{
	m_level = _value.m_level;
	m_filename = _value.m_filename;
	m_fp = _value.m_fp;
	
	_value.m_level = LogLevel::debug;
	_value.m_filename = nullptr;
	_value.m_fp = nullptr;
}


void FileLogAppender::operator=(const FileLogAppender& _value)
{
	clear();
	m_level = _value.m_level;

	m_filename = (char*)malloc(strlen(_value.m_filename) + 1);      // +1�����β��\0
	strcpy(m_filename, _value.m_filename);

	if ((m_fp = fopen(m_filename, "a")) == nullptr) {
		throw std::logic_error("filded to open the file");    // std::logic_error�쳣�����л��Զ������ļ������кŵȵ�����Ϣ
	}
}


void FileLogAppender::operator=(FileLogAppender&& _value) noexcept
{
	clear();

	m_level = _value.m_level;
	m_filename = _value.m_filename;
	m_fp = _value.m_fp;

	_value.m_level = LogLevel::debug;
	_value.m_filename = nullptr;
	_value.m_fp = nullptr;
}

void FileLogAppender::log(const char* _message)
{
	if (m_fp == nullptr) {
		throw std::logic_error("There is no initialization m_file, m_file is nullptr");
	}
	fwrite(_message, strlen(_message), 1, m_fp);
}

void FileLogAppender::clear()
{
	if (m_fp != nullptr) {
		fclose(m_fp);
		m_fp = nullptr;
	}
	if (m_filename != nullptr) {
		free(m_filename);
		m_filename = nullptr;
	}
}

void FileLogAppender::open(const std::string& _filename)
{
	clear();

	m_filename = (char*)malloc(_filename.size() + 1);      // +1�����β��\0
	strcpy(m_filename, _filename.c_str());

	if ((m_fp = fopen(m_filename, "a")) == nullptr) {
		throw std::logic_error("filded to open the file");    // std::logic_error�쳣�����л��Զ������ļ������кŵȵ�����Ϣ
	}
}



// =============================================================================================================
// class LogEventManager

//class LogEventManager
//{
//private:
//	std::deque<LogEvent*> m_logEvent_equeue;
//	HANDLE m_hThread;



LogEventManager::LogEventManager()
{
	m_hThread = CreateThread()
}


LogEventManager::~LogEventManager();



DWORD LogEventManager::thread_main_funtion_deal_logEvent_equeue(void* _value)
{

}

void LogEventManager::add_logEvent(LogEvent _logEvent);


// =============================================================================================================
// class Logger

//class Logger
//{
//protected:
//	std::string				 m_loggername;
//	std::list<LogAppender*>* m_appender_list;
//public:

Logger::Logger(const std::string& _loggername)
{
	m_loggername = _loggername;
	m_appender_list = nullptr;
}


Logger::Logger(const std::string& _loggername, LogAppender::ptr _appender_ptr)
{
	m_loggername = _loggername;
	m_appender_list = new std::list<LogAppender::ptr>();    // ���� std::list �������캯��
	m_appender_list->push_front(_appender_ptr);
}


Logger::~Logger()
{
	clear();
}

void Logger::clear()
{
	m_loggername.clear();
	if (m_appender_list != nullptr) {
		m_appender_list->clear();
		delete m_appender_list;
		m_appender_list == nullptr;
	}
}


void Logger::log(LogLevel _level, const char* _format, ...)
{
	// get_logEventManager()
}




void Logger::debug(const char* _format, ...);
void Logger::info(const char* _format, ...);
void Logger::warn(const char* _format, ...);
void Logger::error(const char* _format, ...);
void Logger::fatal(const char* _format, ...);

void Logger::set_loggername();
void Logger::set_level(LogLevel _level);

bool Logger::add_appender(const LogAppender& _value);
bool Logger::remove_appender();
std::list<LogAppender>& Logger::get_appender_list();





void debug(const char* _format, ...);
void info(const char* _format, ...);
void warn(const char* _format, ...);
void error(const char* _format, ...);
void fatal(const char* _format, ...);