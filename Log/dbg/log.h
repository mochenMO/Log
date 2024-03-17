
/*// 还未解决的问题
* 1.throw std::logic_error("type error, not int value");
*
*
*
*/


/*// 所学到的知识
* 1.C++的抛出异常，和处理异常的方法：
* 
		void checkIntValue(int value) {
			if (value != static_cast<int>(value)) {
				throw std::logic_error("type error, not int value");   // std::logic_error异常对象中会自动包含文件名和行号等调试信息
			}
		}
		
		int main() {
			try {
				checkIntValue(10); // 正确的整数值
				checkIntValue(10.5); // 非整数值，会抛出异常
			} catch (const std::logic_error& e) {
				 std::cerr << "Exception caught at file " << __FILE__ << " line " << __LINE__ << ": " << e.what() << std::endl;
			}
		
			return 0;
		}
* 
* 2. STL存虚基类时，因为无法调用派生类的拷贝或移动函数，因此存虚基类的指针。
* 3.
*/



#pragma once
#ifndef _MOCHEN_LOG_H_
#define _MOCHEN_LOG_H_


#include <string>
#include <list>
#include <deque>
#include <stdexcept>
#include <memory>

#include <Windows.h>


namespace mochen
{

namespace log
{


// 日志级别
enum class LogLevel
{
	debug = 0,
	info,
	warn,
	error,
	fatal
};

const char* logLevelString[5] = { "debug","info","warn","error","fatal" };



// 日志输出目的地
class LogAppender
{
public:
	using ptr = std::shared_ptr<LogAppender>;
protected:
	LogLevel m_level;
public:
	LogAppender(LogLevel _level = LogLevel::debug);
	virtual ~LogAppender() {}                    // 虚析构函数需要实现函数的定义，如果只写函数说明，可能出现链接错误

	virtual void log(const char* _message) = 0;  // 纯虚函数
	LogLevel get_level();
	void set_level(LogLevel _level);
};


class ConsoleLogAppender : public LogAppender
{
public:
	ConsoleLogAppender(LogLevel _level = LogLevel::debug);

	void log(const char* _message) override;
};


class FileLogAppender : public LogAppender
{
private:
	char* m_filename;
	FILE* m_fp;
public:
	FileLogAppender();
	FileLogAppender(const std::string& _filename, LogLevel _level = LogLevel::debug);
	~FileLogAppender();

	FileLogAppender(const FileLogAppender& _value);
	FileLogAppender(FileLogAppender&& _value) noexcept;

	void operator=(const FileLogAppender& _value);
	void operator=(FileLogAppender&& _value) noexcept;

	void log(const char* _message) override;
	void clear();
	void open(const std::string& _filename);
};


// 日志事件
struct LogEvent
{
	unsigned long long			 m_timestamp;
	const char					*m_loggername;
	const char					*m_filename;
	int						     m_line;
	const char                  *m_content;
	std::list<LogAppender::ptr> *m_appender_list;
};


// 日志事件管理器
class LogEventManager
{
private:
	// LogFormatter 。。。。
	std::deque<LogEvent*> m_logEvent_equeue;
	HANDLE m_hThread;
public:
	LogEventManager();
	~LogEventManager();

	LogEventManager(const LogEventManager& _value) = default;
	LogEventManager(LogEventManager&& _value) noexcept = default;

	LogEventManager& operator=(const LogEventManager& _value) = default;
	LogEventManager& operator=(LogEventManager&& _value) noexcept = default;

	static DWORD thread_main_funtion_deal_logEvent_equeue(void* _value);
	void add_logEvent(LogEvent _logEvent);
};



// 日志器
class Logger
{
private:
	std::string m_loggername;
	std::list<LogAppender::ptr> *m_appender_list;    // LogAppender是虚基类，赋值时无法调用派生类的拷贝或移动函数，因此LogAppender*。同时因为满足第三种内存管理情况，为了方便管理内存，能智能指针。
public:
	Logger(const std::string& _loggername);
	Logger(const std::string& _loggername, LogAppender::ptr _appender_ptr);    // 为什么不用std::list<LogAppender::ptr>&
	// 列表初始化????


	~Logger();

	Logger(const Logger& _logger) = delete;
	Logger(Logger&& _logger) noexcept = delete;

	Logger& operator=(const Logger& _logger) = delete;
	Logger& operator=(Logger&& _logger) noexcept = delete;

	void clear();
	void log(LogLevel _level, const char* _format, ...);
	void debug(const char* _format, ...);
	void info(const char* _format, ...);
	void warn(const char* _format, ...);
	void error(const char* _format, ...);
	void fatal(const char* _format, ...);

	void set_loggername();
	void set_level(LogLevel _level);

	bool add_appender(const LogAppender& _value);
	bool remove_appender();
	std::list<LogAppender>& get_appender_list();
};




inline LogAppender::ptr GetDefauleLogAppender()
{
	static LogAppender::ptr defauleLogAppender = std::make_shared<ConsoleLogAppender>();
	return defauleLogAppender;
}

inline Logger* GetDefauleLogger()
{
	static Logger defauleLogger("defauleLogger", GetDefauleLogAppender());
	return &defauleLogger;
}

inline LogEventManager* GetLogEventManager()
{
	static LogEventManager logEventManager{};
	return &logEventManager;
}


void debug(const char* _format, ...);
void info(const char* _format, ...);
void warn(const char* _format, ...);
void error(const char* _format, ...);
void fatal(const char* _format, ...);






};

};



#endif // !_MOCHEN_LOG_H_

