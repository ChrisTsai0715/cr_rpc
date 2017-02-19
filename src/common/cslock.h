
/*! \file	CSLock.h
 *  \author cao yangmin
 *  \date   2008-04-16
 *  \brief	
 *			
 */
#ifndef __CCSLOCK_H__
#define __CCSLOCK_H__
#ifdef __APPLE__
#define PTHREAD_MUTEX_RECURSIVE_NP 1
#endif

#ifndef  HAVE_MUTEX_LOCK 
#  define HAVE_MUTEX_LOCK 1
#endif

#ifdef __MINGW32__ // complie for QT mingw
#  define HAVE_MUTEX_LOCK 0
#endif

#ifndef WIN32
#  include <pthread.h>
#  include <sys/types.h>
#  include <errno.h>
#  include "interlock.h"
#else
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
# 
#  include <windows.h>
#  if HAVE_MUTEX_LOCK 
#    include <atlsecurity.h>
#  endif
#
#endif

class ILockItem
{
public:
	virtual long Lock() = 0;
	virtual long Unlock() = 0;
	virtual ~ILockItem(){}
	ILockItem(){};
private:
	
	ILockItem(const ILockItem&rhs){}
	ILockItem& operator=(const ILockItem& rhs) { return *this; };
};

#ifdef WIN32
  #ifndef __MINGW32__ // Windows compability
	typedef HANDLE pthread_t;
	typedef HANDLE pthread_mutex_t;
	typedef HANDLE pthread_cond_t;
	typedef DWORD pthread_key_t;
  #endif
#endif

