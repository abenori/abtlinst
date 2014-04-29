#include "stdafx.h"
#include <vector>
#include "registory.h"
#include "setting.h"

//extern setting g_setting;

bool Registory::ReadSZ(HKEY key,const TCHAR *path,const TCHAR *entry,ablib::string &rv){
	HKEY handle;
	DWORD pos,type = REG_SZ;
	if(::RegCreateKeyEx(key,path,0,_T(""),REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&handle,&pos) != ERROR_SUCCESS)return false;
	DWORD size;
	if(::RegQueryValueEx(handle,entry,NULL,&type,NULL,&size) != ERROR_SUCCESS){
		::RegCloseKey(handle);
		return false;
	}
	++size;
	TCHAR *str = new TCHAR[size];
	::RegQueryValueEx(handle,entry,NULL,&type,reinterpret_cast<BYTE*>(str),&size);
	str[size] = '\0';//いるんだっけ？
	rv = str;
	delete[] str;
	::RegCloseKey(handle);
	return true;
}

bool Registory::WriteSZ(HKEY key,const TCHAR *path,const TCHAR *entry,const TCHAR *str,bool isexpand){
	HKEY handle;
	DWORD pos;
	bool rv = true;
	DEBUGSTRING(_T("Registroy::WriteSZ %sの%sに%sを書き込み．"),path,entry,str);
	if(::RegCreateKeyEx(key,path,0,_T(""),REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&handle,&pos) != ERROR_SUCCESS){
		DEBUGSTRING(_T("レジストリへのアクセスに失敗．GetLastError() = %d"),::GetLastError());
		return false;
	}
	if(::RegSetValueEx(handle,entry,0,isexpand ? REG_EXPAND_SZ : REG_SZ,reinterpret_cast<BYTE *>(const_cast<TCHAR *>(str)),sizeof(TCHAR)*::lstrlen(str)) != ERROR_SUCCESS){
		DEBUGSTRING(_T("レジストリへの書き込みに失敗．GetLastError() = %d"),::GetLastError());
		rv = false;
	}
	::RegCloseKey(handle);
	return rv;
}

DWORD Registory::ReadDWORD(HKEY key,const TCHAR *path,const TCHAR *entry){
	HKEY handle;
	DWORD pos,type = REG_DWORD,rv,size = sizeof(DWORD);
	if(::RegCreateKeyEx(key,path,0,_T(""),REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&handle,&pos) != ERROR_SUCCESS)return false;
	if(::RegQueryValueEx(key,entry,NULL,&type,reinterpret_cast<BYTE *>(&rv),&size) != ERROR_SUCCESS)return 0;
	::RegCloseKey(handle);
	return rv;
}

bool Registory::WriteDWORD(HKEY key,const TCHAR *path,const TCHAR *entry,DWORD data){
	HKEY handle;
	DWORD pos;
	bool rv = true;
	if(::RegCreateKeyEx(key,path,0,_T(""),REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&handle,&pos) != ERROR_SUCCESS)return false;
	if(::RegSetValueEx(handle,entry,0,REG_DWORD,reinterpret_cast<BYTE *>(&data),sizeof(DWORD)) != ERROR_SUCCESS)rv = false;
	::RegCloseKey(handle);
	return rv;
}

bool Registory::DeleteKey(HKEY key,const TCHAR *path){
	HKEY handle;
	bool rv = true;
	ablib::string target;
	TCHAR str[4096];
	DWORD size = 4096;
	std::vector<ablib::string> keys;
	if(::RegOpenKeyEx(key,path,0,KEY_ALL_ACCESS,&handle) != ERROR_SUCCESS)return false;
	for(int i = 0 ; ; ++i){
		::lstrcpy(str,_T(""));
		size = 4096;
		if(::RegEnumKeyEx(handle,i,str,&size,0,NULL,NULL,NULL) == ERROR_NO_MORE_ITEMS)break;
		target = path;
		target += _T("\\");
		target += str;
		keys.push_back(target);
	}
	for(ablib::string &i : keys)DeleteKey(key,i.c_str());
	for(int i = 0 ; ; ++i){
		::lstrcpy(str,_T(""));
		size = 4096;
		if(::RegEnumValue(handle,i,str,&size,0,NULL,NULL,NULL) == ERROR_NO_MORE_ITEMS)break;
		::RegDeleteValue(handle,str);
	}
	::RegCloseKey(handle);
	target = path;
	target.erase(target.rfind(_T("\\")) + 1);
	if(::RegOpenKeyEx(key,target.c_str(),0,KEY_ALL_ACCESS,&handle) != ERROR_SUCCESS)return false;
	if(::RegDeleteKey(handle,path + target.length()) != ERROR_SUCCESS)rv = false;
	::RegCloseKey(handle);
	return rv;
}

bool Registory::DeleteValue(HKEY key,const TCHAR *path){
	ablib::string target = path;
	target.erase(target.rfind(_T("\\")) + 1);
	HKEY handle;
	if(::RegOpenKeyEx(key,target.c_str(),0,KEY_ALL_ACCESS,&handle) != ERROR_SUCCESS)return false;
	if(::RegDeleteValue(handle,path + target.length()) != ERROR_SUCCESS){
		::RegCloseKey(handle);
		return false;
	}else{
		::RegCloseKey(handle);
		return true;
	}
}

bool Registory::EnumKey(HKEY key,const TCHAR *path,std::vector<ablib::string> &rv){
	HKEY handle;
	if(::RegOpenKeyEx(key,path,0,KEY_ALL_ACCESS,&handle) != ERROR_SUCCESS)return false;
	TCHAR str[4096];
	DWORD size = 4096;
	for(int i = 0 ; ; ++i){
		::lstrcpy(str,_T(""));
		size = 4096;
		if(::RegEnumKeyEx(handle,i,str,&size,0,NULL,NULL,NULL) == ERROR_NO_MORE_ITEMS)break;
		rv.push_back(str);
	}
	::RegCloseKey(handle);
	return true;
}

