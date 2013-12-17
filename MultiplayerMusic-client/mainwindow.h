#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QSound>
#include "soundmanager.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

	void timerEvent(QTimerEvent *);

public slots:
	void connectToServer();
	void sendMessage(int id);

	void read();
    void stop();

private:
	Ui::MainWindow *ui;

	QTcpSocket socket;
	//QSound *sound;
	CSoundManager *soundmgr;
};

#endif // MAINWINDOW_H
