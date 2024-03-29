/*// 介绍与说明
* 1.该日志系统用的是：生产者-消费者多线程模型
* 
*		                 +--------------+
*		线程1 ---------->|              |   LogEventManager异步线程    
*		线程2 ---------->|  锁(限制写入) | -------------------------->
*		线程n ---------->|              |
*		                 +--------------+
* 
*	注意：由于LogEventManager是异步运行的，不会影响主线程，所以输出的日志有滞后性，且由于线程间的抢占，可能出现输出的日志信息是乱序的情况。
* 
* 2.LogEventManager中三个LogEventQueue指针实现无锁结构处理数据
*	(1).LogEventQueue是用自定义的双向链表实现的。
*   (2).read,write,delete 三个指针对应三种操作，这三种操作按先后顺序依次进行，先write后read再delete，
*	    且 read 可以等于 write, delete = read->prev，彼此之间不会冲突。
*   
*		 delete             read               write
*		   |                 |                   |
*		+--+--+       +------+-----+      +------+-----+ 
*		| ... | <---->| LogEvent01 |<---->| LogEvent02 | ----> nullptr 
*		+-----+       +------------+      +------------+ 
* 
* 3.注意Logger的Loggername是具有唯一性的，程序会维护该特性。
* 4.在使用该日志库时，不建议直接创建全局变量，而是将其封装在一个全局的函数中，函数返回static的Logger，这样更安全，更易于共享该Logger。
* 5.要配合配置系统保存当前使用的文件名，才能实现续写功能，不然一旦文件满了，每次启动程序都会发生文件滚动。
* 6.为了多线程下的安全性和输出的日志的稳定性，目前不支持根据loggername查找或删除logaddender
* 7.Logger::setUpSyncDebug 调用该函数后，只会由最先调用该函数的Logger进行同步的日志输出，其他Logger的输出会被忽略，建议写在Logger初始的构建函数中。
* 8.日志输出格式：[年-月-日 时:分:秒][日志器名称][日志等级][文件名]:[行号][日志信息]
* 9.文件滚动的命名格式：文件名_log_0000-00-00#00-00-00.txt
* 10.日志信息的描述格式：完整的函数名():错误类型, 对该问题的描述可以<变量名>更详细的描述该问题
*	常用的错误类型如下:
*		index out of range		下标越界
*		value out of range      数值越界
*		valid key               无效键值
*		null pointer            指针为空 
*		null [data]             数据为空
*		open [file] failed      打开文件失败
*		[data] type error       数据类型错误
*	    [data] must be unique   数据必须唯一
*
*/




/*// 还未解决的问题
* 1. 
* 
* 
*/


