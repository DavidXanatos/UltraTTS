#pragma once

struct SVoice {
	std::wstring Name;
	std::wstring Vendor;
	// // Speaker
	// // Product
	enum EAge
	{
		eAdult = 0,
		eChild = 1,
		eElderly = 2
	};
	EAge Age;
	enum EGender
	{
		eNeutral = 0,
		eFemale = 1,
		eMale = 2
	};
	EGender Gender;
	LANGID LangID;
	std::wstring Lang;
	// // Style
	bool Sapi4;
};

class CTTS 
{
public:
	virtual ~CTTS() {}

	static CTTS*		NewTTS();
	static CTTS*		NewTTS4();
	static CTTS*		NewTTS5();

	virtual bool		Speak(const std::wstring& str, bool ProcessXML = false) = 0;
	virtual bool		Speak2Wave(const std::wstring& path, const std::wstring& str, bool ProcessXML = false) = 0;
	virtual bool		IsSpeaking() = 0;
	virtual void		Pause() = 0;
	virtual bool		IsPaused() = 0;
	virtual void		Skip(int SkipNum) = 0;
	virtual void		Inject(const std::wstring& str) = 0;
	virtual void		Stop() = 0;

	virtual void		SetRate(int Rate) = 0;
	virtual int			GetRate() = 0;
	virtual void		SetPitch(int Pitch) = 0;
	virtual int			GetPitch() = 0;
	virtual void		SetVolume(int Volume) = 0;
	virtual int			GetVolume() = 0;

	virtual int			GetMinRate() = 0;
	virtual int			GetDefaultRate() = 0;
	virtual int			GetMaxRate() = 0;

	virtual int			GetMinPitch() = 0;
	virtual int			GetDefaultPitch() = 0;
	virtual int			GetMaxPitch() = 0;

	virtual int			GetMinVolume() = 0;
	virtual int			GetDefaultVolume() = 0;
	virtual int			GetMaxVolume() = 0;

	virtual std::vector<SVoice*> GetVoices() = 0;
	virtual bool		SelectVoice(const std::wstring& name) = 0;

	virtual bool		EnumVoices(const std::wstring& regKey = L"") = 0;
	virtual void		ClearVoices() = 0;

	void				SetCallBacks(std::function<void()> pOnStop, std::function<void(int pos, int len, int total)> pOnProgress) { m_pOnStop = pOnStop; m_pOnProgress = pOnProgress; }

	void OnStop() { if (m_pOnStop) m_pOnStop(); }
	void OnProgress(int pos, int len, int total) { if (m_pOnProgress) m_pOnProgress(pos, len, total); }

protected:

	std::function<void()> m_pOnStop;
	std::function<void(int pos, int len, int total)> m_pOnProgress;
};

class CTTS_Impl: public CTTS
{
public:
	CTTS_Impl();

	bool		IsSpeaking() { return !m_bStop; }
	bool		IsPaused() { return m_bPause; }

	int			GetMinRate() { return m_MinRate; }
	int			GetDefaultRate() { return m_DefaultRate; }
	int			GetMaxRate() { return m_MaxRate; }

	int			GetMinPitch() { return m_MinPitch; }
	int			GetDefaultPitch() { return m_DefaultPitch; }
	int			GetMaxPitch() { return m_MaxPitch; }

	int			GetMinVolume() { return m_MinVolume; }
	int			GetDefaultVolume() { return m_DefaultVolume; }
	int			GetMaxVolume() { return m_MaxVolume; }

protected:
	bool        m_bPause;
	bool        m_bStop;

	int			m_DefaultVolume;
	int			m_MinVolume;
	int			m_MaxVolume;

	int			m_DefaultRate;
	int			m_MinRate;
	int			m_MaxRate;

	int			m_DefaultPitch;
	int			m_MinPitch;
	int			m_MaxPitch;
};

class CTTSApi : public CTTS
{
public:
	CTTSApi();
	~CTTSApi();

	static const wchar_t CTTSApi::Voices_Platform[];
	static const wchar_t CTTSApi::Voices_OneCore[];
	static const wchar_t CTTSApi::Voices_Speech[];
	static const wchar_t CTTSApi::Voices_Cortana[];

	virtual bool		Speak(const std::wstring& str, bool ProcessXML = false);
	virtual bool		Speak2Wave(const std::wstring& path, const std::wstring& str, bool ProcessXML = false);
	virtual bool		IsSpeaking();
	virtual void		Pause();
	virtual bool		IsPaused();
	virtual void		Skip(int SkipNum);
	virtual void		Inject(const std::wstring& str);
	virtual void		Stop();

	virtual void		SetRate(int Rate);
	virtual int			GetRate();
	virtual void		SetPitch(int Pitch);
	virtual int			GetPitch();
	virtual void		SetVolume(int Volume);
	virtual int			GetVolume();

	virtual int			GetMinRate();
	virtual int			GetDefaultRate();
	virtual int			GetMaxRate();

	virtual int			GetMinPitch();
	virtual int			GetDefaultPitch();
	virtual int			GetMaxPitch();

	virtual int			GetMinVolume();
	virtual int			GetDefaultVolume();
	virtual int			GetMaxVolume();

	virtual std::vector<SVoice*> GetVoices();
	virtual bool		SelectVoice(const std::wstring& name);

	virtual bool		EnumVoices(const std::wstring& regKey = L"");
	virtual void		ClearVoices();

protected:
	class CTTS4*		m_pTTS4;
	class CTTS5*		m_pTTS5;
	bool				m_Use4;


};