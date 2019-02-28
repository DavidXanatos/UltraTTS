#include "stdafx.h"
#include "TTS4.h"


CTTS4::CTTS4()
{
	m_BufferLength = 0;

	InitTTS();

	//EnumVoices();
}

CTTS4::~CTTS4() 
{
	TerminateTTS();

	ClearVoices();
}

bool CTTS4::Speak(const std::wstring& str, bool ProcessXML)
{
	qDebug() << "Speak";

	if (m_bPause)
		Stop();

	m_BufferLength = str.size();

	SDATA text;
	text.dwSize = str.size() + 1;
	text.pData = (TCHAR *)malloc(text.dwSize * sizeof(TCHAR));
	wcscpy((TCHAR*)text.pData, str.c_str());

	VOICECHARSET vcs = CHARSET_TEXT; // CHARSET_IPAPHONETIC

	HRESULT hr;
	hr = m_pITTSCentral->TextData(vcs, ProcessXML, text, m_pTestBufNotify, IID_ITTSBufNotifySink);
	free(text.pData);

	m_bStop = false;
	m_bPause = false;

	return SUCCEEDED(hr);
}

void CTTS4::Pause()
{
	qDebug() << "Pause";

	if (!m_pITTSCentral)
		return;

	if (m_bPause) {
		m_bPause = false;
		m_pITTSCentral->AudioResume();
	}
	else {
		m_bPause = true;
		m_pITTSCentral->AudioPause();
	}
}

void CTTS4::Inject(const std::wstring& str) 
{
	if(m_pITTSCentral)
		m_pITTSCentral->Inject(str.c_str());
}

void CTTS4::Stop() 
{
	qDebug() << "Stop";

	if (m_pITTSCentral)
		m_pITTSCentral->AudioReset(); // this sometimes crashes??? wtf

	bool bWasPaused = m_bPause;

	m_bStop = true;
	m_bPause = false;

	if(bWasPaused)
		OnStop();
}

void CTTS4::ShowGeneralWnd(HWND wnd)
{
	m_pITTSDialogs->GeneralDlg(wnd, NULL);
}

BOOL CTTS4::InitTTS(void)
{
	HRESULT hRes;

	m_pITTSCentral = NULL;
	m_pITTSEnum = NULL;
	m_pITTSAttributes = NULL;
	m_pITTSDialogs = NULL;
	m_pILexPronounce = NULL;
	m_dwRegKey = 0xFFFFFFFF;
	m_pTestNotify = NULL;
	m_pTestBufNotify = NULL;
	m_pIAF = NULL;
#ifdef DIRECTSOUND
	m_pIAD = NULL;
#else
	m_pIMMD = NULL;
#endif

	hRes = CoCreateInstance(CLSID_TTSEnumerator, NULL, CLSCTX_ALL, IID_ITTSEnum, (void**)&m_pITTSEnum);

	if (FAILED(hRes))
	{
		MessageBox(NULL, TEXT("Error creating TTSEnumerator (CoCreateInstance)."), NULL, MB_OK);
		return FALSE;
	}

	if ((m_pTestNotify = new CTestNotify(this)) == NULL)
		MessageBox(NULL, TEXT("Error creating notify pointer."), TEXT("Warning"), MB_OK);

	if ((m_pTestBufNotify = new CTestBufNotify(this)) == NULL)
		MessageBox(NULL, TEXT("Error creating buf notify pointer."), TEXT("Warning"), MB_OK);

	return TRUE;
}

