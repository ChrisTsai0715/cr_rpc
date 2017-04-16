#include "base_comm.h"
#include "select_task.h"

cr_common::base_comm::base_comm(comm_base_listener *listener)
    :	_listener(listener)
{
    _select_tracker.run();
}

cr_common::base_comm::~base_comm()
{
    _select_tracker.stop();
}
