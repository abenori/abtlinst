#ifndef __REGISTORY_H__
#define __REGISTORY_H__

#include <vector>

/*
レジストリを読んだり書いたり．
*/

class Registory
{
public:
	Registory(void){}
	~Registory(void){}
	static bool ReadSZ(HKEY key,const TCHAR *path,const TCHAR *entry,ablib::string &rv);
	static bool WriteSZ(HKEY key,const TCHAR *path,const TCHAR *entry,const TCHAR *str,bool isexpand = false);

	static DWORD ReadDWORD(HKEY key,const TCHAR *path,const TCHAR *entry);
	static bool WriteDWORD(HKEY key,const TCHAR *path,const TCHAR *entry,DWORD data);

	static bool DeleteKey(HKEY key,const TCHAR *path);
	static bool DeleteValue(HKEY key,const TCHAR *path);
	static bool EnumKey(HKEY key,const TCHAR *path,std::vector<ablib::string> &rv);
};

#endif //__REGISTORY_H__