BOOL CTTS4::TerminateTTS(void)
{
	if (m_pITTSEnum) {
		m_pITTSEnum->Release();
		m_pITTSEnum = NULL;
	}
	if (m_pITTSAttributes) {
		m_pITTSAttributes->Release();
		m_pITTSAttributes = NULL;
	}
	if (m_pILexPronounce) {
		m_pILexPronounce->Release();
		m_pILexPronounce = NULL;
	}
	if (m_pITTSDialogs) {
		m_pITTSDialogs->Release();
		m_pITTSDialogs = NULL;
	}
	if (m_pITTSCentral) {
		m_pITTSCentral->UnRegister(m_dwRegKey);
		m_pITTSCentral->Release();
		m_pITTSCentral = NULL;
	}
	if (m_pIAF) {
		m_pIAF->Release();
		m_pIAF = NULL;
	}

#ifdef DIRECTSOUND
	if (m_pIAD) {
		while (m_pIAD->Release());
		m_pIAD = NULL;
	}
#else
	if (m_pIMMD) {
		while (m_pIMMD->Release());
		m_pIMMD = NULL;
	}
#endif	

	if (m_pTestNotify) {
		delete(m_pTestNotify);
		m_pTestNotify = NULL;
	}
	if (m_pTestBufNotify) {
		delete(m_pTestBufNotify);
		m_pTestBufNotify = NULL;
	}
	return TRUE;
}

bool CTTS4::EnumVoices(const std::wstring& regKey)
{
	HRESULT hRes;
	PITTSENUM pClone1;
	TTSMODEINFO TTSModeInfo;
	DWORD dwNumTimes;
	int index = 0;

	hRes = m_pITTSEnum->Clone(&pClone1);
	if (FAILED(hRes))
	{
		MessageBox(NULL, TEXT("Couldn't clone ITTSEnum state, aborting enumeration test"), TEXT("Error"), MB_OK);
		return 0;
	}

	hRes = pClone1->Next(1, &TTSModeInfo, &dwNumTimes);
	if (dwNumTimes == 0) return FALSE;

	while (dwNumTimes) {

		std::wstring name(TTSModeInfo.szModeName);
		if (m_Voices.find(name) == m_Voices.end())
		{
			SVoice4* pVoice = new SVoice4();
			pVoice->Name = name;
			pVoice->Vendor = std::wstring(TTSModeInfo.szMfgName);
			// // Speaker = std::wstring(TTSModeInfo.szSpeaker);
			// // Product = std::wstring(TTSModeInfo.szProductName);
			if (TTSModeInfo.wAge <= 12)
				pVoice->Age = SVoice4::eChild;
			else if (TTSModeInfo.wAge > 60)
				pVoice->Age = SVoice4::eElderly;
			else 
				pVoice->Age = SVoice4::eAdult;
			pVoice->Gender = (SVoice4::EGender)TTSModeInfo.wGender;
			pVoice->LangID = TTSModeInfo.language.LanguageID;
			WCHAR strNameBuffer[LOCALE_NAME_MAX_LENGTH+1];
			//LCIDToLocaleName(pVoice->LangID, strNameBuffer, LOCALE_NAME_MAX_LENGTH, 0);
			GetLocaleInfoW(MAKELCID(pVoice->LangID, SORT_DEFAULT), LOCALE_SENGLANGUAGE, strNameBuffer, LOCALE_NAME_MAX_LENGTH);
			pVoice->Lang = strNameBuffer;
			pVoice->Sapi4 = true;
			// // Style = std::wstring(TTSModeInfo.szStyle);
			pVoice->guid = TTSModeInfo.gModeID;
		
			m_Voices[name] = pVoice;
		}

		hRes = pClone1->Next(1, &TTSModeInfo, &dwNumTimes);
	}

	pClone1->Release();
	pClone1 = NULL;
	return TRUE;
}

std::vector<SVoice*> CTTS4::GetVoices()
{
	std::vector<SVoice*> list;
	for (std::map<std::wstring, SVoice4*>::iterator I = m_Voices.begin(); I != m_Voices.end(); ++I)
		list.push_back(I->second);
	return list;
}

bool CTTS4::SelectVoice(const std::wstring& name)
{
	for (std::map<std::wstring, SVoice4*>::iterator I = m_Voices.begin(); I != m_Voices.end(); ++I)
	{
		if (I->second->Name == name)
			return VoiceChange(I->second);
	}
	return false;
}

void CTTS4::ClearVoices()
{
	for (std::map<std::wstring, SVoice4*>::iterator I = m_Voices.begin(); I != m_Voices.end(); ++I)
	{
		delete I->second;
	}
	m_Voices.clear();
}

