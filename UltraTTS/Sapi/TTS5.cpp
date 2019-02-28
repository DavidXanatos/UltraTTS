#include "stdafx.h"
#include "TTS5.h"

const SPSTREAMFORMAT g_aOutputFormat[NUM_OUTPUTFORMATS] = {
SPSF_8kHz8BitMono, SPSF_8kHz8BitStereo, SPSF_8kHz16BitMono, SPSF_8kHz16BitStereo,
SPSF_11kHz8BitMono, SPSF_11kHz8BitStereo, SPSF_11kHz16BitMono, SPSF_11kHz16BitStereo,
SPSF_12kHz8BitMono, SPSF_12kHz8BitStereo, SPSF_12kHz16BitMono, SPSF_12kHz16BitStereo,
SPSF_16kHz8BitMono, SPSF_16kHz8BitStereo, SPSF_16kHz16BitMono, SPSF_16kHz16BitStereo,
SPSF_22kHz8BitMono, SPSF_22kHz8BitStereo, SPSF_22kHz16BitMono, SPSF_22kHz16BitStereo,
SPSF_24kHz8BitMono, SPSF_24kHz8BitStereo, SPSF_24kHz16BitMono, SPSF_24kHz16BitStereo,
SPSF_32kHz8BitMono, SPSF_32kHz8BitStereo, SPSF_32kHz16BitMono, SPSF_32kHz16BitStereo,
SPSF_44kHz8BitMono, SPSF_44kHz8BitStereo, SPSF_44kHz16BitMono, SPSF_44kHz16BitStereo,
SPSF_48kHz8BitMono, SPSF_48kHz8BitStereo, SPSF_48kHz16BitMono, SPSF_48kHz16BitStereo };

TCHAR* g_aszOutputFormat[NUM_OUTPUTFORMATS] = {
_T("8kHz 8 Bit Mono"), _T("8kHz 8 Bit Stereo"), _T("8kHz 16 Bit Mono"), _T("8kHz 16 Bit Stereo"),
_T("11kHz 8 Bit Mono"), _T("11kHz 8 Bit Stereo"), _T("11kHz 16 Bit Mono"), _T("11kHz 16 Bit Stereo"),
_T("12kHz 8 Bit Mono"), _T("12kHz 8 Bit Stereo"), _T("12kHz 16 Bit Mono"), _T("12kHz 16 Bit Stereo"),
_T("16kHz 8 Bit Mono"), _T("16kHz 8 Bit Stereo"), _T("16kHz 16 Bit Mono"), _T("16kHz 16 Bit Stereo"),
_T("22kHz 8 Bit Mono"), _T("22kHz 8 Bit Stereo"), _T("22kHz 16 Bit Mono"), _T("22kHz 16 Bit Stereo"),
_T("24kHz 8 Bit Mono"), _T("24kHz 8 Bit Stereo"), _T("24kHz 16 Bit Mono"), _T("24kHz 16 Bit Stereo"),
_T("32kHz 8 Bit Mono"), _T("32kHz 8 Bit Stereo"), _T("32kHz 16 Bit Mono"), _T("32kHz 16 Bit Stereo"),
_T("44kHz 8 Bit Mono"), _T("44kHz 8 Bit Stereo"), _T("44kHz 16 Bit Mono"), _T("44kHz 16 Bit Stereo"),
_T("48kHz 8 Bit Mono"), _T("48kHz 8 Bit Stereo"), _T("48kHz 16 Bit Mono"), _T("48kHz 16 Bit Stereo") };

CTTS5::CTTS5()
{
	m_DefaultFormatIndex = 0;

	m_BufferLength = 0;

	m_pTocken = NULL;

	HRESULT hr;

	hr = m_cpVoice.CoCreateInstance(CLSID_SpVoice);

	if (SUCCEEDED(hr))
	{
		CreateWnd();

		SetupSapi();

		//EnumVoices();
	}
}

LRESULT CALLBACK CTTS5::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CTTS5* pThis = (CTTS5 *)::GetWindowLong(hwnd, GWL_USERDATA);
	switch (uMsg)
	{
	case WM_TTSAPPCUSTOMEVENT:
		pThis->MainHandleSynthEvent();
		return TRUE;
	}
	return TRUE;
}

void CTTS5::CreateWnd()
{
	//Create window
	WNDCLASSEX wndclass;
	wndclass.cbSize = sizeof wndclass;
	wndclass.style = 0;
	wndclass.lpfnWndProc = WindowProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = GetModuleHandle(0);
	wndclass.hIcon = 0;
	wndclass.hCursor = 0;
	wndclass.hbrBackground = 0;
	wndclass.lpszMenuName = 0;
	wndclass.lpszClassName = WND_SAPI_HELPER;
	wndclass.hIconSm = 0;
	RegisterClassEx(&wndclass);

	m_hWnd = CreateWindow(WND_SAPI_HELPER, WND_SAPI_HELPER, 0, 0, 0, 0, 0, 0, 0, 0, GetModuleHandle(0));
	SetWindowLong(m_hWnd, GWL_USERDATA, (LONG)this);
}

