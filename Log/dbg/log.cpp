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

void ConsoleLogAppender::log(const char* _massage)
{
	printf("%s", _massage);
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

	m_LogAppenderListMap = new std::map<std::string, std::list<std::shared_ptr<LogAppender>>*>{};
}


LogEventManager::~LogEventManager()
{
	m_isCanExit = true;         // ע�� m_isCanExit = true �Ҵ�������־�������������ݺ�����˳�
	m_thread.join();            // �ȴ���������־��������������
	clearLogEventQueue();       
	clearLogAppenderListMap();
}



void LogEventManager::clearLogEventNode(LogEventQueue* _node)
{
	free(_node->m_data.m_content);        // �ͷ��������Դ
	free(_node);
}

void LogEventManager::clearLogEventQueue()
{
	LogEventQueue* tempNode = nullptr;

	while (m_logEventQueue != nullptr) {
		tempNode = m_logEventQueue;
		m_logEventQueue = m_logEventQueue->m_next;

		free(tempNode->m_data.m_content);  // �ͷ��������Դ
		free(tempNode);
	}
}


void LogEventManager::clearLogAppenderListMap()
{
	for (auto mapIt = m_LogAppenderListMap->begin(); mapIt != m_LogAppenderListMap->end(); ++mapIt) {
		// list = mapIt->second
		for (auto vecterIt = mapIt->second->begin(); vecterIt != mapIt->second->end(); ++vecterIt) {
			vecterIt->reset();   // �ͷ�����ָ��
		}
		delete mapIt->second;
	}
	delete m_LogAppenderListMap;
}


void LogEventManager::addAppender(std::string _loggername, std::shared_ptr<LogAppender> _appender)
{
	(*m_LogAppenderListMap)[_loggername]->push_back(_appender);
}


inline bool LogEventManager::isFindLogger(const std::string& _loggername)
{
	return (m_LogAppenderListMap->find(_loggername) != m_LogAppenderListMap->end());
}


inline void LogEventManager::logFormatter(std::stringstream& _ss, LogLevel _level, LogEvent& _logEvent)
{
	char timeString[24] = { 0 };
	struct tm* pt = nullptr;     // ע�� pt ָ����ڴ�ռ���ϵͳ���������ֶ��ͷ�

	// ����ʱ��
	pt = localtime(&_logEvent.m_timestamp);
	// [0000-00-00 00:00:00]   // ��Ϊÿ��д���ֵ�ĳ��ȶ��ǹ̶��ģ����Բ������ timeString
	sprintf(timeString, "%d-%0.2d-%0.2d %0.2d:%0.2d:%0.2d",
		pt->tm_year + 1900,
		pt->tm_mon + 1,
		pt->tm_mday,
		pt->tm_hour,
		pt->tm_min,
		pt->tm_sec);

	// [��-��-�� ʱ:��:��][��־������][��־�ȼ�][�ļ���]:[�к�][��־��Ϣ]
	_ss << "[" << timeString << "]";
	_ss << "[" << _logEvent.m_loggername << "]";
	_ss << "[" << logLevelString[(int)_level] << "]";
 	_ss << "[" << _logEvent.m_filename << "]";
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


	while (m_isCanExit == false || m_ptrRead->m_next != nullptr)  // ע�� m_isCanExit = true �Ҵ�������־�������������ݺ�����˳�
	{
		if (m_ptrRead->m_next != nullptr) 
		{
			tempNode = m_ptrRead->m_next;
			m_ptrRead = m_ptrRead->m_next;

			tempData = tempNode->m_data;
			tempList = (*m_LogAppenderListMap)[tempData.m_loggername];

			for(auto it = tempList->begin(); it != tempList->end(); ++it) {
				logFormatter(ss, it->get()->getLevel(), tempData);    // ������ (*(*it)).getType();
				it->get()->log(ss.str().c_str());
			}

			if (m_ptrDelete->m_next != m_ptrRead) {
				tempNode = m_ptrDelete->m_next;
				tempNode->m_prev->m_next = tempNode->m_next;
				tempNode->m_next->m_prev = tempNode->m_prev;
				clearLogEventNode(tempNode);
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
	++m_ptrWrite;

	m_mutex.unlock();
}


// =============================================================================================================
// class Logger

Logger::Logger(const std::string& _loggername, LogLevel _level, std::shared_ptr<LogAppender> _appender)
{
	if (defauleLogEventManager.isFindLogger(_loggername) == true) {
		throw std::logic_error("The _loggername already exists");
	}
	if ((int)_level > (int)_appender.get()->getType()) {
		throw std::logic_error("The Appender's LogLevel less than the logger's LogLevel");
	}

	m_loggername = _loggername;
	defauleLogEventManager.addAppender(_loggername, _appender);
}


void Logger::log(LogLevel _level, const char* _format, ...)
{
	if ((int)m_level < (int)_level) {
		return;
	}
	
	
	va_list args;
	va_start(args, _format);



	int size = vsnprintf(nullptr, 0, _format, args);
	char* buffer = (char*)malloc(sizeof(char) * (size + 1));    // +1�����β��"\0"


}




void Logger::debug(const char* _format, ...){}
void Logger::info(const char* _format, ...) {}
void Logger::warn(const char* _format, ...) {}
void Logger::error(const char* _format, ...) {}
void Logger::fatal(const char* _format, ...) {}

void Logger::set_loggername() {}
void Logger::set_level(LogLevel _level) {}

bool Logger::add_appender(const LogAppender& _value) {}
bool Logger::remove_appender() {}
std::list<LogAppender>& Logger::get_appender_list() {}





void debug(const char* _format, ...);
void info(const char* _format, ...);
void warn(const char* _format, ...);
void error(const char* _format, ...);
void fatal(const char* _format, ...);
