// 02-dir_server.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
// 程序可能会发生异常，发生原因之一：因为list.json  id.json 文件没有删除

#include <iostream>
#include "asio.hpp"
#include "upload_json.h"
#include <memory>
#include "upload_file.h"
#include "wget_load_file.h"


//发送文件
void create()
{

    asio::io_context io_context_;
    asio::ip::tcp::endpoint _endpoint(asio::ip::tcp::v4(), std::atoi("12314"));

    auto cs = std::make_shared<upload_file>(io_context_, _endpoint);

    io_context_.run();

}

//断点续传的文件
void create1()
{

    asio::io_context io_context_1;
    asio::ip::tcp::endpoint endpoint_1(asio::ip::tcp::v4(), std::atoi("12313"));

    wget_c_file_accept s(io_context_1, endpoint_1);


    io_context_1.run();

}


int main()
{
    system("del wget_c_file.json");

    std::thread t(std::bind(create));

    t.detach();

    std::thread t1(std::bind(create1));

    t1.detach();

    asio::io_context io_context;

    asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), std::atoi("12312"));

   // std::shared_ptr<server> s = std::make_shared<server>(io_context, endpoint);
     upload_json s(io_context, endpoint);

    io_context.run();
    return 0;
}


