#if !defined(AFX_BASETHREAD_H__CB0E1EB6_F3C0_4663_888F_C66226684788__INCLUDED_)
#define AFX_BASETHREAD_H__CB0E1EB6_F3C0_4663_888F_C66226684788__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <pthread.h>
#include <time.h>

//#include "ObjectMonitor/ObjectMonitor.h"
#include <string>

//! �߳�
/*!
  ��װ�̵߳�һ�����
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
  //! ֹͣ�߳�
  virtual bool stop();
  //! �ж��߳��Ƿ�������
  bool is_running() const;
  //! �ȴ��߳�ֹͣ
  bool wait_for_thread_end(unsigned long timeout = -1);

public:

  pthread_t _hThread;

private:
  virtual bool _run(bool once=false);

protected:
  //! �߳�ѭ��
  /*!
    ���߳̿ռ���ִ��,��Ҫ������ʵ��
    \return true �߳̽�����ѭ������ThreadLoop(),false �߳̽�ֹͣ
  */
  virtual bool thread_loop() = 0;
  //! �ȴ��߳�ֹͣ
  /*!
    ���ú�����Ϊ�ȴ�ֹͣ״̬,�����������߳�,ֱ��CBaseThread�߳�ֹͣ
  */
  virtual bool wait_for_stop(unsigned long timeout = -1);

  bool wait_for_timeout(unsigned long timeout);
  //! �߳�ֹͣʱ������
  /*!
    ���߳̿ռ���ִ��,�߳�ֹͣʱ������
  */
  virtual void on_exit(){}
  //! �߳̿�ʼʱ������
  /*!
    ���߳̿ռ���ִ��,�߳̿�ʼʱ������
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

