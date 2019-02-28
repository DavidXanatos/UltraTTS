#include "stdafx.h"
#include "UltraTTS.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	CUltraTTS w;
	w.show();
	return a.exec();
}