//#ifndef WIN32
//
//inline long InterlockedCompareExchange (long volatile * location,
//				  long value,
//				  long comparand)
//{
//
//#if defined(__WATCOMC__)
///* Don't report that result is not assigned a value before being referenced */
//#pragma disable_message (200)
//#endif
//
//  long result;
//
//  /*
//   * Using the LOCK prefix on uni-processor machines is significantly slower
//   * and it is not necessary. The overhead of the conditional below is
//   * negligible in comparison. Since an optimised DLL will inline this
//   * routine, this will be faster than calling the system supplied
//   * Interlocked routine, which appears to avoid the LOCK prefix on
//   * uniprocessor systems. So one DLL works for all systems.
//   */
//  if (1)
//
///* *INDENT-OFF* */
//
//#if defined(_M_IX86) || defined(_X86_)
//
//#if defined(_MSC_VER) || defined(__WATCOMC__) || (defined(__BORLANDC__) && defined(HAVE_TASM32))
//#define HAVE_INLINABLE_INTERLOCKED_CMPXCHG
//    {
//      _asm {
//	PUSH         ecx
//	PUSH         edx
//	MOV          ecx,dword ptr [location]
//	MOV          edx,dword ptr [value]
//	MOV          eax,dword ptr [comparand]
//	LOCK CMPXCHG dword ptr [ecx],edx
//	MOV          dword ptr [result], eax
//	POP          edx
//	POP          ecx
//      }
//    }
//  else
//    {
//      _asm {
//	PUSH         ecx
//	PUSH         edx
//	MOV          ecx,dword ptr [location]
//	MOV          edx,dword ptr [value]
//	MOV          eax,dword ptr [comparand]
//	CMPXCHG      dword ptr [ecx],edx
//	MOV          dword ptr [result], eax
//	POP          edx
//	POP          ecx
//      }
//    }
//
//#elif defined(__GNUC__)
//#define HAVE_INLINABLE_INTERLOCKED_CMPXCHG
//
//    {
//      __asm__ __volatile__
//	(
//	 "lock\n\t"
//	 "cmpxchgl       %2,%1"      /* if (EAX == [location])  */
//	                             /*   [location] = value    */
//                                     /* else                    */
//                                     /*   EAX = [location]      */
//	 :"=a" (result)
//	 :"m"  (*location), "r" (value), "a" (comparand));
//    }
//  else
//    {
//      __asm__ __volatile__
//	(
//	 "cmpxchgl       %2,%1"      /* if (EAX == [location])  */
//	                             /*   [location] = value    */
//                                     /* else                    */
//                                     /*   EAX = [location]      */
//	 :"=a" (result)
//	 :"m"  (*location), "r" (value), "a" (comparand));
//    }
//
//#endif
//
//#else
//#error "InterlockedCompareExchange"
//  /*
//   * If execution gets to here then we're running on a currently
//   * unsupported processor or compiler.
//   */
//
//  result = 0;
//
//#endif
//
///* *INDENT-ON* */
//
//  return result;
//
//#if defined(__WATCOMC__)
//#pragma enable_message (200)
//#endif
//
//}
//
///*
// * ptw32_InterlockedExchange --
// *
// * We now use this version wherever possible so we can inline it.
// */
//
//inline long 
//InterlockedExchange (long volatile* location,
//			   long value)
//{
//
//#if defined(__WATCOMC__)
///* Don't report that result is not assigned a value before being referenced */
//#pragma disable_message (200)
//#endif
//
//  long result;
//
//  /*
//   * The XCHG instruction always locks the bus with or without the
//   * LOCKED prefix. This makes it significantly slower than CMPXCHG on
//   * uni-processor machines. The Windows InterlockedExchange function
//   * is nearly 3 times faster than the XCHG instruction, so this routine
//   * is not yet very useful for speeding up pthreads.
//   */
//  if (1)
//
///* *INDENT-OFF* */
//
//#if defined(_M_IX86) || defined(_X86_)
//
//#if defined(_MSC_VER) || defined(__WATCOMC__) || (defined(__BORLANDC__) && defined(HAVE_TASM32))
//#define HAVE_INLINABLE_INTERLOCKED_XCHG
//
//    {
//      _asm {
//	PUSH         ecx
//	MOV          ecx,dword ptr [location]
//	MOV          eax,dword ptr [value]
//	XCHG         dword ptr [ecx],eax
//	MOV          dword ptr [result], eax
//        POP          ecx
//      }
//    }
//  else
//    {
//      /*
//       * Faster version of XCHG for uni-processor systems because
//       * it doesn't lock the bus. If an interrupt or context switch
//       * occurs between the MOV and the CMPXCHG then the value in
//       * 'location' may have changed, in which case we will loop
//       * back to do the MOV again.
//       *
//       * FIXME! Need memory barriers for the MOV+CMPXCHG combo?
//       *
//       * Tests show that this routine has almost identical timing
//       * to Win32's InterlockedExchange(), which is much faster than
//       * using the inlined 'xchg' instruction above, so it's probably
//       * doing something similar to this (on UP systems).
//       *
//       * Can we do without the PUSH/POP instructions?
//       */
//      _asm {
//	PUSH         ecx
//	PUSH         edx
//	MOV          ecx,dword ptr [location]
//	MOV          edx,dword ptr [value]
//L1:	MOV          eax,dword ptr [ecx]
//	CMPXCHG      dword ptr [ecx],edx
//	JNZ          L1
//	MOV          dword ptr [result], eax
//	POP          edx
//        POP          ecx
//      }
//    }
//
//#elif defined(__GNUC__)
//#define HAVE_INLINABLE_INTERLOCKED_XCHG
//
//    {
//      __asm__ __volatile__
//	(
//	 "xchgl          %2,%1"
//	 :"=r" (result)
//	 :"m"  (*location), "0" (value));
//    }
//  else
//    {
//      /*
//       * Faster version of XCHG for uni-processor systems because
//       * it doesn't lock the bus. If an interrupt or context switch
//       * occurs between the movl and the cmpxchgl then the value in
//       * 'location' may have changed, in which case we will loop
//       * back to do the movl again.
//       *
//       * FIXME! Need memory barriers for the MOV+CMPXCHG combo?
//       *
//       * Tests show that this routine has almost identical timing
//       * to Win32's InterlockedExchange(), and is much faster than
//       * using an inlined 'xchg' instruction, so Win32 is probably
//       * doing something similar to this (on UP systems).
//       */
//      __asm__ __volatile__
//	(
//	 "0:\n\t"
//	 "movl           %1,%%eax\n\t"
//	 "cmpxchgl       %2,%1\n\t"
//	 "jnz            0b"
//	 :"=&a" (result)
//	 :"m"  (*location), "r" (value));
//    }
//
//#endif
//
//#else
//	#error "InterlockedExchange"
//  /*
//   * If execution gets to here then we're running on a currently
//   * unsupported processor or compiler.
//   */
//
//  result = 0;
//
//#endif
//
///* *INDENT-ON* */
//
//  return result;
//
//#if defined(__WATCOMC__)
//#pragma enable_message (200)
//#endif
//
//}
//
//#endif
#ifndef _AUTO_LOCK_NAME
#define _AUTO_LOCK_NAME CAutoLock
#endif

