#ifndef CONSOLE_H
#define CONSOLE_H

#include <QTextEdit>

extern QTextEdit* g_pConsole;

void SetConsole(QTextEdit* console);

void Msg(QString str);

void Warn(QString str);

void Dev(QString str);

#endif // CONSOLE_H