/*// 所学到的知识
* 1.
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

#include <Windows.h>
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


// 日志事件
struct LogEvent
{
	time_t	     m_timestamp;
	std::string *m_loggername; // 申请内存了需要手动释放 
	LogLevel     m_LogLevel;
	const char  *m_filename;
	int		     m_line;
	char        *m_content;    // 申请内存了需要手动释放 
};


// 格式化函数
typedef void (*PLogFormatFuntion)(std::stringstream& _ss, LogEvent& _logEvent) ;

// [年-月-日 时:分:秒][日志器名称][日志等级][文件名]:[行号][日志信息]
extern inline void logFormatFuntion_detailed(std::stringstream& _ss, LogEvent& _logEvent);

// [时:分:秒][日志等级][简要的文件名]:[行号][日志信息]
extern inline void logFormatFuntion_briefly(std::stringstream& _ss, LogEvent& _logEvent);


// 日志输出目的地
class LogAppender
{
protected:
	PLogFormatFuntion m_pLogFormatFuntion;
public:
	LogAppender();
	virtual ~LogAppender() {}                    // 虚析构函数需要实现函数的定义，如果只写函数说明，可能出现链接错误

	virtual void log(const char* _massage) = 0;  // 纯虚函数
	PLogFormatFuntion getFormatFuntion();
	void setFormatFuntion(PLogFormatFuntion _pLogFormatFuntion);
};


class ConsoleLogAppender : public LogAppender
{
public:
	ConsoleLogAppender();
	ConsoleLogAppender(PLogFormatFuntion _pLogFormatFuntion);
	void log(const char* _massage) override;
};


class FileLogAppender : public LogAppender
{
private:
	std::string m_filename;
	FILE* m_fp;
	int m_maxSize;
#define M_FILEMAXSIZE 409600
public:
	FileLogAppender();
	FileLogAppender(const std::string& _filename, int _maxSize = M_FILEMAXSIZE);
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
	inline void setFileSize(int _size);
	void scrolling();   // 日志的滚动机制，要配合配置系统保存当前使用的文件名，才能实现续写功能！！！
};


// 日志事件管理器
class LogEventManager
{
private:
	struct LogEventQueue
	{
		LogEvent m_data;
		LogEventQueue* m_next;
	};
private:
	std::thread    m_thread;
	std::mutex     m_mutex;
	LogEventQueue* m_logEventQueue;
	LogEventQueue* m_ptrWrite;   // 原子操作的对象
	LogEventQueue* m_ptrRead;    // 原子操作的对象
	bool m_isCanExit;
	std::map<std::string, std::list<std::shared_ptr<LogAppender>>>* m_LogAppenderListMap; // LogAppender是虚基类，赋值时无法调用派生类的拷贝或移动函数，因此LogAppender*。同时因为满足第三种内存管理情况，为了方便管理内存，能智能指针。
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
	void addAppender(std::string _loggername, std::shared_ptr<LogAppender> _appender);   // 注意该数由Logger::addAppender调用，空间是独立的，因此不会线程安全问题
	inline bool isFindLogger(const std::string& _loggername);
	inline std::map<std::string, std::list<std::shared_ptr<LogAppender>>>* getLogAppenderListMap();

	void dealLogEvent_threadFuntion();
	void addLogEvent(LogEvent _logEvent);   // 该函数需要线程安全
};

// 声明全局函数，代替在头文件中声明全局变量，且返回的指针类型，避免调用拷贝或移动相关函数
extern inline LogEventManager* getDefaultLogEventManager();


// 日志器
class Logger
{
private:
	static std::string *m_syncDebugSetting;   // 存 m_loggername 的地址。原子操作对象
	std::mutex m_mutex;
	std::string m_loggername; 
	LogLevel m_level;
public:
	Logger(const std::string& _loggername, LogLevel _level = LogLevel::debug, std::shared_ptr<LogAppender> _appender = std::make_shared<ConsoleLogAppender>());   // 注意：虽然DefaultLogAppender先被创建，但它的资源释放由LogEventManager控制
	~Logger() = default;   // LogAppender的释放交给LogEventManager

	Logger(const Logger& _logger) = delete;
	Logger(Logger&& _logger) noexcept = delete;

	Logger& operator=(const Logger& _logger) = delete;
	Logger& operator=(Logger&& _logger) noexcept = delete;

	bool setUpSyncDebug();
	void log(LogLevel _level,const char* _filename, int _line, const char* _format, ...);

	// inline void setLoggerName(const std::string _loggerName);    // 禁止用户改 loggerName
	inline std::string getLoggerName();

	bool addAppender(std::shared_ptr<LogAppender> _appender);       // 注意建议在创建线程之前用该函数，以免造成冲突的情况
	// bool deleteAppender(....);                                   // 根据实际考量，暂时不实现deleteAppender功能

	inline LogLevel getLogLevel();
	inline void setLogLevel(LogLevel _level);
};

// 声明全局函数，代替在头文件中声明全局变量，且返回的指针类型，避免调用拷贝或移动相关函数
extern inline Logger* getDefaultLogger();


// 注意因为宏函数要在其他文件中展开，所以函数中所有用到的函数或者变量，都要加上完整的命名空间名
#define default_debug(_format, ...) \
	mochen::log::getDefaultLogger()->log(mochen::log::LogLevel::debug, __FILE__, __LINE__, _format, ##__VA_ARGS__)

#define default_info(_format, ...) \
	mochen::log::getDefaultLogger()->log(mochen::log::LogLevel::info, __FILE__, __LINE__, _format, ##__VA_ARGS__)

#define default_warn(_format, ...) \
	mochen::log::getDefaultLogger()->log(mochen::log::LogLevel::warn, __FILE__, __LINE__, _format, ##__VA_ARGS__)

#define default_error(_format, ...) \
	mochen::log::getDefaultLogger()->log(mochen::log::LogLevel::error, __FILE__, __LINE__, _format, ##__VA_ARGS__)

#define default_fatal(_format, ...) \
	mochen::log::getDefaultLogger()->log(mochen::log::LogLevel::fatal, __FILE__, __LINE__, _format, ##__VA_ARGS__)


#define logger_debug(_logger, _format, ...) \
	(_logger)->log(mochen::log::LogLevel::debug, __FILE__, __LINE__, _format, ##__VA_ARGS__)

#define logger_info(_logger, _format, ...) \
	(_logger)->log(mochen::log::LogLevel::info, __FILE__, __LINE__, _format, ##__VA_ARGS__)

#define logger_warn(_logger, _format, ...) \
	(_logger)->log(mochen::log::LogLevel::warn, __FILE__, __LINE__, _format, ##__VA_ARGS__)

#define logger_error(_logger, _format, ...) \
	(_logger)->log(mochen::log::LogLevel::error, __FILE__, __LINE__, _format, ##__VA_ARGS__)

#define logger_fatal(_logger, _format, ...) \
	(_logger)->log(mochen::log::LogLevel::fatal, __FILE__, __LINE__, _format, ##__VA_ARGS__)



};

};



#endif // !_MOCHEN_LOG_H_


