#ifndef CCONDITION
#define CCONDITION

#include <pthread.h>
#include <sys/time.h>
#include "common/cslock.h"

namespace cr_common
{
    class condition
    {
    public:
        explicit condition()
            :   _cond(PTHREAD_COND_INITIALIZER),
                _mutex(PTHREAD_MUTEX_INITIALIZER)
        {
        }

        ~condition()
        {
            pthread_mutex_destroy(&_mutex);
            pthread_cond_destroy(&_cond);
        }

        int time_wait(int timeout_ms)
        {
            struct timespec ts;
            struct timeval tt;
            gettimeofday(&tt,NULL);
            __useconds_t  timeout_sec = timeout_ms / 1000;
            __suseconds_t timeout_us = (timeout_ms % 1000) * 1000;
            ts.tv_sec = tt.tv_sec + timeout_sec;
            ts.tv_nsec = tt.tv_usec * 1000 + timeout_us * 1000;
            ts.tv_sec += ts.tv_nsec / (1000 * 1000 *1000);
            ts.tv_nsec %= (1000 * 1000 * 1000);
            if (ts.tv_nsec >= 1000 * 1000 * 1000)
            {
                ts.tv_nsec -= 1000 * 1000 * 1000;
                ts.tv_sec++;
            }

            return pthread_cond_timedwait(&_cond, &_mutex, &ts);
        }

        int lock()
        {
            return pthread_mutex_lock(&_mutex);
        }

        int unlock()
        {
            return pthread_mutex_unlock(&_mutex);
        }

        int wait()
        {
            return pthread_cond_wait(&_cond, &_mutex);
        }

        int signal()
        {
            return pthread_cond_signal(&_cond);
        }

        int broadcast()
        {
            return pthread_cond_broadcast(&_cond);
        }

    private:
        pthread_cond_t _cond;
        pthread_mutex_t _mutex;
    };

    class auto_cond
    {
    public:
        explicit auto_cond(condition& cond)
            :   m_cond(cond)
        {
            m_cond.lock();
        }

        ~auto_cond()
        {
            m_cond.unlock();
        }

        int time_wait(int timeout_ms)
        {
            return m_cond.time_wait(timeout_ms);
        }

        int wait()
        {
            return m_cond.wait();
        }

        int signal()
        {
            m_cond.unlock();
            return m_cond.signal();
        }

        int broadcast()
        {
            m_cond.unlock();
            return m_cond.broadcast();
        }

    private:
        condition &m_cond;
    };
}

#endif // CCONDITION

