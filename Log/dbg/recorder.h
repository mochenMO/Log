/*// ������˵��
* 1.���locateMemoryLeaks��λʧ�ܣ�Ҫôû�Ѹú�������ǰ�棬Ҫô�ڴ�й¶�Ķ�����static�������ȫ�ֶ���
* 2.���locateMemoryLeaksĪ�����λ����Ĺ��캯��������������ֵĵط�����ô������������뵥���࣬������ڲ�����STLֵ���ͻ����������ڴ档
*   �����û��޷����ù��캯������ôϵͳҲ�޷�ֱ�ӵ����������������ǵ�������static���Բ��õ��ģ�ϵͳ���ڴ���static����ʱ����ͳһ����������������
*/

/*// ��ѧ����֪ʶ
* 1.vprintf   ...
* 2.�ɱ���ر��ǣ�va_start���÷�
* 
* 
*
*
*/

/*// ��δ���������
* 1._CrtDumpMemoryLeaks(); �ú���Ӧ�÷��ڳ�ʼ������
* 2.���̻߳�������μ���ڴ�й¶ https://msdn.microsoft.com/en-us/library/x98tx3cf.aspx
* 3.����Ͳ�������ֵ���͵�STL����ô���ڴ��ݲ���ʱ�õ��ǿ����������ʻ�������ڴ�
* 4.�������˳�򣺾ֲ��� > ��̬�� > ȫ���ࡣ�����������Ⱥ�˳�����ഴ����˳���෴��������ջ��
*
*/



#pragma once
#ifndef _MOCHEN_DETECTION_H_
#define _MOCHEN_DETECTION_H_

#define _CRTDBG_MAP_ALLOC   // ��ӡ����ϸ���ڴ�й©����

#include <string>

#include <stdlib.h>         
#include <crtdbg.h>          
#include <windows.h>        // ���ڼ�������ʱ��
#include <stdarg.h>         // �ɱ����
#include <stdio.h>





namespace mochen
{

namespace dbg
{

// �ü���ڴ�й¶����ֻ����debugģʽ�²�����Ч
class MemoryRecorder
{
private:
	_CrtMemState m_startMemState;       // ���濪ʼʱ�ڴ��ʹ��״̬
	_CrtMemState m_endMemState;         // �������ʱ�ڴ��ʹ��״̬
	_CrtMemState m_differenceMemState;  // ��������״̬֮���ڴ�Ĳ�ֵ���ò�ֵ����ȫ�ֱ�����Ӱ��
public:
	MemoryRecorder();
	~MemoryRecorder();

	MemoryRecorder(const MemoryRecorder&) = delete;
	MemoryRecorder(MemoryRecorder&&) = delete;

	MemoryRecorder& operator=(const MemoryRecorder&) noexcept = delete;
	MemoryRecorder& operator=(MemoryRecorder&&) noexcept = delete;

	void recordStartMemState();
	void recordEndtMemState(const char* _format = "", ...);    // ע�����ﲻ�����������ͣ�Ҳ������std::string����Ϊstd::string����ʱ�Ὺ���ڴ棬����������ڴ�й¶����ʾ
	void locateMemoryLeaks(long _vulue);       // �ú���Ҫ����main�����Ŀ�ͷ
};




class RunTimeRecorder
{
private:
	LARGE_INTEGER m_frequency;    // ��������һ�������м��Σ����������Ƶ��
	LARGE_INTEGER m_startCount;   // ���濪ʼʱ��������еĴ���
	LARGE_INTEGER m_endCount;     // �������ʱ�����˻��еĴ���
public:
	RunTimeRecorder();
	~RunTimeRecorder();

	RunTimeRecorder(const RunTimeRecorder&) = delete;
	RunTimeRecorder(RunTimeRecorder&&) = delete;

	RunTimeRecorder& operator=(const RunTimeRecorder&) noexcept = delete;
	RunTimeRecorder& operator=(RunTimeRecorder&&) noexcept = delete;

	void recordStartTime();
	void recordEndTime();
	double getRunTime();      // ���ؽ�������û��Լ�ת�������ʵ�ʱ�䵥λ��Ĭ�ϵ�λ���룩
};




#ifdef _INITIATE_GLOBAL_MEMORY_RECORDER_
	class MemoryRecorder globalMemoryRecorder;    // ����ȫ�ֵ��ڴ��¼����
#endif

};

};





#endif // !_MOCHEN_DETECTION_H_











