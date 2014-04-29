#pragma once
#include <vector>
#include <system/Time.h>
#include <functional>


class Network
{
public:
	Network(void){}
	~Network(void){}

	struct FileData{
		ablib::string file;
		ablib::system::Time time;
		unsigned int size;
	};
	
#ifndef INSTALL_FROM_LOCAL
	void SetProxy(const ablib::string &proxy,int port){m_proxy = ProxyData(proxy,port);}
	struct ProxyData{
		ProxyData(const ablib::string &s,int p) : proxy(s),port(p){}
		ProxyData(){}
		ablib::string proxy;
		int port;
	};
	ProxyData GetProxy(){return m_proxy;}

	bool GetFileData(std::vector<FileData> &rv,const ablib::string &url,MSGFUNC &msgfunc);
private:
	bool GetFileDataFTP(std::vector<FileData> &rv,const ablib::string &server,const ablib::string &path,MSGFUNC &msgfunc);
	bool GetFileDataHTTP(std::vector<FileData> &rv,const ablib::string &server,const ablib::string &path,MSGFUNC &msgfunc);
	int FromStrSize2IntSize(const ablib::string &s);
	DWORD FromStrMonth2IntMonth(const ablib::string &month);

	ProxyData m_proxy;
#endif // INSTALL_FROM_LOCAL
};