template <class T> 
class _AUTO_LOCK_NAME
{
public:
	_AUTO_LOCK_NAME(T& lock);
	~_AUTO_LOCK_NAME();
private:
	T& m_rLock;
};

template <class T>
_AUTO_LOCK_NAME<T>::_AUTO_LOCK_NAME(T& lock)
:m_rLock(lock)
{
	m_rLock.Lock();
}

template <class T>
_AUTO_LOCK_NAME<T>::~_AUTO_LOCK_NAME()
{
	m_rLock.Unlock();
}

template <class T> 
class CAutoLockEx
{
public:
	CAutoLockEx(T& lock,bool block = true,bool bManu = false);
	~CAutoLockEx();
	bool Locked(){return m_bLocked;}
	bool TryLock(){m_bLocked = m_rLock.TryLock();return m_bLocked;}
	void UnLock(){if(m_bLocked){ m_bLocked=false; m_rLock.Unlock();}}
private:
	T& m_rLock;
	bool m_bLocked;
};

template <class T>
CAutoLockEx<T>::CAutoLockEx(T& lock,bool block ,bool bManu )
:m_rLock(lock)
,m_bLocked(false)
{
	if(!bManu)
	{
		if (block == true)
		{
			m_rLock.Lock();
			m_bLocked = true;
		}
		else
		{
			m_bLocked = m_rLock.TryLock();
		}
	}
}

template <class T>
CAutoLockEx<T>::~CAutoLockEx()
{
	if(m_bLocked)
		m_rLock.Unlock();
}



#if HAVE_MUTEX_LOCK 
class CMutexLock
	:public ILockItem
{
public:
	CMutexLock(const char* name = 0)
	{
	#ifdef WIN32
		if(name)
		{
			SID_IDENTIFIER_AUTHORITY siaWorld=SECURITY_WORLD_SID_AUTHORITY;   
			PSID psidEveryone=NULL;
			if(!AllocateAndInitializeSid(&siaWorld,1,     
				SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0,     
				&psidEveryone))   
			{   
				printf("AllocateAndInitializeSid()   failed   with   error   %d\n",GetLastError());  
			} 

			ATL::CSid ace(*((SID*)psidEveryone));
			ATL::CDacl dacl;
			dacl.AddAllowedAce(ace,0x10000000);

			ATL::CSecurityDesc sd;
			sd.SetDacl(dacl,false);

			ATL::CSecurityAttributes security(sd,false);
			m_hMutex  = CreateMutexA(&security, FALSE, name); 
			if(psidEveryone)   
				FreeSid(psidEveryone);  
		}
		else
		{
			m_hMutex  = CreateMutexA(NULL, FALSE, name); 
		}
	#else
		pthread_mutexattr_t attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE_NP);
		pthread_mutex_init(&m_hMutex, &attr);
	#endif

	}
	~CMutexLock()
	{
		#ifdef WIN32
			CloseHandle(m_hMutex);
		#else
			pthread_mutex_destroy(&m_hMutex);
		#endif
		
		
	}
	bool TryLock() 
	{
		#ifdef WIN32
			return WaitForSingleObject(m_hMutex,0) == WAIT_OBJECT_0 ;
		#else
			return pthread_mutex_trylock(&m_hMutex) != EBUSY;
		#endif
	}
	long Lock()
	{
	#ifdef WIN32
		if(WaitForSingleObject(m_hMutex,INFINITE) == WAIT_OBJECT_0) 
			return 1;
		return 0;
	#else
		pthread_mutex_lock(&m_hMutex);
		return 1;
	#endif

	}
	long Unlock()
	{
		

	#ifdef WIN32
		ReleaseMutex(m_hMutex);
	#else
		pthread_mutex_unlock(&m_hMutex);
	#endif
		
		return 1;
	}

