
/*// ��δ���������
* 1.throw std::logic_error("type error, not int value");
* 2.��־�Ĺ������ƣ�Ҫ�������ϵͳ���浱ǰʹ�õ��ļ���������ʵ����д���ܣ�����
* 3.void addAppender(std::string _loggername, std::shared_ptr<LogAppender> _appender);   // ע�⽨���ڴ����߳�֮ǰ�øú�����������ɳ�ͻ�����
*
*/


/*// ��ѧ����֪ʶ
* 1.C++���׳��쳣���ʹ����쳣�ķ�����
* 
		void checkIntValue(int value) {
			if (value != static_cast<int>(value)) {
				throw std::logic_error("type error, not int value");   // std::logic_error�쳣�����л��Զ������ļ������кŵȵ�����Ϣ
			}
		}
		
		int main() {
			try {
				checkIntValue(10); // ��ȷ������ֵ
				checkIntValue(10.5); // ������ֵ�����׳��쳣
			} catch (const std::logic_error& e) {
				 std::cerr << "Exception caught at file " << __FILE__ << " line " << __LINE__ << ": " << e.what() << std::endl;
			}
		
			return 0;
		}
* 
* 2.STL�������ʱ����Ϊ�޷�����������Ŀ������ƶ���������˴�������ָ�롣
* 3.����ָ��ͨ����ֵ������Ϊ��������
* 4.ȫ�ֺ����ͱ������ܵ������ռ��Լ�������겻��
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

// ��־����
enum class LogLevel
{
	debug = 0,
	info,
	warn,
	error,
	fatal
};

// ��־���Ŀ�ĵ�
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
	virtual ~LogAppender() {}                    // ������������Ҫʵ�ֺ����Ķ��壬���ֻд����˵�������ܳ������Ӵ���

	virtual void log(const char* _massage) = 0;  // ���麯��
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

	FileLogAppender(const FileLogAppender& _value) = delete;  // ���ܿ�������Ϊʵ�ֹ�������ʱҪ�ı��ļ�����
	FileLogAppender(FileLogAppender&& _value) noexcept ;

	void operator=(const FileLogAppender& _value) = delete;
	void operator=(FileLogAppender&& _value) noexcept;

	void log(const char* _massage) override;
	void clear();
	void open(const std::string& _filename);
	// inline std::string setFilename();      // Ϊ��m_filename��m_fp��ƥ�䣬��ֹ�û�ֱ�Ӹ��ļ���������ͨ��openȥ���ļ���
	inline std::string getFilename();

	inline int getFileSize();
	void scrolling();   // ��־�Ĺ������ƣ�Ҫ�������ϵͳ���浱ǰʹ�õ��ļ���������ʵ����д���ܣ�����
};


// ��־�¼�
struct LogEvent
{



	time_t	     m_timestamp;
	std::string *m_loggername; // �����ڴ�����Ҫ�ֶ��ͷ� 
	LogLevel     m_LogLevel;
	const char  *m_filename;  
	int		     m_line;
	char        *m_content;    // �����ڴ�����Ҫ�ֶ��ͷ� 
	// std::list<std::shared_ptr<LogAppender>> *m_appenderList;    // ������Ϊlogger����LogEventManager֮ǰ���٣��ñ�������Ч��

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


// ��־�¼�������
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
	std::map<std::string, std::list<std::shared_ptr<LogAppender>>> *m_LogAppenderListMap; // LogAppender������࣬��ֵʱ�޷�����������Ŀ������ƶ����������LogAppender*��ͬʱ��Ϊ����������ڴ���������Ϊ�˷�������ڴ棬������ָ�롣
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
	void addAppender(std::string _loggername, std::shared_ptr<LogAppender> _appender);   // ע�⽨���ڴ����߳�֮ǰ�øú�����������ɳ�ͻ�����
	inline bool isFindLogger(const std::string& _loggername);

	void logFormatter(std::stringstream& _ss, LogEvent& _logEvent);
	void dealLogEvent_threadFuntion();
	void addLogEvent(LogEvent _logEvent);   // �ú�����Ҫ�̰߳�ȫ
};


// ��־��
class Logger
{
private:
	// LogEventManager defauleLogEventManager // ��ȫ�ֵ�defauleLogEventManager
	std::string m_loggername; 
	LogLevel m_level;
public:
	Logger(const std::string& _loggername, LogLevel _level, std::shared_ptr<LogAppender> _appender);
	~Logger() = default;   // LogAppender���ͷŽ���LogEventManager

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



	// inline void setLoggerName(const std::string _loggerName);    // ��ֹ�û��� loggerName
	inline std::string getLoggerName();

	bool addAppender(std::shared_ptr<LogAppender> _appender);       // ע�⽨���ڴ����߳�֮ǰ�øú�����������ɳ�ͻ�����
	// bool deleteAppender(....);                                   // ����ʵ�ʿ�������ʱ��ʵ��deleteAppender����

	inline LogLevel getLogLevel();
	inline void setLogLevel(LogLevel _level);
};


extern inline void debug(const char* _format, ...);   // ȫ�ֺ����ͱ������ܵ������ռ��Լ�������겻��
extern inline void info(const char* _format, ...);
extern inline void warn(const char* _format, ...);
extern inline void error(const char* _format, ...);
extern inline void fatal(const char* _format, ...);



};

};






#endif // !_MOCHEN_LOG_H_


