#include "recorder.h"

using namespace mochen::dbg;


// =============================================================================================================
// class DetcetionMemory

MemoryRecorder::MemoryRecorder()
{
	_CrtMemCheckpoint(&m_startMemState);   // ��ȡ��ʼʱ�ڴ��ʹ��״̬
}

MemoryRecorder::~MemoryRecorder()
{
	printf("~MemoryRecorder\n"); // ��������������������������������������������������������������������������������������

	_CrtMemCheckpoint(&m_endMemState);     // ��ȡ����ʱ�ڴ��ʹ��״̬
	if (_CrtMemDifference(&m_differenceMemState, &m_startMemState, &m_endMemState) == true) {
		printf("����������ڴ�й¶��\n");    // !@#@!#@!#!#!@#@!@#!!#!#@!#
		_CrtMemDumpStatistics(&m_differenceMemState); 
		_CrtDumpMemoryLeaks();             // ע�����������������δ�ͷŵ�ȫ�ֱ�����STL���ֶ�new�Ķ���ȣ�Ҫ�ֶ����ˡ�ͬʱ��ε��øú���������е�һ�Σ���˸ú���Ҫ����main�����
	}
}


void MemoryRecorder::recordStartMemState()
{
	_CrtMemCheckpoint(&m_startMemState);
}


void MemoryRecorder::recordEndtMemState(const char* _format, ...)
{
	_CrtMemCheckpoint(&m_endMemState);     // ��ȡ����ʱ�ڴ��ʹ��״̬
	if (_CrtMemDifference(&m_differenceMemState, &m_startMemState, &m_endMemState) == true) {
		_CrtMemDumpStatistics(&m_differenceMemState);

		if (_format != "") {
			va_list args;

			va_start(
				args,                // ����va_list�ɱ�����б�
				_format              // ����ú��������һ������ȷ���Ĳ�����ע�⣺�ò������Ͳ������������͡�
			);  

			vprintf(_format, args);  // vprintfͨ��va_list�����ղ�����printfͨ��...�����ղ���
			va_end(args);            // ����va_list
		}

		printf("�ڴ�й¶��\n");    // !@#@!#@!#!#!@#@!@#!!#!#@!#
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
	QueryPerformanceFrequency(&m_frequency); // ��ü����һ�������м��Σ����������Ƶ��
	QueryPerformanceCounter(&m_startCount);       // ��ÿ�ʼʱ��������еĴ���
}


RunTimeRecorder::~RunTimeRecorder() 
{

	QueryPerformanceCounter(&m_endCount);	
	printf("����������ó�������ʱ��Ϊ��%lf΢��\n", (double)(m_endCount.QuadPart - m_startCount.QuadPart) / m_frequency.QuadPart * 1000000);
}


void RunTimeRecorder::recordStartTime()
{
	QueryPerformanceCounter(&m_startCount); // ��ÿ�ʼʱ��������еĴ���
}


void RunTimeRecorder::recordEndTime() 
{
	QueryPerformanceCounter(&m_endCount); // ��ÿ�ʼʱ��������еĴ���
}



double RunTimeRecorder::getRunTime()
{
	return (double)(m_endCount.QuadPart - m_startCount.QuadPart) / m_frequency.QuadPart;
}

