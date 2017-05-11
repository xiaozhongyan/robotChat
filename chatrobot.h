#ifndef CHATROBOT_H
#define CHATROBOT_H

#include<QString>
#include<iconv.h>
#include<iostream>
#include<QImage>
#include<QDebug>
#include<QCoreApplication>
#include<opencv2/highgui.hpp>
#include<opencv.hpp>
#include"BRAnyChatCoreSDK.h"

using namespace cv;

class chatrobot
{
public:
    chatrobot();
private:
    int code_convert(const char *from_charset,const char *to_charset,char *inbuf,size_t inlen,char *outbuf,size_t outlen);//编码变换
private:
    QString ServerIp;
    QString UserName;
    QString UserPwd;
    QString Userappid;
    QString RoomPwd;
    int ServerPort;
    int RoomID;
    int myselfID;
    int VideoWidth;
    int VideoHight;
    int FPS;
    bool EnableInputvideoEx=false;
    char*   myLocalVideoFrame;    //本地视频缓冲
    int     myLocalVideoSize;      //本地视频缓冲大小

public:
    void chatInit();
    void Set_chatLogin_Info(QString serverIP,int serverPort,QString userName,QString userPwd,QString userappid,int roomID,QString roomPwd);
    void enableInputFromEX(bool inputvideoEx);
    void SetInputVideoFormat(int videoWith,int videoHight,int fps);
    void SetMsgFucRec(void(*msgR)(QString msg,int len));
    void chatLogin();
    void chatLeaveRoom();
    void chatLogoutServer();
    void (*chatMsgReceive)(QString msg,int msglen);
    void chatMsgSend(char *msg,int msglen);
    void chatInputVideo(uchar* data,int dataSize,int timeStamp);
    //以下为异步消息回调，初始化的时候设置
    long OnGVClientConnect(WPARAM wParam, LPARAM lParam);
    long OnGVClientLogin(WPARAM wParam, LPARAM lParam);
    long OnGVClientEnterRoom(WPARAM wParam, LPARAM lParam);
    long OnGVClientOnlineUser(WPARAM wParam, LPARAM lParam);
    long OnGVClientMicStateChange(WPARAM wParam, LPARAM lParam);
    long OnGVClientUserAtRoom(WPARAM wParam, LPARAM lParam);
    long OnGVClientLinkClose(WPARAM wParam, LPARAM lParam);

    virtual void OnAnyChatCameraStateChgMessage(DWORD dwUserId, DWORD dwState) {}    // 用户摄像头状态改变消息
    virtual void OnAnyChatActiveStateChgMessage(DWORD dwUserId, DWORD dwState) {}    // 用户活动状态发生变化消息
    virtual void OnAnyChatP2PConnectStateMessage(DWORD dwUserId, DWORD dwState){}    // P2P连接状态变化消息
    virtual void OnAnyChatPrivateRequestMessage(DWORD dwUserId, DWORD dwRequestId) {}// 用户私聊请求消息
    virtual void OnAnyChatPrivateEchoMessage(DWORD dwUserId, DWORD dwErrorCode) {}   // 用户私聊请求回复消息
    virtual void OnAnyChatPrivateExitMessage(DWORD dwUserId, DWORD dwErrorCode) {}   // 用户退出私聊消息
    virtual void OnAnyChatSDKWarningMessage(DWORD dwErrorCode, DWORD dwReserved) {}  // SDK警告消息
    virtual void OnAnyChatNotifyMessageCallBack(DWORD dwNotifyMsg, DWORD wParam, DWORD lParam) {}// 异步消息通知回调函数

    void static CALLBACK NotifyMessage_CallBack(DWORD dwNotifyMsg, DWORD wParam, DWORD lParam, LPVOID lpUserValue);
    void static CALLBACK TextMessage_CallBack(DWORD dwFromUserid, DWORD dwToUserid, BOOL bSecret, LPCTSTR lpMsgBuf, DWORD dwLen, LPVOID lpUserValue);
    void static CALLBACK VideoData_CallBack(DWORD dwUserid, LPVOID lpBuf, DWORD dwLen, BITMAPINFOHEADER bmiHeader, LPVOID lpUserValue);

    void  DrawUserVideo(int dwUserid, LPVOID lpBuf, int dwLen, int bmpWidth,int bmpHeight);
};

#endif // CHATROBOT_H
