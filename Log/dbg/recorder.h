/*// 介绍与说明
* 1.如果locateMemoryLeaks定位失败，要么没把该函数放在前面，要么内存泄露的对象是static对象或者全局对象
* 2.如果locateMemoryLeaks莫名奇妙定位到类的构造函数，或者其他奇怪的地方，那么该类可能类似与单例类，且类的内部用了STL值类型或者申请了内存。
*   由于用户无法调用构造函数，那么系统也无法直接调用析构函数。但是单例类是static所以不用担心，系统会在处理static对象时，在统一调用其析构函数。
*/

/*// 所学到的知识
* 1.vprintf   ...
* 2.可变参特别是，va_start的用法
* 
* 
*
*
*/

/*// 还未解决的问题
* 1._CrtDumpMemoryLeaks(); 该函数应该放在初始化类中
* 2.多线程环境下如何检测内存泄露 https://msdn.microsoft.com/en-us/library/x98tx3cf.aspx
* 3.如果型参类型是值类型的STL，那么由于传递参数时用的是拷贝函数，故会申请堆内存
* 4.类的析构顺序：局部类 > 静态类 > 全局类。具体析构的先后顺序与类创建的顺序相反，类似于栈。
*
*/



#pragma once
#ifndef _MOCHEN_DETECTION_H_
#define _MOCHEN_DETECTION_H_

#define _CRTDBG_MAP_ALLOC   // 打印更详细的内存泄漏报告

#include <string>

#include <stdlib.h>         
#include <crtdbg.h>          
#include <windows.h>        // 用于计算运行时间
#include <stdarg.h>         // 可变参数
#include <stdio.h>





namespace mochen
{

namespace dbg
{

// 该检测内存泄露的类只能在debug模式下才能生效
class MemoryRecorder
{
private:
	_CrtMemState m_startMemState;       // 保存开始时内存的使用状态
	_CrtMemState m_endMemState;         // 保存结束时内存的使用状态
	_CrtMemState m_differenceMemState;  // 保存两个状态之间内存的差值，用差值过滤全局变量的影响
public:
	MemoryRecorder();
	~MemoryRecorder();

	MemoryRecorder(const MemoryRecorder&) = delete;
	MemoryRecorder(MemoryRecorder&&) = delete;

	MemoryRecorder& operator=(const MemoryRecorder&) noexcept = delete;
	MemoryRecorder& operator=(MemoryRecorder&&) noexcept = delete;

	void recordStartMemState();
	void recordEndtMemState(const char* _format = "", ...);    // 注意这里不能用引用类型，也不能用std::string，因为std::string传递时会开辟内存，错误的引发内存泄露的提示
	void locateMemoryLeaks(long _vulue);       // 该函数要放在main函数的开头
};




class RunTimeRecorder
{
private:
	LARGE_INTEGER m_frequency;    // 保存计算机一秒钟运行几次，即计算机的频率
	LARGE_INTEGER m_startCount;   // 保存开始时计算机运行的次数
	LARGE_INTEGER m_endCount;     // 保存结束时计算运机行的次数
public:
	RunTimeRecorder();
	~RunTimeRecorder();

	RunTimeRecorder(const RunTimeRecorder&) = delete;
	RunTimeRecorder(RunTimeRecorder&&) = delete;

	RunTimeRecorder& operator=(const RunTimeRecorder&) noexcept = delete;
	RunTimeRecorder& operator=(RunTimeRecorder&&) noexcept = delete;

	void recordStartTime();
	void recordEndTime();
	double getRunTime();      // 返回结果，让用户自己转换到合适的时间单位（默认单位是秒）
};




#ifdef _INITIATE_GLOBAL_MEMORY_RECORDER_
	class MemoryRecorder globalMemoryRecorder;    // 创建全局的内存记录器。
#endif

};

};





#endif // !_MOCHEN_DETECTION_H_











