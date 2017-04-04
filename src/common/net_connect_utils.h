#ifndef NETCONNECTUTILS
#define NETCONNECTUTILS

/*
 * Copyright:   cairui
 * Author:      cairui
 * Date:        2016-10-19
 * Description: using tcp connect to test net.can bind interface or ip, can set timeout.
 *
 * eg: NETCONNECTUTILS("www.google.com", 80, "eth0.2", 3000);
 *     NETCONNECTUTILS("www.baidu.com", 80, "192.168.1.1:34567", 3000);
 *     NETCONNECTUTILS("www.qq.com", 80, 0, 3000);
 */

#include <sys/select.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <assert.h>
#include <stdexcept>
#include "common/ccondition.h"
#include "common/cslock.h"
#include "common/base_thread.h"

#define CONNECTUTILS_BIND NetConnectUtils()
#define CONNECTUTILS_TEST NetConnectUtils()

typedef enum
{
    NCR_OK = 0,
    NCR_GETHOST_TIMEOUT,        //timeout
    NCR_CONNECT_TIMEOUT,
    NCR_CONNECT_ERROR,
    NCR_PRE,            //pre thread not complete
    NCR_BIND_ERROR,     //scoket bind error
}NetConnectRval;

static const char *netconnect_errstr_list[NCR_BIND_ERROR + 1] = {"NCR_OK", "NCR_GETHOST_TIMEOUT", "NCR_CONNECT_TIMEOUT",
                                                                 "NCR_CONNECT_ERROR", "NCR_PRE", "NCR_BIND_ERROR"};

class NetConnectUtils
{
public:
    NetConnectUtils()
        : m_hostent(0),
          m_getHostThread(0)
    {

    }

    ~NetConnectUtils()
    {
        if (m_getHostThread != 0)
            m_getHostThread->SetOuterDelete();
    }

    //bind socket to interface/ip
    int operator()(int sockfd, const char* bindAddr, int bindPort = 0)
    {
        if (bindAddr != 0)
        {
            char buf0[10] = {0};
            char buf1[10] = {0};
            char buf2[10] = {0};
            char buf3[10] = {0};
            if (4 != sscanf(bindAddr, "%[^.].%[^.].%[^.].%[^.]", buf0, buf1, buf2, buf3))
            {
                if (BindSockToIF(sockfd, bindAddr) != 0 || (bindPort != 0 && BindSockToIP(sockfd, INADDR_ANY, bindPort) != 0))
                    return NCR_BIND_ERROR;
            }
            else
            {
                if (!CheckNumber(buf0) || !CheckNumber(buf1) || !CheckNumber(buf2) || BindSockToIP(sockfd, bindAddr, bindPort) != 0)
                    return NCR_BIND_ERROR;
            }
        }

        return NCR_OK;
    }

    //test tcp connect
    int operator()(const char* domain, int port, const char* bindAddr, int timeout_ms)
    {
        int ret = NCR_OK;

        struct in_addr server_host;
        {
            CAutoLock<CMutexLock> cslock(m_threadMutex);
            server_host = this->GetHostName(domain, timeout_ms);
        }

        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) throw std::runtime_error(netconnect_errstr_list[NCR_BIND_ERROR]);

        if (this->operator()(sockfd, bindAddr) != NCR_OK)
        {
            close(sockfd);
            throw std::runtime_error(netconnect_errstr_list[NCR_BIND_ERROR]);
        }

        ret = ConnectToServer(sockfd, &server_host, port, timeout_ms);
        close(sockfd);
        return ret;
    }

