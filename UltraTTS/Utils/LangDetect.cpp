#include "stdafx.h"
#include "LangDetect.h"


#define CLD_WINDOWS

#include "../cld/encodings/compact_lang_det/compact_lang_det.h"
#include "../cld/encodings/compact_lang_det/ext_lang_enc.h"
#include "../cld/encodings/compact_lang_det/unittest_data.h"
#include "../cld/encodings/proto/encodings.pb.h"
#include "../cld/languages/proto/languages.pb.h"

QPair<QString, int> DetectLanguage(const QString& text, int* pLang)
{
	bool is_plain_text = true;
	bool do_allow_extended_languages = true;
	bool do_pick_summary_language = false;
	bool do_remove_weak_matches = false;
	bool is_reliable;
	const char* tld_hint = NULL;
	int encoding_hint = UTF8; //  UNKNOWN_ENCODING;
	Language language_hint = pLang ? (Language)*pLang : UNKNOWN_LANGUAGE;
	double normalized_score3[3];
	Language language3[3];
	int percent3[3];
	int text_bytes;

	QByteArray str = text.toUtf8();
	const char* src = str.data();
	Language lang = CompactLangDet::DetectLanguage(0, src, strlen(src), is_plain_text,
		do_allow_extended_languages, do_pick_summary_language, do_remove_weak_matches,
		tld_hint, encoding_hint, language_hint,
		language3, percent3, normalized_score3,
		&text_bytes, &is_reliable);

	if (pLang)
		*pLang = lang;

	QString Lang = QString(LanguageName(lang));

	return QPair<QString, int>(Lang.mid(0, 1) + Lang.mid(1).toLower(), percent3[0]);
}