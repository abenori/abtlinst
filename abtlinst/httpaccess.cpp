#include "stdafx.h"
#include <stdio.h>
#include <shlobj.h>
#include <vector>
#include <boost/lexical_cast.hpp>
#include <regex>
#include "httpaccess.h"
#include "Setting.h"

#ifndef INSTALL_FROM_LOCAL
extern Setting g_Setting;

HttpAccess::HttpAccess(const TCHAR *server,const TCHAR *proxy) : m_servername(server),m_hreq(NULL),m_connect(NULL){
	if(proxy && *proxy){
		m_open = ::InternetOpen(_T("abtexinst"),INTERNET_OPEN_TYPE_PROXY,proxy,_T("<local>"),0);
	}else{
		m_open = ::InternetOpen(_T("abtexinst"),INTERNET_OPEN_TYPE_PRECONFIG,NULL,NULL,0);
	}
}

HttpAccess::~HttpAccess(){
	Close();
}

bool HttpAccess::Connect(int timeout){
	m_timeout = timeout * 1000;
	m_connect = ::InternetConnect(m_open,m_servername.c_str(),INTERNET_DEFAULT_HTTP_PORT,NULL,NULL,INTERNET_SERVICE_HTTP,0,NULL);
	if(m_connect == NULL){
		DWORD d = ::GetLastError();
		g_Setting.Log(ablib::string::sprintf(_T("InternetConnect:GetLastErrorのエラーコード %d"),d));
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

void HttpAccess::Close(){
	::InternetCloseHandle(m_hreq);
	::InternetCloseHandle(m_connect);
	::InternetCloseHandle(m_open);
}


bool HttpAccess::SendHEAD(const TCHAR *file,const TCHAR *addheader){
	if(m_hreq != NULL)::InternetCloseHandle(m_hreq);
	m_hreq = ::HttpOpenRequest(m_connect,_T("HEAD"),file,_T("HTTP/1.1"),(_T("http://") + m_servername + file).c_str(),NULL,0,NULL);
	if(m_hreq == NULL)return false;
	struct Data : public WorkerData{
		Data(HINTERNET req,const TCHAR *addh) : r(req),ah(addh){};
		void operator()(){
			rb = ::HttpSendRequest(r,ah,-1,NULL,0);
			if(rb == FALSE){
				g_Setting.Log(ablib::string::sprintf(_T("in HttpAccess::SendHEAD, HttpSendRequest:GetLastError = %d"),::GetLastError()));
			}
		}
		BOOL rb;HINTERNET r;const TCHAR *ah;
	}f(m_hreq,addheader);
	if(!WorkWorker(&f))g_Setting.Log(_TR("HEADリクエストでタイムアウト"));
	return f.rb != FALSE;
}

bool HttpAccess::SendGET(const TCHAR *f,const TCHAR *addheader){
	ablib::string file = f;
	ablib::string server = m_servername;
	for(int i = 0 ; i < 5 ; ++i){
		if(m_hreq != NULL)::InternetCloseHandle(m_hreq);
		m_hreq = ::HttpOpenRequest(m_connect,_T("GET"),file.c_str(),_T("HTTP/1.1"),(_T("http://") + server + file).c_str(),NULL,0,NULL);
		if(m_hreq == NULL){
			g_Setting.Log(ablib::string::sprintf(_T("HttpOpenRequest:GetLastError = %d"),::GetLastError()));
			return false;
		}
		struct Data : public WorkerData{
			Data(HINTERNET req,const TCHAR *addh) : r(req),ah(addh){};
			void operator()(){
				rb = ::HttpSendRequest(r,ah,-1,NULL,0);
				if(rb == FALSE){
					g_Setting.Log(ablib::string::sprintf(_T("in HttpAccess::SendGET, HttpSendRequest:GetLastError = %d"),::GetLastError()));
				}
			}
			BOOL rb;HINTERNET r;const TCHAR *ah;
		}f(m_hreq,addheader);
		if(!WorkWorker(&f))g_Setting.Log(_TR("GETリクエストでタイムアウト"));
		int sc = status_code();
//		g_Setting.Log(server + file + _T("に接続：status code = ") + boost::lexical_cast<ablib::string>(sc));
		if((sc / 100) != 3)return f.rb != FALSE;
		TCHAR loc[1024];
		DWORD size = sizeof(loc);
		file = _T("");server = _T("");
		if(::HttpQueryInfo(m_hreq,HTTP_QUERY_LOCATION,&loc,&size,0)){
			std::wregex reg;
			try{reg.assign(_T("^.*?://(.*?)(/.*)$"));}catch(...){ASSERT(false);}
			std::wsmatch res;
			if(std::regex_search(ablib::string(loc),res,reg)){
				server = res.str(1);
				file = res.str(2);
			}
		}
		if(file == _T("") || server == _T("")){
			g_Setting.Log(_T("リダイレクト先の取得に失敗"));
			return false;
		}
	}
	g_Setting.Log(_T("5回以上のリダイレクトが行われたので中止します．"));
	return false;
}

int HttpAccess::status_code(){
	ASSERT(m_hreq != NULL);
	DWORD sc;
	DWORD size = sizeof(DWORD);
	BOOL ret = ::HttpQueryInfo(m_hreq,HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER,&sc,&size,0);
	if(ret == FALSE){
		g_Setting.Log(ablib::string::sprintf(_T("in HttpAccess::status_code, HttpQueryInfo:GetLastError = %d"),::GetLastError()));
	}
	return (ret != FALSE ? sc : 400);
}

HttpAccess::size_type HttpAccess::content_length(){
	DWORD size = sizeof(size_type);
	size_type rv = 0;
	BOOL ret = ::HttpQueryInfo(m_hreq,HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER,&rv,&size,0);
	if(ret == FALSE){
		g_Setting.Log(ablib::string::sprintf(_T("in HttpAccess::content_length, HttpQueryInfo:GetLastError = %d"),::GetLastError()));
	}
	return ret != FALSE ? rv : 0;
}

ablib::system::Time HttpAccess::last_modified(){
	SYSTEMTIME st;
	DWORD size = sizeof(SYSTEMTIME);
	BOOL ret = ::HttpQueryInfo(m_hreq,HTTP_QUERY_LAST_MODIFIED  | HTTP_QUERY_FLAG_SYSTEMTIME,&st,&size,0);
	if(ret == FALSE){
		g_Setting.Log(ablib::string::sprintf(_T("in HttpAccess::last_modified, HttpQueryInfo:GetLastError = %d"),::GetLastError()));
	}
	ablib::system::Time rv(st);
	if(!ret)rv.SetValid(false);
	return rv;
}

bool HttpAccess::data(std::vector<BYTE> &rv){
	struct Data : public WorkerData{
		Data(HINTERNET req) : hreq(req){}
		void operator()(){
			rv = ::InternetReadFile(hreq,buf,4095,&size);
			if(rv == FALSE){
				DWORD d = ::GetLastError();
				g_Setting.Log(ablib::string::sprintf(_T("InternetReadFile:GetLastError = %d"),d));
				DWORD len = 4096;
				TCHAR b[4096];
				if(d = ERROR_INTERNET_EXTENDED_ERROR && ::InternetGetLastResponseInfo(&d,b,&len)){
					g_Setting.Log(ablib::string::sprintf(_TR("InternetReadFile:InternetGetLastResponsInfo code = %d, msg = %s"),d,buf));
				}
			}
		}
		BYTE buf[4096];HINTERNET hreq;DWORD size;BOOL rv;
	}f(m_hreq);
	do{
		if(!WorkWorker(&f)){g_Setting.Log(_TR("GETリクエストでタイムアウト"));return false;}
		if(!f.rv)return false;
		rv.insert(rv.end(),f.buf,f.buf + f.size);
	}while(f.size);
	return true;
}


DWORD HttpAccess::Worker(void *d){
	(*reinterpret_cast<WorkerData*>(d))();
	return 0;
}

bool HttpAccess::WorkWorker(WorkerData *d){
	DWORD ThreadID;
	HANDLE thread = ::CreateThread(NULL,0,Worker,d,0,&ThreadID);
	if(::WaitForSingleObject(thread,m_timeout) == WAIT_TIMEOUT){
		Close();
		::WaitForSingleObject(thread,INFINITE);
		::CloseHandle(thread);
		return false;
	}
	::CloseHandle(thread);
	return true;
}
#endif // INSTALL_FROM_LOCAL