private:

	pthread_mutex_t  m_hMutex;            // Alias name of the mutex to be protected

};
typedef CMutexLock CMutexLockEx;
#endif

#ifdef WIN32
class CCrtSection
	:public ILockItem
{
public:
	CCrtSection()
	{
		InitializeCriticalSection(&m_csLock);
	}
	~CCrtSection(){
		DeleteCriticalSection(&m_csLock);
	}
	long Lock()
	{
		EnterCriticalSection(&m_csLock);
		return m_csLock.LockCount;
	}
	long Unlock()
	{
		LeaveCriticalSection(&m_csLock);
		return m_csLock.LockCount;
	}
	bool TryLock()
	{
		return TryEnterCriticalSection(&m_csLock) != 0;
	}
private:
	CRITICAL_SECTION m_csLock;
};
typedef CCrtSection CCrtSectionEx;
#else
typedef CMutexLock CCrtSection;
#endif //WIN32

typedef  _AUTO_LOCK_NAME<CCrtSection> CCSLock;


class CAutoCount
{
public:
	CAutoCount (long &lCount)
		:m_lCount(lCount)
	{
		#ifdef WIN32
				InterlockedIncrement(&m_lCount);
		#else
				m_lCount++;
		#endif
			
	}
	~CAutoCount()
	{
		#ifdef WIN32
				InterlockedDecrement(&m_lCount);
		#else
				m_lCount--;
		#endif
				
	}
protected:
	volatile long& m_lCount;
};

#ifdef __APPLE__
#include <libkern/OSAtomic.h>
//
//inline long AtomicIncrement(volatile long* value) {
//    return OSAtomicIncrement32(reinterpret_cast<volatile long*>(value));
//}
//
//inline long AtomicDecrement(volatile long* value) {
//    return OSAtomicDecrement32(reinterpret_cast<volatile long*>(value));
//}

inline long AtomicSwap(volatile long* target, long new_value) {
    long old_value;
    do {
        old_value = *target;
    } while (!OSAtomicCompareAndSwap32(old_value, new_value,
                                       reinterpret_cast<volatile int32_t*>(target)));
    return old_value;
}

class CSingleEntrance
{
public:
	CSingleEntrance():	m_lIn(0)	{	}
	~CSingleEntrance()				{	}
	bool TryLock()					{	return AtomicSwap(&m_lIn,1) == 0;	}
	long Lock()						{	return TryLock();	}
	long Unlock()					{	AtomicSwap(&m_lIn,0);return 0;}
	long state()const{return m_lIn;}
private:
	volatile long m_lIn;
};

#else
class CSingleEntrance
{
public:
	CSingleEntrance():	m_lIn(0)	{	}
	~CSingleEntrance()				{	}
	bool TryLock()					{	return InterlockedCompareExchange(&m_lIn,1,0) == 0;	}
	long Lock()						{	return TryLock();	}
	long Unlock()					{	InterlockedExchange(&m_lIn,0);return 0;}
	long state()const{return m_lIn;}
private:
	volatile long m_lIn;
};

#endif

#endif //__CCSLOCK_H__
