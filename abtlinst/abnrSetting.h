#pragma once
class abnrSetting
{
public:
	abnrSetting(){}
	~abnrSetting(){}

	void Set(bool doinst){m_doinst = doinst;}
	// dviout�̃����N��ύX
	// TeXworks��pdfplatex��ǉ��D
	bool Install(HWND hwnd,MSGFUNC &msgfunc);
	bool DoInst() const{return m_doinst;}

	void ReadSetting();
	void WriteSetting();

private:
	void GetTeXLiveDir();
	ablib::string m_tldir;
	ablib::string m_tlstartmenu;
	bool m_doinst;
	int m_year;

	bool TeXworksSetting(HWND hwnd,MSGFUNC &msgfunc);
	bool writetexmfcnf(HWND hwnd,MSGFUNC &msgfunc);
	bool kanji_config_updmap(HWND hwnd,MSGFUNC &msgfunc,const ablib::string &font);
};

