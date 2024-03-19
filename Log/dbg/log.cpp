#include "log.h"

using namespace mochen::log;

// =============================================================================================================
// 全局区

// 创建全局的 defauleLogEventManager （注意 LogEventManager 是个单例）
LogEventManager defauleLogEventManager{};

// 全局函数
std::shared_ptr<LogAppender> defauleLogAppender = std::make_shared<ConsoleLogAppender>();

inline std::shared_ptr<LogAppender>* mochen::log::getDefaultLogAppender() 
{
	// static std::shared_ptr<LogAppender> defauleLogAppender = std::make_shared<ConsoleLogAppender>();
	// 注意不用static的是因为，在多线程中同时创建static的是有的风险的。
	return &defauleLogAppender;
}


// 全局函数
Logger defauleLogger("defauleLogger", LogLevel::debug, (*getDefaultLogAppender()));

inline Logger* mochen::log::getDefaultLogger()
{
	// static Logger defauleLogger("defauleLogger", LogLevel::debug, (*getDefaultLogAppender()));
	// 注意不用static的是因为，在多线程中同时创建static的是有的风险的。
	return &defauleLogger;
}



// 声明全局的 logLevelString
const char* logLevelString[5] = { "debug","info","warn","error","fatal" };


// =============================================================================================================
// class LogAppender

LogAppender::LogAppender() : m_type(Type::withoutLogAppender)
{    

}


inline LogAppender::Type LogAppender::getType()
{
	return m_type;
}




// =============================================================================================================
// class ConsoleLogAppender

ConsoleLogAppender::ConsoleLogAppender(LogLevel _level)
{
	m_type = Type::ConsoleLogAppender;
}

void ConsoleLogAppender::log(const char* _massage)
{
	printf("%s", _massage);
}


// =============================================================================================================
// class FileLogAppender

FileLogAppender::FileLogAppender()
{
	m_type = Type::FileLogAppender;
	m_filename = "";
	m_fp = nullptr;
	m_maxSize = 1024;
}


FileLogAppender::FileLogAppender(const std::string& _filename, int _maxSize, LogLevel _level)
{
	m_type = Type::FileLogAppender;
	m_maxSize = 1024;
	m_filename = _filename;

	std::string temp = _filename;
	temp += ".txt";

	if ((m_fp = fopen(temp.c_str(), "a")) == nullptr) {
		throw std::logic_error("filded to open the file");    // std::logic_error异常对象中会自动包含文件名和行号等调试信息
	}
}


FileLogAppender::~FileLogAppender()
{
	clear();
}


FileLogAppender::FileLogAppender(FileLogAppender&& _value) noexcept
{
	m_filename = _value.m_filename;
	m_fp = _value.m_fp;
	
	_value.m_filename = "";
	_value.m_fp = nullptr;
}


void FileLogAppender::operator=(FileLogAppender&& _value) noexcept
{
	clear();

	m_filename = _value.m_filename;
	m_fp = _value.m_fp;

	_value.m_filename = "";
	_value.m_fp = nullptr;
}


void FileLogAppender::log(const char* _message)
{
	if (m_fp == nullptr) {
		throw std::logic_error("There is no initialization m_file, m_file is nullptr");
	}

	if (getFileSize() >= m_maxSize) {
		scrolling();  // 滚动
	}

	fwrite(_message, strlen(_message), 1, m_fp);
	fflush(m_fp);
}

void FileLogAppender::clear()
{
	if (m_fp != nullptr) {
		fclose(m_fp);
		m_fp = nullptr;
	}
	m_filename = "";
}

void FileLogAppender::open(const std::string& _filename)
{
	clear();

	m_filename = _filename;
	std::string temp = _filename;
	temp += ".txt";

	if ((m_fp = fopen(temp.c_str(), "a")) == nullptr) {
		throw std::logic_error("filded to open the file");    // std::logic_error异常对象中会自动包含文件名和行号等调试信息
	}
}


inline std::string FileLogAppender::getFilename()
{
	return m_filename;
}


inline int FileLogAppender::getFileSize()
{
	int size = 0;
	fseek(m_fp, 0, SEEK_END);    // 移动文件指针到文件结尾
	size = ftell(m_fp);          // 获取当前文件指针的位置，即文件大小，单位字节
	fseek(m_fp, 0, SEEK_SET);    // 移动文件指针到文件开头  ？？？？？？？？？？？？？？？？？
	return size;
}

