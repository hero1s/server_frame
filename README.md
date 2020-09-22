#### 介绍
游戏服务器框架及测试

1:server_frame 为服务器框架

实现以下功能：

1： 异步redis

2： 异步mysql

3： 基于共享内存stl库，方便数据缓存及回档，减轻压力

4： xls表格读取，生成Protobuf文件，方便游戏表格数据处理，支持lua

5： lua的支持，基于sol2插件，实现c++，lua的混合编程

6： 实现Lua服务器基本框架功能，未确定的功能未加入，暂时实现lua的protobuf,json的格式消息

7： 基于单线程的asio网络库(多线程对于高频的游戏类型没有明显提升,增加复杂度)，支持tcp,websocket协议，udp暂无实现

8： 基于四叉树的高性能AOI模块

9： 常用加密解密，及数学计算模块

10：modern c++的一些常用功能，线程池，wheel定时器等等

11：常用的基础格式解析json,xml，ini,cvs等

12：服务器一些基本框架及自检测，自启动等等


#### 依赖:
1 c++ 17 gcc 8.x (linux 下面可以使用命令自动升级到gcc 8.x)

命令：

yum install centos-release-scl

yum install devtoolset-8-toolchain -y

scl enable devtoolset-8 bash

source /opt/rh/devtoolset-8/enable (永久生效请添加到 .bash_profile)

2 linux centeros 7

3 mysqlclient protobuf 

4 cmake


#### 作者：toney QQ:8242117 QQ交流群：196027848

#### 个人项目，持续更新，不保证兼容性，学习交流，具体项目自行二次开发



