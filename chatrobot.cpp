#include "chatrobot.h"

chatrobot::chatrobot()
{

}

//字符编码转换
int chatrobot::code_convert(const char *from_charset,const char *to_charset,char *inbuf,size_t inlen,char *outbuf,size_t outlen)
{
   iconv_t cd;
   char **pin = &inbuf;
   char **pout = &outbuf;
   cd = iconv_open(to_charset,from_charset);
   if (cd==0) return -1;
   memset(outbuf,0,outlen);
   if ((int)iconv(cd,pin,&inlen,pout,&outlen)==-1)
   {
       iconv_close(cd);
       return -1;
   }
   iconv_close(cd);
   return 0;
}
//初始化通信环境
void chatrobot::chatInit()
{
    //获取SDK的版本信息,显示版本信息。
    DWORD dwMainVer,dwSubVer;
    char szCompileTime[100] = {0};
    BRAC_GetSDKVersion(dwMainVer,dwSubVer,szCompileTime,sizeof(szCompileTime));
    QString logstr;
    logstr.sprintf("#INFO# AnyChat Core SDK Version:%d.%d(%s)",dwMainVer,dwSubVer,szCompileTime);
    std::cout<<logstr.toStdString()<<std::endl;

    //打开SDK的日志记录功能
    BRAC_ActiveCallLog(true);

    //设置SDK核心组件所在目录（注：demo程序只是设置为当前目录，项目中需要设置为实际路径）
    QString szCoreSDKPath;
    szCoreSDKPath = QCoreApplication::applicationDirPath();   //获取当前应用程序路径
    (strrchr((char*)szCoreSDKPath.toStdString().c_str(),'/'))[1] = 0;
    BRAC_SetSDKOption(BRAC_SO_CORESDK_PATH,
                                (char*)szCoreSDKPath.toStdString().c_str(),
                                strlen((char*)szCoreSDKPath.toStdString().c_str()));


    // 初始化SDK，根据BRAC_InitSDK的第二个参数：dwFuncMode，来告诉SDK该如何处理相关的任务（详情请参考开发文档）
    DWORD dwFuncMode = BRAC_FUNC_VIDEO_CBDATA/*BRAC_FUNC_VIDEO_AUTODISP | BRAC_FUNC_AUDIO_AUTOPLAY */| BRAC_FUNC_CHKDEPENDMODULE |
                       /*BRAC_FUNC_AUDIO_VOLUMECALC | */BRAC_FUNC_NET_SUPPORTUPNP | BRAC_FUNC_FIREWALL_OPEN |
                       /*BRAC_FUNC_AUDIO_AUTOVOLUME | BRAC_FUNC_AUDIO_VOLUMECALC | */BRAC_FUNC_CONFIG_LOCALINI;
    BRAC_InitSDK(this, dwFuncMode);//初始化SDK

    //设置外部视频输入
    if(EnableInputvideoEx){
        std::cout<<"##DEBUG INFO## set input video data from external"<<std::endl;
        BOOL bExtVideoInput = 1;
        BRAC_SetSDKOption(BRAC_SO_CORESDK_EXTVIDEOINPUT,(CHAR*)&bExtVideoInput,sizeof(DWORD));
    }
    //设置回调函数
    BRAC_SetVideoDataCallBack(BRAC_PIX_FMT_RGB24,VideoData_CallBack, this);//设置视频数据回调
    BRAC_SetNotifyMessageCallBack(NotifyMessage_CallBack,this);            //设置异步接收消息回调
    BRAC_SetTextMessageCallBack(TextMessage_CallBack,this);                //设置消息发送回调
    // 设置服务器认证密码，确保 SDK 能正常连接到服务器，为了防止未授权SDK连接服务器
    QString pwd = "BaiRuiTech";
   BRAC_SetServerAuthPass((LPCTSTR)pwd.toStdString().c_str());
}
//设置登录信息
void chatrobot::Set_chatLogin_Info(QString serverIP, int serverPort, QString userName,
                              QString userPwd, QString userappid, int roomID, QString roomPwd){
    ServerIp=serverIP;
    ServerPort=serverPort;
    UserName=userName;
    UserPwd=userPwd;
    Userappid=userappid;
    RoomID=roomID;
    RoomPwd=roomPwd;
}
//设置外部视频输入格式
void chatrobot::SetInputVideoFormat(int videoWidth,int videoHight,int fps){
    VideoWidth=videoWidth;
    VideoHight=videoHight;
    FPS=fps;
}
//设置消息接收回调函数
void chatrobot::SetMsgFucRec(void(*msgR)(QString msg,int len)){
    chatMsgReceive=msgR;
}
//使能外部视频数据输入函数
void chatrobot::enableInputFromEX(bool inputvideoEx){
    EnableInputvideoEx=inputvideoEx;
}
//登入
void chatrobot::chatLogin()
{
    BRAC_SetSDKOption(BRAC_SO_CLOUD_APPGUID,Userappid.toStdString().c_str(),sizeof(Userappid.toStdString().c_str()));//设置appid
    BRAC_Connect(ServerIp.toStdString().c_str(),ServerPort);  //连接服务器 :connect to server
}
//离开房间
void chatrobot::chatLeaveRoom(){
    if(!EnableInputvideoEx){
        //关闭本地用户视频
        BRAC_UserCameraControl(myselfID,0);
    }
    //离开当前房间
    BRAC_LeaveRoom(-1);
}
//登离
void chatrobot::chatLogoutServer(){
    std::cout<<"#DEBUG INFO#logout server:"<<std::endl;
    BRAC_Logout();
}
//外部视频数据输入
void chatrobot::chatInputVideo(uchar *data, int dataSize, int timeStamp){
    int ret=BRAC_InputVideoData(data,dataSize,timeStamp);
    if(ret){
        std::cout<<"#DEBUG INFO#input video data fail,error code:"<<ret<<std::endl;
    }

}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
// 异步消息通知回调函数定义
void CALLBACK chatrobot::NotifyMessage_CallBack(DWORD dwNotifyMsg, DWORD wParam, DWORD lParam, LPVOID lpUserValue)
{
        chatrobot*	rb = (chatrobot*)lpUserValue;
        if(!rb)
                return;
        switch(dwNotifyMsg)
        {
        case WM_GV_CONNECT:
            rb->OnGVClientConnect(wParam,lParam);
            break;
        case WM_GV_LOGINSYSTEM:
            rb->OnGVClientLogin(wParam,lParam);            
            break;
        case WM_GV_ENTERROOM:
            rb->OnGVClientEnterRoom(wParam,lParam);          
            break;
        case WM_GV_MICSTATECHANGE:
            rb->OnGVClientMicStateChange(wParam,lParam);           
            break;
        case WM_GV_USERATROOM:
            rb->OnGVClientUserAtRoom(wParam,lParam);            
            break;
        case WM_GV_LINKCLOSE:
            rb->OnGVClientLinkClose(wParam, lParam);           
            break;
        case WM_GV_ONLINEUSER:
            rb->OnGVClientOnlineUser(wParam,lParam);           
            break;
        case WM_GV_CAMERASTATE:
            rb->OnAnyChatCameraStateChgMessage(wParam,lParam);           
            break;
        case WM_GV_ACTIVESTATE:
            rb->OnAnyChatActiveStateChgMessage(wParam,lParam);           
            break;
        case WM_GV_P2PCONNECTSTATE:
            rb->OnAnyChatP2PConnectStateMessage(wParam,lParam);           
            break;
        case WM_GV_SDKWARNING:
            rb->OnAnyChatSDKWarningMessage(wParam,lParam);            
            break;

        default:
                break;
        }
}

