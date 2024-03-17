
/*// ��δ���������
* 1.throw std::logic_error("type error, not int value");
*
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
* 2. STL�������ʱ����Ϊ�޷�����������Ŀ������ƶ���������˴�������ָ�롣
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
protected:
	LogLevel m_level;
public:
	LogAppender(LogLevel _level = LogLevel::debug);
	virtual ~LogAppender() {}                    // ������������Ҫʵ�ֺ����Ķ��壬���ֻд����˵�������ܳ������Ӵ���

	virtual void log(const char* _message) = 0;  // ���麯��
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


// ��־�¼�
struct LogEvent
{
	unsigned long long			 m_timestamp;
	const char					*m_loggername;
	const char					*m_filename;
	int						     m_line;
	const char                  *m_content;
	std::list<LogAppender::ptr> *m_appender_list;
};


// ��־�¼�������
class LogEventManager
{
private:
	// LogFormatter ��������
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



// ��־��
class Logger
{
private:
	std::string m_loggername;
	std::list<LogAppender::ptr> *m_appender_list;    // LogAppender������࣬��ֵʱ�޷�����������Ŀ������ƶ����������LogAppender*��ͬʱ��Ϊ����������ڴ���������Ϊ�˷�������ڴ棬������ָ�롣
public:
	Logger(const std::string& _loggername);
	Logger(const std::string& _loggername, LogAppender::ptr _appender_ptr);    // Ϊʲô����std::list<LogAppender::ptr>&
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

