#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "chatrobot.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    chatrobot robot;
    QTimer *mytimer;

    VideoCapture myvideo;
    Mat myframe,myframe1;
    uchar *frametemp=NULL;
    int local_frame_size;
signals:
    void videostart();

private slots:
   void  loginServer();
    void logoutServer();
    void Start();
    void videoPro();
    void videoTimerPro();
};

#endif // MAINWINDOW_H
