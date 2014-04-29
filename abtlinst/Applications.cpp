#include "stdafx.h"
#include "Applications.h"

void Applications::Abort(){
	m_TeXLive.Abort();
	m_TeX2img.Abort();
	m_ImageMagick.Abort();
	m_nkf.Abort();
}

void Applications::ReadSetting(){
	m_TeXLive.ReadSetting();
	m_abnrSetting.ReadSetting();
	m_TeX2img.ReadSetting();
	m_ImageMagick.ReadSetting();
	m_nkf.ReadSetting();
}

void Applications::WriteSetting(){
	m_TeXLive.WriteSetting();
	m_abnrSetting.WriteSetting();
	m_TeX2img.WriteSetting();
	m_ImageMagick.WriteSetting();
	m_nkf.WriteSetting();
}
