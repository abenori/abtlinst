#include "abnrSetting_funcs.h"

namespace TeX2img_funcs{
bool Set(
	const ablib::string &tldir,
	std::function<void (const ablib::string&)> msg,
	std::function<void (const ablib::string&)> detail,
	std::function<void (const ablib::string&)> error
);
inline bool Set(
	std::function<void (const ablib::string&)> msg,
	std::function<void (const ablib::string&)> detail,
	std::function<void (const ablib::string&)> error
){
	ablib::string tldir;if(!abnrSetting_funcs::GetTeXLiveDir(tldir,error))return false;
	return Set(tldir,msg,detail,error);
}

}