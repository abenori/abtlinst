#ifndef __HTTPACCESS_H__
#define __HTTPACCESS_H__

#include <vector>
#include <system/time.h>

/*
HTTPによるアクセス by WinInet．あんまり偉くない．
*/

#ifndef INSTALL_FROM_LOCAL

class HttpAccess  
{
public:
	typedef size_t size_type;
	HttpAccess(const TCHAR *server,const TCHAR *proxy);
	~HttpAccess();
	bool IsOpen(){return m_open != NULL;}
	bool Connect(int timeout = 15);
	void Close();

	bool SendHEAD(const TCHAR *file,const TCHAR *addheader = NULL);
	bool SendGET(const TCHAR *file,const TCHAR *addheader = NULL);

	int status_code();
	size_type content_length();
	ablib::system::Time last_modified();
	bool data(std::vector<BYTE> &rv);

private:
	HINTERNET m_open,m_connect;
	HINTERNET m_hreq;

	DWORD m_timeout;

	ablib::string m_servername;

	struct WorkerData{
		virtual void operator()() = 0;
	};
	static DWORD WINAPI Worker(void *d);
	bool WorkWorker(WorkerData *w);
};

#endif // INSTALL_FROM_LOCAL

#endif //__HTTPACCESS_H__

