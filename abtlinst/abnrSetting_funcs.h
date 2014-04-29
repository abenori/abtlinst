#pragma once

namespace abnrSetting_funcs
{

bool GetTeXLiveDirFromStartMenu(int &year,ablib::string &tldir,ablib::string &tlstartmenu);
bool GetTeXLiveDir(ablib::string &tldir,std::function<void (const ablib::string&)> msg);
bool TeXworksSetting(
	const ablib::string &tldir,
	std::function<void (const ablib::string&)> msg,
	std::function<void (const ablib::string&)> detail,
	std::function<void (const ablib::string&)> error
);
inline bool TeXworksSetting(
	std::function<void (const ablib::string&)> msg,
	std::function<void (const ablib::string&)> detail,
	std::function<void (const ablib::string&)> error
){
	ablib::string tldir;if(!GetTeXLiveDir(tldir,error))return false;
	return TeXworksSetting(tldir,msg,detail,error);
}

bool writetexmfcnf(
	const ablib::string &tldir,
	std::function<void (const ablib::string&)> msg,
	std::function<void (const ablib::string&)> detail,
	std::function<void (const ablib::string&)> error
);
inline bool writetexmfcnf(
	std::function<void (const ablib::string&)> msg,
	std::function<void (const ablib::string&)> detail,
	std::function<void (const ablib::string&)> error
){
	ablib::string tldir;if(!GetTeXLiveDir(tldir,error))return false;
	return writetexmfcnf(tldir,msg,detail,error);
}

bool kanji_config_updmap(
	const ablib::string &tldir,
	std::function<void (const ablib::string&)> msg,
	std::function<void (const ablib::string&)> detail,
	std::function<void (const ablib::string&)> error,
	const ablib::string &font
);
inline bool kanji_config_updmap(
	std::function<void (const ablib::string&)> msg,
	std::function<void (const ablib::string&)> detail,
	std::function<void (const ablib::string&)> error,
	const ablib::string &font
){
	ablib::string tldir;if(!GetTeXLiveDir(tldir,error))return false;
	return kanji_config_updmap(tldir,msg,detail,error,font);
}


}