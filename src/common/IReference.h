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
 * ���ü����ӿڣ���֧�������ã�����ʹ��IReferenceEx����
 * �������ü����Ļ������������������
 * ʹ��boost::shared_ptr�����
 */
class IReference
{
public:
	virtual ~IReference(){};
	/**
	 * ��������
	 */
	virtual unsigned long	__stdcall AddRef() = 0;
	/**
	 * �ͷ�����
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
 * ���ü����ӿڣ���֧�������ã�����ʹ��IReferenceEx����
 * �������ü����Ļ������������������
 * ͨ��ʹ��boost::shared_ptr�����
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
 * ���ü����ľۺϣ�ͨ�������������������������������������ڷֱ𱣴�Է���ָ�룬
 * �ǿ��ܻ���Ұָ������⣬������ౣ�����ã��ᵼ��ѭ�����������»��඼�����ͷţ���
 * ����ͨ���ۺϽ��
 * ��\code
  struct A; //Aͨ�����������ڸ����Ķ�����A����������������B�ǵ�����������
  {
     CInternalRefObj<B> b; //A����B����ʱʹ��CInternalRefObj��������
  };
  struct B:CAggRef_T<A> ;
  {
     B(A&a)
		:CAggRef_T<A>(a) // B ����A��ʹ��CAggRef_T��������
	 {
	 }
  }
 * \endcode
 *
 * !!!!!!!!!!!���и��õķ����ǣ�ʹ�������ã��򵥣����������!!!!!!!!!!!
 * !!!!!!!!!!!���и����õķ����ǣ�ʹ��boost��������!!!!!!!!!!!
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
