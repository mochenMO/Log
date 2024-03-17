#include "log.h"

using namespace mochen::log;


// =============================================================================================================
// class LogAppender

LogAppender::LogAppender() : m_level(LogLevel::debug), m_type(Type::withoutLogAppender)
{    

}


inline LogLevel LogAppender::getLevel()
{
	return m_level;
}


inline void LogAppender::setLevel(LogLevel _level)
{
	m_level = _level;
}

inline LogAppender::Type LogAppender::getType()
{
	return m_type;
}




// =============================================================================================================
// class ConsoleLogAppender

ConsoleLogAppender::ConsoleLogAppender(LogLevel _level)
{
	m_level = _level;
	m_type = Type::ConsoleLogAppender;
}

void ConsoleLogAppender::log(LogEvent _event)
{
	// [������ʱ����][��־������][��־�ȼ�][��־�ȼ�][�ļ���][][]
	//printf("%s", );
}


// =============================================================================================================
// class FileLogAppender

FileLogAppender::FileLogAppender()
{
	m_level = LogLevel::debug;
	m_type = Type::FileLogAppender;
	m_filename = "";
	m_fp = nullptr;
	m_maxSize = 1024;
}


FileLogAppender::FileLogAppender(const std::string& _filename, int _maxSize, LogLevel _level)
{
	m_level = _level;
	m_type = Type::FileLogAppender;
	m_maxSize = 1024;
	m_filename = _filename;

	std::string temp = _filename;
	temp += ".txt";

	if ((m_fp = fopen(temp.c_str(), "a")) == nullptr) {
		throw std::logic_error("filded to open the file");    // std::logic_error�쳣�����л��Զ������ļ������кŵȵ�����Ϣ
	}
}


FileLogAppender::~FileLogAppender()
{
	clear();
}


FileLogAppender::FileLogAppender(FileLogAppender&& _value) noexcept
{
	m_level = _value.m_level;
	m_filename = _value.m_filename;
	m_fp = _value.m_fp;
	
	_value.m_level = LogLevel::debug;
	_value.m_filename = "";
	_value.m_fp = nullptr;
}


void FileLogAppender::operator=(FileLogAppender&& _value) noexcept
{
	clear();

	m_level = _value.m_level;
	m_filename = _value.m_filename;
	m_fp = _value.m_fp;

	_value.m_level = LogLevel::debug;
	_value.m_filename = "";
	_value.m_fp = nullptr;
}

void FileLogAppender::log(const char* _message)
{
	if (m_fp == nullptr) {
		throw std::logic_error("There is no initialization m_file, m_file is nullptr");
	}

	if (getFileSize() >= m_maxSize) {
		scrolling();  // ����
	}

	fwrite(_message, strlen(_message), 1, m_fp);
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
		throw std::logic_error("filded to open the file");    // std::logic_error�쳣�����л��Զ������ļ������кŵȵ�����Ϣ
	}
}


inline std::string FileLogAppender::getFilename()
{
	return m_filename;
}


inline int FileLogAppender::getFileSize()
{
	int size = 0;
	fseek(m_fp, 0, SEEK_END);    // �ƶ��ļ�ָ�뵽�ļ���β
	size = ftell(m_fp);          // ��ȡ��ǰ�ļ�ָ���λ�ã����ļ���С����λ�ֽ�
	fseek(m_fp, 0, SEEK_SET);    // �ƶ��ļ�ָ�뵽�ļ���ͷ  ����������������������������������
	return size;
}

void FileLogAppender::scrolling()
{
	if (m_fp != nullptr) {   // ����û��ʵ��setFilename���������� m_fp != nullptr ʱ m_filename != ""
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
	struct tm* pt = localtime(&timestamp);    // ע�� pt ָ����ڴ�ռ���ϵͳ���������ֶ��ͷ�

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
		throw std::logic_error("filded to open the file");    // std::logic_error�쳣�����л��Զ������ļ������кŵȵ�����Ϣ
	}
}



// =============================================================================================================
// class LogEventManager

LogEventManager::LogEventManager() : 
	m_thread(&LogEventManager::dealLogEvent_threadFuntion, this),  // ע��ʹ�ó�Ա����ָ��ʱҪ��������
	m_isCanExit(false)
{
	m_logEventQueue = (LogEventQueue*)malloc(sizeof(LogEventQueue));
	m_logEventQueue->m_next = nullptr;
	m_logEventQueue->m_prev = nullptr;

	m_ptrWrite = m_logEventQueue;
	m_ptrRead = m_logEventQueue;
	m_ptrDelete = m_logEventQueue;
}


LogEventManager::~LogEventManager()
{
	m_isCanExit = true;  // ע�� m_isCanExit = true �Ҵ�������־�������������ݺ�����˳�
	m_thread.join();
	clearLogEventQueue();
}



void LogEventManager::clearLogEventNode(LogEventQueue* _node)
{
	free(_node->m_data.m_content);
	free(_node);
}

void LogEventManager::clearLogEventQueue()
{
	LogEventQueue* temp = nullptr;

	while (m_logEventQueue != nullptr) {
		temp = m_logEventQueue;
		m_logEventQueue = m_logEventQueue->m_next;
		free(temp);
	}
}

void LogEventManager::dealLogEvent_threadFuntion()
{
	LogEventQueue* temp = nullptr;
	LogEvent tempData = { 0 };
	char timeString[24] = { 0 };   
	struct tm* pt = nullptr;     // ע�� pt ָ����ڴ�ռ���ϵͳ���������ֶ��ͷ�

	while (m_isCanExit == false || m_ptrRead->m_next != nullptr)  // ע�� m_isCanExit = true �Ҵ�������־�������������ݺ�����˳�
	{
		if (m_ptrRead->m_next != nullptr) 
		{
			temp = m_ptrRead->m_next;
			m_ptrRead = m_ptrRead->m_next;
			tempData = temp->m_data;

			// ����ʱ��
			pt = localtime(&tempData.m_timestamp);
			// [0000-00-00 00:00:00]   // ��Ϊÿ��д���ֵ�ĳ��ȶ��ǹ̶��ģ����Բ������ timeString
			sprintf(timeString, "[%d-%0.2d-%0.2d %0.2d:%0.2d:%0.2d]",
				pt->tm_year + 1900,
				pt->tm_mon + 1,
				pt->tm_mday,
				pt->tm_hour,
				pt->tm_min,
				pt->tm_sec);


			for(auto it = temp->m_data.m_appenderList->begin(); it != temp->m_data.m_appenderList->end(); ++it) {
				
				it->get()->log("asdsad");

				
			
			}




		}
	
	
	
	
	}


}




void LogEventManager::addLogEvent(LogEvent _logEvent);


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
