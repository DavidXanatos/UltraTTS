#pragma once

#include "TTS.h"

#include <initguid.h>
//#include <mmsystem.h> 
#include "../Sapi4/Include/speech.h"

class CTestNotify;
class CTestBufNotify;

class CTTS4: public CTTS_Impl
{
public:
	CTTS4();
	virtual ~CTTS4();

	bool		Speak(const std::wstring& str, bool ProcessXML = false);
	bool		Speak2Wave(const std::wstring& path, const std::wstring& str, bool ProcessXML = false);
	void		Pause();
	void		Skip(int SkipNum) {}
	void		Inject(const std::wstring& str);
	void		Stop();

	void		SetRate(int Rate);
	int			GetRate();
	void		SetPitch(int Pitch);
	int			GetPitch();
	void		SetVolume(int Volume);
	int			GetVolume();

	virtual std::vector<SVoice*> GetVoices();
	bool		SelectVoice(const std::wstring& name); 

	void		ShowGeneralWnd(HWND wnd);

	bool		EnumVoices(const std::wstring& regKey = L"");
	void		ClearVoices();

protected:
	friend class CTestNotify;
	friend class CTestBufNotify;

	BOOL InitTTS(void);
	BOOL TerminateTTS(void);

	struct SVoice4 : SVoice {
		GUID guid;
	};

	bool VoiceChange(SVoice4* pVoice, PIAUDIOFILE dev = NULL);

	std::map<std::wstring, SVoice4*> m_Voices;

	PITTSENUM		m_pITTSEnum;
	PITTSCENTRAL    m_pITTSCentral;
	PITTSATTRIBUTES m_pITTSAttributes;
	PITTSDIALOGS	m_pITTSDialogs;
	PILEXPRONOUNCE	m_pILexPronounce;
	CTestNotify*	m_pTestNotify;
	CTestBufNotify*	m_pTestBufNotify;
#ifdef DIRECTSOUND
	PIAUDIODIRECT   m_pIAD;
#else
	PIAUDIOMULTIMEDIADEVICE m_pIMMD;
#endif
	PIAUDIOFILE		m_pIAF;
	DWORD           m_dwRegKey;

	GUID			m_guid;

	int				m_BufferLength;
};


class CTestNotify : public ITTSNotifySink {
private:
	CTTS4 *m_pImpl;
public:
	CTestNotify(CTTS4 *pImpl);
	~CTestNotify(void);

	// IUnkown members that delegate to m_punkOuter
	// Non-delegating object IUnknown
	STDMETHODIMP         QueryInterface(REFIID, LPVOID FAR *);
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);

	// ITTSNotifySink
	STDMETHOD(AttribChanged)  (DWORD);
	STDMETHOD(AudioStart)     (QWORD);
	STDMETHOD(AudioStop)      (QWORD);
	STDMETHOD(Visual)         (QWORD, WCHAR, WCHAR, DWORD, PTTSMOUTH);
};

class CTestBufNotify : public ITTSBufNotifySink {
private:
	CTTS4 *m_pImpl;
public:
	CTestBufNotify(CTTS4 *pImpl);
	~CTestBufNotify(void);

	// IUnkown members that delegate to m_punkOuter
	// Non-delegating object IUnknown
	STDMETHODIMP         QueryInterface(REFIID, LPVOID FAR *);
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);

	// ITTSNotifySink
	STDMETHOD(BookMark)		   	(QWORD, DWORD);
	STDMETHOD(TextDataDone)   	(QWORD, DWORD);
	STDMETHOD(TextDataStarted)   (QWORD);
	STDMETHOD(WordPosition)      (QWORD, DWORD);
};