// 收到消息：客户端连接服务器 wParam（BOOL）表示是否连接成功
long chatrobot::OnGVClientConnect(WPARAM wParam, LPARAM lParam)
{
    int len=UserName.toStdString().length();
    char gbk_username[30]={0};
    bool bSuccess = (bool)wParam;
    if(bSuccess){
        code_convert("utf-8","gb2312",(char*)UserName.toStdString().c_str(),len,gbk_username,len);
        BRAC_Login(gbk_username, UserPwd.toStdString().c_str(), 0);  //登陆服务器 :loging to server
    }
    QString logstr(bSuccess ? "#INFO# Connect to server OK----" : "#INFO# Connect to Server error");
    std::cout<<logstr.toStdString().c_str()<<std::endl;
    return 0;
}

// 收到消息：客户端登录系统 wParam （INT）表示自己的用户ID号, lParam （INT）表示登录结果：0 成功，否则为出错代码，参考出错代码定义
long chatrobot::OnGVClientLogin(WPARAM wParam, LPARAM lParam)
{
    DWORD dwUserID = wParam;
    if(lParam == 0)
    {
        myselfID= dwUserID;
        BRAC_EnterRoom(RoomID, (LPCTSTR)RoomPwd.toStdString().c_str() , 0); //进入房间
    }
    return 0;
}

