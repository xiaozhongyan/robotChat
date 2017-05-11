#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    mytimer=new QTimer;

    connect(ui->loginBtn,SIGNAL(clicked(bool)),this,SLOT(loginServer()));
    connect(ui->logoutBtn,SIGNAL(clicked(bool)),this,SLOT(logoutServer()));
    connect(ui->startBtn,SIGNAL(clicked(bool)),this,SLOT(Start()));
    connect(this,SIGNAL(videostart()),this,SLOT(videoTimerPro()));

    robot.enableInputFromEX(true);//设置外部视频数据输入
}

MainWindow::~MainWindow()
{
    free(frametemp);
    delete mytimer;
    delete ui;
}
void MainWindow::loginServer(){
    robot.Set_chatLogin_Info("cloud.anychat.cn",8906,"276232909@qq.com","yan1993","e8583d7a-fa16-4b57-bab5-5468458237ca",12,"");
    robot.chatInit();
    robot.chatLogin();
    myvideo.open(0);
    if(myvideo.isOpened()){
        myvideo>>myframe1;
        cv::resize(myframe1,myframe,cvSize(640,480));
        std::cout<<"#DEBUG INFO#video opened...VIDEO_COLS:"<<myframe.cols<<std::endl;
        robot.SetInputVideoFormat(myframe.cols,myframe.rows,10);
    }
}
void MainWindow::Start(){
    emit videoTimerPro();
}

void MainWindow::logoutServer(){
    mytimer->stop();
    robot.chatLeaveRoom();
    robot.chatLogoutServer();
}
void MainWindow::videoPro(){
    myvideo>>myframe1;
       cv::resize(myframe1,myframe,cvSize(640,480));
    int framesize=myframe.cols*myframe.rows*3;

    if( !frametemp || local_frame_size < framesize)
    {
        if(frametemp)
            free(frametemp);
        frametemp = (uchar*)realloc(frametemp, framesize);
        if(!frametemp){
            std::cout<<"realloc error"<<std::endl;
             return;
        }
        local_frame_size = framesize;
    }
    memcpy(frametemp,myframe.data,framesize);
    robot.chatInputVideo(frametemp,framesize,0);
}
void MainWindow::videoTimerPro(){
    mytimer->setInterval(100);
    connect(mytimer,SIGNAL(timeout()),this,SLOT(videoPro()));
    mytimer->start();
    std::cout<<"#DEBUG INFO#videotimer start"<<std::endl;
}
