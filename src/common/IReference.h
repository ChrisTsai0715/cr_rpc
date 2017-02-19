#ifndef __IREFERENCE__
#define __IREFERENCE__

#ifdef WIN32
  #ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
  #endif
	#include <windows.h>
#else
    #include "interlock.h"
//#define _GLIBCXX_ATOMIC_BUILTINS 1
	#ifndef __stdcall
	#define __stdcall 
	#endif
	//#include <atomic.h>
#endif
/**
 * 引用计数接口，不支持弱引用，建议使用IReferenceEx代替
 * 关于引用计数的基本概念，请搜索互联网
 * 使用boost::shared_ptr会更好
 */
class IReference
{
public:
	virtual ~IReference(){};
	/**
	 * 增加引用
	 */
	virtual unsigned long	__stdcall AddRef() = 0;
	/**
	 * 释放引用
	 */
	virtual unsigned long	__stdcall Release()= 0;
};

class CReference
{
public:
	virtual ~CReference(){};
	CReference ()
		: m_lRef(0),m_lDel(1)	
	{
	}
	virtual unsigned long __stdcall AddRef()
	{
		return InterlockedIncrement(&m_lRef);
	}
	virtual unsigned long __stdcall Release()
	{
		if(InterlockedDecrement(&m_lRef) == 0  )
			if(InterlockedDecrement(&m_lDel) == 0) {
		   	delete this;
		   	return 0;
		   }
		return m_lRef;
	}
public:
	volatile long m_lRef;
	volatile long m_lDel;
};

template<class T>
class CReference_T 
	:virtual private CReference
	,public T
	,virtual public IReference
{
public:	
	typedef T _TYPE;
	typedef IReference _REF;

	CReference_T() : T(){}

	template<typename P>
	explicit CReference_T(P p) : T(p){}

	template<typename P1, typename P2>
	CReference_T(P1 p1, P2 p2) : T(p1, p2){}

	template<typename P1, typename P2, typename P3>
	CReference_T(P1 p1, P2 p2, P3 p3) : T(p1, p2, p3){}

	template<typename P1, typename P2, typename P3, typename P4>
	CReference_T(P1 p1, P2 p2, P3 p3, P4 p4) : T(p1, p2, p3, p4){}

	template<typename P1, typename P2, typename P3, typename P4, typename P5>
	CReference_T(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)	: T(p1, p2, p3, p4, p5){}

	template<typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
	CReference_T(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)	: T(p1, p2, p3, p4, p5, p6){}


	virtual unsigned long __stdcall AddRef()
	{
		return CReference::AddRef();
	}
	virtual unsigned long __stdcall Release()
	{
		return CReference::Release();
	}
protected:
	virtual ~CReference_T(){}
};


/**
 * 引用计数接口，不支持弱引用，建议使用IReferenceEx代替
 * 关于引用计数的基本概念，请搜索互联网
 * 通常使用boost::shared_ptr会更好
 */
template<class T>
class CRefObj
{
public:
	typedef T _PtrClass;
	CRefObj()
	{
		p=0;
	}
	CRefObj(T* lp)
	{
		if ((p = lp) != 0)
			p->AddRef();
	}
	CRefObj(const CRefObj<T>& lp)
	{
		if ((p = lp.p) != 0)
			p->AddRef();
	}
	~CRefObj()
	{
		if (p)
			p->Release();
	}
	operator  T*() const
	{
		return  p;
	}
	T& operator*() const
	{
		return *p;
	}

	/*T** operator&()
	{
		return &p;
	}*/
	T* operator->() const
	{
		return p;
	}
	T* operator=(T* lp)
	{
		if (lp != 0)
			lp->AddRef();
		if (p)
			p->Release();
		p = lp;
		return p;
	}
	T* operator=(const CRefObj<T>& lp)
	{
		if (lp != 0)
			lp->AddRef();
		if (p)
			p->Release();
		p = lp;
		return p;
	}
	bool operator!() const
	{
		return (p == 0);
	}
	
// 	bool operator==(const T* pT) const
// 	{
// 		return p == pT;
// 	}
	T*GetObj(){return p;}

 	const T*GetObj()const{return p;}
	T*p;
};
/**
 * 引用计数的聚合，通常，如果两个对象互相依赖，如果在两个对象内分别保存对方的指针，
 * 那可能会有野指针的问题，如果互相保存引用，会导致循环依赖，导致互相都不会释放，此
 * 问题通过聚合解决
 * 如\code
  struct A; //A通常是生命周期更长的对象，如A是主机管理器对象，B是单个主机对象
  {
     CInternalRefObj<B> b; //A依赖B，此时使用CInternalRefObj保持引用
  };
  struct B:CAggRef_T<A> ;
  {
     B(A&a)
		:CAggRef_T<A>(a) // B 依赖A，使用CAggRef_T保持引用
	 {
	 }
  }
 * \endcode
 *
 * !!!!!!!!!!!还有更好的方法是，使用弱引用，简单，更容易理解!!!!!!!!!!!
 * !!!!!!!!!!!还有更更好的方法是，使用boost的弱引用!!!!!!!!!!!
 *
 */
template<class T>
class CAggRef_T
	:private CReference
	,public IReference
{
public:
	unsigned long __stdcall AddRef()
	{
		m_outer.AddRef();
		return InternalAddRef();
	}
	unsigned long __stdcall Release()
	{
		m_outer.Release();
		return InternalRelease();
	}
	virtual unsigned long InternalAddRef()
	{
		return CReference::AddRef();
	}
	virtual unsigned long InternalRelease()
	{
		return CReference::Release();
	}
	struct INTERNAL_REF:IReference
	{
		INTERNAL_REF()
			:m_pThis(0)
		{

		}
		CAggRef_T * m_pThis;
		unsigned long __stdcall AddRef()
		{
			return m_pThis->InternalAddRef();
		}
		unsigned long __stdcall Release()
		{
			return m_pThis->InternalRelease();
		}
	}m_in;

	T & m_outer;
	CAggRef_T(T&out)
		:m_outer(out)
	{
		m_in.m_pThis = this;
	}
	CAggRef_T(CAggRef_T &out)
		:m_outer(out)
	{
		m_in.m_pThis = this;
	}
	virtual ~CAggRef_T(){}
};
typedef CAggRef_T<IReference> CAggRef;
template<class T>
class CQIPtr
{
public:
	typedef typename T::_TYPE _TYPE;
	typedef typename T::_REF _REF;

	CQIPtr()
		:r(0)
		,p(0)
	{
	}
	template<class X>
	CQIPtr(X*lp)
		:r(0)
		,p(0)
	{
		if ((r = lp) != 0)
		{
			p = lp;
			r->AddRef();
		}
	}
	CQIPtr(const CQIPtr<T>& lp)
	{
		if ((r = lp.r) != 0)
		{
			p = lp.p;
			r->AddRef();
		}
	}
	~CQIPtr()
	{
		if (r)
			r->Release();
	}
	operator  _TYPE*() const
	{
		return  p;
	}
	_TYPE& operator*() const
	{
		return *p;
	}
	_TYPE* operator->() const
	{
		return p;
	}
	template<class Y>
	_TYPE* operator=( Y* lp)
	{
		_TYPE* plp =lp; 
		_REF * rlp = lp;
		if (rlp != 0)
			rlp->AddRef();
		if (r)
			r->Release();
		p = plp;
		r = rlp;
		return p;
	}
	bool operator!() const
	{
		return (p == 0);
	}

private:
	_TYPE*p;
	_REF *r;
}; 
#endif //__IREFERENCE__