void FileLogAppender::scrolling()
{
	if (m_fp != nullptr) {   // 由于没有实现setFilename函数，所以 m_fp != nullptr 时 m_filename != ""
		fclose(m_fp);
		m_fp = nullptr;
	}
	else {
		throw std::logic_error("There is no initialization m_file, m_file is nullptr");
	}

	std::string temp = m_filename;
	temp += "_log_";
	char timeString[24] = { 0 };

	time_t timestamp = time(nullptr);
	struct tm* pt = localtime(&timestamp);    // 注意 pt 指向的内存空间由系统管理，无需手动释放

	// 0000-00-00#00-00-00
	sprintf(timeString, "%d-%0.2d-%0.2d#%0.2d-%0.2d-%0.2d",
		pt->tm_year + 1900,
		pt->tm_mon + 1,
		pt->tm_mday,
		pt->tm_hour,
		pt->tm_min,
		pt->tm_sec);

	// m_filename_log_0000-00-00#00-00-00
	temp += timeString;
	temp += ".txt";

	if ((m_fp = fopen(temp.c_str(), "a")) == nullptr) {
		throw std::logic_error("filded to open the file");    // std::logic_error异常对象中会自动包含文件名和行号等调试信息
	}
}



// =============================================================================================================
// class LogEventManager

LogEventManager::LogEventManager() : 
	m_thread(&LogEventManager::dealLogEvent_threadFuntion, this),  // 注意使用成员函数指针时要加作用域
	m_isCanExit(false)
{
	m_logEventQueue = (LogEventQueue*)malloc(sizeof(LogEventQueue));
	m_logEventQueue->m_next = nullptr;
	m_logEventQueue->m_prev = nullptr;

	m_ptrWrite = m_logEventQueue;
	m_ptrRead = m_logEventQueue;
	m_ptrDelete = m_logEventQueue;

	m_LogAppenderListMap = new std::map<std::string, std::list<std::shared_ptr<LogAppender>>>{};
}


LogEventManager::~LogEventManager()
{
	m_isCanExit = true;         // 注意 m_isCanExit = true 且处理完日志队列中所有数据后才能退出
	m_thread.join();            // 等待处理完日志队列中所有数据
	clearLogEventQueue();       
	clearLogAppenderListMap();

	printf("~LogEventManager\n");
	_CrtDumpMemoryLeaks();   // 检测内存是否泄露  》》》》》》》》》》》》》》》》》》》》》》》》》》》》》》》》》》》》》》》》》》》
}



void LogEventManager::clearLogEventNodeData(LogEventQueue* _node)
{
	delete _node->m_data.m_loggername;
	free(_node->m_data.m_content);        // 释放申请的资源
	free(_node);
}

void LogEventManager::clearLogEventQueue()
{
	LogEventQueue* tempNode = m_logEventQueue;
	m_logEventQueue = m_logEventQueue->m_next;
	free(tempNode);      // 先把哨兵节点销毁掉，因为哨兵节点中没有存数据，所以不能 free(m_data.m_content)

	while (m_logEventQueue != nullptr) {
		tempNode = m_logEventQueue;
		m_logEventQueue = m_logEventQueue->m_next;
		clearLogEventNodeData(tempNode);
	}
	m_logEventQueue = nullptr;
}


void LogEventManager::clearLogAppenderListMap()
{
	for (auto mapIt = m_LogAppenderListMap->begin(); mapIt != m_LogAppenderListMap->end(); ++mapIt) {
		// list = mapIt->second
		for (auto vecterIt = mapIt->second.begin(); vecterIt != mapIt->second.end(); ++vecterIt) {
			vecterIt->reset();   // 释放智能指针
		}
	}
	delete m_LogAppenderListMap;
}


void LogEventManager::addAppender(std::string _loggername, std::shared_ptr<LogAppender> _appender)
{
	(*m_LogAppenderListMap)[_loggername].push_back(_appender);
}


inline bool LogEventManager::isFindLogger(const std::string& _loggername)
{
	return (m_LogAppenderListMap->find(_loggername) != m_LogAppenderListMap->end());
}


