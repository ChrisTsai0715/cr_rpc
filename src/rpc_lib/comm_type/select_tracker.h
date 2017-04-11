#ifndef SELECT_TRACKER_H
#define SELECT_TRACKER_H

#include "base_thread.h"
#include "IReference.h"
#include "select_task.h"

namespace cr_rpc {

    class select_task;

    class select_tracker : private base_thread
    {
    public:
        select_tracker();
        bool add_task(CRefObj<select_task> task);
        bool del_task(CRefObj<select_task> task);
        bool del_task(int socket_fd, int task_type);

        virtual bool run()
        {
            _stop_flag = false;
            return base_thread::run();
        }

        virtual bool stop()
        {
            _stop_flag = true;
            return base_thread::stop();
        }

    private:
        virtual bool thread_loop();

    private:
        typedef std::list<CRefObj<select_task> > select_task_list_type;
        cr_common::condition  _task_cond;
        select_task_list_type _uncomplete_task_lists;
        select_task_list_type _complete_task_lists;
        bool _stop_flag;
    };
}
#endif // SELECT_TRACKER_H