// 收到消息：客户端进入房间  wParam （INT）表示所进入房间的ID号,进入房间后打开本地视频和音频(使用外部音视频时不打开本地音视频)。
// lParam （INT）表示是否进入房间：0成功进入，否则为出错代码
long chatrobot::OnGVClientEnterRoom(WPARAM wParam, LPARAM lParam)
{
        QString logstr;
        DWORD state;
        RoomID = (int)wParam;
        if(lParam == 0) //自己成功进入房间，然后打开视频和音频
        {
                logstr.sprintf("#INFO# success enter room：%d,user ",RoomID);
                if(EnableInputvideoEx){
                    if(BRAC_SetInputVideoFormat(BRAC_PIX_FMT_RGB24,VideoWidth,VideoHight,FPS,0))
                        std::cout<<"failed set input video format..."<<std::endl;
                }
                else{
                    state=BRAC_UserCameraControl(-1,TRUE);//本例程使用的是外部视频
                    std::cout<<"#DEBUG INFO#camera opened :"<<state<<std::endl;
                    TCHAR *p=NULL;
                    DWORD Num;
                    int ret=BRAC_EnumVideoCapture(&p, Num);
                    std::cout<<"#DEBUG INFO#local video dev NUM:"<<Num<<" dev:"<<p<<std::endl;
                    ret=BRAC_SelectVideoCapture(p);
                    std::cout<<"#DEBUG INFO # SELECT VIDEO STATE:"<<ret<<std::endl;
                    BRAC_UserCameraControl(-1,TRUE);
                }
        }else
        {
                logstr.sprintf("#INFO# can not enter room，error code： %d ",lParam);
        }
        std::cout<<logstr.toStdString().c_str()<<std::endl;
        return 0;
}

// 收到消息：收到当前房间的在线用户信息 wParam （INT）表示在线用户数（不包含自己）,
//                       lParam  （INT）表示房间ID
long chatrobot::OnGVClientOnlineUser(WPARAM wParam, LPARAM lParam)
 {
     QString logstr;
     int onlinenum = (int)wParam;
     logstr.sprintf("#INFO# the room id：%d\n#INFO# total %d user online",lParam, onlinenum);
     std::cout<<logstr.toStdString().c_str()<<std::endl;
     return 0;
 }

// 收到消息：用户的音频设备状态变化消息  wParam （INT）表示用户ID号 lParam （BOOL）表示该用户是否已打开音频采集设备
long chatrobot::OnGVClientMicStateChange(WPARAM wParam, LPARAM lParam)
{
    QString logstr;
    logstr.sprintf("#INFO# user id：%d ", wParam);
    logstr.append( lParam ? "open":"close");
    logstr.append(" Local Mic Device");
    std::cout<<logstr.toStdString().c_str()<<std::endl;
    return 0;
}

