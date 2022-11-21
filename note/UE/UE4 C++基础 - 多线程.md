# UE4 C++基础 - 多线程



## 1. 线程概念

线程是操作系统能够进行运行调度的最小单位。大部分情况下，它被包含在进程之中，是进程中的实际运作单位。一条线程指的的是进程中一个单一顺序的控制流，一个进程中可以并发多个线程，每条线程并行执行不同的任务。

典型的UNIX进程可以看成只有一个控制线程：一个进程在同一时刻只做一件事情。有了多个线程以后，在程序设计时可以把进程设计成在同一时刻能够做不止一件事，每个线程处理各自独立的任务。这种方法有很多好处[[1\]](https://zhuanlan.zhihu.com/p/133921916#ref_1)。

- 通过为每种事件类型的处理分配单独的线程，能够简化处理异步事件的代码。每个线程在进行事件处理时可以采用同步编程模式，同步编程模式要比异步编程模式简单得多。
- 多个进程必须使用操作系统提供的复杂机制才能实现内存和文件描述符的共享。而多个线程自动地可以访问相同的储存地址空间和文件描述符。
- 有些问题可以通过将其分解从而改善整个程序的吞吐量。在只有一个控制线程的情况下，单个进程需要完成多个任务时，实际上需要把这些任务串行化；有了多个控制线程，相互独立的任务的处理就可以交叉进行，只需要为每个任务分配一个单独的线程，当然只有在处理过程互不依赖的情况下，两个任务的执行才可以穿插进行。
- 交互的程序同样可以通过使用多线程实现响应时间的改善，多线程可以把程序中处理用户输入输出的部分与其他部分分开。

## 2. UE4中的多线程 FRunnable & FRunnableThread

UE4是跨平台的引擎，对各个平台线程实现进行了封装，抽象出了 ***FRunnable\*** [[2\]](https://zhuanlan.zhihu.com/p/133921916#ref_2)。引擎中大部分的需要多线程执行逻辑都是继承这个类实现的多线程，如

- [FAsyncWriter](https://link.zhihu.com/?target=https%3A//docs.unrealengine.com/en-US/API/Runtime/Core/Misc/FAsyncWriter/index.html)
- [FUdpSocketReceiver](https://link.zhihu.com/?target=https%3A//docs.unrealengine.com/en-US/API/Runtime/Networking/Common/FUdpSocketReceiver/index.html)
- [FTcpListener](https://link.zhihu.com/?target=https%3A//docs.unrealengine.com/en-US/API/Runtime/Networking/Common/FTcpListener/index.html)

一个可执行的线程必须包含一个 runnable 对象，用来执行业务逻辑代码。样例如下：

```cpp
#include "HAL/Runnable.h"

class MyRunnable : public FRunnable {
public:
	virtual bool Init() override;  // 初始化 runnable 对象
	virtual uint32 Run() override; // 运行 runnable 对象
	virtual void Stop() override;  // 停止 runnable 对象,线程提前终止时被调用
	virtual void Exit() override;  // 退出 runnable 对象
};

bool MyRunnable::Init() { return true; }
uint32 MyRunnable::Run() { return 0; }
void MyRunnable::Stop() {}
void MyRunnable::Exit() {} 
```

调用顺序是 **Init()**, **Run()**, **Exit()**。Runnable对象初始化操作在 **Init()** 函数中完成，并通过返回值确定是否成功。初始化失败，则该线程停止执行，并返回一个错误码；成功，则会执行 **Run()** ；执行完毕后，则会调用 **Exit()** 执行清理操作。

## 2.1 多线程创建

**Runnable**负责具体业务逻辑的执行，UE4中使用 **[FRunnableThread](https://link.zhihu.com/?target=https%3A//docs.unrealengine.com/en-US/API/Runtime/Core/HAL/FRunnableThread/index.html)** 表示一个可执行的线程。 可以通过调用 **[FRunnableThread::Create](https://link.zhihu.com/?target=https%3A//docs.unrealengine.com/en-US/API/Runtime/Core/HAL/FRunnableThread/Create/index.html)** 完成线程的创建：

```cpp
#include "HAL/RunnableThread.h"

static FRunnableThread * Create
(
    class FRunnable * InRunnable, // Runnable 对象
    const TCHAR * ThreadName,     // 线程名称
    uint32 InStackSize,           // 线程栈大小,0表示使用当前线程的栈大小
    EThreadPriority InThreadPri,  // 线程优先级
    uint64 InThreadAffinityMask
);

// 返回值：若成功则返回创建的线程，否则返回 nullptr
```

样例代码如下：

```cpp
#include "HAL/RunnableThread.h"

FRunnable * Runnable = new MyRunnable();
FRunnableThread* RunnableThread = FRunnableThread::Create(Runnable, TEXT("LaLaLaDeMaXiYa!"));
```

线程对象创建成功后即开始执行Runnable对象的 ***Init\* ()** 函数，如果成功则分别执行***Run()\*** 和 ***Exit()\*** 函数。

## 2.2 线程标识

每个线程都有一个线程ID，线程ID在它所属的进程环境中有效。为增加标识性，UE4还增加了线程名称。线程ID是唯一的，线程名称可以重复。可通过[GetThreadID](https://link.zhihu.com/?target=https%3A//docs.unrealengine.com/en-US/API/Runtime/Core/HAL/FRunnableThread/GetThreadID/index.html) 和 [GetThreadName](https://link.zhihu.com/?target=https%3A//docs.unrealengine.com/en-US/API/Runtime/Core/HAL/FRunnableThread/GetThreadName/index.html) 获取线程ID和名称。

```cpp
const uint32 GetThreadID() const;
const FString & GetThreadName() const;
```

## 2.3 线程终止

单个线程可以通过如下三种方式退出。

1. 线程执行完 runnable 对象的 Run() 和 Exit() 函数后正常退出
2. 调用 [WaitForCompletion()](https://link.zhihu.com/?target=https%3A//docs.unrealengine.com/en-US/API/Runtime/Core/HAL/FRunnableThread/WaitForCompletion/index.html) 函数，阻塞调用例程直到线程执行完毕
3. 调用 [Kill(bool bShouldWait=false)](https://link.zhihu.com/?target=https%3A//docs.unrealengine.com/en-US/API/Runtime/Core/HAL/FRunnableThread/Kill/index.html) 函数，会先执行 runnable 对象的 stop 函数,然后根据 bShouldWait 参数决定是否等待线程执行完毕。如果不等待，则强制杀死线程，可能会造成内存泄漏。

```cpp
void WaitForCompletion(); // 阻塞调用例程，直到线程执行完毕
bool Kill(bool bShouldWait); // 强制杀掉线程
```

## 2.4 引擎样例代码

- Engine\Source\Programs\TestPAL\Private\Main.cpp

## 3. 实现细节 - FThreadManager

通过[FRunnableThread](https://link.zhihu.com/?target=https%3A//docs.unrealengine.com/en-US/API/Runtime/Core/HAL/FRunnableThread/index.html) 创建的线程是通过 [FThreadManager](https://link.zhihu.com/?target=https%3A//docs.unrealengine.com/en-US/API/Runtime/Core/HAL/FThreadManager/index.html) 进行统一管理。

```cpp
// ThreadingBase.cpp FRunnableThread::Create 函数
// Call the thread's create method
if (NewThread->CreateInternal(InRunnable,ThreadName,InStackSize,InThreadPri,InThreadAffinityMask) == false)
```

CreateInternal根据平台的不同实现不同，常用平台中，Android和iOS都是采用的 pthread标准线程库，Windows平台是单独实现的。线程创建完毕后会统一调用

```cpp
FThreadManager::Get().AddThread(ThreadID, this);
```

将线程本身添加至管理器。如 WindowsRunnableThread.h FRunnableThreadWin::CreateInternal 函数。标准线程对象 FRunnableThreadPThread 则是在入口点:

```cpp
virtual PthreadEntryPoint GetThreadEntryPoint() {
	return _ThreadProc;
}

static void *STDCALL _ThreadProc(void *pThis) {
	check(pThis);
	FRunnableThreadPThread* ThisThread = (FRunnableThreadPThread*)pThis;
	// cache the thread ID for this thread (defined by the platform)
	ThisThread->ThreadID = FPlatformTLS::GetCurrentThreadId();
	// ====================>>这里将线程本身加入管理器 <<==========================
	FThreadManager::Get().AddThread(ThisThread->ThreadID, ThisThread);
	// set the affinity.  This function sets affinity on the current thread, so don't call in the Create function which will trash the main thread affinity.
	FPlatformProcess::SetThreadAffinityMask(ThisThread->ThreadAffinityMask);		
	// run the thread!
	ThisThread->PreRun();
	ThisThread->Run();
	ThisThread->PostRun();
	pthread_exit(NULL);
	return NULL;
}
```



## 3. 线程池[[3\]](https://zhuanlan.zhihu.com/p/133921916#ref_3)

线程过多会带来调度开销，进而影响缓存局部性和整体性能。频繁创建和销毁线程也会带来极大的开销。通常我们更加关心的是任务可以并发执行，并不想管理线程的创建，销毁和调度。通过将任务处理成队列，交由线程池统一执行，可以提升任务的执行效率。UE4提供了对应的线程池来满足我们的需求。异步任务统一都继承至 **IQueuedWork**[[4\]](https://zhuanlan.zhihu.com/p/133921916#ref_4)，属于抽象接口类，可供我们直接使用的是

- **FAsyncTask**[[5\]](https://zhuanlan.zhihu.com/p/133921916#ref_5)异步任务，自动加入线程池
- **FAutoDeleteAsyncTask**[[6\]](https://zhuanlan.zhihu.com/p/133921916#ref_6) 异步任务，任务完成后会自动销毁

![img](https://pic1.zhimg.com/80/v2-20560dc1106cc350482d957b91ef8870_720w.webp)

图 1-1 线程池

异步任务通常继承 **FNonAbandonableTask**[[7\]](https://zhuanlan.zhihu.com/p/133921916#ref_7)，表明该任务不可被抛弃，必须被执行完毕。样例代码如下：

```cpp
idi#include "Async/AsyncWork.h"

class ExampleAsyncTask : public FNonAbandonableTask
{
	friend class FAsyncTask<ExampleAsyncTask>;
	friend class FAutoDeleteAsyncTask<ExampleAsyncTask>;
	int32 ExampleData;
	ExampleAsyncTask(int32 InExampleData)
		: ExampleData(InExampleData){}

	void DoWork() {
		UE_LOG(LogBlankProgram, Display, TEXT("ExampleAsyncTask %d Work."), ExampleData);
	}

	FORCEINLINE TStatId GetStatId() const {
		RETURN_QUICK_DECLARE_CYCLE_STAT(ExampleAsyncTask, STATGROUP_ThreadPoolAsyncTasks);
	}
};

void Example {
	// 2.1 线程池异步队列
	FAsyncTask<ExampleAsyncTask>* MyTask = new FAsyncTask<ExampleAsyncTask>(1);
	// 交由后台控制任务开始执行时机
	MyTask->StartBackgroundTask();
	// 确保线程被执行完成
	MyTask->EnsureCompletion();
	delete MyTask;
}
```

## 4. 线程同步

当多个线程共享相同的内存时，需要确保每个线程看到一致的数据视图。如果每个线程使用的变量都是其他线程不会读取或者修改的，那么就不存在一致性问题。同样地，如果变量是只读的，多个线程同时读取该变量也不会有一致性问题。但是，当某个线程可以修改变量，而其他线程也可以读取或者修改这个变量的时候，就需要对这些线程进行同步，以确保它们在访问变量的存储内容时不会访问到无效的数值。这个时候就需要用到线程同步机制。UE4提供了以下几个不同类别的同步机制：

## 4.1 Atomics 原子机制

Atomic operations(原子操作) 保证CPU在读取和写入内存时总线操作是不可分割的。它是许多高级同步机制的基础，主要优势是可以进行比较快的进行比较和解锁操作。一个用Atomics实现的样例如下：

```cpp
class FThreadSafeCounter{
public:
int32 Add( int32 Amount ) {
        return FPlatformAtomics::InterlockedAdd(&Counter, Amount);
    }
private:
    volatile int32 Counter; // 因为值可能以编译器无法预测的异步方式被改变，声明为volatile禁用优化
};
```

## 4.2 Locking 锁机制

在UE4中常用的两种锁机制是 Critical Sections(临界区）和 SpinLocks(自旋锁）。

- FSpinLock 自旋锁
- **FScopeLock**[[8\]](https://zhuanlan.zhihu.com/p/133921916#ref_8) 区域锁
- **FCriticalSection** 临界区
- **FRWLock**[[9\]](https://zhuanlan.zhihu.com/p/133921916#ref_9) 读写锁

## 4.3 Signaling 信号机制

- **FSemaphore**[[10\]](https://zhuanlan.zhihu.com/p/133921916#ref_10)信号量与互斥锁类型，但是他包含了一种信号机制。缺点是不是所有平台都支持。更加常用的线程间通信机制是 **FEvent**[[11\]](https://zhuanlan.zhihu.com/p/133921916#ref_11)。

## 4.4 Waiting

- **FEvent**[[11\]](https://zhuanlan.zhihu.com/p/133921916#ref_11) 事件

- - 阻塞直至被触发或者超时
  - 经常被用来激活其他工作线程

- **FScopedEvent**[[12\]](https://zhuanlan.zhihu.com/p/133921916#ref_12)区域事件

- - 对FEvent的一次包装，阻塞在域代码退出时

```cpp
{
    FScopedEvent MyEvent;
    SendReferenceOrPointerToSomeOtherThread(&MyEvent); // Other thread calls MyEvent->Trigger() ;
    // MyEvent destructor is here, we wait here.
}
```

其中 FCriticalSection 是根据各个平台的互斥锁进行的抽象。Windows 平台是基于Windows平台的临界区。常用的iOS, Android,Linux平台则是使用的[POSIX](https://link.zhihu.com/?target=https%3A//zh.wikipedia.org/wiki/POSIX)的[线程](https://link.zhihu.com/?target=https%3A//zh.wikipedia.org/wiki/%E7%BA%BF%E7%A8%8B)标准实现[[13\]](https://zhuanlan.zhihu.com/p/133921916#ref_13)。

## 4.5 其他

UE4常见的容器类【TArray, TMap, TSet】通常都不是线程安全的，需要我们仔细编写代码保证线程安全。下面是几个常见的线程安全类：

- **FThreadSafeCounter**[[14\]](https://zhuanlan.zhihu.com/p/133921916#ref_14)计数器
- **FThreadSingleton** 单例类
- **FThreadIdleStats** 线程空闲状态统计类
- **TLockFreePointerList 无锁队列**
- **TQueue**[[15\]](https://zhuanlan.zhihu.com/p/133921916#ref_15)队列

下面是一个简单的线程安全TSet，附带FCriticalSection使用示例。

```cpp
/** Simple thread safe proxy for TSet<FName> */
template <typename T>
class FThreadSafeSet
{
	TSet<T> InnerSet;
	FCriticalSection SetCritical;
public:
	void Add(T InValue) {
		FScopeLock SetLock(&SetCritical);
		InnerSet.Add(InValue);
	}

	bool AddUnique(T InValue) {
		FScopeLock SetLock(&SetCritical);
		if (!InnerSet.Contains(InValue))
		{
			InnerSet.Add(InValue);
			return true;
		}
		return false;
	}

	bool Contains(T InValue) {
		FScopeLock SetLock(&SetCritical);
		return InnerSet.Contains(InValue);
	}

	void Remove(T InValue) {
		FScopeLock SetLock(&SetCritical);
		InnerSet.Remove(InValue);
	}

	void Empty() {
		FScopeLock SetLock(&SetCritical);
		InnerSet.Empty();
	}

	void GetValues(TSet<T>& OutSet) {
		FScopeLock SetLock(&SetCritical);
		OutSet.Append(InnerSet);
	}

	int32 Num() { return InnerSet.Num();}
};
```

## 完整代码

```cpp
// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
#include "BlankProgram.h"
#include "RequiredProgramMainCPPInclude.h"
#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"
#include "Async/AsyncWork.h"

DEFINE_LOG_CATEGORY_STATIC(LogBlankProgram, Log, All);
IMPLEMENT_APPLICATION(BlankProgram, "BlankProgram");

class MyRunnable : public FRunnable {
public:
	virtual bool Init() override;  // 初始化 runnable 对象
	virtual uint32 Run() override; // 运行 runnable 对象
	virtual void Stop() override;  // 停止 runnable 对象,线程提前终止时被调用
	virtual void Exit() override;  // 退出 runnable 对象
};

bool MyRunnable::Init() {
	UE_LOG(LogBlankProgram, Display, TEXT("Thread Init."));
	return true;
}

uint32 MyRunnable::Run() {
	UE_LOG(LogBlankProgram, Display, TEXT("Thread Run."));
	return 0;
}

void MyRunnable::Stop() {}

void MyRunnable::Exit() {
	UE_LOG(LogBlankProgram, Display, TEXT("Thread Exit."));
}

// 任务队列
class ExampleAsyncTask : public FNonAbandonableTask {
	friend class FAsyncTask<ExampleAsyncTask>;
	friend class FAutoDeleteAsyncTask<ExampleAsyncTask>;

	int32 ExampleData;

	ExampleAsyncTask(int32 InExampleData)
		: ExampleData(InExampleData){}

	void DoWork() {
		UE_LOG(LogBlankProgram, Display, TEXT("ExampleAsyncTask %d Work."), ExampleData);
	}

	FORCEINLINE TStatId GetStatId() const {
		RETURN_QUICK_DECLARE_CYCLE_STAT(ExampleAsyncTask, STATGROUP_ThreadPoolAsyncTasks);
	}
};

INT32_MAIN_INT32_ARGC_TCHAR_ARGV()
{
	GEngineLoop.PreInit(ArgC, ArgV);
	UE_LOG(LogBlankProgram, Display, TEXT("UE4 Multithreading Example."));

	// 1. FRunnable 使用示例
	FRunnable * Runnable = new MyRunnable();
	FRunnableThread* RunnableThread = FRunnableThread::Create(Runnable, TEXT("LaLaLaDeMaXiYa!"));
	RunnableThread->WaitForCompletion();

	// 2.1 线程池异步队列
	FAsyncTask<ExampleAsyncTask>* MyTask = new FAsyncTask<ExampleAsyncTask>(1);
	// 交由后台控制任务开始执行时机
	MyTask->StartBackgroundTask();
	// 确保线程被执行完成
	MyTask->EnsureCompletion();
	delete MyTask;

	// 2.2 线程池异步队列
	FAsyncTask<ExampleAsyncTask>* MyTask2 = new FAsyncTask<ExampleAsyncTask>(2);
	// 直接在当前线程中执行
	MyTask2->StartSynchronousTask();
	// 检查任务是否完成
	if (MyTask2->IsDone()) {
		UE_LOG(LogBlankProgram, Display, TEXT("MyTask2 is Done."));
	}
	MyTask2->EnsureCompletion();
	delete MyTask2;

	// 2.3 带自动销毁的异步任务
	// 交由后台控制任务开始执行时机
	(new FAutoDeleteAsyncTask<ExampleAsyncTask>(3))->StartBackgroundTask();

	// 直接在当前线程中开始执行
	(new FAutoDeleteAsyncTask<ExampleAsyncTask>(4))->StartSynchronousTask();

	return 0;
}
```



## 推荐阅读资料

《UNIX环境高级编程-第十一章 线程》

[![img](https://zhstatic.zhihu.com/assets/zhihu-components/file-icon/zhimg_answer_editor_file_other.svg)concurrency-parallelism-in-ue4.pptx4M·百度网盘](https://pan.baidu.com/link/zhihu/7VhDzOuShniDeQpWdEbaNLhXVtTIpUMQQwo0==)



## 参考

1. [^](https://zhuanlan.zhihu.com/p/133921916#ref_1_0)编程范式 - 多线程基础 https://www.bilibili.com/video/BV1Cx411S7HJ?p=15
2. [^](https://zhuanlan.zhihu.com/p/133921916#ref_2_0)UE4多线程资料 https://docs.unrealengine.com/en-US/API/Runtime/Core/HAL/FRunnable/index.html
3. [^](https://zhuanlan.zhihu.com/p/133921916#ref_3_0)Thread pool https://en.wikipedia.org/wiki/Thread_pool
4. [^](https://zhuanlan.zhihu.com/p/133921916#ref_4_0)IQueuedWork https://docs.unrealengine.com/en-US/API/Runtime/Core/Misc/IQueuedWork/index.html
5. [^](https://zhuanlan.zhihu.com/p/133921916#ref_5_0)FAsyncTask https://docs.unrealengine.com/en-US/API/Runtime/Core/Async/FAsyncTask/index.html
6. [^](https://zhuanlan.zhihu.com/p/133921916#ref_6_0)FAutoDeleteAsyncTask https://docs.unrealengine.com/en-US/API/Runtime/Core/Async/FAutoDeleteAsyncTask/index.html
7. [^](https://zhuanlan.zhihu.com/p/133921916#ref_7_0)FNonAbandonableTask https://docs.unrealengine.com/en-US/API/Runtime/Core/Async/FNonAbandonableTask/index.html
8. [^](https://zhuanlan.zhihu.com/p/133921916#ref_8_0)FScopeLock https://docs.unrealengine.com/en-US/API/Runtime/Core/Misc/FScopeLock/index.html
9. [^](https://zhuanlan.zhihu.com/p/133921916#ref_9_0)FRWLock https://docs.unrealengine.com/en-US/API/Runtime/Core/Unix/FRWLock/index.html
10. [^](https://zhuanlan.zhihu.com/p/133921916#ref_10_0)FSemaphore https://docs.unrealengine.com/en-US/API/Runtime/Core/GenericPlatform/FGenericPlatformProcess/FSemaphore/index.html
11. ^[a](https://zhuanlan.zhihu.com/p/133921916#ref_11_0)[b](https://zhuanlan.zhihu.com/p/133921916#ref_11_1)FEvent https://docs.unrealengine.com/en-US/API/Runtime/Core/HAL/FEvent/index.html
12. [^](https://zhuanlan.zhihu.com/p/133921916#ref_12_0)FScopedEvent https://docs.unrealengine.com/en-US/API/Runtime/Core/Misc/FScopedEvent/index.html
13. [^](https://zhuanlan.zhihu.com/p/133921916#ref_13_0)POSIX Threads https://en.wikipedia.org/wiki/POSIX_Threads
14. [^](https://zhuanlan.zhihu.com/p/133921916#ref_14_0)FThreadSafeCounter https://docs.unrealengine.com/en-US/API/Runtime/Core/HAL/FThreadSafeCounter/index.html
15. [^](https://zhuanlan.zhihu.com/p/133921916#ref_15_0)TQueue https://docs.unrealengine.com/en-US/API/Runtime/Core/Containers/TQueue/index.html