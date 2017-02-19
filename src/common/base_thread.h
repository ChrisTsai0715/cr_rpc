#if !defined(AFX_BASETHREAD_H__CB0E1EB6_F3C0_4663_888F_C66226684788__INCLUDED_)
#define AFX_BASETHREAD_H__CB0E1EB6_F3C0_4663_888F_C66226684788__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <pthread.h>
#include <time.h>

//#include "ObjectMonitor/ObjectMonitor.h"
#include <string>

//! 线程
/*!
  封装线程的一般操作
*/
class base_thread
{
// constructor/destructor
public:
  base_thread();
  virtual ~base_thread();

// methods
public:
  // normal thread start 
  virtual bool run();
  // dettached thread start with no calling "Stop()"
  virtual bool RunOnce();
  //! 停止线程
  virtual bool stop();
  //! 判断线程是否在运行
  bool is_running() const;
  //! 等待线程停止
  bool wait_for_thread_end(unsigned long timeout = -1);

public:

  pthread_t _hThread;

private:
  virtual bool _run(bool once=false);

protected:
  //! 线程循环
  /*!
    在线程空间内执行,需要由子类实现
    \return true 线程将继续循环调用ThreadLoop(),false 线程将停止
  */
  virtual bool thread_loop() = 0;
  //! 等待线程停止
  /*!
    调用后将设置为等待停止状态,并阻塞调用线程,直到CBaseThread线程停止
  */
  virtual bool wait_for_stop(unsigned long timeout = -1);

  bool wait_for_timeout(unsigned long timeout);
  //! 线程停止时被调用
  /*!
    在线程空间内执行,线程停止时被调用
  */
  virtual void on_exit(){}
  //! 线程开始时被调用
  /*!
    在线程空间内执行,线程开始时被调用
  */
  virtual void on_begin(){}

  volatile bool _bRunning;
  volatile bool _bWaitStop;
  volatile bool _bExited;
  // run once flag 
  volatile bool run_once;

  static void* thread_func( void* pArguments );
  
};

#endif // !defined(AFX_BASETHREAD_H__CB0E1EB6_F3C0_4663_888F_C66226684788__INCLUDED_)