// 收到消息：用户进入（离开）房间 wParam （INT）表示用户ID号,
//                           lParam （BOOL）表示该用户是进入（TRUE）或离开（FALSE）房间
long chatrobot::OnGVClientUserAtRoom(WPARAM wParam, LPARAM lParam)
{
    QString logstr;
     int userid = (int)wParam;
     bool bEnter = (bool)lParam;

     char username[30]={0};
     BRAC_GetUserName(userid,username,sizeof(username));
     char str_username[30];
     code_convert("gb2312","utf-8",username,30,str_username,30);// gbk to utf-8
     logstr.sprintf("#INFO# User id：%d ,User name(%s) ",userid,str_username);
     logstr.append(bEnter ? "Enter" : "Leave");
     logstr.append("Room");
     std::cout<<logstr.toStdString().c_str()<<std::endl;
      return 0;
}

//收到消息：网络连接已关闭    (该消息只有在客户端连接服务器成功之后，网络异常中断之时触发)
long chatrobot::OnGVClientLinkClose(WPARAM wParam, LPARAM lParam)
{
    DWORD errorid=wParam;
    QString logstr;
    switch (errorid) {
    case 0:
        logstr.sprintf("#DEBUG INFO#network disconected");
        break;
     case 1:
        logstr.sprintf("#DEBUG INFO#login another computer");
        break;
     case 2:
        logstr.sprintf("#DEBUG INFO#The server function is restricted");
        break;
     case 3:
        logstr.sprintf("#DEBUG INFO#Received a connection from another network");
        break;
    case 4:
        logstr.sprintf("#DEBUG INFO#SDK version is too old");
       break;
    case 5:
        logstr.sprintf("#DEBUG INFO#Connection server authentication failed");
       break;
    case 6:
        logstr.sprintf("#DEBUG INFO#Network connection timed out");
       break;
    case 7:
        logstr.sprintf("#DEBUG INFO#The server does not support embedded device");
       break;
    default:
        break;
    }
    std::cout<<logstr.toStdString().c_str()<<std::endl;
    return 0;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
//视频数据显示--Debug
void chatrobot::DrawUserVideo(int dwUserid, LPVOID lpBuf, int dwLen, int bmpWidth,int bmpHeight)
{
    if(dwLen <= 0 || lpBuf == NULL)
        return;

    //判断用户id选择不同的显示区域
    if(myselfID == dwUserid) //本地用户视频
    {
        Mat frame(bmpHeight,bmpWidth,CV_8UC3,(uchar*)lpBuf,Mat::AUTO_STEP);
        imshow("localvideo",frame);
    }
    else  //远程用户视频
    {
      std::cout<<"#DEBUG INFO#remote VideoFrame ...."<<std::endl;
    }
}

//这里调用本地定时器触发去执行刷新----no end
// video data callback
void CALLBACK chatrobot::VideoData_CallBack(DWORD dwUserid, LPVOID lpBuf, DWORD dwLen, BITMAPINFOHEADER bmiHeader, LPVOID lpUserValue)
{
    chatrobot * rb=(chatrobot*)lpUserValue;
    rb->DrawUserVideo(dwUserid,lpBuf,dwLen,bmiHeader.biWidth,bmiHeader.biHeight);
}

// 文字消息回调函数定义--no end
void CALLBACK chatrobot::TextMessage_CallBack(DWORD dwFromUserid, DWORD dwToUserid, BOOL bSecret, LPCTSTR lpMsgBuf, DWORD dwLen, LPVOID lpUserValue)
{
    chatrobot * rb=(chatrobot*)lpUserValue;
    char str_msg[dwLen];
    QString message;
    rb->code_convert("gb2312","utf-8",(char*)lpMsgBuf,dwLen,str_msg,dwLen);//msg gbk to utf8
    if(rb->chatMsgReceive){
        message.sprintf("#INFO# user(%d)-->user(%d):%s\n", dwFromUserid,dwToUserid,str_msg);
        std::cout<<message.toStdString()<<std::endl;
    }else{
         (*(rb->chatMsgReceive))(message,dwLen);
    }
}
void chatrobot::chatMsgSend(char *msg, int msglen){
    BRAC_SendTextMessage(-1,0,(const char *)msg,msglen);
}



