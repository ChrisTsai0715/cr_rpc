#include "base_comm.h"

bool cr_rpc::base_comm::_read(int id)
{
    CAutoLock<CMutexLock> cslock(_read_mutex);
    if (id == -1) return false;
    char read_buf[2048] = {0};
    ssize_t read_size = 0;
    do
    {
        read_size = ::read(id, read_buf, sizeof read_buf);
        if (read_size == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                return _select_tracker.add_task(
                            read_task::new_instance(id, this)
                            );
            }
            return false;
        }
        _on_data_receive(id, read_buf, read_size);
    }while(read_size == sizeof read_buf);

    return true;
}

bool cr_rpc::base_comm::_write(int id, const char *buf, size_t size)
{
    CAutoLock<CMutexLock> cslock(_write_mutex);
    if (id == -1) return false;

    ssize_t write_size = 0;
    do
    {
        write_size = ::write(id, buf, size);
        if(write_size == -1)
        {
            if (errno != EAGAIN && errno != EWOULDBLOCK)
            {
                return _select_tracker.add_task(
                            write_task::new_instance(id, this, buf, size)
                            );
            }

            return false;
        }
        _on_data_send(id, write_size);
        size -= write_size;
        buf  += write_size;
    }while(write_size > 0 && size > 0);

    return true;
}
