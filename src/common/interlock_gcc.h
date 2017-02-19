#pragma once
/*
Abstract:

    Implementation of Interlocked functions for the Intel x86
    platform. These functions are processor dependent.

--*/

//SET_DEFAULT_DEBUG_CHANNEL(SYNC);

/*++
Function:
  InterlockedIncrement

The InterlockedIncrement function increments (increases by one) the
value of the specified variable and checks the resulting value. The
function prevents more than one thread from using the same variable
simultaneously.

Parameters

lpAddend 
       [in/out] Pointer to the variable to increment. 

Return Values

The return value is the resulting incremented value. 

--*/
inline long
InterlockedIncrement(long volatile *lpAddend)
{
    return __sync_add_and_fetch(lpAddend, (long)1);
}


/*++
Function:
  InterlockedDecrement

The InterlockedDecrement function decrements (decreases by one) the
value of the specified variable and checks the resulting value. The
function prevents more than one thread from using the same variable
simultaneously.

Parameters

lpAddend 
       [in/out] Pointer to the variable to decrement. 

Return Values

The return value is the resulting decremented value.

--*/
inline long
InterlockedDecrement(long volatile *lpAddend)
{
    return __sync_sub_and_fetch(lpAddend, 1);
}


/*++
Function:
  InterlockedExchange

The InterlockedExchange function atomically exchanges a pair of
values. The function prevents more than one thread from using the same
variable simultaneously.

Parameters

Target 
       [in/out] Pointer to the value to exchange. The function sets
       this variable to Value, and returns its prior value.
Value 
       [in] Specifies a new value for the variable pointed to by Target. 

Return Values

The function returns the initial value pointed to by Target. 

--*/
inline long
InterlockedExchange(long volatile *Target, long Value)
{
    return __sync_lock_test_and_set(Target, Value);
}
/*++
Function:
  InterlockedCompareExchange

The InterlockedCompareExchange function performs an atomic comparison
of the specified values and exchanges the values, based on the outcome
of the comparison. The function prevents more than one thread from
using the same variable simultaneously.

If you are exchanging pointer values, this function has been
superseded by the InterlockedCompareExchangePointer function.

Parameters

Destination     [in/out] Specifies the address of the destination value. The sign is ignored. 
Exchange        [in]     Specifies the exchange value. The sign is ignored. 
Comperand       [in]     Specifies the value to compare to Destination. The sign is ignored. 

Return Values

The return value is the initial value of the destination.

--*/
inline long
InterlockedCompareExchange(long volatile *ptr, long new_value, long old_value)
{
    return __sync_val_compare_and_swap(ptr, old_value, new_value); 
}

inline void
MemoryBarrier(void)
{
    __sync_synchronize();
}

