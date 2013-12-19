#include "console.h"

QTextEdit* g_pConsole;

void SetConsole(QTextEdit* console) {
	g_pConsole = console;
}

void Msg(QString str) {
	g_pConsole->append(str);
}

void Warn(QString str) {
	Msg(str);
}

void Dev(QString str) {
	Msg(str);
}