bool CTTS4::VoiceChange(SVoice4* pVoice, PIAUDIOFILE dev)
{
	if (pVoice && m_guid == pVoice->guid)
		return true;

	HRESULT hRes;

	if (m_pITTSAttributes) {
		m_pITTSAttributes->Release();
		m_pITTSAttributes = NULL;
	}
	if (m_pILexPronounce) {
		m_pILexPronounce->Release();
		m_pILexPronounce = NULL;
	}
	if (m_pITTSDialogs) {
		m_pITTSDialogs->Release();
		m_pITTSDialogs = NULL;
	}
	if (m_pITTSCentral) {
		m_pITTSCentral->UnRegister(m_dwRegKey);
		m_pITTSCentral->Release();

		m_pITTSCentral = NULL;
	}
#ifdef DIRECTSOUND
	if (m_pIAD) {
		while (m_pIAD->Release());
		m_pIAD = NULL;
	}
#else // DIRECTSOUND
	if (m_pIMMD) {
		while (m_pIMMD->Release());
		m_pIMMD = NULL;
	}
#endif

#ifdef DIRECTSOUND
	hRes = CoCreateInstance(CLSID_AudioDestDirect, NULL, CLSCTX_ALL, IID_IAudioDirect, (void**)&m_pIAD);
	if (FAILED(hRes))
	{
		MessageBox(TEXT("Can't find CLSID_AudioDestDirect"), NULL, MB_OK);
		return;
	}

	// crreate direct sound stuff
	LPDIRECTSOUND lpDirectSound;
	hRes = CoCreateInstance(CLSID_DirectSound, NULL,
		CLSCTX_ALL, IID_IDirectSound, (LPVOID*)&lpDirectSound);
	if (hRes) {
		MessageBox(TEXT("Can't find IID_IDirectSound"), NULL, MB_OK);
		return;
	}
	hRes = lpDirectSound->Initialize(NULL);
	if (hRes) {
		MessageBox(TEXT("Can't initialize DirectSound"), NULL, MB_OK);
		return;
	}
	hRes = lpDirectSound->SetCooperativeLevel(m_hWnd, DSSCL_NORMAL);

	// tell the audio object about our stuff
	m_pIAD->Init((PVOID)lpDirectSound, IID_IDirectSound);

	m_pIAD->AddRef();
	if (dev)
		hRes = m_pITTSEnum->Select(pVoice ? pVoice->guid : m_guid, &m_pITTSCentral, dev);
	else
		hRes = m_pITTSEnum->Select(pVoice ? pVoice->guid m_guid, &m_pITTSCentral, m_pIAD);
#else // DIRECTSOUND
	hRes = CoCreateInstance(CLSID_MMAudioDest, NULL, CLSCTX_ALL, IID_IAudioMultiMediaDevice, (void**)&m_pIMMD);
	if (FAILED(hRes))
	{
		MessageBox(NULL, TEXT("Error creating AudioDest Object(CoCreateInstance)."), NULL, MB_OK);
	}

	hRes = m_pIMMD->DeviceNumSet(0XFFFFFFFF);

	m_pIMMD->AddRef();
	if(dev)
		hRes = m_pITTSEnum->Select(pVoice ? pVoice->guid : m_guid, &m_pITTSCentral, dev);
	else
		hRes = m_pITTSEnum->Select(pVoice ? pVoice->guid : m_guid, &m_pITTSCentral, m_pIMMD);

#endif // DIRECTSOUND
	if (FAILED(hRes))
		return FALSE;

	if(pVoice)
		m_guid = pVoice->guid;

	m_pITTSCentral->QueryInterface(IID_ITTSAttributes, (void**)&m_pITTSAttributes);

	m_pITTSCentral->QueryInterface(IID_ITTSDialogs, (void**)&m_pITTSDialogs);
		
	m_pITTSCentral->QueryInterface(IID_ILexPronounce, (void**)&m_pILexPronounce);


	WORD wMin, wMax, wNor;

	m_pITTSAttributes->PitchSet(TTSATTR_MINPITCH);
	m_pITTSAttributes->PitchGet(&wMin);
	m_MinPitch = wMin;

	m_pITTSAttributes->PitchSet(TTSATTR_MAXPITCH);
	m_pITTSAttributes->PitchGet(&wMax);
	m_MaxPitch = wMax;

	DWORD dwMin, dwMax, dwNor;

	m_pITTSAttributes->SpeedSet(TTSATTR_MINSPEED);
	m_pITTSAttributes->SpeedGet(&dwMin);
	m_MinRate = dwMin;

	m_pITTSAttributes->SpeedSet(TTSATTR_MAXSPEED);
	m_pITTSAttributes->SpeedGet(&dwMax);
	m_MaxRate = dwMax;

	m_MinVolume = 0;
	m_MaxVolume = 100;


	// Reset the voice
	m_pITTSCentral->Inject(L"\\rst\\");

	hRes = m_pITTSAttributes->PitchGet(&wNor);
	if (hRes != S_OK)
		m_DefaultPitch = wNor;
	
	hRes = m_pITTSAttributes->SpeedGet(&dwNor);
	if (hRes != S_OK)
		m_DefaultRate = dwNor;

	hRes = m_pITTSAttributes->VolumeGet(&dwNor);
	dwNor &= 0xffff;
	dwNor /= 655;
	if (hRes != S_OK)
		m_DefaultVolume = dwNor;

	m_pITTSCentral->Register((void*)m_pTestNotify, IID_ITTSNotifySink, &m_dwRegKey);
	
	// get the gender
	//TTSMODEINFO tm;
	//m_pITTSCentral->ModeGet(&tm);
	//PaintGender(tm.wGender == GENDER_MALE);
	return TRUE;
}