CTTS5::~CTTS5()
{
	DestroyWindow(m_hWnd);

	ClearVoices();
}

void CTTS5::MainHandleSynthEvent()
{
	CSpEvent        event; 
	HRESULT 		hr = S_OK;

	while (event.GetFrom(m_cpVoice) == S_OK)
	{
		switch (event.eEventId)
		{
		case SPEI_START_INPUT_STREAM:

			break;

		case SPEI_END_INPUT_STREAM:
			// Set global boolean stop to TRUE when finished speaking
			m_bStop = TRUE;
			OnStop();

			break;

		case SPEI_VOICE_CHANGE:
			
			break;

		case SPEI_TTS_BOOKMARK:
			// event.String()
			break;

		case SPEI_WORD_BOUNDARY:
		{
			if (m_bStop)
				break;

			SPVOICESTATUS   Stat;

			hr = m_cpVoice->GetStatus(&Stat, NULL);

			OnProgress(Stat.ulInputWordPos, Stat.ulInputWordLen, m_BufferLength);

			break;
		}
		case SPEI_PHONEME:

			break;

		case SPEI_VISEME:
			// Get the current mouth viseme position and map it to one of the 
			// 7 mouth bitmaps. 
			/*g_iBmp = g_aMapVisemeToImage[event.Viseme()]; // current viseme

			InvalidateRect(m_hChildWnd, NULL, FALSE);
			*/
			break;

		case SPEI_SENTENCE_BOUNDARY:

			break;

		case SPEI_TTS_AUDIO_LEVEL:
			//printf(L"Audio level: %d\r\n", (ULONG)event.wParam);
			break;

		case SPEI_TTS_PRIVATE:

			break;

		default:
			break;
		}
	}
}

BOOL CTTS5::SetupSapi()
{
	HRESULT hr = S_OK;

	if (!m_cpVoice)
	{
		hr = E_FAIL;
	}

	// Set the default output format as the current selection.
	if (SUCCEEDED(hr))
	{
		CComPtr<ISpStreamFormat> cpStream;
		HRESULT hrOutputStream = m_cpVoice->GetOutputStream(&cpStream);

		if (hrOutputStream == S_OK)
		{
			CSpStreamFormat Fmt;
			hr = Fmt.AssignFormat(cpStream);
			if (SUCCEEDED(hr))
			{
				SPSTREAMFORMAT eFmt = Fmt.ComputeFormatEnum();
				for (int i = 0; i < NUM_OUTPUTFORMATS; i++)
				{
					if (g_aOutputFormat[i] == eFmt)
						m_DefaultFormatIndex = i;
				}
			}
		}
	}

	// Use the SAPI5 helper function in sphelper.h to initialize the Voice combo box.
	/*if (SUCCEEDED(hr))
	{
		EnumVoices();
	}*/

	if (SUCCEEDED(hr))
	{
		SpCreateDefaultObjectFromCategoryId(SPCAT_AUDIOOUT, &m_cpOutAudio);
	}

	// Set default voice data 
	//VoiceChange();

	// Set the notification message for the voice
	if (SUCCEEDED(hr))
	{
		m_cpVoice->SetNotifyWindowMessage(m_hWnd, WM_TTSAPPCUSTOMEVENT, 0, 0);
	}

	// We're interested in all TTS events
	if (SUCCEEDED(hr))
	{
		hr = m_cpVoice->SetInterest(SPFEI_ALL_TTS_EVENTS, SPFEI_ALL_TTS_EVENTS);
	}

	return SUCCEEDED(hr);
}

