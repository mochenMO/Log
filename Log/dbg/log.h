
/*// 还未解决的问题
* 1.throw std::logic_error("type error, not int value");
* 2.日志的滚动机制，要配合配置系统保存当前使用的文件名，才能实现续写功能！！！
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
* 2.STL存虚基类时，因为无法调用派生类的拷贝或移动函数，因此存虚基类的指针。
* 3.智能指针通常以值类型作为参数类型
* 
* 
* 
* 
*/



#pragma once
#ifndef _MOCHEN_LOG_H_
#define _MOCHEN_LOG_H_


#include <string>
#include <list>
#include <stdexcept>  // throw std::logic_error 
#include <memory>     // std::share_ptr
#include <thread>
#include <mutex>



// #include <Windows.h>


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
public:
	enum class Type
	{
		withoutLogAppender = 0,
		ConsoleLogAppender,
		FileLogAppender
	};
protected:
	Type m_type;
	LogLevel m_level;
public:
	LogAppender();
	virtual ~LogAppender() {}                    // 虚析构函数需要实现函数的定义，如果只写函数说明，可能出现链接错误

	virtual void log(const char* _message) = 0;  // 纯虚函数
	inline LogLevel getLevel();
	inline void setLevel(LogLevel _level);
	inline Type getType();
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
	int m_maxSize;
public:
	FileLogAppender();
	FileLogAppender(const std::string& _filename, int _maxSize = 1024, LogLevel _level = LogLevel::debug);
	~FileLogAppender();

	FileLogAppender(const FileLogAppender& _value) = delete;  // 不能拷贝，因为实现滚动机制时要改变文件名。
	FileLogAppender(FileLogAppender&& _value) noexcept ;

	void operator=(const FileLogAppender& _value) = delete;
	void operator=(FileLogAppender&& _value) noexcept;

	void log(const char* _message) override;
	void clear();
	void open(const std::string& _filename);
	inline std::string getFilename();

	inline int getFileSize();
	void scrolling();   // 日志的滚动机制，要配合配置系统保存当前使用的文件名，才能实现续写功能！！！
};


// 日志事件
struct LogEvent
{
	unsigned long long			 m_timestamp;
	const char					*m_loggername;
	const char					*m_filename;
	int						     m_line;
	const char                  *m_content;
	std::list<LogAppender::ptr> *m_appenderList;
};


// 日志事件管理器
class LogEventManager
{
public:
	using ptr = std::shared_ptr<LogEventManager>;
private:
	struct LogEventQueue
	{
		LogEvent m_logEvent;
		LogEventQueue* next;
		LogEventQueue* prev;
	};
private:
	std::thread m_thread;
	std::mutex m_mutex;
	LogEventQueue* m_logEventQueue;
	LogEventQueue* m_ptrWrite;
	LogEventQueue* m_ptrRead;
	LogEventQueue* m_ptrDelete;
	bool isCanExit;

public:
	LogEventManager();
	~LogEventManager();

	LogEventManager(const LogEventManager& _value) = delete;
	LogEventManager(LogEventManager&& _value) noexcept = delete;

	LogEventManager& operator=(const LogEventManager& _value) = delete;
	LogEventManager& operator=(LogEventManager&& _value) noexcept = delete;

	void clear();
	void dealLogEvent_threadFuntion();
	void addLogEvent(LogEvent _logEvent);
};



// 日志器
class Logger
{
private:
	LogEventManager::ptr m_logEventManager;   
	std::string m_loggerName;
	std::list<LogAppender::ptr> *m_appenderList;    // LogAppender是虚基类，赋值时无法调用派生类的拷贝或移动函数，因此LogAppender*。同时因为满足第三种内存管理情况，为了方便管理内存，能智能指针。
public:
	Logger(const std::string& _loggername);
	Logger(const std::string& _loggername, LogAppender::ptr _appender_ptr);    // 注意智能指针通常以值类型作为参数类型
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

	inline void setLogEventManager(LogEventManager::ptr _logEventManager);   // 注意智能指针通常以值类型作为参数类型
	inline LogEventManager* getLogEventManager();
	inline bool clearLogEventManager();      // 不会释放默认的LogEventManager

	inline void setLoggerName(const std::string _loggerName);
	inline std::string getLoggerName();

	bool addAppender(const LogAppender& _value);
	inline std::list<LogAppender>* getAppenderList();   // 自己取实现 removeAppender
};




inline LogAppender::ptr getDefauleLogAppender()
{
	static LogAppender::ptr defauleLogAppender = std::make_shared<ConsoleLogAppender>();
	return defauleLogAppender;
}

inline Logger* getDefauleLogger()
{
	static Logger defauleLogger("defaule", getDefauleLogAppender());
	return &defauleLogger;
}


// LogEventManager logEventManager{};  // 在全局区创建 LogEventManager

inline LogEventManager* getDefauleLogEventManager()
{
	// return &logEventManager;    // 返回全局的 LogEventManager
}



inline void debug(const char* _format, ...);
inline void info(const char* _format, ...);
inline void warn(const char* _format, ...);
inline void error(const char* _format, ...);
inline void fatal(const char* _format, ...);









};

};



#endif // !_MOCHEN_LOG_H_

