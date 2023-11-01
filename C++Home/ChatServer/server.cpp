#include<iostream>
#include<string>
#include<sys/epoll.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netinet/in.h>
#include<map>
const int MAX_Conn = 1024;
struct Client
{
	int sockfd;
	std::string name;//用户名
};

int main(){
	//epoll
	int epid = epoll_create1(0);
	if (epid<0)
	{
		perror("epoll create error");
		return -1;
	}
	//socket
	int sockfd=socket(AF_INET, SOCK_STREAM,0);
	if (sockfd<0)
	{
		perror("socket error");
		return -1;
	}
	//绑定 ip 端口
	struct sockaddr_in addr1;
	addr1.sin_family = AF_INET;
	addr1.sin_addr.s_addr = htonl(INADDR_ANY);
	addr1.sin_port = htons(9999);

	int ret = bind(sockfd, (struct sockaddr*)&addr1, sizeof(addr1));
	if (sockfd < 0)
	{
		std::cout << "bind error" << std::endl;
		return -1;
	}
	//监听
	ret=listen(sockfd, 1024);
	if (ret < 0)
	{
		std::cout << "listen error" << std::endl;
		return -1;
	}
	//进入epoll
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = sockfd;

	ret = epoll_ctl(epid, EPOLL_CTL_ADD, sockfd, &ev);
	if (ret < 0)
	{
		std::cout << "epoll_ctl error" << std::endl;
		return -1;
	}
	std::map<int, Client>clients;

	while (1)
	{
		struct epoll_event evs[MAX_Conn];
		int n=epoll_wait(epid, evs, MAX_Conn, -1);
		if (n < 0)
		{
			std::cout << "epoll_wait error" << std::endl;
			break;
		}
		for (int i = 0;i < n;i++)//自身消息
		{
			int fd = evs[i].data.fd;
			if (fd == sockfd)
			{
				struct  sockaddr_in client_addr;
				socklen_t client_addr_len = sizeof(client_addr);
				int client_sockfd = accept(sockfd, (struct sockaddr*)&client_addr,&client_addr_len);
				if (client_sockfd < 0)
				{
					std::cout << "accept error" << std::endl;
					continue;
				}
				//epoll 加入epoll
				struct epoll_event ev_client;
				ev_client.events = EPOLLIN;//检测消息
				ev_client.data.fd = client_sockfd;

				ret = epoll_ctl(epid, EPOLL_CTL_ADD, client_sockfd, &ev_client);
				if (ret < 0)
				{
					std::cout << "customer epoll_ctl error" << std::endl;
					break;
				}
				//std::cout<<client_addr.sin_addr.s_addr << "connecting...."<< std::endl;
				//保存客户端信息
				Client client;
				client.sockfd = client_sockfd;
				client.name = "";
				clients[client_sockfd] = client;
			}
			else//客户端消息
			{
				char buffer[1024];
				int n = read(fd, buffer, 1024);
				if (n < 0)
				{
					break;//错误
				}
				else if(n==0)
				{
					close(fd);
					epoll_ctl(epid, EPOLL_CTL_DEL, fd, 0);
					clients.erase(fd);
				}
				else
				{
					std::string msg(buffer, n);
					if (clients[fd].name == "")
					{
						//name 为空 自身消息
						clients[fd].name = msg;
					}
					else
					{
						std::string name = clients[fd].name;
						for (auto& c : clients)
						{
							if(c.first!=fd)
							{
								write(c.first, ('[' + name + ']' + ": " + msg).c_str(), msg.size() + name.size() + 4);
							}
						}
					}
				}
			}
		}
	}
	//exit epoll
	close(epid);
	close(sockfd);
}