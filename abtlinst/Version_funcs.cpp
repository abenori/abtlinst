#include "stdafx.h"
#include "Version.h"
#include <boost/algorithm/string.hpp>
#include <vector>

void OutputVersionInfo(){
	::OutputDebugString((_T("__DATE__ = ")  + VersionDate() + _T("\n__TIME__ = ") + VersionTime() + _T("\n")).c_str());
}


ablib::string Version(){
	TCHAR path[MAX_PATH + 1];
	::GetModuleFileName(NULL,path,sizeof(TCHAR)*MAX_PATH);
	DWORD zero = 0;
	DWORD size = ::GetFileVersionInfoSize(path,&zero);
	if(size <= 0)return _T("");
	BYTE *buf = new BYTE[size];
	if(!::GetFileVersionInfo(path,0,size,buf))return _T("");
	void *verbuf;
	UINT len;
	if(!::VerQueryValue(buf,_T("\\VarFileInfo\\Translation"),&verbuf,&len)){
		OUTPUTLASTERROR();
		return _T("");
	}
	DWORD lang = *reinterpret_cast<DWORD*>(verbuf);
	TCHAR subblock[128];
	::_stprintf(subblock,_T("\\StringFileInfo\\%04x%04x\\ProductVersion"),LOWORD(lang),HIWORD(lang));
	::VerQueryValue(buf,subblock,&verbuf,&len);
	ablib::string r = reinterpret_cast<TCHAR*>(verbuf);
	delete[] buf;

	std::vector<ablib::string> strvec;
	boost::algorithm::split(strvec,r,boost::algorithm::is_any_of(_T(",")));
	for(auto &s : strvec)boost::algorithm::trim(s);

	return strvec[0] + _T(".") + strvec[1] + _T(".") + strvec[2];
}


