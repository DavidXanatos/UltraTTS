#include "stdafx.h"
#include "TTS.h"

#include "TTS4.h"
#include "TTS5.h"


CTTS* CTTS::NewTTS()
{
	return new CTTSApi();
}

CTTS* CTTS::NewTTS4()
{
	return new CTTS4();
}

CTTS* CTTS::NewTTS5()
{
	return new CTTS5();
}

/////////////////////////////////////////////////////////////////////

CTTS_Impl::CTTS_Impl()
{
	m_bPause = false;
	m_bStop = true;

	m_DefaultVolume = 0;
	m_MinVolume = 0;
	m_MaxVolume = 0;

	m_DefaultRate = 0;
	m_MinRate = 0;
	m_MaxRate = 0;

	m_DefaultPitch = 0;
	m_MinPitch = 0;
	m_MaxPitch = 0;
}

/////////////////////////////////////////////////////////////////////

const wchar_t CTTSApi::Voices_Platform[] = L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Speech Server\\v11.0\\Voices";
const wchar_t CTTSApi::Voices_OneCore[] = L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Speech_OneCore\\Voices";
const wchar_t CTTSApi::Voices_Speech[] = L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Speech\\Voices";
const wchar_t CTTSApi::Voices_Cortana[] = L"HKEY_CLASSES_ROOT\\Local Settings\\Software\\Microsoft\\Windows\\CurrentVersion\\AppContainer\\Storage\\microsoft.windows.cortana_cw5n1h2txyewy\\SOFTWARE\\Microsoft\\Speech_OneCore\\Isolated\\jWXZvMzcRxToSdOzNgXV_L3ZSrLDNbZuY5NZNWCCUd8\\HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Speech_OneCore\\Voices";


CTTSApi::CTTSApi()
{
	m_Use4 = false;

	CTTSApi* that = this;

	m_pTTS4 = new CTTS4();
	m_pTTS4->SetCallBacks([that]() {
		if (that->m_Use4)
			that->OnStop();
	}, [that](int pos, int len, int total) {
		if (that->m_Use4)
			that->OnProgress(pos,len, total);
	});

	m_pTTS5 = new CTTS5();
	m_pTTS5->SetCallBacks([that]() {
		if (!that->m_Use4)
			that->OnStop();
	}, [that](int pos, int len, int total) {
		if (!that->m_Use4)
			that->OnProgress(pos, len, total);
	});
}

CTTSApi::~CTTSApi()
{
	delete m_pTTS4;
	delete m_pTTS5;
}

bool CTTSApi::Speak(const std::wstring& str, bool ProcessXML)
{
	if (m_Use4)
		return m_pTTS4->Speak(str);
	return m_pTTS5->Speak(str, ProcessXML);
}

bool CTTSApi::Speak2Wave(const std::wstring& path, const std::wstring& str, bool ProcessXML)
{
	if (m_Use4)
		return m_pTTS4->Speak2Wave(path, str, ProcessXML);
	return m_pTTS5->Speak2Wave(path, str, ProcessXML);
}

bool CTTSApi::IsSpeaking()
{
	if (m_Use4)
		return m_pTTS4->IsSpeaking();
	return m_pTTS5->IsSpeaking();
}

void CTTSApi::Pause()
{
	if (m_Use4)
		return m_pTTS4->Pause();
	return m_pTTS5->Pause();
}

bool CTTSApi::IsPaused()
{
	if (m_Use4)
		return m_pTTS4->IsPaused();
	return m_pTTS5->IsPaused();
}

void CTTSApi::Skip(int SkipNum)
{
	if (m_Use4)
		return m_pTTS4->Skip(SkipNum);
	return m_pTTS5->Skip(SkipNum);
}

void CTTSApi::Inject(const std::wstring& str)
{
	if (m_Use4)
		return m_pTTS4->Inject(str);
	return m_pTTS5->Inject(str);
}

void CTTSApi::Stop()
{
	if (m_Use4)
		return m_pTTS4->Stop();
	return m_pTTS5->Stop();
}


void CTTSApi::SetRate(int Rate)
{
	if (m_Use4)
		return m_pTTS4->SetRate(Rate);
	return m_pTTS5->SetRate(Rate);
}

int CTTSApi::GetRate()
{
	if (m_Use4)
		return m_pTTS4->GetRate();
	return m_pTTS5->GetRate();
}

void CTTSApi::SetPitch(int Pitch)
{
	if (m_Use4)
		return m_pTTS4->SetPitch(Pitch);
	return m_pTTS5->SetPitch(Pitch);
}

int CTTSApi::GetPitch()
{
	if (m_Use4)
		return m_pTTS4->GetPitch();
	return m_pTTS5->GetPitch();
}

void CTTSApi::SetVolume(int Volume)
{
	if (m_Use4)
		return m_pTTS4->SetVolume(Volume);
	return m_pTTS5->SetVolume(Volume);
}

int CTTSApi::GetVolume()
{
	if (m_Use4)
		return m_pTTS4->GetVolume();
	return m_pTTS5->GetVolume();
}

int CTTSApi::GetMinRate()
{
	if (m_Use4)
		return m_pTTS4->GetMinRate();
	return m_pTTS5->GetMinRate();
}
int CTTSApi::GetDefaultRate()
{
	if (m_Use4)
		return m_pTTS4->GetDefaultRate();
	return m_pTTS5->GetDefaultRate();
}
int CTTSApi::GetMaxRate()
{
	if (m_Use4)
		return m_pTTS4->GetMaxRate();
	return m_pTTS5->GetMaxRate();
}

int CTTSApi::GetMinPitch()
{
	if (m_Use4)
		return m_pTTS4->GetMinPitch();
	return m_pTTS5->GetMinPitch();
}
int CTTSApi::GetDefaultPitch()
{
	if (m_Use4)
		return m_pTTS4->GetDefaultPitch();
	return m_pTTS5->GetDefaultPitch();
}
int CTTSApi::GetMaxPitch()
{
	if (m_Use4)
		return m_pTTS4->GetMaxPitch();
	return m_pTTS5->GetMaxPitch();
}

int CTTSApi::GetMinVolume()
{
	if (m_Use4)
		return m_pTTS4->GetMinVolume();
	return m_pTTS5->GetMinVolume();
}
int CTTSApi::GetDefaultVolume()
{
	if (m_Use4)
		return m_pTTS4->GetDefaultVolume();
	return m_pTTS5->GetDefaultVolume();
}
int CTTSApi::GetMaxVolume()
{
	if (m_Use4)
		return m_pTTS4->GetMaxVolume();
	return m_pTTS5->GetMaxVolume();
}

std::vector<SVoice*> CTTSApi::GetVoices()
{
	std::vector<SVoice*> list = m_pTTS5->GetVoices();
	std::vector<SVoice*> list4 = m_pTTS4->GetVoices();
	if (!list4.empty())
		list.insert(list.end(), list4.begin(), list4.end());
	return list;
}

bool CTTSApi::SelectVoice(const std::wstring& name)
{
	if (m_pTTS5->SelectVoice(name))
		m_Use4 = false;
	else if (m_pTTS4->SelectVoice(name))
		m_Use4 = true;
	else
		return false;
	return true;
}

bool CTTSApi::EnumVoices(const std::wstring& regKey)
{
	if (regKey.empty())
		return m_pTTS4->EnumVoices(regKey);
	return m_pTTS5->EnumVoices(regKey);
}

void CTTSApi::ClearVoices()
{
	m_pTTS5->ClearVoices();
	m_pTTS4->ClearVoices();
}