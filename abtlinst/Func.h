#ifndef __FUNC_H__
#define __FUNC_H__

#include <vector>

/*
ä÷êîãlÇﬂçáÇÌÇπ
*/

int lstrcmpn(const TCHAR *a,const TCHAR *b,int n);
//void split(std::vector<ablib::string> &rvalue,const TCHAR *value,const TCHAR *key);
void YenToSlash(TCHAR *str);
bool GetLocalFileTime(const TCHAR *file,FILETIME *ft);
bool ExistFile(const TCHAR *filename);
bool CreateDirectoryReflex(const TCHAR *path);
int CheckSameFile(const TCHAR *filename1,const TCHAR *filename2);
ablib::string TransRegularPath(const ablib::string &path);
void WriteToEditCtrl(HWND hwnd,const ablib::string &msg);
bool CopyDirectory(const ablib::string &from,const ablib::string &to,bool failifexist = false);

#endif //__FUNC_H__

