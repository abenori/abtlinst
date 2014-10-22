#include "stdafx.h"
#include <algorithm>
#include <boost/lexical_cast.hpp>
#include <system/Time.h>
#include "ftpaccess.h"
#include "Setting.h"

#ifndef INSTALL_FROM_LOCAL
extern Setting g_Setting;

FtpAccess::FtpAccess(const TCHAR *server,const TCHAR *user,const TCHAR *pass)
: m_server(server),m_user(user),m_pass(pass),m_open(NULL),m_connect(NULL)
{
	m_open = ::InternetOpen(_T("abtexinst"),INTERNET_OPEN_TYPE_PRECONFIG,NULL,NULL,0);
	if(m_open == NULL){
		DWORD d = ::GetLastError();
		g_Setting.Log(ablib::string::sprintf(_TR("InternetOpen:GetLastErrorのエラーコード%d"),d));
	}
}

bool FtpAccess::Connect(int timeout){
	if(m_open == NULL)return false;
	m_timeout = timeout*1000;
	struct Data : public WorkerData{
		ablib::string server,user,pass;
		HINTERNET open;
		Data(HINTERNET o,const ablib::string &s,const ablib::string &u,const ablib::string &p) : open(o),server(s),user(u),pass(p){}
		HINTERNET rv;
		void operator()(){rv = ::InternetConnect(open,server.c_str(),
			INTERNET_DEFAULT_FTP_PORT,user.c_str(),pass.c_str(),
			INTERNET_SERVICE_FTP,INTERNET_FLAG_PASSIVE,0);}
	}f(m_open,m_server,m_user,m_pass);
	if(!WorkWorker(&f)){
		g_Setting.Log(ablib::string::sprintf(_TR("%sへの接続でタイムアウト"),m_server.c_str()));
		return false;
	}
	m_connect = f.rv;
	if(m_connect == NULL){
		DWORD d = ::GetLastError();
		g_Setting.Log(ablib::string::sprintf(_TR("InternetConnect:GetLastErrorのエラーコード %d"),d));
		DWORD len = 4096;
		TCHAR buf[4096];
		if(::InternetGetLastResponseInfo(&d,buf,&len)){
			if(d != 0){
				g_Setting.Log(ablib::string::sprintf(_TR("InternetGetLastResponsInfoは，コード%dと，メッセージ%sを返しました"),d,buf));
			}
		}
		return false;
	}
	::InternetSetOption(m_open,INTERNET_OPTION_RECEIVE_TIMEOUT, &m_timeout,sizeof(DWORD));
	::InternetSetOption(m_open,INTERNET_OPTION_SEND_TIMEOUT, &m_timeout,sizeof(DWORD));
	::InternetSetOption(m_open,INTERNET_OPTION_CONNECT_TIMEOUT, &m_timeout,sizeof(DWORD));
	return true;
}

void FtpAccess::Close(){
	if(m_connect){::InternetCloseHandle(m_connect);m_connect = NULL;}
	if(m_open){::InternetCloseHandle(m_open);m_open = NULL;}
}

bool FtpAccess::ls(const TCHAR *pattern,std::vector<ablib::string> &rv){
	std::vector<FileInfo> fis;
	if(!ls(pattern,fis))return false;
	std::for_each(fis.begin(),fis.end(),[&rv](const FileInfo &info){rv.push_back(info.name);});
	return true;
}

bool FtpAccess::ls(const TCHAR *pattern,std::vector<FileInfo> &rv){
	struct FindFirstData : public WorkerData{
		HINTERNET find,connect;
		WIN32_FIND_DATA wfd;
		const TCHAR *filename,*ptr;
		DWORD lasterr;
		FindFirstData(const TCHAR *p,HINTERNET c) : ptr(p),find(NULL),connect(c){::ZeroMemory(&wfd,sizeof(WIN32_FIND_DATA));}
		~FindFirstData(){::InternetCloseHandle(find);}
		void operator()(){
			find = ::FtpFindFirstFile(connect,ptr,&wfd,INTERNET_FLAG_RELOAD,0);
			if(find == NULL)lasterr = ::GetLastError();
		}
	}ffd(pattern,m_connect);
	if(!WorkWorker(&ffd)){g_Setting.Log(_TR("ファイル一覧取得でタイムアウト"));return false;}
	if(ffd.find == NULL){
		DWORD d = ffd.lasterr;
		g_Setting.Log(ablib::string::sprintf(_TR("FtpFindFirstFile:GetLastErrorのエラーコード %d"),d));
		DWORD len = 4096;
		TCHAR buf[4096];
		if(::InternetGetLastResponseInfo(&d,buf,&len)){
			if(d != 0){
				g_Setting.Log(ablib::string::sprintf(_TR("InternetGetLastResponsInfoは，コード%dと，メッセージ%sを返しました"),d,buf));
			}
		}
		return false;
	}
	rv.clear();
	struct FindNextData : public WorkerData{
		HINTERNET find;
		WIN32_FIND_DATA wfd;
		FindNextData(HINTERNET f,WIN32_FIND_DATA w) : find(f),rv(FALSE),wfd(w){::ZeroMemory(&wfd,sizeof(WIN32_FIND_DATA));}
		BOOL rv;
		DWORD lasterr;
		void operator()(){rv = ::InternetFindNextFile(find,&wfd);lasterr = ::GetLastError();}
	}fnd(ffd.find,ffd.wfd);

	do{
		FileInfo fi;
		fi.name = fnd.wfd.cFileName;
		fi.size = fnd.wfd.nFileSizeLow;
		SYSTEMTIME si;
		::FileTimeToSystemTime(&(fnd.wfd.ftLastWriteTime),&si);
		fi.time = si;
		rv.push_back(fi);
		if(!WorkWorker(&fnd)){g_Setting.Log(_TR("ファイル一覧取得でタイムアウト"));return false;}
	}while(fnd.rv);
	return true;
}

