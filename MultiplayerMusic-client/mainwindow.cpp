#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qdebug.h"

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

#include "Windows.h"



LRESULT CALLBACK HookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    // process event...
    //qDebug() << nCode << wParam << lParam;

    if (wParam == WM_KEYUP) {
        KBDLLHOOKSTRUCT *kb = (KBDLLHOOKSTRUCT*)lParam;

        if (kb->vkCode == VK_F12) {
            g_pWindow->sendMessage();
        } else if (kb->vkCode == VK_F11) {
            g_pWindow->stop();
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

    connect(ui->connectButton, SIGNAL(clicked()), SLOT(connectToServer()));
    connect(ui->playButton, SIGNAL(clicked()), SLOT(sendMessage()));

    connect(ui->stopButton, SIGNAL(clicked()), SLOT(stop()));

    connect(&socket, SIGNAL(readyRead()), SLOT(read()));

    // TODO: Error handling
    sound = new QSound("./sound.wav");

#ifndef WIN32
    grab_key();
#else
    if (SetWindowsHookEx(WH_KEYBOARD_LL, HookProc, qWinAppInst(), NULL) == 0)
        qDebug() << "Hook failed for application instance" << qWinAppInst() << "with error:" << GetLastError();
#endif

    startTimer(100);
}

MainWindow::~MainWindow()
{
#ifndef WIN32
    ungrab_key();
#endif
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

void MainWindow::stop() {
    socket.write("stop");
    sound->stop();
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
