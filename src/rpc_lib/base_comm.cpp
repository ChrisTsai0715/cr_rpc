#include "base_comm.h"
#include "select_task.h"

cr_common::base_comm::base_comm(comm_base_listener *listener)
    :	_listener(listener)
{
}

cr_common::base_comm::~base_comm()
{
}
