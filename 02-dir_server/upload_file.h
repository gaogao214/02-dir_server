#pragma once

#include "asio.hpp"
#include <iostream>
#include "common.h"
#include "GxJsonUtility.h"
#include "rapidjson/filereadstream.h"
#include "upload_json.h"
#define SEND_COUNT_SIZE 4096
using namespace std;
using namespace asio::ip;

class upload_file_session :public std::enable_shared_from_this<upload_file_session>
{

public:
	upload_file_session(tcp::socket socket);
	void start();

private:
	void do_recive_filename();
	void do_send_file(const string filename);

	asio::ip::tcp::socket socket_;
	char refile_name[1024] = {0};//�������ֵĳ���	
	std::string file_path_name;//·��+�ļ���
	char* count_file_buf;//�������ַ�
	size_t file_size;//�ļ����ܴ�С	
	string file_string;//�ļ�������


};

class upload_file
{
public:
	upload_file(asio::io_context& io_context, const asio::ip::tcp::endpoint& endpoint)
		:acceptor_(io_context, endpoint)
	{
		do_accept();
	}

	void do_accept()
	{
		acceptor_.async_accept(
			[this](std::error_code ec, asio::ip::tcp::socket socket)
			{
				if (!ec)
				{
					cout << "����˶˿� 12314 �Ϳͻ��˶˿� 12314 ���ӳɹ�\n";
					auto fs = std::make_shared<upload_file_session>(std::move(socket));
					fs->start();
					sessions.push_back(fs);
				}
				else {
					cout << "����˶˿� 12314 �Ϳͻ��˶˿� 12314 ����ʧ��\n";
				}
				do_accept();
			});
	}

private:
	std::vector<std::shared_ptr<upload_file_session>> sessions;
	asio::ip::tcp::acceptor acceptor_;

};