void CTTS4::SetRate(int Rate)
{
	if(m_pITTSAttributes)
		m_pITTSAttributes->SpeedSet(Rate);
}

int CTTS4::GetRate()
{
	DWORD speed;
	if (m_pITTSAttributes)
		m_pITTSAttributes->SpeedGet(&speed);
	return speed;
}

void CTTS4::SetPitch(int Pitch)
{
	if(m_pITTSAttributes)
		m_pITTSAttributes->PitchSet(Pitch);
}

int CTTS4::GetPitch()
{
	WORD pitch;
	if (m_pITTSAttributes)
		m_pITTSAttributes->PitchGet(&pitch);
	return pitch;
}

void CTTS4::SetVolume(int Volume)
{
	DWORD dwVolume = 0xffff * Volume / 100;
	dwVolume |= dwVolume << 16;
	if(m_pITTSAttributes)
		m_pITTSAttributes->VolumeSet(dwVolume);
}

int CTTS4::GetVolume()
{
	DWORD volume;
	if (m_pITTSAttributes)
		m_pITTSAttributes->VolumeGet(&volume);
	volume &= 0xffff;
	volume /= 655;
	return volume;
}

bool CTTS4::Speak2Wave(const std::wstring& path, const std::wstring& str, bool ProcessXML)
{
	HRESULT hRes;

	hRes = CoCreateInstance(CLSID_AudioDestFile, NULL, CLSCTX_ALL, IID_IAudioFile, (void**)&m_pIAF);

	VoiceChange(NULL, m_pIAF);

	m_pIAF->RealTimeSet(0xFFFFFFFF);

	m_pIAF->Set(path.c_str(), 1);

	bool bRet = Speak(str, ProcessXML);

	BOOL fGotMessage;
	MSG msg;
	while ((fGotMessage = GetMessage(&msg, (HWND)NULL, 0, 0)) != 0 && fGotMessage != -1 && !m_bStop) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	m_pIAF->Flush();

	m_pIAF->Release();
	m_pIAF = NULL;

	// reset to default outpit
	VoiceChange(NULL, NULL);

	return bRet;
}

/*************************************************************************
CTestNotify - Notification object.
*/
CTestNotify::CTestNotify(CTTS4 *pImpl)
{
	m_pImpl = pImpl;
}

