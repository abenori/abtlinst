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
	detail(_T("TeX2img ÇÃê›íËÇçsÇ¢Ç‹Ç∑ÅD\n"));
	ablib::system::Process process;
	ablib::system::Process::ProcessStartInfo si;
	si.FileName = tldir + _T("bin\\win32\\TeX2img.exe");
//	si.Arguments = _T("\"") + dir + 
//		_T("TeX2img.exe\" ") + 
	si.Arguments = _T("/platex \"") + tldir + _T("bin\\win32\\platex.exe\" /dvipdfmx \"") + 
		tldir + _T("bin\\win32\\dvipdfmx.exe\" /gs \"") + tldir + _T("bin\\win32\\rungs.exe\" /exit");
	si.RedirectStandardError = si.RedirectStandardInput = si.RedirectStandardOutput = false;
	si.ShowState = si.show;
	si.WorkingDirectory = tldir + _T("bin\\win32");
	process.StartInfo(si);
	if(!process.Start()){
		error(_T("TeX2img ÇÃê›íËÇ…é∏îsÅDGetLastError = ") + boost::lexical_cast<ablib::string>(ablib::system::GetLastError()));
		return false;
	}else{
		process.WaitForExit();
		return true;
	}
}
}