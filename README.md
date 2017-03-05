# cr_rpc
>
Copyright: cairui</br>
Author: cairui</br>
Date: 2017-02-25</br>
Description: using tcp connect to test net.can bind interface or ip, can set timeout.</br>

## Usage:
在项目里，include "rpc_lib/base_rpc_client.h" 或是 "rpc_lib/base_rpc_server.h"，取决于希望成为rpc服务的客户端还是服务端；</br>
写一个类来继承base_rpc_client或是base_rpc_server，默认使用unix socket通讯方式进行rpc通讯，也可以将父类的构造函数传入fifo参数以使用fifo的方式进行rpc通讯；</br>
写一个类来继承base_rpc_service，实现invoke纯虚方法。此类的作用为实现rpc服务中的命令方法，具体可参考源码中的实现。</br>
