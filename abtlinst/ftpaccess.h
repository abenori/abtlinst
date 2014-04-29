#ifndef __FTPACCESS_H__
#define __FTPACCESS_H__

#include <vector>
#include <system/Time.h>

#ifndef INSTALL_FROM_LOCAL

class FtpAccess{
public:
	typedef size_t size_type;
	FtpAccess(const TCHAR *server,const TCHAR *user,const TCHAR *pass);
	~FtpAccess(){Close();}
	bool IsOpen() const{return m_open != NULL;}
	bool Connect(int timeout = 15);
	void Close();

	bool SetProxy(const TCHAR *server);
	bool cd(const TCHAR *dir);

	size_type size(const TCHAR *file);
	ablib::system::Time time(const TCHAR *file);
	bool ls(const TCHAR *pattern,std::vector<ablib::string> &rv);
	struct FileInfo{
		ablib::string name;
		ablib::system::Time time;
		size_type size;
	};
	bool ls (const TCHAR *pattern,std::vector<FileInfo> &rv);
	bool get(const TCHAR *file,std::vector<BYTE> &rv);

private:
	HINTERNET m_open,m_connect;
	DWORD m_timeout;
	ablib::string m_server,m_user,m_pass;


	struct WorkerData{
		virtual void operator()() = 0;
	};
	static DWORD WINAPI Worker(void *d);
	bool WorkWorker(WorkerData *w);
};

#endif // INSTALL_FROM_LOCAL

#endif //__FTPACCESS_H__
