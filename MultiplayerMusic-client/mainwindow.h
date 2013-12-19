#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
//#include <QSound>
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

    void LowerVolume();
    void IncreaseVolume();
public slots:
	void connectToServer();
	void sendMessage(int id);

	void read();
    void stop();

private slots:
	void onConnected();
	void onDisconnected();
    void volumeSliderChanged(int newVolume);
    void disconnectFromServer();
private:
	Ui::MainWindow *ui;

	QTcpSocket socket;
	//QSound *sound;
	CSoundManager *soundmgr;
};

#endif // MAINWINDOW_H