inline void LogEventManager::logFormatter(std::stringstream& _ss, LogEvent& _logEvent)
{
	char timeString[24] = { 0 };
	struct tm* pt = nullptr;     // 注意 pt 指向的内存空间由系统管理，无需手动释放

	// 处理时间
	pt = localtime(&_logEvent.m_timestamp);
	// [0000-00-00 00:00:00]   // 因为每次写入的值的长度都是固定的，所以不用清空 timeString
	sprintf(timeString, "%d-%0.2d-%0.2d %0.2d:%0.2d:%0.2d",
		pt->tm_year + 1900,
		pt->tm_mon + 1,
		pt->tm_mday,
		pt->tm_hour,
		pt->tm_min,
		pt->tm_sec);

	// [年-月-日 时:分:秒][日志器名称][日志等级][文件名]:[行号][日志信息]
	_ss << "[" << timeString << "]";
	_ss << "[" << *(_logEvent.m_loggername) << "]";
	_ss << "[" << logLevelString[(int)_logEvent.m_LogLevel] << "]";
 	_ss << "[" << _logEvent.m_filename << "]:";
	_ss << "[" << _logEvent.m_line << "]";
	_ss << "[" << _logEvent.m_content << "]";
	_ss << "\n";
}


void LogEventManager::dealLogEvent_threadFuntion()
{
	LogEventQueue* tempNode = nullptr;
	std::list<std::shared_ptr<LogAppender>>* tempList = nullptr;
	LogEvent tempData = { 0 };
	std::stringstream ss;


	while (m_isCanExit == false || m_ptrRead->m_next != nullptr)  // 注意 m_isCanExit = true 且处理完日志队列中所有数据后才能退出
	{
		if (m_ptrRead->m_next != nullptr) 
		{
			tempNode = m_ptrRead->m_next;
			m_ptrRead = m_ptrRead->m_next;

			tempData = tempNode->m_data;
			tempList = &(*m_LogAppenderListMap)[*(tempData.m_loggername)];

			for(auto it = tempList->begin(); it != tempList->end(); ++it) {  // 或者用 (*(*it)).getType();
				logFormatter(ss, tempData);    
				it->get()->log(ss.str().c_str());
				ss.str("");   // 清空 stringstream
			}

			if (m_ptrDelete->m_next != m_ptrRead) {
				tempNode = m_ptrDelete->m_next;
				tempNode->m_prev->m_next = tempNode->m_next;
				tempNode->m_next->m_prev = tempNode->m_prev;
				clearLogEventNodeData(tempNode);
			}
			
		}
	
	}


}

void LogEventManager::addLogEvent(LogEvent _logEvent)
{
	m_mutex.lock();
	
	m_ptrWrite->m_next = (LogEventQueue*)malloc(sizeof(LogEventQueue));
	
	m_ptrWrite->m_next->m_next = nullptr;
	m_ptrWrite->m_next->m_prev = m_ptrWrite;
	m_ptrWrite->m_next->m_data = _logEvent;

	m_ptrWrite = m_ptrWrite->m_next;

	m_mutex.unlock();
}


// =============================================================================================================
// class Logger

Logger::Logger(const std::string& _loggername, LogLevel _level, std::shared_ptr<LogAppender> _appender)
{
	if (defauleLogEventManager.isFindLogger(_loggername) == true) {
		throw std::logic_error("The _loggername already exists");
	}
	
	m_level = _level;

	m_loggername = _loggername;
	defauleLogEventManager.addAppender(_loggername, _appender);

	return; /// 
}


