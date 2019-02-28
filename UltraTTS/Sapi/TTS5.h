#pragma once

#include "TTS.h"

#include <olectl.h>         // Required for showing property page
#include <sapi.h>           // SAPI includes
#include <sphelper.h>
#include <spuihelp.h>

#define WM_TTSAPPCUSTOMEVENT    WM_APP // Window message used for systhesis events
#define WND_SAPI_HELPER			_T("CTTS5 Helper Window")
#define NUM_OUTPUTFORMATS       36

class CTTS5: public CTTS_Impl
{
public:
	CTTS5();
	virtual ~CTTS5();

	bool		Speak(const std::wstring& str, bool ProcessXML = false);
	bool		Speak2Wave(const std::wstring& path, const std::wstring& str, bool ProcessXML = false);
	void		Pause();
	void		Skip(int SkipNum);
	void		Inject(const std::wstring& str) {}
	void		Stop();

	void		SetRate(int Rate);
	int			GetRate();
	void		SetPitch(int Pitch) {}
	int			GetPitch() { return 0; }
	void		SetVolume(int Volume);
	int			GetVolume();
	void		SetFormat(SPSTREAMFORMAT Format);

	virtual std::vector<SVoice*> GetVoices();
	bool		SelectVoice(const std::wstring& name);

	bool		EnumVoices(const std::wstring& regKey = L"");
	void		ClearVoices();

protected:
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void		MainHandleSynthEvent();

	void		CreateWnd();
	BOOL		SetupSapi();

	struct SVoice5: SVoice {
		ISpObjectToken* pToken;
	};

	HRESULT		VoiceChange(SVoice5* pVoice);

	CComPtr<ISpVoice>   m_cpVoice;
	CComPtr<ISpAudio>   m_cpOutAudio;

	HWND				m_hWnd;

	int                 m_DefaultFormatIndex;

	std::map<std::wstring, SVoice5*> m_Voices;

	int					m_BufferLength;

	ISpObjectToken*		m_pTocken;
};