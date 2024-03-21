#include "log.h"

using namespace mochen::log;

// =============================================================================================================
// 全局区

// 创建全局的 defauleLogEventManager (注意：定义全局函数时也要加上命名空间)
inline LogEventManager* mochen::log::getDefaultLogEventManager() // 注意在C++11及以上版本标准中，规定静态局部变量初始化的线程安全性
{
	static LogEventManager defauleLogEventManager{};
	return &defauleLogEventManager;
}

// 全局函数
inline Logger* mochen::log::getDefaultLogger()
{
	static Logger defauleLogger("defauleLogger", LogLevel::debug, std::make_shared<ConsoleLogAppender>());
	return &defauleLogger;
}


// 声明全局的 logLevelString
const char* logLevelString[5] = { "debug","info","warn","error","fatal" };


// =============================================================================================================
// 格式化函数


// [年-月-日 时:分:秒][日志器名称][日志等级][文件名]:[行号][日志信息]
extern inline void mochen::log::logFormatFuntion_detailed(std::stringstream& _ss, LogEvent& _logEvent)
{
	char timeString[24] = { 0 };
	struct tm* pt = nullptr;     // 注意 pt 指向的内存空间由系统管理，无需手动释放

	// 处理时间
	pt = localtime(&_logEvent.m_timestamp);
	// [0000-00-00 00:00:00]
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


// [时:分:秒][日志等级][简要的文件名]:[行号][日志信息]
extern inline void mochen::log::logFormatFuntion_briefly(std::stringstream& _ss, LogEvent& _logEvent)
{
	char timeString[24] = { 0 };
	struct tm* pt = nullptr;     // 注意 pt 指向的内存空间由系统管理，无需手动释放

	// 提取文件名
	const char* temp = _logEvent.m_filename + strlen(_logEvent.m_filename);   // 把 temp 移到 _logEvent.m_filename 的结尾
	while (temp != _logEvent.m_filename) {
		if (*temp == '\\') {
			++temp;
			break;
		} 
		--temp;
	}

	// 处理时间
	pt = localtime(&_logEvent.m_timestamp);
	// [00:00:00]
	sprintf(timeString, "%0.2d:%0.2d:%0.2d",
		pt->tm_hour,
		pt->tm_min,
		pt->tm_sec);

	// [时:分:秒][日志等级][简要的文件名]:[行号][日志信息]
	_ss << "[" << timeString << "]";
	_ss << "[" << logLevelString[(int)_logEvent.m_LogLevel] << "]";
	_ss << "[" << temp << "]:";
	_ss << "[" << _logEvent.m_line << "]";
	_ss << "[" << _logEvent.m_content << "]";
	_ss << "\n";
}


// =============================================================================================================
// class LogAppender

LogAppender::LogAppender()
{
	m_pLogFormatFuntion = &logFormatFuntion_briefly;
}

PLogFormatFuntion LogAppender::getFormatFuntion()
{
	return m_pLogFormatFuntion;
}

void LogAppender::setFormatFuntion(PLogFormatFuntion _pLogFormatFuntion)
{
	m_pLogFormatFuntion = _pLogFormatFuntion;
}



// =============================================================================================================
// class ConsoleLogAppender

ConsoleLogAppender::ConsoleLogAppender()
{
	m_pLogFormatFuntion = &logFormatFuntion_briefly;
}

ConsoleLogAppender::ConsoleLogAppender(PLogFormatFuntion _pLogFormatFuntion)
{
	m_pLogFormatFuntion = _pLogFormatFuntion;
}

void ConsoleLogAppender::log(const char* _massage)
{
	printf("%s", _massage);
}


// =============================================================================================================
// class FileLogAppender

FileLogAppender::FileLogAppender()
{
	m_pLogFormatFuntion = &logFormatFuntion_detailed;
	m_filename = "";
	m_fp = nullptr;
	m_maxSize = M_FILEMAXSIZE;
}


FileLogAppender::FileLogAppender(const std::string& _filename, int _maxSize)
{
	m_pLogFormatFuntion = &logFormatFuntion_detailed;
	m_maxSize = _maxSize;
	m_filename = _filename;

	std::string temp = _filename;
	temp += ".txt";

	if ((m_fp = fopen(temp.c_str(), "a")) == nullptr) {
		throw std::logic_error("FileLogAppender::FileLogAppender(): open file filded");    // std::logic_error异常对象中会自动包含文件名和行号等调试信息
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
		throw std::logic_error("FileLogAppender::log(): null pointer, <m_fp> is nullptr");
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
		throw std::logic_error("FileLogAppender::open(): open file filded");    // std::logic_error异常对象中会自动包含文件名和行号等调试信息
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
	fseek(m_fp, 0, SEEK_SET);    // 移动文件指针到文件开头
	return size;
}

inline void FileLogAppender::setFileSize(int _size)
{
	m_maxSize = _size;
}

void FileLogAppender::scrolling()
{
	if (m_fp != nullptr) {   // 由于没有实现setFilename函数，所以 m_fp != nullptr 时 m_filename != ""
		fclose(m_fp);
		m_fp = nullptr;
	}
	else {
		throw std::logic_error("FileLogAppender::scrolling(): null pointer, <m_fp> is nullptr");
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
		throw std::logic_error("FileLogAppender::scrolling(): open file failded");    // std::logic_error异常对象中会自动包含文件名和行号等调试信息
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

	m_ptrWrite = m_logEventQueue;
	m_ptrRead = m_logEventQueue;

	m_LogAppenderListMap = new std::map<std::string, std::list<std::shared_ptr<LogAppender>>>{};
}


LogEventManager::~LogEventManager()
{
	m_isCanExit = true;         // 注意 m_isCanExit = true 且处理完日志队列中所有数据后才能退出
	m_thread.join();            // 等待处理完日志队列中所有数据
	clearLogEventQueue();
	clearLogAppenderListMap();

	printf("~LogEventManager\n");  // LogEventManager应该是最后一个析构的
	_CrtDumpMemoryLeaks();         // 检测内存是否泄露
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
	m_mutex.lock();  // 注意：千万别忘记加锁因为树结构有个元素之间有关联性它不是独立的 ！！！！
	(*m_LogAppenderListMap)[_loggername].push_back(_appender);   // map是个复杂的黑盒类，无法用原子操作
	m_mutex.unlock();
}


inline bool LogEventManager::isFindLogger(const std::string& _loggername)
{
	return (m_LogAppenderListMap->find(_loggername) != m_LogAppenderListMap->end());
}


inline std::map<std::string, std::list<std::shared_ptr<LogAppender>>>* LogEventManager::getLogAppenderListMap()
{
	return m_LogAppenderListMap;
}

void LogEventManager::dealLogEvent_threadFuntion()
{
	LogEventQueue* tempNode = nullptr;    // 临时保存可读的节点，读取完后，辅助删除
	std::list<std::shared_ptr<LogAppender>>* tempList = nullptr;  // 用指针类型避免调用拷贝或者移动函数
	LogEvent tempData = { 0 };
	std::stringstream ss;

	 
	while (m_isCanExit == false || m_ptrRead->m_next != nullptr)  // 当 m_ptrRead = m_ptrWrite 时 m_ptrRead->m_next 也会受到内存屏障保护
	{
		if (m_ptrRead->m_next != nullptr)
		{
			tempNode = m_ptrRead->m_next;
			m_ptrRead = m_ptrRead->m_next;

			tempData = tempNode->m_data;
			tempList = &(*m_LogAppenderListMap)[*(tempData.m_loggername)];

			// 遍历 tempList 中所有的 LogAppender
			for (auto it = tempList->begin(); it != tempList->end(); ++it) {  // 或者用 (*(*it)).getType();
				it->get()->getFormatFuntion()(ss, tempData);
				it->get()->log(ss.str().c_str());
				ss.str("");   // 清空 stringstream
			}

			if (m_logEventQueue-> m_next != m_ptrRead) {   // 头删法。当 m_logEventQueue = m_ptrWrite 时 m_logEventQueue->m_next 也会受到内存屏障保护
				tempNode = m_logEventQueue->m_next;
				m_logEventQueue->m_next = tempNode->m_next;
				clearLogEventNodeData(tempNode);
			}

		}

	}

}


void LogEventManager::addLogEvent(LogEvent _logEvent)
{
	LogEventQueue* newNode = (LogEventQueue*)malloc(sizeof(LogEventQueue));
	newNode->m_next = nullptr;
	newNode->m_data = _logEvent;

	while (true) // 原子操作通常要配合自旋锁
	{  
		LogEventQueue* currentWrite = m_ptrWrite;     // m_ptrWrite是原子操作的对象，因此可以直接读取用于判断，但不能 m_ptrWrite = XXXX 或 m_ptrWrite->next = XXXX
		LogEventQueue* next = currentWrite->m_next;   // 注意：不是 m_ptrWrite->m_next

		if (currentWrite == m_ptrWrite) {
			if(next == nullptr) {
				if (InterlockedCompareExchangePointer((PVOID*)(&(currentWrite->m_next)), newNode, nullptr) == nullptr) {   // 注意用 currentWrite->m_next 而不是 m_ptrWrite->next 是因为 m_ptrWrite 可能会变。同时当 currentWrite == m_ptrWrite 时，m_ptrWrite->next 也会受到内存屏障保护
					InterlockedExchangePointer((PVOID*)(&(m_ptrWrite)), newNode); // 注意用 newNode 赋值，注意最后即使 m_ptrWrite 不在最尾端也没关系
					break;
				}
			}
		}
		else {
			InterlockedExchangePointer((PVOID*)(&(m_ptrWrite)), next);  // 注意用 next 赋值
		}
	}
}




// =============================================================================================================
// class Logger

std::string* Logger::m_syncDebugSetting = nullptr;  // 注意：静态成员变量要先在类外初始化后，才能在成员函数中赋值，不晚会报错，无法解析的外部符号

Logger::Logger(const std::string& _loggername, LogLevel _level, std::shared_ptr<LogAppender> _appender)
{
	// 注意 getDefaultLogEventManager() 这段代码保证了 LogEventManager 在 Logger 之前创建，确保了 LogEventManager 最后才销毁
	if (getDefaultLogEventManager()->isFindLogger(_loggername) == true) {
		throw std::logic_error("Logger::Logger(): loggername must be unqiue, <_loggername> is already exists");
	}

	m_level = _level;
	m_loggername = _loggername;
	getDefaultLogEventManager()->addAppender(_loggername, _appender);

	return;
}

bool Logger::setUpSyncDebug()
{
	void* temp = &m_loggername;
	if (m_syncDebugSetting == nullptr) {
		InterlockedCompareExchangePointer((PVOID*)(&m_syncDebugSetting), temp, nullptr);
		return true;
	}
	return false;
}



void Logger::log(LogLevel _level, const char* _filename, int _line, const char* _format, ...)
{
	if ((int)m_level > (int)_level || (m_syncDebugSetting != nullptr && m_syncDebugSetting != &m_loggername)) {
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
	data.m_LogLevel = _level;
	data.m_filename = _filename;
	data.m_line = _line;
	data.m_content = buffer;

	if (m_syncDebugSetting == nullptr) {
		getDefaultLogEventManager()->addLogEvent(data);
	}
	else {
		std::stringstream ss;
		auto* tempMap = getDefaultLogEventManager()->getLogAppenderListMap();
		std::list<std::shared_ptr<LogAppender>>* tempList = &(*tempMap)[m_loggername];

		for (auto it = tempList->begin(); it != tempList->end(); ++it) {  // 或者用 (*(*it)).getType();
			it->get()->getFormatFuntion()(ss, data);
			it->get()->log(ss.str().c_str());
			ss.str("");   // 清空 stringstream
		}
		free(data.m_content);
		delete data.m_loggername;
	}
}

inline std::string Logger::getLoggerName()
{
	return m_loggername;
}

bool Logger::addAppender(std::shared_ptr<LogAppender> _appender)
{
	getDefaultLogEventManager()->addAppender(m_loggername, _appender);
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







