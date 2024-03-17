
/*// ��δ���������
* 1.throw std::logic_error("type error, not int value");
* 2.��־�Ĺ������ƣ�Ҫ�������ϵͳ���浱ǰʹ�õ��ļ���������ʵ����д���ܣ�����
*
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


// ��־����
enum class LogLevel
{
	debug = 0,
	info,
	warn,
	error,
	fatal
};

const char* logLevelString[5] = { "debug","info","warn","error","fatal" };



// ��־���Ŀ�ĵ�
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
	virtual ~LogAppender() {}                    // ������������Ҫʵ�ֺ����Ķ��壬���ֻд����˵�������ܳ������Ӵ���

	virtual void log(const char* _message) = 0;  // ���麯��
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

	FileLogAppender(const FileLogAppender& _value) = delete;  // ���ܿ�������Ϊʵ�ֹ�������ʱҪ�ı��ļ�����
	FileLogAppender(FileLogAppender&& _value) noexcept ;

	void operator=(const FileLogAppender& _value) = delete;
	void operator=(FileLogAppender&& _value) noexcept;

	void log(const char* _message) override;
	void clear();
	void open(const std::string& _filename);
	inline std::string getFilename();

	inline int getFileSize();
	void scrolling();   // ��־�Ĺ������ƣ�Ҫ�������ϵͳ���浱ǰʹ�õ��ļ���������ʵ����д���ܣ�����
};


// ��־�¼�
struct LogEvent
{
	unsigned long long			 m_timestamp;
	const char					*m_loggername;
	const char					*m_filename;
	int						     m_line;
	const char                  *m_content;
	std::list<LogAppender::ptr> *m_appenderList;
};


// ��־�¼�������
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



// ��־��
class Logger
{
private:
	LogEventManager::ptr m_logEventManager;   
	std::string m_loggerName;
	std::list<LogAppender::ptr> *m_appenderList;    // LogAppender������࣬��ֵʱ�޷�����������Ŀ������ƶ����������LogAppender*��ͬʱ��Ϊ����������ڴ���������Ϊ�˷�������ڴ棬������ָ�롣
public:
	Logger(const std::string& _loggername);
	Logger(const std::string& _loggername, LogAppender::ptr _appender_ptr);    // ע������ָ��ͨ����ֵ������Ϊ��������
	// �б��ʼ��????


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

	inline void setLogEventManager(LogEventManager::ptr _logEventManager);   // ע������ָ��ͨ����ֵ������Ϊ��������
	inline LogEventManager* getLogEventManager();
	inline bool clearLogEventManager();      // �����ͷ�Ĭ�ϵ�LogEventManager

	inline void setLoggerName(const std::string _loggerName);
	inline std::string getLoggerName();

	bool addAppender(const LogAppender& _value);
	inline std::list<LogAppender>* getAppenderList();   // �Լ�ȡʵ�� removeAppender
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


// LogEventManager logEventManager{};  // ��ȫ�������� LogEventManager

inline LogEventManager* getDefauleLogEventManager()
{
	// return &logEventManager;    // ����ȫ�ֵ� LogEventManager
}



inline void debug(const char* _format, ...);
inline void info(const char* _format, ...);
inline void warn(const char* _format, ...);
inline void error(const char* _format, ...);
inline void fatal(const char* _format, ...);









};

};



#endif // !_MOCHEN_LOG_H_