bool CTTS5::EnumVoices(const std::wstring& regKey)
{
	HRESULT hr;
	ISpObjectToken * pToken;        // NOTE:  Not a CComPtr!  Be Careful.
	CComPtr<IEnumSpObjectTokens> cpEnum;
	hr = SpEnumTokens(regKey.empty() ? SPCAT_VOICES : regKey.c_str(), NULL /*pszRequiredAttrib*/, NULL /*pszOptionalAttrib*/, &cpEnum);
	if (hr != S_OK)
		return false;

	while (cpEnum->Next(1, &pToken, NULL) == S_OK)
	{
		CSpDynamicString dstrDesc;
		hr = SpGetDescription(pToken, &dstrDesc);
		if (SUCCEEDED(hr))
		{
			LANGID langID;
			hr = SpGetLanguageFromToken(pToken, &langID);

			std::wstring name(dstrDesc.m_psz);

			if (m_Voices.find(name) == m_Voices.end())
			{
				CComPtr<ISpDataKey> cpDataKeyAttribs;
				hr = pToken->OpenKey(SPTOKENKEY_ATTRIBUTES, &cpDataKeyAttribs);

				CSpDynamicString Vendor, Gender, Age;
				//cpDataKeyAttribs->GetStringValue(L"Language", &langId);
				cpDataKeyAttribs->GetStringValue(L"Vendor", &Vendor);
				cpDataKeyAttribs->GetStringValue(L"Gender", &Gender);
				cpDataKeyAttribs->GetStringValue(L"Age", &Age);

				SVoice5* pVoice = new SVoice5();
				pVoice->Name = name;
				pVoice->Vendor = std::wstring(Vendor);
				if (wcscmp(Gender, L"Elderly") == 0)
					pVoice->Age = SVoice5::eElderly;
				else if (wcscmp(Gender, L"Child") == 0)
					pVoice->Age = SVoice5::eChild;
				else
					pVoice->Age = SVoice5::eAdult;
				if (wcscmp(Gender, L"Male") == 0)
					pVoice->Gender = SVoice5::eMale;
				else if (wcscmp(Gender, L"Female") == 0)
					pVoice->Gender = SVoice5::eFemale;
				else
					pVoice->Gender = SVoice5::eNeutral;
				pVoice->LangID = langID;
				WCHAR strNameBuffer[LOCALE_NAME_MAX_LENGTH+1];
				//LCIDToLocaleName(pVoice->LangID, strNameBuffer, LOCALE_NAME_MAX_LENGTH, 0);
				GetLocaleInfoW(MAKELCID(pVoice->LangID, SORT_DEFAULT), LOCALE_SENGLANGUAGE, strNameBuffer, LOCALE_NAME_MAX_LENGTH);
				pVoice->Lang = strNameBuffer;
				pVoice->Sapi4 = false;
				pVoice->pToken = pToken;
				m_Voices[name] = pVoice;
			}
			else
				hr = E_FAIL;
		}
		if (FAILED(hr))
		{
			pToken->Release();
		}
	}
	
	return true;
}

void CTTS5::ClearVoices()
{
	for(std::map<std::wstring, SVoice5*>::iterator I = m_Voices.begin(); I != m_Voices.end(); ++I)
	{
		I->second->pToken->Release();
		delete I->second;
	}
	m_Voices.clear();
}

HRESULT CTTS5::VoiceChange(SVoice5* pVoice)
{
	if (m_pTocken == pVoice->pToken)
		return S_OK;
	m_pTocken = pVoice->pToken;

	HRESULT         hr = S_OK;
	GUID*           pguidAudioFormat = NULL;
	int             iFormat = 0;

	// Get the token associated with the selected voice
	ISpObjectToken* pToken = pVoice->pToken;

	//Determine if it is the current voice
	CComPtr<ISpObjectToken> pOldToken;
	hr = m_cpVoice->GetVoice(&pOldToken);

	if (SUCCEEDED(hr))
	{
		if (pOldToken != pToken)
		{
			// Stop speaking. This is not necesary, for the next call to work,
			// but just to show that we are changing voices.
			hr = m_cpVoice->Speak(NULL, SPF_PURGEBEFORESPEAK, 0);
			// And set the new voice on the global voice object
			if (SUCCEEDED(hr))
			{
				hr = m_cpVoice->SetVoice(pToken);
			}
		}
	}

	if (SUCCEEDED(hr))
	{
		long DefaultRate;
		hr = m_cpVoice->GetRate(&DefaultRate);
		m_DefaultRate = DefaultRate;
		m_MinRate = SPMIN_RATE;
		m_MaxRate = SPMAX_RATE;
	}

	m_MinPitch = 0;
	m_MaxPitch = 0;

	if (SUCCEEDED(hr))
	{
		USHORT DefaultVolume;
		hr = m_cpVoice->GetVolume(&DefaultVolume);
		m_DefaultVolume = DefaultVolume;
		m_MinVolume = SPMIN_VOLUME;
		m_MaxVolume = SPMAX_VOLUME;
	}

	return hr;
}

std::vector<SVoice*> CTTS5::GetVoices()
{
	std::vector<SVoice*> list;
	for (std::map<std::wstring, SVoice5*>::iterator I = m_Voices.begin(); I != m_Voices.end(); ++I)
		list.push_back(I->second);
	return list;
}

bool CTTS5::SelectVoice(const std::wstring& name)
{
	for (std::map<std::wstring, SVoice5*>::iterator I = m_Voices.begin(); I != m_Voices.end(); ++I)
	{
		if (I->second->Name == name)
			return VoiceChange(I->second) == S_OK;
	}
	return false;
}