bool FtpAccess::cd(const TCHAR *dir){
	struct Data : public WorkerData{
		Data(HINTERNET c,const TCHAR *d) : connect(c),directory(d),rv(FALSE){}
		HINTERNET connect;
		const TCHAR *directory;
		BOOL rv;
		DWORD lasterr;
		void operator()(){rv = ::FtpSetCurrentDirectory(connect,directory);lasterr = ::GetLastError();}
	}d(m_connect,dir);
	if(!WorkWorker(&d)){g_Setting.Log(_TR("カレントディレクトリの変更でタイムアウト"));return false;}
	if(d.rv == FALSE){
		g_Setting.Log(ablib::string::sprintf(_TR("FtpSetCurrentDirectory:GetLastErrorのエラーコード%d"),d.lasterr));
		DWORD len = 4096;
		TCHAR buf[4096];
		DWORD e;
		if(::InternetGetLastResponseInfo(&e,buf,&len)){
			if(e != 0){
				g_Setting.Log(ablib::string::sprintf(_TR("InternetGetLastResponsInfoは，コード%dと，メッセージ%sを返しました"),d,buf));
			}
		}
		return false;
	}else return true;

}

/*
FtpAccess::size_type FtpAccess::size(const TCHAR *file){
	struct Data : public WorkerData{
		HINTERNET find,connect;
		WIN32_FIND_DATA wfd;
		const TCHAR *filename;
		Data(const TCHAR *f,HINTERNET c) : filename(f),find(NULL),connect(c){::ZeroMemory(&wfd,sizeof(WIN32_FIND_DATA));}
		~Data(){::InternetCloseHandle(find);}
		void operator()(){find = ::FtpFindFirstFile(connect,filename,&wfd,INTERNET_FLAG_RELOAD,0);}
	}d(file,m_connect);
	if(!WorkWorker(&d)){g_Setting.Log(ablib::string(file) + _T("のサイズ取得でタイムアウト"));return 0;}
	if(d.find == NULL)return 0;
	return d.wfd.nFileSizeLow;
}

ablib::system::Time FtpAccess::time(const TCHAR *file){
	WIN32_FIND_DATA wfd;
	::ZeroMemory(&wfd,sizeof(WIN32_FIND_DATA));
	HINTERNET find = ::FtpFindFirstFile(m_connect,file,&wfd,INTERNET_FLAG_RELOAD,0);
	if(find == NULL)return ablib::system::Time();
	::InternetCloseHandle(find);
	SYSTEMTIME st;
	if(::FileTimeToSystemTime(&(wfd.ftLastWriteTime),&st))return ablib::system::Time(st);
	else return ablib::system::Time();
}
*/
bool FtpAccess::get(const TCHAR *file,std::vector<BYTE> &rv){
	HINTERNET stream = ::FtpOpenFile(m_connect,file,GENERIC_READ,FTP_TRANSFER_TYPE_BINARY,0);
	if(stream == NULL)return false;
	BYTE buf[4096];
	DWORD size;
	rv.clear();
	do{
		if(!::InternetReadFile(stream,buf,4095,&size)){
			::InternetCloseHandle(stream);
			return false;
		}
		rv.insert(rv.end(),buf,buf + size);
	}while(size);
	::InternetCloseHandle(stream);
	return true;
}

DWORD FtpAccess::Worker(void *d){
	(*reinterpret_cast<WorkerData*>(d))();
	return 0;
}

bool FtpAccess::WorkWorker(WorkerData *d){
	DWORD ThreadID;
	HANDLE thread = ::CreateThread(NULL,0,Worker,d,0,&ThreadID);
	if(::WaitForSingleObject(thread,m_timeout) == WAIT_TIMEOUT){
		Close();
//		::WaitForSingleObject(thread,INFINITE);
		// InternetCloseHandleしても止まらないことがあるので，強制終了． 
		if(::WaitForSingleObject(thread,1000) == WAIT_TIMEOUT)::TerminateThread(thread,10);
		::CloseHandle(thread);
		return false;
	}
	::CloseHandle(thread);
	return true;
}
#endif // INSTALL_FROM_LOCAL