// boost::algorithm::splitのwarningを抑制……なんでstdafx.hを囲めば消えるんだ？
#pragma warning(disable:4996)
#include "stdafx.h"
#pragma warning(default:4996)
#include <stdio.h>
#include <vector>
#include <map>
#include <algorithm>
#include <ablib/IsKanji.h>
#include <atlstr.h>
#include <boost/algorithm/string.hpp>
#include <system/FindFile.h>
#include "func.h"

int lstrcmpn(const TCHAR *a,const TCHAR *b,int n){
	while (--n > 0 && *a != '\0' && *b != '\0' && *a == *b){++a;++b;}
	return (*a - *b);
}



bool GetLocalFileTime(const TCHAR *file,FILETIME *ft){
	WIN32_FIND_DATA wfd;
	HANDLE hfind = ::FindFirstFile(file,&wfd);
	if(hfind == INVALID_HANDLE_VALUE)return false;
	::FindClose(hfind);
//	::CopyMemory(ft ,&wfd.ftLastWriteTime,sizeof(FILETIME));
	*ft = wfd.ftLastWriteTime;
	return true;
}
/*
bool ExistFile(TCHAR *filename){
	HANDLE hFind;
	WIN32_FIND_DATA w32fd;
	hFind = ::FindFirstFile(filename,&w32fd);
	if(hFind == INVALID_HANDLE_VALUE)return false;
	::FindClose(hFind);
	return true;
}
*/

bool ExistFile(const TCHAR *filename){
	if(::GetFileAttributes(filename) == 0xFFFFFFFF)return false;
	else return true;
}

bool CreateDirectoryReflex(const TCHAR *path){
	TCHAR *q = new TCHAR[::lstrlen(path) + 2],*p;
	::lstrcpy(q,path);
#ifdef UNICODE
	for(p = q ; *p ; ++p){
#else
	for(p = q ; *p ; p += ablib::IsKanji(*p) ? 2 : 1){
#endif
		if(*p == '\\'){
			*p = '\0';
			if(ExistFile(q) == false && ::CreateDirectory(q,NULL) == FALSE){
				DEBUGSTRING(_T("CreateDirectoryReflex: ディレクトリ[%s]の作成に失敗．GetLastError() = %d"),q,::GetLastError());
				delete[] q;
				return false;
			}
			*p = '\\';
		}
	}
	delete[] q;
	::CreateDirectory(path,NULL);
	return true;
}

int CheckSameFile(const TCHAR *filename1,const TCHAR *filename2){
	if(::lstrcmp(filename1,_T("")) == 0 || ::lstrcmp(filename2,_T("")) == 0)return FALSE;
	ablib::string f1 = TransRegularPath(filename1);f1.tolower();
	ablib::string f2 = TransRegularPath(filename2);f2.tolower();
	return (f1 == f2 ? TRUE : FALSE);
}

ablib::string TransRegularPath(const ablib::string &fn){
	std::vector<ablib::string> sa,sa2;
	std::vector<ablib::string>::size_type i;
	boost::algorithm::split(sa,fn,boost::algorithm::is_any_of(_T("\\/")));
//	split(sa,const_cast<TCHAR *>(fn),_T("\\"));
	for(i = 0 ; i < sa.size() ; ++i){
		if(sa[i] == _T(".") || sa[i] == _T(""))continue;
		else if(sa[i] == _T(".."))sa2.pop_back();
		else sa2.push_back(sa[i]);
	}
	ablib::string rv = boost::algorithm::join(sa2,_T("\\"));
/*
	for(i = 0 ; i < sa2.size() ; ++i)len += sa2[i].length();
	rv = new TCHAR[len + sa2.size() + 2];
	::lstrcpy(rv,_T(""));
	for(i = 0 ; i < sa2.size() ; ++i){
		if(i != 0)::lstrcat(rv,_T("\\"));
		::lstrcat(rv,sa2[i].c_str());
	}
*/	return rv;
}

void WriteToEditCtrl(HWND hwnd,const ablib::string &msg){
	if(!::IsWindow(hwnd))return;
	static ablib::string m;
	static ATL::CString str;
	static std::map<HWND,bool> last_return;
	if(last_return.find(hwnd) == last_return.end())last_return[hwnd] = false;
	m = msg;
	if(last_return[hwnd])m = _T("\n") + m;
	if(m.rfind(_T("\n")) == m.length() - 1){
		m.erase(m.length() - 1);
		last_return[hwnd] = true;
	}else last_return[hwnd] = false;
	m.replace(_T("\n"),_T("\r\n"));
/*
	if(rewrite){
		CEdit(this->GetDlgItem(id)).GetWindowText(str);
		int r = str.ReverseFind('\n') + 1;
		str.Delete(r,str.GetLength() - r);
		str += m.c_str();
		CEdit(this->GetDlgItem(id)).SetWindowText(str.AllocSysString());
	}else */CEdit(hwnd).AppendText(m.c_str());
}

bool CopyDirectory(const ablib::string &from,const ablib::string &to,bool failfifexist){
	if(::GetFileAttributes(from.c_str()) == -1)return true;
	::CreateDirectory(to.c_str(),NULL);
	WIN32_FIND_DATA w32fd;
	ablib::system::FindFile ff;
	bool rv = true;
	if(ff.Begin(from + _T("\\*.*"))){
		while(ff.Next(&w32fd)){
			if(w32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
				rv = rv && CopyDirectory(from + _T("\\") + w32fd.cFileName,to + _T("\\") + w32fd.cFileName,failfifexist);
			}else{
				rv = rv && (::CopyFile((from + _T("\\") + w32fd.cFileName).c_str(),(to + _T("\\") + w32fd.cFileName).c_str(),failfifexist ? TRUE : FALSE) != FALSE);
			}
		}
	}
	ff.Close();
	return rv;
}