//void Logger::log(LogLevel _level, const char* _format, va_list _args)
//{
//	if ((int)m_level > (int)_level) {
//		return;
//	}
//	int size = vsnprintf(nullptr, 0, _format, _args);
//	char* buffer = (char*)malloc(sizeof(char) * (size + 1));    // +1保存结尾的"\0"
//	vsprintf(buffer, _format, _args);
//
//	LogEvent data;
//	data.m_timestamp = time(nullptr);
//	data.m_loggername = new std::string(m_loggername);
//	data.m_LogLevel = m_level;
//	data.m_filename = __FILE__;
//	data.m_line = __LINE__;
//	data.m_content = buffer;
//
//	defauleLogEventManager.addLogEvent(data);
//}
//
//
//void Logger::log(LogLevel _level, const char* _format, ...)
//{
//	if ((int)m_level > (int)_level) {
//		return;
//	}
//	va_list args;
//	va_start(args, _format);
//	log(_level, _format, args);
//	va_end(args);
//}
//
//void Logger::debug(const char* _format, ...)
//{
//	if ((int)m_level > (int)LogLevel::debug) {
//		return;
//	}
//	va_list args;
//	va_start(args, _format);
//	log(LogLevel::debug, _format, args);
//	va_end(args);
//}
//
//void Logger::info(const char* _format, ...)
//{
//	if ((int)m_level > (int)LogLevel::info) {
//		return;
//	}
//	va_list args;
//	va_start(args, _format);
//	log(LogLevel::info, _format, args);
//	va_end(args);
//}
//
//void Logger::warn(const char* _format, ...)
//{
//	if ((int)m_level > (int)LogLevel::warn) {
//		return;
//	}
//	va_list args;
//	va_start(args, _format);
//	log(LogLevel::warn, _format, args);
//	va_end(args);
//}
//
//void Logger::error(const char* _format, ...)
//{
//	if ((int)m_level > (int)LogLevel::error) {
//		return;
//	}
//	va_list args;
//	va_start(args, _format);
//	log(LogLevel::error, _format, args);
//	va_end(args);
//}
//
//void Logger::fatal(const char* _format, ...)
//{
//	if ((int)m_level > (int)LogLevel::fatal) {
//		return;
//	}
//	va_list args;
//	va_start(args, _format);
//	log(LogLevel::fatal, _format, args);
//	va_end(args);
//}


void Logger::log(LogLevel _level, const char* _filename, int _line, const char* _format, ...)
{
	if ((int)m_level > (int)_level) {
		return;
	}

	va_list args;
	va_start(args, _format);
	int size = vsnprintf(nullptr, 0, _format, args);
	char* buffer = (char*)malloc(sizeof(char) * (size + 1));    // +1保存结尾的"\0"
	vsprintf(buffer, _format, args);
	va_end(args);

	LogEvent data;
	data.m_timestamp = time(nullptr);
	data.m_loggername = new std::string(m_loggername);
	data.m_LogLevel = m_level;
	data.m_filename = _filename;
	data.m_line = _line;
	data.m_content = buffer;

	defauleLogEventManager.addLogEvent(data);
}

inline std::string Logger::getLoggerName()
{
	return m_loggername;
}

bool Logger::addAppender(std::shared_ptr<LogAppender> _appender)
{
	defauleLogEventManager.addAppender(m_loggername, _appender);
	return true;
}

inline LogLevel Logger::getLogLevel()
{
	return m_level;
}

inline void Logger::setLogLevel(LogLevel _level)
{
	m_level = _level;
}


// =============================================================================================================
// 全局的函数模块

// 注意全局函数在实现时，也要加命名空间
//inline void mochen::log::debug(const char* _format, ...)
//{
//	if ((int)defauleLogger.getLogLevel() > (int)LogLevel::debug) {
//		return;
//	}
//	va_list args;
//	va_start(args, _format);
//	defauleLogger.log(LogLevel::debug, _format, args);
//	va_end(args);
//}
//
//inline void mochen::log::info(const char* _format, ...)
//{
//	if ((int)defauleLogger.getLogLevel() > (int)LogLevel::info) {
//		return;
//	}
//	va_list args;
//	va_start(args, _format);
//	defauleLogger.log(LogLevel::info, _format, args);
//	va_end(args);
//}
//
//inline void mochen::log::warn(const char* _format, ...)
//{
//	if ((int)defauleLogger.getLogLevel() > (int)LogLevel::warn) {
//		return;
//	}
//	va_list args;
//	va_start(args, _format);
//	defauleLogger.log(LogLevel::warn, _format, args);
//	va_end(args);
//}
//
//inline void mochen::log::error(const char* _format, ...)
//{
//	if ((int)defauleLogger.getLogLevel() > (int)LogLevel::error) {
//		return;
//	}
//	va_list args;
//	va_start(args, _format);
//	defauleLogger.log(LogLevel::error, _format, args);
//	va_end(args);
//}
//
//inline void mochen::log::fatal(const char* _format, ...)
//{
//	if ((int)defauleLogger.getLogLevel() > (int)LogLevel::fatal) {
//		return;
//	}
//	va_list args;
//	va_start(args, _format);
//	defauleLogger.log(LogLevel::fatal, _format, args);
//	va_end(args);
//}