bool CTTS5::Speak(const std::wstring& str, bool ProcessXML)
{
	HRESULT hr = S_OK;

	if (m_bPause) 
		Stop();

	m_bStop = FALSE;

	m_BufferLength = str.length();

	hr = m_cpVoice->Speak(str.c_str(), SPF_ASYNC | (ProcessXML ? SPF_IS_XML : SPF_IS_NOT_XML), 0);
	
	if (SUCCEEDED(hr))
	{
		m_bPause = FALSE;
		hr = m_cpVoice->Resume();
	}

	return SUCCEEDED(hr);
}

void CTTS5::Pause()
{
	if (m_bStop)
		return;
	
	if (!m_bPause)
	{
		m_cpVoice->Pause();
		m_bPause = TRUE;
	}
	else
	{
		m_cpVoice->Resume();
		m_bPause = FALSE;
	}
}

void CTTS5::Skip(int SkipNum)
{
	ULONG ulGarbage = 0;
	WCHAR szGarbage[] = L"Sentence"; // only supported type
	m_cpVoice->Skip(szGarbage, SkipNum, &ulGarbage);
}

void CTTS5::Stop()
{
	bool bWasPaused = m_bPause;

	m_bPause = false;
	m_bStop = true;

	// Stop current rendering with a PURGEBEFORESPEAK...
	HRESULT hr = m_cpVoice->Speak(NULL, SPF_PURGEBEFORESPEAK, 0);

	if (bWasPaused)
		OnStop();
}

void CTTS5::SetRate(int Rate)
{
	m_cpVoice->SetRate(Rate);
}

int CTTS5::GetRate()
{
	long DefaultRate;
	m_cpVoice->GetRate(&DefaultRate);
	return DefaultRate;
}

void CTTS5::SetVolume(int Volume)
{
	m_cpVoice->SetVolume(Volume);
}

int CTTS5::GetVolume()
{
	USHORT DefaultVolume;
	m_cpVoice->GetVolume(&DefaultVolume);
	return DefaultVolume;
}

void CTTS5::SetFormat(SPSTREAMFORMAT eFmt)
{
	HRESULT hr = S_OK;

	CSpStreamFormat Fmt;
	Fmt.AssignFormat(eFmt);
	if (m_cpOutAudio)
	{
		hr = m_cpOutAudio->SetFormat(Fmt.FormatId(), Fmt.WaveFormatExPtr());
	}
	else
	{
		hr = E_FAIL;
	}

	if (SUCCEEDED(hr))
	{
		hr = m_cpVoice->SetOutput(m_cpOutAudio, FALSE);
	}
}

bool CTTS5::Speak2Wave(const std::wstring& path, const std::wstring& str, bool ProcessXML)
{
	CComPtr<ISpStream> cpWavStream;
	CComPtr<ISpStreamFormat> cpOldStream;
	HRESULT hr = S_OK;
	bool Ret = false;

	CSpStreamFormat OriginalFmt;
	hr = m_cpVoice->GetOutputStream(&cpOldStream);
	if (hr == S_OK)
	{
		hr = OriginalFmt.AssignFormat(cpOldStream);
	}
	else
	{
		hr = E_FAIL;
	}
	// User SAPI helper function in sphelper.h to create a wav file
	if (SUCCEEDED(hr))
	{
		hr = SPBindToFile(path.c_str(), SPFM_CREATE_ALWAYS, &cpWavStream, &OriginalFmt.FormatId(), OriginalFmt.WaveFormatExPtr());
	}
	if (SUCCEEDED(hr))
	{
		// Set the voice's output to the wav file instead of the speakers
		hr = m_cpVoice->SetOutput(cpWavStream, TRUE);
	}

	if (SUCCEEDED(hr))
	{
		// Do the Speak
		Ret = Speak(str, ProcessXML);
	}

	// Set output back to original stream
	// Wait until the speak is finished if saving to a wav file so that
	// the smart pointer cpWavStream doesn't get released before its
	// finished writing to the wav.
	m_cpVoice->WaitUntilDone(INFINITE);
	cpWavStream.Release();

	// Reset output
	m_cpVoice->SetOutput(cpOldStream, FALSE);

	return Ret;
}


/*
	CComPtr<ISpStream>       cpWavStream;
	WCHAR                       szwWavFileName[NORM_SIZE] = L"";;

	USES_CONVERSION;
	wcscpy( szwWavFileName, T2W( szAFileName ) );

	// User helper function found in sphelper.h to open the wav file and
	// get back an IStream pointer to pass to SpeakStream
	hr = SPBindToFile( szwWavFileName, SPFM_OPEN_READONLY, &cpWavStream );

	if( SUCCEEDED( hr ) )
	{
		hr = m_cpVoice->SpeakStream( cpWavStream, SPF_ASYNC, NULL );
	}

	if( FAILED( hr ) )
	{
		TTSAppStatusMessage( m_hWnd, _T("Speak error\r\n") );
	}
*/