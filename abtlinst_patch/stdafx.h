// stdafx.h : �W���̃V�X�e�� �C���N���[�h �t�@�C���̃C���N���[�h �t�@�C���A�܂���
// �Q�Ɖ񐔂������A�����܂�ύX����Ȃ��A�v���W�F�N�g��p�̃C���N���[�h �t�@�C��
// ���L�q���܂��B
//

#pragma once

#define WIN32_LEAN_AND_MEAN             // Windows �w�b�_�[����g�p����Ă��Ȃ����������O���܂��B
// winsock2.h��windows.h�����include����K�v������H
#include <winsock2.h>
#include <windows.h>
#include <tchar.h>
#include <commctrl.h>
#include <Shlobj.h>
#include <wininet.h>
#include <shellapi.h>

#include <atlbase.h>
#include <atlwin.h>
#include <atlapp.h>
#include <atlcrack.h>
#include <atlmisc.h>
#include <atldlgs.h>
#include <atlctrls.h>

#include <ablib/string.h>
#include <functional>

struct MSGFUNC{
	std::function<void (const ablib::string&)> msg,detail;
};

