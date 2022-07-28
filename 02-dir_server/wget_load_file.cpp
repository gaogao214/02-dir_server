#include "wget_load_file.h"

wget_load_file::wget_load_file(asio::ip::tcp::socket socket)
	:socket_(std::move(socket))
	,ujs(std::move(socket))
{

}

void wget_load_file::start()
{
	recive_wget_c_file_name();
	ujs.do_opendir();

}

/*接收断点续传名字*/
void wget_load_file::recive_wget_c_file_name()
{
	socket_.async_read_some(asio::buffer(refile_name, 1024),
		[this](std::error_code ec, std::size_t)
		{
			if (!ec)
			{
				std::memcpy(&filelen, refile_name, sizeof(size_t));  //名字的长度

				std::string file_name(refile_name+sizeof(size_t));//名字
				recive_wget_c_file(file_name);
			}
		});

}

/*接收断点续传的文件*/
void wget_load_file::recive_wget_c_file(const string& file_name)  
{
	auto self = shared_from_this();
	//asio::async_read(socket_, asio::buffer(refile_file_len, sizeof(size_t)),   //接收断点续传文件的内容
	socket_.async_read_some(asio::buffer(refile_file_len, sizeof(size_t)),   //接收断点续传文件的内容
		[self,this, file_name](std::error_code ec, std::size_t)
		{
			if (!ec)
			{
			
				memcpy(&buf_len,refile_file_len,sizeof(size_t));   //文件的大小
				cout << "buf_len  " << buf_len << endl;

				wget_c_file_text.resize(buf_len);                 
				
				//asio::async_read(socket_, asio::buffer(wget_c_file_text, buf_len),   //接收
				socket_.async_read_some(asio::buffer(wget_c_file_text, buf_len),   //接收
					[self, this, file_name](std::error_code ec, std::size_t)
					{
						if (!ec)
						{
							const char* readbufs = wget_c_file_text.data();
							

							ofstream wget_c_file(file_name.data(), ios::binary);

							wget_c_file.write(wget_c_file_text.c_str(), buf_len);

							wget_c_file.close();
						
							do_wget_c_file(file_name);
							send_file();
						
						}
					});
			}
		});
		
}

/*解析json文件*/
void wget_load_file::do_wget_c_file(const string& file_name)
{

	//string readbuffer = send_file_context(file_name);
	string readbuffer = ujs.open_json_file(file_name);


	wcfi.deserializeFromJSON(readbuffer.c_str());


}

/*发送 名字 偏移量 内容长度   内容*/    /*比较偏移量*/
void wget_load_file::send_file()
{


	file_size = 0;
	remaining_total = 0;

	
	//wget();
	for (auto& iter : wcfi.wget_c_file_list)//遍历断点续传中的文件
	{ 
		wget_name = iter.wget_name;    //名字
		wget_offset = iter.offset;     //偏移量                            

		file_path_name = profile_.path + "\\" + wget_name;  //找到断点时 本地的文件

		file_size = get_file_len(file_path_name);          //计算字符串长度
		if (wget_offset == file_size)                       //偏移量和文本长度相等   不用发送
		{
			continue;
		}
		else if (wget_offset < file_size)
		{

			remaining_total = file_size - wget_offset;   //计算余下的长度
			cout << remaining_total << endl;
			cout << "余下的长度 >:" << remaining_total << endl;

			//send_file_file(file_path_name, remaining_total, wget_offset, wget_name);
			count_file_buf = get_file_context(file_path_name.c_str(), remaining_total, wget_offset);  //余下的内容
			name_and_offset_remaining = to_string(remaining_total) + "!" + wget_name + "," + to_string(wget_offset) + "*" + count_file_buf;
			size_t list_name_offset_len_buf = name_and_offset_remaining.size();//计算出余下的大小 名字  偏移量  余下的内容  拼在一起的大小

			send_wget_name_and_offset_len.resize(sizeof(size_t) + list_name_offset_len_buf);
			std::memcpy(send_wget_name_and_offset_len.data(), &list_name_offset_len_buf, sizeof(size_t));     //把余下的长度给send_wget_name_and_offset_len.data()
			sprintf(&send_wget_name_and_offset_len[sizeof(size_t) /*+ 1*/], "%s", name_and_offset_remaining.c_str());


			write(send_wget_name_and_offset_len);
		}
	}
}




void wget_load_file::write(const std::string& msg)
{
	std::unique_lock lock(write_mtx_);
	bool write_in_progress = !write_msgs_.empty();
	write_msgs_.push_back(msg);
	if (!write_in_progress)
	{
		do_write();
	}
}



void wget_load_file::do_write()
{


	asio::async_write(socket_, asio::buffer(write_msgs_.front().data(), write_msgs_.front().length()),
		[this](std::error_code ec, std::size_t /*length*/)
		{
			if (!ec)
			{
				std::unique_lock lock(write_mtx_);
				write_msgs_.pop_front();
				if (!write_msgs_.empty())
				{
					cout << "文件发送成功\n";
					do_write();

				}
			}
			else
			{
				socket_.close();
			}
		});
}









