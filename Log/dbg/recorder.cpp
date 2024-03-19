#include "recorder.h"

using namespace mochen::dbg;


// =============================================================================================================
// class DetcetionMemory

MemoryRecorder::MemoryRecorder()
{
	_CrtMemCheckpoint(&m_startMemState);   // 获取开始时内存的使用状态
}

MemoryRecorder::~MemoryRecorder()
{
	printf("~MemoryRecorder\n"); // 》》》》》》》》》》》》》》》》》》》》》》》》》》》》》》》》》》》》》》》》》》》

	_CrtMemCheckpoint(&m_endMemState);     // 获取结束时内存的使用状态
	if (_CrtMemDifference(&m_differenceMemState, &m_startMemState, &m_endMemState) == true) {
		printf("程序结束：内存泄露了\n");    // !@#@!#@!#!#!@#@!@#!!#!#@!#
		_CrtMemDumpStatistics(&m_differenceMemState); 
		_CrtDumpMemoryLeaks();             // 注意该输出结果，包含还未释放的全局变量如STL，手动new的对象等，要手动过滤。同时多次调用该函数则会运行第一次，因此该函数要放在main的最后
	}
}


void MemoryRecorder::recordStartMemState()
{
	_CrtMemCheckpoint(&m_startMemState);
}


void MemoryRecorder::recordEndtMemState(const char* _format, ...)
{
	_CrtMemCheckpoint(&m_endMemState);     // 获取结束时内存的使用状态
	if (_CrtMemDifference(&m_differenceMemState, &m_startMemState, &m_endMemState) == true) {
		_CrtMemDumpStatistics(&m_differenceMemState);

		if (_format != "") {
			va_list args;

			va_start(
				args,                // 传入va_list可变参数列表
				_format              // 传入该函数中最后一个可以确定的参数，注意：该参数类型不能是引用类型。
			);  

			vprintf(_format, args);  // vprintf通过va_list来接收参数，printf通过...来接收参数
			va_end(args);            // 销毁va_list
		}

		printf("内存泄露了\n");    // !@#@!#@!#!#!@#@!@#!!#!#@!#
	}
}




void MemoryRecorder::locateMemoryLeaks(long _vulue)
{
	_CrtSetBreakAlloc(_vulue);
}


// =============================================================================================================
// class DetcetionTime


RunTimeRecorder::RunTimeRecorder()
{
	QueryPerformanceFrequency(&m_frequency); // 获得计算机一秒钟运行几次，即计算机的频率
	QueryPerformanceCounter(&m_startCount);       // 获得开始时计算机运行的次数
}


RunTimeRecorder::~RunTimeRecorder() 
{

	QueryPerformanceCounter(&m_endCount);	
	printf("程序结束，该程序运行时长为：%lf微秒\n", (double)(m_endCount.QuadPart - m_startCount.QuadPart) / m_frequency.QuadPart * 1000000);
}


void RunTimeRecorder::recordStartTime()
{
	QueryPerformanceCounter(&m_startCount); // 获得开始时计算机运行的次数
}


void RunTimeRecorder::recordEndTime() 
{
	QueryPerformanceCounter(&m_endCount); // 获得开始时计算机运行的次数
}



double RunTimeRecorder::getRunTime()
{
	return (double)(m_endCount.QuadPart - m_startCount.QuadPart) / m_frequency.QuadPart;
}

