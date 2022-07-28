#pragma once
#include "asio.hpp"
#include "rapidjson/rapidjson.h"
#include "GxJsonUtility.h"
#include "common.h"
#include <iostream>
#include <vector>
#include <io.h>
#include <fstream>
#include <filesystem>
#include "rapidjson/filereadstream.h"



using namespace std;

namespace filestruct {

	struct ip_and_port {
		std::string ip;
		std::string port;
		bool operator<(const ip_and_port& b)const { return  port < b.port; }
		GX_JSON(ip, port);
	};
	struct id_and_server {
		int id;
		set<ip_and_port> server;
		GX_JSON(id, server);
	};
	struct files_Server {
		unordered_map<int, id_and_server> blocks;
		//vector<id_and_server> blocks;
		GX_JSON(blocks);
	};
	struct profile {//配置文件
		std::string path;
		std::string port;
		GX_JSON(path,port);
	};
	struct wget_c_file
	{
		std::string wget_name;
		uint32_t offset;
		GX_JSON(wget_name, offset);
	};
	struct wget_c_file_info
	{
		vector<wget_c_file> wget_c_file_list;
		GX_JSON(wget_c_file_list);
	};
};

//class upload_json_session;
//extern upload_json_session* upload_file_json;
extern filestruct::profile profile_; 

class upload_json_session
{
public:

	upload_json_session(asio::ip::tcp::socket socket);

	void start();
	
	std::string open_json_file(const std::string& json_name);

	int get_file_len(const std::string& filename);//读文件的长度

	std::string get_file_context(const std::string& filename);//读文件的内容

	void do_opendir();

	void save_json_file(filestruct::files_Server files_id, const string& name);
	
	void parse_id_json();
	
private:

	void delete_all_ip_port();

	void do_send_list();//发送 list.json 名字长度和名字

	void do_send_id();//发送 id.json 名字长度和名字

	void recive_id_port();//接收客户端传过来的id和port
	
	void gsh(std::string& strtxt);

	void delete_ip_port(filestruct::ip_and_port delete_ip_port);
	
	void save_file_server_ip_port_id_json();//在id.json里添加传输(a.txt...)的服务器ip port

	asio::ip::tcp::socket socket_;

	string profile_name = "open.json";//配置文件名
	string list_name = "list.json";
	string id_name = "id.json";
	string list_name_text;//list.josn的长度+名字
	string send_list_file;//发送list.json文本的长度+内容
	string send_id_name;//id.josn的长度+名字
	string send_id_file;//发送id.json文本的长度+内容

	char id_port[1024] = {0};//接收的内容
	int id;
	filestruct::files_Server files_id;
	filestruct::files_Server files_id1;
	filestruct::ip_and_port save_ip_port;
	
};

class upload_json
{
public:
	upload_json(asio::io_context& io_context, const asio::ip::tcp::endpoint& endpoint)
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
					cout << "服务端端口 12312 和客户端端口 12312 连接成功\n";
					auto s = std::make_shared<upload_json_session>(std::move(socket));
					s->start();
					sessions.push_back(s);
				}
				else {
					cout << "服务端端口 12312 和客户端端口 12312 连接失败\n";
				}
				do_accept();
			});
	}


private:
	asio::ip::tcp::acceptor acceptor_;
	std::vector<std::shared_ptr<upload_json_session>> sessions;
};