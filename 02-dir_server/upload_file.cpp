#include "upload_file.h"
#include "fstream"

	//upload_json_session* upload_file_json = nullptr;
	upload_file_session::upload_file_session(asio::ip::tcp::socket socket)
		:socket_(std::move(socket))
	{
		
	}

	void upload_file_session::start()
	{
		do_recive_filename();	
	}

	void upload_file_session::do_recive_filename()//接收文件名字
	{
		std::memset(refile_name, 0, 1024);//清空内存
		socket_.async_read_some(asio::buffer(refile_name, 1024),
			[this](std::error_code ec, std::size_t sz)
			{
				if (!ec)
				{
					std::size_t name_len = 0;
					std::memcpy(&name_len, refile_name, sizeof(size_t));

					std::string file_name(refile_name + sizeof(size_t));
					cout << "发送文件的名字>: " << file_name << endl;
					do_send_file(file_name);
				} 
			});
	}

	void upload_file_session::do_send_file(const string filename)//发送文件内容
	{	
		file_string.clear();	
		file_size = 0;
		file_path_name = profile_.path + "\\" + filename;
			//一次性读出来  开始

 		ifstream file(file_path_name.c_str(), ios::in | ios::binary);
		if (!file.is_open())//检查文件是否存在
			return;

		file.seekg(0, ios_base::end); 
		file_size = file.tellg();//list.json文本的大小
		file.seekg(0, ios_base::beg);
		
		count_file_buf = new char[file_size];
		while (!file.eof())//没有结束
		{
			file.read(count_file_buf, file_size);	
		}			
		file.close();
		//结束
	
		file_string.resize(sizeof(size_t) + file_size + 1024);
		std::memcpy(file_string.data(), &file_size, sizeof(size_t));
		
		file_string[sizeof(size_t)] = '*';
		
		std::sprintf(&file_string[sizeof(size_t)+1], "%s", count_file_buf);
		
		delete count_file_buf;
		count_file_buf = nullptr;
		asio::async_write(socket_, asio::buffer(file_string.data(), file_string.size()),
			[this,filename](std::error_code ec, std::size_t sz)
			{
				if (!ec)
				{
					std::cout <<filename.data() << "  文件发送成功\n";
					do_recive_filename();									
				}				
			});
	}



