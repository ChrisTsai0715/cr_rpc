#ifndef SELECT_TRACKER_H
#define SELECT_TRACKER_H

#include <list>
#include "base_thread.h"
#include "IReference.h"
#include "ccondition.h"
//#include "select_task.h"

namespace cr_common {

    class select_task;

    class select_tracker : private base_thread,
                           public c_ref
    {
    public:
        select_tracker();
        bool add_task(ref_obj<select_task> task);
        bool del_task(ref_obj<select_task> task);
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
        typedef std::list<ref_obj<select_task> > select_task_list_type;
        condition  _task_cond;
        select_task_list_type _uncomplete_task_lists;
        select_task_list_type _complete_task_lists;
        bool _stop_flag;
    };
}
#endif // SELECT_TRACKER_H
