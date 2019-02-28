#include "stdafx.h"
#include "Functions.h"

StrPair Split2(const QString& String, QString Separator, bool Back)
{
	int Sep = Back ? String.lastIndexOf(Separator) : String.indexOf(Separator);
	if (Sep != -1)
		return qMakePair(String.left(Sep).trimmed(), String.mid(Sep + Separator.length()).trimmed());
	return qMakePair(String.trimmed(), QString());
}

QStringList SplitStr(const QString& String, QString Separator)
{
	QStringList List = String.split(Separator);
	for (int i = 0; i < List.count(); i++)
	{
		List[i] = List[i].trimmed();
		if (List[i].isEmpty())
			List.removeAt(i--);
	}
	return List;
}

void SendCtrlC()
{
	INPUT ip;
	ip.type = INPUT_KEYBOARD;
	ip.ki.wScan = 0;
	ip.ki.time = 0;
	ip.ki.dwExtraInfo = 0;

	// Press the "Ctrl" key
	ip.ki.wVk = VK_CONTROL;
	ip.ki.dwFlags = 0; // 0 for key press
	SendInput(1, &ip, sizeof(INPUT));

	// Press the "V" key
	ip.ki.wVk = 'C';
	ip.ki.dwFlags = 0; // 0 for key press
	SendInput(1, &ip, sizeof(INPUT));

	// Release the "V" key
	ip.ki.wVk = 'C';
	ip.ki.dwFlags = KEYEVENTF_KEYUP;
	SendInput(1, &ip, sizeof(INPUT));

	// Release the "Ctrl" key
	ip.ki.wVk = VK_CONTROL;
	ip.ki.dwFlags = KEYEVENTF_KEYUP;
	SendInput(1, &ip, sizeof(INPUT));
}