#pragma once

typedef QPair<QString, QString> StrPair;
StrPair Split2(const QString& String, QString Separator = "=", bool Back = false);
QStringList SplitStr(const QString& String, QString Separator);


void SendCtrlC();