CTestNotify::~CTestNotify(void)
{
	// this space intentionally left blank
}

STDMETHODIMP CTestNotify::QueryInterface(REFIID riid, LPVOID *ppv)
{
	*ppv = NULL;

	/* always return our IUnknown for IID_IUnknown */
	if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_ITTSNotifySink))
	{
		*ppv = (LPVOID)this;
		return S_OK;
	}

	// otherwise, cant find
	return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP_(ULONG) CTestNotify::AddRef(void)
{
	// normally this increases a reference count, but this object
	// is going to be freed as soon as the app is freed, so it doesn't
	// matter
	return 1;
}

STDMETHODIMP_(ULONG) CTestNotify::Release(void)
{
	// normally this releases a reference count, but this object
	// is going to be freed when the application is freed so it doesnt
	// matter
	return 1;
}

STDMETHODIMP CTestNotify::AttribChanged(DWORD dwAttribID)
{
	//m_pImpl->UpdateSliders(); // todo
	return NOERROR;
}

STDMETHODIMP CTestNotify::AudioStart(QWORD qTimeStamp)
{
	return NOERROR;
}

STDMETHODIMP CTestNotify::AudioStop(QWORD qTimeStamp)
{
	if (!m_pImpl->m_bPause) {
		m_pImpl->m_bStop = true;
		m_pImpl->OnStop();
	}
	return NOERROR;
}

STDMETHODIMP CTestNotify::Visual(QWORD qTimeStamp, WCHAR cIPAPhoneme, WCHAR cEnginePhoneme, DWORD dwHints, PTTSMOUTH pTTSMouth)
{
	if (!pTTSMouth)
		return NOERROR;

	/*HRESULT ret = m_pImpl->m_pITTSCentral->PosnGet(&qTimeStamp);
	qDebug() << ret;*/

	/*RECT r;
	HWND hWndMouth;
	hWndMouth = ::GetDlgItem(m_pCTTSAPPDlg->m_hWnd, IDC_MOUTHBOX);
	HDC hdc = GetDC(hWndMouth);
	::GetClientRect(hWndMouth, &r);
	PaintToDC(hdc, &r, pTTSMouth);

	ReleaseDC(hWndMouth, hdc);*/

	return NOERROR;
}

/*************************************************************************
CTestBufNotify - Notification object.
*/
CTestBufNotify::CTestBufNotify(CTTS4 *pImpl)
{
	m_pImpl = pImpl;
}

CTestBufNotify::~CTestBufNotify(void)
{
	// this space intentionally left blank
}

STDMETHODIMP CTestBufNotify::QueryInterface(REFIID riid, LPVOID *ppv)
{
	*ppv = NULL;

	/* always return our IUnknown for IID_IUnknown */
	if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_ITTSBufNotifySink))
	{
		*ppv = (LPVOID)this;
		return S_OK;
	}

	// otherwise, cant find
	return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP_(ULONG) CTestBufNotify::AddRef(void)
{
	// normally this increases a reference count, but this object
	// is going to be freed as soon as the app is freed, so it doesn't
	// matter
	return 1;
}

STDMETHODIMP_(ULONG) CTestBufNotify::Release(void)
{
	// normally this releases a reference count, but this object
	// is going to be freed when the application is freed so it doesnt
	// matter
	return 1;
}

STDMETHODIMP CTestBufNotify::BookMark(QWORD qTimeStamp, DWORD dwMarkNum)
{
	return NOERROR;
}

STDMETHODIMP CTestBufNotify::TextDataDone(QWORD qTimeStamp, DWORD dwFlags)
{
	return NOERROR;
}

STDMETHODIMP CTestBufNotify::TextDataStarted(QWORD qTimeStamp)
{
	return NOERROR;
}

STDMETHODIMP CTestBufNotify::WordPosition(QWORD qTimeStamp, DWORD dwByteOffset)
{
	m_pImpl->OnProgress(dwByteOffset / sizeof(wchar_t), 0, m_pImpl->m_BufferLength);
	return NOERROR;
}

