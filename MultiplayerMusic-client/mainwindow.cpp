#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qdebug.h"
#include "console.h"

#include <QDir>

MainWindow* g_pWindow;

#ifndef WIN32
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
#else

#define WINDOWS_LEAN_AND_MEAN
#include "Windows.h"
#undef PlaySound

inline int translate_numpad(int vk) {
	switch (vk) {
	case VK_NUMPAD0:
    //case VK_INSERT:
		return 0;
	case VK_NUMPAD1:
    //case VK_END:
		return 1;
	case VK_NUMPAD2:
    //case VK_DOWN:
		return 2;
	case VK_NUMPAD3:
		return 3;
	case VK_NUMPAD4:
		return 4;
	case VK_NUMPAD5:
		return 5;
	case VK_NUMPAD6:
		return 6;
	case VK_NUMPAD7:
		return 7;
	case VK_NUMPAD8:
		return 8;
	case VK_NUMPAD9:
		return 9;
	default:
		return -1;
	}
}

LRESULT CALLBACK HookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode < 0) {
        return CallNextHookEx(NULL, nCode, wParam, lParam);
    }
    // process event...
    //qDebug() << nCode << wParam << lParam;

    if (wParam == WM_KEYUP) {
        KBDLLHOOKSTRUCT *kb = (KBDLLHOOKSTRUCT*)lParam;

		int numpad = translate_numpad(kb->vkCode);

		if (numpad != -1) {
			if (numpad == 0) {
				g_pWindow->stop();
			} else {
				g_pWindow->sendMessage(numpad);
			}

            return 1;
        }

		if (kb->vkCode == VK_ADD) {
            g_pWindow->IncreaseVolume();
            return 1;
        } else if (kb->vkCode == VK_SUBTRACT) {
            g_pWindow->LowerVolume();
            return 1;
        }
    }

    return CallNextHookEx(NULL, nCode, wParam, lParam);
}
#endif

MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::MainWindow)
{
    g_pWindow = this;
    ui->setupUi(this);

	SetConsole(ui->console);
	ui->console->setReadOnly(true);

	Msg("Console initialized");

    connect(ui->connectButton, SIGNAL(clicked()), SLOT(connectToServer()));
    connect(ui->disconnectButton, SIGNAL(clicked()), SLOT(disconnectFromServer()));

    connect(ui->stopButton, SIGNAL(clicked()), SLOT(stop()));

	connect(&socket, SIGNAL(connected()), SLOT(onConnected()));
	connect(&socket, SIGNAL(disconnected()), SLOT(onDisconnected()));
    connect(&socket, SIGNAL(readyRead()), SLOT(read()));

	soundmgr = new CSoundManager();

	connect(ui->volumeSlider, SIGNAL(valueChanged(int)), SLOT(volumeSliderChanged(int)));
#ifndef WIN32
    grab_key();
	startTimer(100);
#else
    if (SetWindowsHookEx(WH_KEYBOARD_LL, HookProc, qWinAppInst(), NULL) == 0)
        qDebug() << "Hook failed for application instance" << qWinAppInst() << "with error:" << GetLastError();
#endif

}

MainWindow::~MainWindow()
{
#ifndef WIN32
    ungrab_key();
#endif
    delete ui;
	delete soundmgr;
    socket.disconnectFromHost();
}

void MainWindow::read() {
	QByteArray header = socket.read(2);

	quint8 len = header[0];
	quint8 type = header[1];

	if (type == 0) {
		qDebug() << "Received stop message";
		//sound->stop();
		soundmgr->Stop();
	} else if (type == 1) {
		QByteArray body = socket.read(len - 2);
		quint8 sound_id = body[0];

		qDebug() << "Received message to play sound" << sound_id;
        soundmgr->PlaySound(sound_id);
    }
}

void MainWindow::connectToServer() {
    QString server = ui->serverEdit->text();

    bool ok;
    int port = ui->portEdit->text().toInt(&ok);

    if (ok) {
        socket.connectToHost(server, port);
    } else {
        Msg("Invalid port number");
    }
}

void MainWindow::disconnectFromServer() {
    socket.disconnectFromHost();
}

void MainWindow::onConnected() {
	Msg("Connected");
    ui->connectButton->setEnabled(false);
    ui->disconnectButton->setEnabled(true);
}

void MainWindow::onDisconnected() {
	Msg("Disconnected");
    ui->connectButton->setEnabled(true);
    ui->disconnectButton->setEnabled(false);
}

void MainWindow::LowerVolume() {
    ui->volumeSlider->setValue( ui->volumeSlider->value() - 10 );
}

void MainWindow::IncreaseVolume() {
    ui->volumeSlider->setValue( ui->volumeSlider->value() + 10 );
}

void MainWindow::volumeSliderChanged(int newVolume) {
	float volume = newVolume / 100.0f;
    soundmgr->SetVolume(volume);
}

void MainWindow::sendMessage(int id) {
    qDebug() << "sendMessage";
    if (socket.isOpen()) {
		QByteArray msg;
		msg.append((char)3);
		msg.append((char)1);
		msg.append((char)id);

		socket.write(msg);
	} else {
		qDebug() << "Socket is not open";
	}
}

void MainWindow::stop() {
	soundmgr->Stop();

	if (socket.isOpen()) {
		QByteArray msg;
		msg.append((char)2);
		msg.append((char)0);

		socket.write(msg);
	} else {
		qDebug() << "Socket is not open";
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
				sendMessage(1);
			} else if (sym == XK_F11) {
				sendMessage(2);
			} else if (sym == XK_F10) {
				stop();
			}
            return;
        default:
            break;
        }
    }
#else
#endif
}