private:
    int ConnectToServer(int sockfd, struct in_addr* server_host, int server_port, int timeout_ms)
    {
        struct sockaddr_in server_addr;
        bzero(&server_addr, sizeof(struct sockaddr_in));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr = *server_host;
        server_addr.sin_port = htons(server_port);

        fcntl(sockfd, F_SETFL, O_NONBLOCK);

        int connect_err;
        if (0 == (connect_err = connect(sockfd, (struct sockaddr*)&server_addr, sizeof(struct sockaddr_in))))
            return NCR_OK;
        else if (errno != EINPROGRESS)
            throw std::runtime_error(netconnect_errstr_list[NCR_GETHOST_TIMEOUT]);

        fd_set connect_socket;
        FD_ZERO(&connect_socket);
        FD_SET(sockfd, &connect_socket);
        struct timeval connect_timeout;
        connect_timeout.tv_sec = timeout_ms / 1000;
        connect_timeout.tv_usec = (timeout_ms % 1000) * 1000;

        int select_ret = 0;
        if ((select_ret = select(sockfd + 1, NULL, &connect_socket, NULL, &connect_timeout)) > 0)
        {
            if (FD_ISSET(sockfd, &connect_socket) == 0)
                throw std::runtime_error(netconnect_errstr_list[NCR_CONNECT_TIMEOUT]);

            int err;
            socklen_t err_len = sizeof(err);
            getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &err, &err_len);
            if (err != 0)
                throw std::runtime_error(netconnect_errstr_list[NCR_CONNECT_ERROR]);
        }
        else
        {
            throw std::runtime_error(netconnect_errstr_list[NCR_CONNECT_TIMEOUT]);
        }

        return NCR_OK;
    }

    struct in_addr& GetHostName(const std::string& domain, int timeout_ms)
    {
        if (m_getHostThread != 0)
            m_getHostThread->SetOuterDelete();

        m_hostent = 0;
        m_getHostThread = new GetHostThread(domain, this);
        m_getHostThread->run_once();

        {
            cr_common::auto_cond autoCond(m_cond);
            if (0 != autoCond.time_wait(timeout_ms))
            {
                throw std::runtime_error(netconnect_errstr_list[NCR_GETHOST_TIMEOUT]);
            }
        }

        return *((struct in_addr*)m_hostent->h_addr);
    }

    int BindSockToIF(int sockfd, const char* if_name)
    {
        assert(0 != if_name);

        return setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, if_name, strlen(if_name));
    }

    int BindSockToIP(int sockfd, const char* bind_ip, int bind_port = 0)
    {
        char bind_temp[128] = {0};

        if (0 != bind_ip)
        {
            strncpy(bind_temp, bind_ip, sizeof(bind_temp));
            bind_temp[sizeof(bind_temp) - 1] = 0;
            char *szBindPort = strrchr(bind_temp,':');
            if(szBindPort != NULL)
            {
                bind_port = atoi(&szBindPort[1]);
                *szBindPort = 0;
            }
        }

        struct sockaddr_in bind_addr;
        bzero(&bind_addr, sizeof(struct sockaddr_in));
        bind_addr.sin_family = AF_INET;
        bind_addr.sin_port = htons(bind_port);
        bind_addr.sin_addr.s_addr = bind_ip == 0 ? INADDR_ANY : inet_addr(bind_temp);

        return bind(sockfd, (struct sockaddr*)&bind_addr, sizeof(struct sockaddr_in));
    }

    bool CheckNumber(const char* str)
    {
        if(NULL == str)
        {
            return false;
        }
        else
        {
            while(*str != '\0')
            {
                if(*str <= '9' && *str++ >= '0')
                    continue;
                else
                    return false;
            }
        }

        return true;
    }

private:
    class GetHostThread : private base_thread
    {
    public:
        GetHostThread(const std::string& domain, NetConnectUtils* outer)
            :   m_outer(outer),
                m_domain(domain)
        {
        }

        virtual bool run_once()
        {
            return base_thread::RunOnce();
        }

        virtual void OnExit()
        {
            if (m_outer)
                m_outer->m_getHostThread = 0;
            delete this;
        }

        void SetOuterDelete()
        {
            CAutoLock<CMutexLock> cslock(m_mutex);
            m_outer = 0;
        }

    private:
        virtual bool thread_loop()
        {
            struct hostent* server_ip = gethostbyname(m_domain.c_str());

            CAutoLock<CMutexLock> cslock(m_mutex);
            if (m_outer)
            {
                cr_common::auto_cond autoCond(m_outer->m_cond);
                if (server_ip == NULL)
                    return false;
                m_outer->m_hostent = server_ip;
                autoCond.signal();
            }

            return true;
        }

    private:
        CMutexLock m_mutex;
        std::string m_domain;
        NetConnectUtils* m_outer;
    };

private:
    CMutexLock m_threadMutex;

public:
    cr_common::condition m_cond;
    struct hostent* m_hostent;
    GetHostThread* m_getHostThread;
};
#endif // NETCONNECTUTILS

