#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qdebug.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>

Display*    dpy     ;
Window      root    ;
XEvent      ev;
unsigned int    modifiers       ;
int             keycode         ;
Window          grab_window     ;
Bool            owner_events    ;
int             pointer_mode    ;
int             keyboard_mode   ;

void grab_key() {
	dpy     = XOpenDisplay(0);
	root    = DefaultRootWindow(dpy);
	modifiers       = ControlMask | ShiftMask;
	keycode         = XKeysymToKeycode(dpy,XK_F12);
	grab_window     =  root;
	owner_events    = False;
	pointer_mode    = GrabModeAsync;
	keyboard_mode   = GrabModeAsync;

	XGrabKey(dpy, keycode, modifiers, grab_window, owner_events, pointer_mode,
			 keyboard_mode);

	XSelectInput(dpy, root, KeyPressMask);
}

void ungrab_key() {
	XUngrabKey(dpy,keycode,modifiers,grab_window);
	XCloseDisplay(dpy);
}

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	connect(ui->connectButton, SIGNAL(clicked()), SLOT(connectToServer()));
	connect(ui->playButton, SIGNAL(clicked()), SLOT(sendMessage()));

	connect(&socket, SIGNAL(readyRead()), SLOT(read()));

	// TODO: Error handling
	sound = new QSound("./sound.wav");

	grab_key();

	startTimer(100);
}

MainWindow::~MainWindow()
{
	ungrab_key();
	delete ui;
	socket.disconnectFromHost();
}

void MainWindow::read() {
	QString str(socket.read(4));

	qDebug() << "Received: " << str;

	if (str == "play") {
		sound->play();
	} else if (str == "stop") {
		sound->stop();
	}
}

void MainWindow::connectToServer() {
	qDebug() << "Connect";

	QString server = ui->serverEdit->text();

	bool ok;
	int port = ui->portEdit->text().toInt(&ok);

	if (!ok) {
		qDebug() << "Not a port number";
		return;
	}

	socket.connectToHost(server, port);
	socket.waitForConnected();

	if (!socket.isOpen()) {
		qDebug() << "Failed to connect to " << server << ":" << port;
	}
}

void MainWindow::sendMessage() {
	qDebug() << "sendMessage";
	if (socket.isOpen()) {
		socket.write("play");
	}
}

void MainWindow::timerEvent(QTimerEvent *) {
#ifndef WIN32
	if (XPending(dpy) == 0)
		return;

	while (XPending(dpy) > 0) {
		XNextEvent(dpy, &ev);
		XKeyEvent* k = (XKeyEvent*)&ev;
		int sym = XKeycodeToKeysym(dpy, k->keycode, 0);

		switch(ev.type)
		{
		case KeyPress:
			qDebug() << k->keycode;

			if (sym == XK_F12){
				qDebug() << "Hot key pressed!";
			}
			return;
		default:
			break;
		}
	}
#else
#endif
}
