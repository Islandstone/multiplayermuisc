#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QSound>

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
	void sendMessage();

	void read();
    void stop();

private:
	Ui::MainWindow *ui;

	QTcpSocket socket;
	QSound *sound;
};

#endif // MAINWINDOW_H
