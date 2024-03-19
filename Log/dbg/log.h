
/*// 还未解决的问题
* 1.throw std::logic_error("type error, not int value");
* 2.日志的滚动机制，要配合配置系统保存当前使用的文件名，才能实现续写功能！！！
* 3.void addAppender(std::string _loggername, std::shared_ptr<LogAppender> _appender);   // 注意建议在创建线程之前用该函数，以免造成冲突的情况
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
* 2.STL存虚基类时，因为无法调用派生类的拷贝或移动函数，因此存虚基类的指针。
* 3.智能指针通常以值类型作为参数类型
* 4.全局函数和变量会受到命名空间的约束，但宏不会
* 
* 
* 
*/



#pragma once
#ifndef _MOCHEN_LOG_H_
#define _MOCHEN_LOG_H_

#define _CRT_SECURE_NO_WARNINGS


#include <stdexcept>  // throw std::logic_error 
#include <memory>     // std::share_ptr
#include <thread>
#include <mutex>
#include <string>
#include <sstream>
#include <list>
#include <map>

#include <stdarg.h>
#include <time.h>

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

// 日志输出目的地
class LogAppender
{
public:
	enum class Type
	{
		withoutLogAppender = 0,
		ConsoleLogAppender,
		FileLogAppender
	};
protected:
	Type m_type;
public:
	LogAppender();
	virtual ~LogAppender() {}                    // 虚析构函数需要实现函数的定义，如果只写函数说明，可能出现链接错误

	virtual void log(const char* _massage) = 0;  // 纯虚函数
	inline Type getType();
};


class ConsoleLogAppender : public LogAppender
{
public:
	ConsoleLogAppender(LogLevel _level = LogLevel::debug);

	void log(const char* _massage) override;
};



class FileLogAppender : public LogAppender
{
private:
	std::string m_filename;
	FILE* m_fp;
	int m_maxSize;
public:
	FileLogAppender();
	FileLogAppender(const std::string& _filename, int _maxSize = 1024, LogLevel _level = LogLevel::debug);
	~FileLogAppender();

	FileLogAppender(const FileLogAppender& _value) = delete;  // 不能拷贝，因为实现滚动机制时要改变文件名。
	FileLogAppender(FileLogAppender&& _value) noexcept ;

	void operator=(const FileLogAppender& _value) = delete;
	void operator=(FileLogAppender&& _value) noexcept;

	void log(const char* _massage) override;
	void clear();
	void open(const std::string& _filename);
	// inline std::string setFilename();      // 为了m_filename和m_fp相匹配，禁止用户直接改文件名，而是通过open去改文件名
	inline std::string getFilename();

	inline int getFileSize();
	void scrolling();   // 日志的滚动机制，要配合配置系统保存当前使用的文件名，才能实现续写功能！！！
};


// 日志事件
struct LogEvent
{



	time_t	     m_timestamp;
	std::string *m_loggername; // 申请内存了需要手动释放 
	LogLevel     m_LogLevel;
	const char  *m_filename;  
	int		     m_line;
	char        *m_content;    // 申请内存了需要手动释放 
	// std::list<std::shared_ptr<LogAppender>> *m_appenderList;    // 错误，因为logger会在LogEventManager之前销毁，该变量会无效。

	//LogEvent(const LogEvent& _logEvent) {
	//	m_timestamp = _logEvent.m_timestamp;
	//	m_timestamp = _logEvent.m_timestamp;
	//	m_line = _logEvent.m_line;
	//	m_filename = _logEvent.m_filename;
	//	m_content = _logEvent.m_content;
	//	m_LogLevel = _logEvent.m_LogLevel;
	//	m_loggername = _logEvent.m_loggername;
	//}
};


// 日志事件管理器
class LogEventManager
{
private:
	struct LogEventQueue
	{
		LogEvent m_data;
		LogEventQueue* m_next;
		LogEventQueue* m_prev;
	};
private:
	std::thread    m_thread;
	std::mutex     m_mutex;
	LogEventQueue *m_logEventQueue;
	LogEventQueue *m_ptrWrite;
	LogEventQueue *m_ptrRead;
	LogEventQueue *m_ptrDelete;
	bool m_isCanExit;
	std::map<std::string, std::list<std::shared_ptr<LogAppender>>> *m_LogAppenderListMap; // LogAppender是虚基类，赋值时无法调用派生类的拷贝或移动函数，因此LogAppender*。同时因为满足第三种内存管理情况，为了方便管理内存，能智能指针。
public:
	LogEventManager();
	~LogEventManager();

	LogEventManager(const LogEventManager& _value) = delete;
	LogEventManager(LogEventManager&& _value) noexcept = delete;

	LogEventManager& operator=(const LogEventManager& _value) = delete;
	LogEventManager& operator=(LogEventManager&& _value) noexcept = delete;
	
	void clearLogEventNodeData(LogEventQueue* _node);
	void clearLogEventQueue();

	void clearLogAppenderListMap();
	void addAppender(std::string _loggername, std::shared_ptr<LogAppender> _appender);   // 注意建议在创建线程之前用该函数，以免造成冲突的情况
	inline bool isFindLogger(const std::string& _loggername);

	void logFormatter(std::stringstream& _ss, LogEvent& _logEvent);
	void dealLogEvent_threadFuntion();
	void addLogEvent(LogEvent _logEvent);   // 该函数需要线程安全
};


// 日志器
class Logger
{
private:
	// LogEventManager defauleLogEventManager // 用全局的defauleLogEventManager
	std::string m_loggername; 
	LogLevel m_level;
public:
	Logger(const std::string& _loggername, LogLevel _level, std::shared_ptr<LogAppender> _appender);
	~Logger() = default;   // LogAppender的释放交给LogEventManager

	Logger(const Logger& _logger) = delete;
	Logger(Logger&& _logger) noexcept = delete;

	Logger& operator=(const Logger& _logger) = delete;
	Logger& operator=(Logger&& _logger) noexcept = delete;

	void log(LogLevel _level, const char* _format, va_list _args);
	void log(LogLevel _level, const char* _format, ...);
	void debug(const char* _format, ...);
	void info(const char* _format, ...);
	void warn(const char* _format, ...);
	void error(const char* _format, ...);
	void fatal(const char* _format, ...);



	// inline void setLoggerName(const std::string _loggerName);    // 禁止用户改 loggerName
	inline std::string getLoggerName();

	bool addAppender(std::shared_ptr<LogAppender> _appender);       // 注意建议在创建线程之前用该函数，以免造成冲突的情况
	// bool deleteAppender(....);                                   // 根据实际考量，暂时不实现deleteAppender功能

	inline LogLevel getLogLevel();
	inline void setLogLevel(LogLevel _level);
};


extern inline void debug(const char* _format, ...);   // 全局函数和变量会受到命名空间的约束，但宏不会
extern inline void info(const char* _format, ...);
extern inline void warn(const char* _format, ...);
extern inline void error(const char* _format, ...);
extern inline void fatal(const char* _format, ...);



};

};






#endif // !_MOCHEN_LOG_H_


