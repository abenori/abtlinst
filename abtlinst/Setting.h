#pragma once

#include <vector>

class Setting
{
public:
	Setting(void);
	~Setting(void){}

	void Log(const ablib::string &log);
	ablib::string GetTmpDir() const{return m_tmpdir;}
	const ablib::string GetInstallDir() const{return m_installdir;}
	void SetInstallDir(const ablib::string &dir){m_installdir = AddLastYen(dir);if(!m_downloaddir_from_ini)m_downloaddir = dir + _T("download\\");}
	ablib::string GetDownloadDir() const{return m_downloaddir;}

	ablib::string ReadINI(const ablib::string &sec,const ablib::string &key) const{return Read(m_ini,sec,key);}
	ablib::string ReadSysINI(const ablib::string &sec,const ablib::string &key) const{return Read(m_sysini,sec,key);}
	void WriteINI(const ablib::string &sec,const ablib::string &key,const ablib::string &val){Write(m_ini,sec,key,val);}
	void WriteSysINI(const ablib::string &sec,const ablib::string &key,const ablib::string &val){Write(m_sysini,sec,key,val);}

	bool SetPath() const{return m_setpath;}
	void SetPath(bool b){m_setpath = b;}
	bool WriteINI() const{return (ReadSysINI(_T("abtlinst"),_T("WriteINI")) != _T("0"));}

	void GenerateSysINI();

	void ReadSetting();
	void WriteSetting();
	
	ablib::string ToAbsolutePath(const ablib::string &path);

	std::vector<ablib::string> INIKeys(const ablib::string &sec);

private:
	ablib::string Read(const ablib::string &file,const ablib::string &sec,const ablib::string &key) const;
	void Write(const ablib::string &file,const ablib::string &sec,const ablib::string &key,const ablib::string &val);
	std::vector<ablib::string> Keys(const ablib::string &ini,const ablib::string &sec);
	
	ablib::string m_tmpdir,m_installdir;
	ablib::string m_ini,m_sysini,m_downloaddir;
	ablib::string m_logfile;
	bool m_downloaddir_from_ini;
	ablib::string AddLastYen(const ablib::string d){if(d != _T("") && d[d.length() - 1] != '\\')return d + _T("\\");else return d;}
	void InitLogFileName(int logtype);
	bool m_setpath;

};

