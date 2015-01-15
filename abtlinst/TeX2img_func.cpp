#include "stdafx.h"
#include "TeX2img_func.h"
#include <system/Process.h>
#include <boost/lexical_cast.hpp>

namespace TeX2img_funcs{
bool Set(
	const ablib::string &tldir,
	std::function<void (const ablib::string&)> msg,
	std::function<void (const ablib::string&)> detail,
	std::function<void (const ablib::string&)> error
){
	detail(_T("TeX2img の設定を行います．\n"));
	ablib::system::Process process;
	ablib::system::Process::ProcessStartInfo si;
	si.FileName = tldir + _T("bin\\win32\\TeX2img.exe");
//	si.Arguments = _T("\"") + dir + 
//		_T("TeX2img.exe\" ") + 
	// TeX2imgが受け取る引数の一つ目はTeX2img.exe本体だと思っていたのだけど，こう起動するとArguments全体になった
	// 謎だが第一引数にダミーを渡すことで処理する．古いバージョンでも問題にはならないはず．
	si.Arguments = _T("/dummy /platex \"") + tldir + _T("bin\\win32\\platex.exe\" /dvipdfmx \"") + 
		tldir + _T("bin\\win32\\dvipdfmx.exe\" /gs \"") + tldir + _T("bin\\win32\\rungs.exe\" /exit");
	si.RedirectStandardError = si.RedirectStandardInput = si.RedirectStandardOutput = false;
	si.ShowState = si.show;
	si.WorkingDirectory = tldir + _T("bin\\win32");
	//MessageBox(NULL, si.Arguments.c_str(), NULL, 0);
	process.StartInfo(si);
	if(!process.Start()){
		error(_T("TeX2img の設定に失敗．GetLastError = ") + boost::lexical_cast<ablib::string>(ablib::system::GetLastError()));
		return false;
	}else{
		process.WaitForExit();
		return true;
	}
}
}