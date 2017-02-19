#include "fifo_comm.h"

using namespace cr_rpc;

int fifo_comm::_create_path(const std::string &path)
{
    int ret = mkfifo(path.c_str(), 0644);
    if (ret < 0 && errno != EEXIST)
        return ret;

    return open(path.c_str(), O_RDWR | O_NONBLOCK, 0); //open as RDWR,
}

/* ****************************
 *
 *	fifo_comm_server
 *
 * ****************************/
bool fifo_comm_server::accept()
{
    typedef bool(fifo_comm_server::*func)(const char*, ssize_t);
    return _select_tracker.add_task(
                    fifo_accept_task<fifo_comm_server*, func>::new_instance(_accept_fd, this, &fifo_comm_server::accept_done)
                );
}

bool fifo_comm_server::read()
{
    return _read(_read_fd);
}

bool fifo_comm_server::write(const char *buf, size_t size)
{
    return _write(_write_fd, buf, size);
}

bool fifo_comm_server::start_listen(const std::string &path)
{
    _accept_fd = _create_path(path);
    return this->accept();
}

bool fifo_comm_server::accept_done(const char* buf, ssize_t size)
{
    if (size == 0)
    {
        this->accept();
        return false;
    }

    _client_pid = atoi(buf);
    char write_path[30] = {0};
    sprintf(write_path, "/tmp/%d_s2c", _client_pid);
    char read_path[30] = {0};
    sprintf(read_path, "/tmp/%d_c2s", _client_pid);

    _write_fd = ::open(write_path, O_WRONLY | O_NONBLOCK, 0);
    _read_fd  = ::open(read_path,  O_RDONLY | O_NONBLOCK, 0);
    if (_write_fd < 0 || _read_fd < 0)
    {
        perror("accept error");
        return false;
    }

    char reply_buf[30] = {0};
    sprintf(reply_buf, "connect ok:%s", read_path);
    this->write(reply_buf, strlen(reply_buf) + 1);

    _accept_flag = true;

    while(_accept_flag)
    {
        usleep(200 * 1000);
        this->read();
    }

    if (_listener)
        _listener->_client_connect(_client_pid);

    return true;
}

bool fifo_comm_server::stop_listen()
{
    return _select_tracker.del_task(_accept_fd, select_task::TASK_SELECT_ACCEPT)
            && close(_accept_fd) == 0;
}

bool fifo_comm_server::disconnect_client()
{
    if (_client_pid > 0)
    {
        _select_tracker.del_task(_read_fd, select_task::TASK_SELECT_READ);
        _select_tracker.del_task(_write_fd, select_task::TASK_SELECT_WRITE);
        close(_read_fd);
        close(_write_fd);
        _read_fd = _write_fd = _client_pid = -1;
    }
    return true;
}

bool fifo_comm_server::_read_done(int id, char *buf, ssize_t size)
{
    if (_accept_flag)
    {
        if (size == 0) return false;
        _accept_flag = false;
        return true;
    }

    return base_comm_server::_read_done(id, buf, size);
}

/* ****************************
 *
 *	fifo_comm_client
 *
 * ****************************/
bool fifo_comm_client::start_connect(const std::string &path, unsigned int timeout_ms)
{
    if ((_connect_fd = ::open(path.c_str(), O_WRONLY | O_NONBLOCK, 0)) < 0)
    {
        perror("fifo open err");
        throw std::runtime_error("file not exsit");
    }

    pid_t my_pid = getpid();
    char pid_buf[20] = {0};
    sprintf(pid_buf, "%d", my_pid);
    char read_path[20] = {0};
    char write_path[20] = {0};
    sprintf(read_path, "/tmp/%d_s2c", my_pid);
    sprintf(write_path, "/tmp/%d_c2s", my_pid);

    if (mkfifo(read_path, 0644) < 0)
    {
        if (errno != EEXIST)
        {
            perror("mkfifo read err");
            throw std::runtime_error("mkfifo err");
        }
    }

    if (mkfifo(write_path, 0644) < 0)
    {
        if (errno != EEXIST)
        {
            perror("mkfifo write err");
            throw std::runtime_error("mkfifo err");
        }
    }

    _read_fd  = ::open(read_path,  O_RDONLY | O_NONBLOCK, 0);

    if (_read_fd < 0)
    {
        perror("open connect err");
        throw std::runtime_error("open connect err");
    }

    _connect_flag = true;
    _write(_connect_fd, pid_buf, strlen(pid_buf));

#define sleep_time_ms 200 * 1000
    int sleep_count = timeout_ms * 1000 / sleep_time_ms;
    do
    {
        this->read();
        sleep_count --;
        usleep(sleep_time_ms);
    } while(_connect_flag && sleep_count > 0);

    if (sleep_count <= 0)
    {
        close(_read_fd);
        _read_fd = 0;
        return false;
    }

    close(_connect_fd);
    return true;
}

bool fifo_comm_client::disconnect_server()
{
    _select_tracker.del_task(_read_fd, select_task::TASK_SELECT_READ);
    _select_tracker.del_task(_write_fd, select_task::TASK_SELECT_WRITE);

    close(_read_fd);
    close(_write_fd);

    _read_fd = _write_fd = -1;

    return true;
}

bool fifo_comm_client::read()
{
    return _read(_read_fd);
}

bool fifo_comm_client::write(const char *buf, size_t size)
{
    return _write(_write_fd, buf, size);
}

bool fifo_comm_client::_read_done(int id, char *buf, ssize_t size)
{
    if (_connect_flag)
    {
        if (size == 0) return false;
        _connect_flag = false;
        char write_path[20] = {0};
        sscanf(buf, "connect ok:%s", write_path);
        _write_fd = ::open(write_path, O_WRONLY | O_NONBLOCK, 0);
        if (_write_fd < 0) return false;
        this->write("accept ok", 10);
        return true;
    }

    return base_comm_client::_read_done(id, buf, size);
}
