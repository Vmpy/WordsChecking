#include <windows.h>
#include <windowsx.h> 
#include <fstream>
#include <cstdio>
#include <ctime> 
#include "Data.h" 

WordsChecking::WorkWindowData WinData;

bool IsMenuDisplay = true;

LRESULT CALLBACK WndProc(HWND,UINT,WPARAM,LPARAM);

void WriteFile(void);
void CheckWords(void);

int ReloadWordsFile(const char*,WordsChecking::Word*,int);
void PrepareWords(void); 
 
int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow)
{
    WNDCLASSEX wc;
    MSG msg;

    memset(&wc,0,sizeof(wc));
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL,IDC_ARROW);
    
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszClassName = "WordMemory";
    wc.hIcon = LoadIcon(NULL,IDI_APPLICATION);
    wc.hIconSm = LoadIcon(NULL,IDI_APPLICATION);
    
    if(!RegisterClassEx(&wc))
    {
        MessageBox(NULL, "Window Registration Failed!","Error!",MB_ICONEXCLAMATION|MB_OK);
        return 0;
    }

    WinData.Window = CreateWindowEx(WS_EX_CLIENTEDGE,"WordMemory","",WS_VISIBLE|WS_OVERLAPPEDWINDOW^WS_THICKFRAME^WS_MAXIMIZEBOX,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        640,
        480,
        NULL,NULL,hInstance,NULL);

    if(WinData.Window == NULL)
    {
        MessageBox(NULL,"Window Creation Failed!","Error!",MB_ICONEXCLAMATION|MB_OK);
        return 0;
    }

    while(GetMessage(&msg,NULL,0,0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return msg.wParam;
}

/**
*主回调函数. 
*/
LRESULT CALLBACK WndProc(HWND hwnd,UINT Message,WPARAM wParam,LPARAM lParam)
{
    switch(Message)
    {
        case WM_CREATE:
        {
            WinData.hMainMenu = CreateMenu();
            AppendMenu(WinData.hMainMenu,MF_STRING,ID_BACKMENU,TEXT("返回主菜单"));
            AppendMenu(WinData.hMainMenu,MF_STRING,ID_CLOSEEXE,TEXT("关闭程序"));
            AppendMenu(WinData.hMainMenu,MF_STRING,ID_ABOUT,TEXT("关于"));
            SetMenu(hwnd,WinData.hMainMenu);
            break;
        }
        
        case WM_SIZE:
        {
            if(IsMenuDisplay)
            {
                WinData.Window = hwnd;
                WinData.WindowWidth = GET_X_LPARAM(lParam);
                WinData.WindowHeight = GET_Y_LPARAM(lParam);
                
                WinData.MainMenu();
            }
            break;
        }
        
        case WM_CTLCOLORSTATIC:
        {
            HBRUSH hBrush;
            if((HWND)lParam == WinData.BlockOne_WriteFile)
            {
                hBrush = CreateSolidBrush(WriteFileRGB);
                SetBkColor((HDC)wParam,WriteFileRGB);
            }
            if((HWND)lParam == WinData.BlockTwo_CheckWords)
            {
                hBrush = CreateSolidBrush(CheckFileRGB);
                SetBkColor((HDC)wParam,CheckFileRGB);
            }
            if((HWND)lParam == WinData.CheckWindow.Word)
            {
                hBrush = CreateSolidBrush(RGB(0,255,127));
                SetBkColor((HDC)wParam,RGB(0,255,127));
            }
            if((HWND)lParam == WinData.CheckWindow.ChoiceA)
            {
                hBrush = CreateSolidBrush(RGB(255,255,255));
                SetBkColor((HDC)wParam,RGB(255,255,255));
            }
            if((HWND)lParam == WinData.CheckWindow.ChoiceB)
            {
                hBrush = CreateSolidBrush(RGB(255,255,255));
                SetBkColor((HDC)wParam,RGB(255,255,255));
            }
            if((HWND)lParam == WinData.CheckWindow.ChoiceC)
            {
                hBrush = CreateSolidBrush(RGB(255,255,255));
                SetBkColor((HDC)wParam,RGB(255,255,255));
            }
            if((HWND)lParam == WinData.CheckWindow.ChoiceD)
            {
                hBrush = CreateSolidBrush(RGB(255,255,255));
                SetBkColor((HDC)wParam,RGB(255,255,255));
            }
            return (INT_PTR)hBrush;
        }
        
        case WM_COMMAND:
        {
            if(lParam)  //子窗口控件. 
            {
                if((HWND)lParam == WinData.BlockOne_WriteFile && HIWORD(wParam) == STN_CLICKED)
                {
                    DestroyWindow(WinData.BlockOne_WriteFile);
                    DestroyWindow(WinData.BlockTwo_CheckWords);
                    WinData.BlockOne_WriteFile = WinData.BlockTwo_CheckWords = 0;
                    
                    HDC hdc = GetDC(WinData.BlockOne_WriteFile);
                    SetTextColor(hdc,RGB(255,255,255));
                    ReleaseDC(WinData.BlockOne_WriteFile,hdc); 
                    WriteFile();
                }
                else if((HWND)lParam == WinData.BlockTwo_CheckWords && HIWORD(wParam) == STN_CLICKED)
                {
                    DestroyWindow(WinData.BlockOne_WriteFile);
                    DestroyWindow(WinData.BlockTwo_CheckWords);
                    WinData.BlockOne_WriteFile = WinData.BlockTwo_CheckWords = 0;
                    
                    HDC hdc = GetDC(WinData.BlockTwo_CheckWords);
                    SetTextColor(hdc,RGB(255,255,255));
                    ReleaseDC(WinData.BlockOne_WriteFile,hdc);
                    PrepareWords();
                    if(WinData.CheckWindow.MaxNum)
                    {
                        CheckWords();
                    }
                    else
                    {
                        WinData.MainMenu();
                    }
                }
                else if((HWND)lParam == WinData.WriteWindow.OK)
                {
                    std::ofstream File;
                    WordsChecking::Word TmpWord;
                    SendMessage(WinData.WriteWindow.Word,WM_GETTEXT,(WPARAM)35,(LPARAM)TmpWord.Word); 
                    SendMessage(WinData.WriteWindow.Meaning,WM_GETTEXT,(WPARAM)35,(LPARAM)TmpWord.Meaning); 
                    SendMessage(WinData.WriteWindow.Part_of_speech,WM_GETTEXT,(WPARAM)10,(LPARAM)TmpWord.Part_of_speech);
                    if(std::string(TmpWord.Word) == "" || std::string(TmpWord.Meaning) == "" || std::string(TmpWord.Part_of_speech) == "")
                    {
                        MessageBox(hwnd,"单词输入有误，请重新输入.","提示",MB_OK|MB_ICONERROR);
                        break;
                    } 
                    SendMessage(WinData.WriteWindow.Word,WM_SETTEXT,0,0); 
                    SendMessage(WinData.WriteWindow.Meaning,WM_SETTEXT,0,0); 
                    SendMessage(WinData.WriteWindow.Part_of_speech,WM_SETTEXT,0,0);
                    File.open("SomeTempFile.tmp",std::ios_base::app | std::ios_base::binary);
                    File.write((char*)&TmpWord,sizeof(WordsChecking::Word));
                    File.close();
                    WinData.WriteWindow.Count++;
                    if(MessageBox(hwnd,"是否继续录入单词?","提示",MB_YESNO) == IDNO)
                    {
                        if(WinData.WriteWindow.Count < 4)
                        {
                            MessageBox(hwnd,"已录入的单词词条数量太少,请继续录入.","提示",MB_ICONWARNING);
                        }
                        else
                        {
                            char WorkName[100];
                            GetModuleFileName(0,WorkName,100);
                            char FileName[50] = "NoName";
                            OPENFILENAME ofn;
                            memset(&ofn,0,sizeof(ofn));
                            ofn.lStructSize = sizeof(ofn);
                            ofn.hwndOwner = WinData.Window;
                            ofn.lpstrFile = FileName;
                            ofn.lpstrFile[0] = '\0';
                            ofn.nMaxFile = sizeof(FileName);
                            ofn.lpstrFilter = "WordTable Files(*.wsm)\0*.wsm\0All Files(*.*)\0*.*\0\0";
                            ofn.nFilterIndex = 1;
                            ofn.lpstrFileTitle = NULL;
                            ofn.nMaxFileTitle = 0;
                            ofn.lpstrInitialDir = WorkName;
                            ofn.Flags = OFN_SHOWHELP | OFN_OVERWRITEPROMPT;
                            
                            if(GetSaveFileName(&ofn))
                            {
                                std::string FileNameS = std::string(FileName) + std::string(".wsm");
                                rename("SomeTempFile.tmp",FileNameS.c_str());
                                SetWindowText(WinData.Window,"单词记忆");
                            }
                            else
                            {
                                remove("SomeTempFile.tmp");
                            }
                            
                            //设置标志位，返回主菜单. 
                            IsMenuDisplay = true;
                            //摧毁输入控件 
                            WinData.WriteWindow.Count = 0;
                            WinData.WriteWindow.DestoryWindows();
                            WinData.MainMenu(); //再次显示菜单. 
                        }
                    }
                }
                else if((HWND)lParam == WinData.CheckWindow.ChoiceOK)
                {
                    int iValue;
                    switch(WinData.CheckWindow.NowIndex)
                    {
                        case 1:iValue = (int)SendMessage(WinData.CheckWindow.ChoiceA,BM_GETCHECK,0,0);break;
                        case 2:iValue = (int)SendMessage(WinData.CheckWindow.ChoiceB,BM_GETCHECK,0,0);break;
                        case 3:iValue = (int)SendMessage(WinData.CheckWindow.ChoiceC,BM_GETCHECK,0,0);break;
                        case 4:iValue = (int)SendMessage(WinData.CheckWindow.ChoiceD,BM_GETCHECK,0,0);break;
                    }
                    if(iValue)
                    {
                        MessageBox(hwnd,"选择正确!","提示",MB_OK|MB_ICONINFORMATION);
                        WinData.CheckWindow.AnalyseData.RightNum++;
                    }
                    else
                    {
                        std::string Message = std::string("选择错误!\n正确答案:") + std::string(WinData.CheckWindow.WordTable[WinData.CheckWindow.RightAnswerIndex].Meaning);
                        MessageBox(hwnd,Message.c_str(),"提示",MB_OK|MB_ICONWARNING);
                        WinData.CheckWindow.AnalyseData.WrongNum++;
                    }
                    
                    WinData.CheckWindow.DestroyWindows();
                    
                    if(WinData.CheckWindow.IsDone())
                    {
                        WinData.CheckWindow.AnalyseData.Accuracy = 100*((float)WinData.CheckWindow.AnalyseData.RightNum/WinData.CheckWindow.AnalyseData.All);
                        std::string Message = std::string("测试完毕!\n") + std::string("正确率:") + std::to_string(WinData.CheckWindow.AnalyseData.Accuracy) + std::string("%");
                        MessageBox(hwnd,Message.c_str(),"提示",MB_OK|MB_ICONINFORMATION);
                        delete [] WinData.CheckWindow.WordTable;    //清理内存空间. 
                        WinData.CheckWindow.WordTable = nullptr;
                        WinData.CheckWindow.AnalyseData.Destorys();
                        WinData.MainMenu();
                    }
                    else
                    {
                        CheckWords();
                    }
                }
            }
            else    //lParam为0时，处理菜单. 
            {
                switch(LOWORD(wParam))
                {
                    case ID_CLOSEEXE:DestroyWindow(hwnd);break;
                    case ID_BACKMENU:
                    {
                        WinData.WriteWindow.DestoryWindows();
                        WinData.CheckWindow.DestroyWindows();
                        WinData.MainMenu();
                        break;
                    }
                    case ID_ABOUT:
                    {
                        MessageBox(WinData.Window,"  单词记忆程序,用于写入单词文件和检测记忆.\n\t\t\t----By Vmpy","About",MB_OK|MB_ICONINFORMATION);
                        break; 
                    } 
                }
            }
            break;
        }
        
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            break;
        }

        default:
            return DefWindowProc(hwnd,Message,wParam,lParam);
    }
    return 0;
}

/**
*写入单词界面绘制. 
*/
void WriteFile(void)
{
    //设置窗口标题 
    SetWindowText(WinData.Window,"单词录入"); 
    WinData.WriteWindow.WordIn = CreateWindowEx(0,"STATIC","单词:",WS_VISIBLE|WS_CHILD|SS_LEFT,0,0,WinData.TextData.tmAveCharWidth*6,WinData.TextData.tmHeight+5,WinData.Window,(HMENU)WinData.WriteWindow.WordInId,0,0);
    WinData.WriteWindow.MeaningIn = CreateWindowEx(0,"STATIC","单词意义:",WS_VISIBLE|WS_CHILD|SS_LEFT,0,(WinData.TextData.tmHeight+5)*1,WinData.TextData.tmAveCharWidth*10,WinData.TextData.tmHeight+5,WinData.Window,(HMENU)WinData.WriteWindow.MeaningInId,0,0);
    WinData.WriteWindow.SpeechIn = CreateWindowEx(0,"STATIC","单词词性:",WS_VISIBLE|WS_CHILD|SS_LEFT,0,(WinData.TextData.tmHeight+5)*2,WinData.TextData.tmAveCharWidth*10,WinData.TextData.tmHeight+5,WinData.Window,(HMENU)WinData.WriteWindow.SpeechInId,0,0);
    //隐藏主菜单 
    IsMenuDisplay = false;
    WinData.WriteWindow.Word = CreateWindowEx(0,"EDIT","",WS_VISIBLE|WS_CHILD|WS_BORDER|ES_MULTILINE|ES_LEFT,WinData.TextData.tmAveCharWidth*6,0,WinData.TextData.tmAveCharWidth*20,WinData.TextData.tmHeight+5,WinData.Window,(HMENU)WinData.WriteWindow.WordId,0,0);
    WinData.WriteWindow.Meaning = CreateWindowEx(0,"EDIT","",WS_VISIBLE|WS_CHILD|WS_BORDER|ES_MULTILINE|ES_LEFT,WinData.TextData.tmAveCharWidth*10,(WinData.TextData.tmHeight+5)*1,WinData.TextData.tmAveCharWidth*20,WinData.TextData.tmHeight+5,WinData.Window,(HMENU)WinData.WriteWindow.MeaningId,0,0);
    WinData.WriteWindow.Part_of_speech = CreateWindowEx(0,"EDIT","",WS_VISIBLE|WS_CHILD|WS_BORDER|ES_MULTILINE|ES_LEFT,WinData.TextData.tmAveCharWidth*10,(WinData.TextData.tmHeight+5)*2,WinData.TextData.tmAveCharWidth*20,WinData.TextData.tmHeight+5,WinData.Window,(HMENU)WinData.WriteWindow.Part_of_speechId,0,0);
    WinData.WriteWindow.OK = CreateWindowEx(0,"BUTTON","录入",WS_VISIBLE|WS_CHILD|BS_PUSHBUTTON,50,(WinData.TextData.tmHeight+5)*3,WinData.TextData.tmAveCharWidth*7,WinData.TextData.tmHeight+8,WinData.Window,(HMENU)WinData.WriteWindow.OKId,0,0);
    
    SendMessage(WinData.WriteWindow.WordIn,WM_SETFONT,(WPARAM)WinData.hFont,0);
    SendMessage(WinData.WriteWindow.MeaningIn,WM_SETFONT,(WPARAM)WinData.hFont,0);
    SendMessage(WinData.WriteWindow.SpeechIn,WM_SETFONT,(WPARAM)WinData.hFont,0);
    SendMessage(WinData.WriteWindow.Word,WM_SETFONT,(WPARAM)WinData.hFont,0);
    SendMessage(WinData.WriteWindow.Meaning,WM_SETFONT,(WPARAM)WinData.hFont,0);
    SendMessage(WinData.WriteWindow.Part_of_speech,WM_SETFONT,(WPARAM)WinData.hFont,0);
    SendMessage(WinData.WriteWindow.OK,WM_SETFONT,(WPARAM)WinData.hFont,0);
}

/**
*装填准备WinData.CheckWindow.XXX信息的函数. 
*/
void PrepareWords(void)
{
    char FileName[50];
    OPENFILENAME ofn;
    char WorkName[100];
    std::fstream File;
    GetModuleFileName(0,WorkName,100);
    
    memset(&ofn,0,sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = WinData.Window;
    ofn.lpstrFile = FileName;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(FileName);
    ofn.lpstrFilter = "WordTable Files(*.wsm)\0*.wsm\0All Files(*.*)\0*.*\0\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = WorkName;
    ofn.Flags = OFN_OVERWRITEPROMPT;
    
    if(GetOpenFileName(&ofn))
    {
        File.open(FileName,std::ios_base::binary);
    }
    else
    {   
        //打开失败后的处理 
        WinData.CheckWindow.MaxNum = 0;
        WinData.CheckWindow.WordTable = nullptr;
        return;
    }

    WinData.CheckWindow.MaxNum = WinData.CheckWindow.GetWordNum(FileName);
    WinData.CheckWindow.WordTable = new WordsChecking::Word[WinData.CheckWindow.MaxNum];
    ReloadWordsFile(FileName,WinData.CheckWindow.WordTable,WinData.CheckWindow.MaxNum);
    WinData.CheckWindow.Tested = new bool[WinData.CheckWindow.MaxNum];
    memset(WinData.CheckWindow.Tested,false,WinData.CheckWindow.MaxNum);
    return;
}

/**
*装填内存空间，有指针指向. 
*/
int ReloadWordsFile(const char* FileName,WordsChecking::Word* BeLoaded,int size)
{
    std::ifstream File;
    File.open(FileName,std::ios_base::binary);
    for(int i = 0 ;i < size;i++)
    {
        File.read((char*)(BeLoaded+i),sizeof(WordsChecking::Word));
    }
    File.close();
    return 0;
}

/**
*测试单词界面绘制. 
*/
void CheckWords(void)
{
    srand(time(0));
    WinData.CheckWindow.Word = CreateWindowEx(0,"STATIC","单词",WS_VISIBLE|WS_CHILD|SS_CENTER|SS_CENTERIMAGE,0,0,0,0,WinData.Window,(HMENU)WinData.CheckWindow.WordId,0,0);
    WinData.CheckWindow.Choice = CreateWindowEx(0,"BUTTON","选项",WS_VISIBLE|WS_CHILD|BS_GROUPBOX,0,0,0,0,WinData.Window,(HMENU)WinData.CheckWindow.ChoiceId,0,0);
    WinData.CheckWindow.ChoiceA = CreateWindowEx(0,"BUTTON","选项A",WS_VISIBLE|WS_CHILD|BS_AUTORADIOBUTTON,0,0,0,0,WinData.CheckWindow.Choice,(HMENU)WinData.CheckWindow.ChoiceAId,0,0);
    WinData.CheckWindow.ChoiceB = CreateWindowEx(0,"BUTTON","选项B",WS_VISIBLE|WS_CHILD|BS_AUTORADIOBUTTON,0,0,0,0,WinData.CheckWindow.Choice,(HMENU)WinData.CheckWindow.ChoiceBId,0,0);
    WinData.CheckWindow.ChoiceC = CreateWindowEx(0,"BUTTON","选项C",WS_VISIBLE|WS_CHILD|BS_AUTORADIOBUTTON,0,0,0,0,WinData.CheckWindow.Choice,(HMENU)WinData.CheckWindow.ChoiceCId,0,0);
    WinData.CheckWindow.ChoiceD = CreateWindowEx(0,"BUTTON","选项D",WS_VISIBLE|WS_CHILD|BS_AUTORADIOBUTTON,0,0,0,0,WinData.CheckWindow.Choice,(HMENU)WinData.CheckWindow.ChoiceDId,0,0);
    WinData.CheckWindow.ChoiceOK = CreateWindowEx(0,"BUTTON","确认选项",WS_VISIBLE|WS_CHILD|BS_PUSHBUTTON,0,0,0,0,WinData.Window,(HMENU)WinData.CheckWindow.ChoiceDId,0,0);
    WinData.CheckWindow.Speech = CreateWindowEx(0,"STATIC","词性",WS_VISIBLE|WS_CHILD|SS_CENTER|SS_CENTERIMAGE,0,0,0,0,WinData.Window,(HMENU)WinData.CheckWindow.SpeechId,0,0);
    
    SendMessage(WinData.CheckWindow.Word,WM_SETFONT,(WPARAM)WinData.hFont,0);
    SendMessage(WinData.CheckWindow.Speech,WM_SETFONT,(WPARAM)WinData.hFont,0);
    SendMessage(WinData.CheckWindow.Choice,WM_SETFONT,(WPARAM)WinData.ShFont,0);
    SendMessage(WinData.CheckWindow.ChoiceA,WM_SETFONT,(WPARAM)WinData.ShFont,0);
    SendMessage(WinData.CheckWindow.ChoiceB,WM_SETFONT,(WPARAM)WinData.ShFont,0);
    SendMessage(WinData.CheckWindow.ChoiceC,WM_SETFONT,(WPARAM)WinData.ShFont,0);
    SendMessage(WinData.CheckWindow.ChoiceD,WM_SETFONT,(WPARAM)WinData.ShFont,0);
    SendMessage(WinData.CheckWindow.ChoiceOK,WM_SETFONT,(WPARAM)WinData.hFont,0);
    
    //隐藏主菜单 
    IsMenuDisplay = false;
    
    HWND Tmp;
    int index;
    
    do
    {
        index = rand()%WinData.CheckWindow.MaxNum; 
    }while(WinData.CheckWindow.Tested[index]);

    WinData.CheckWindow.RightAnswerIndex = index;
    WinData.CheckWindow.Tested[index] = true;
    
    SetWindowText(WinData.CheckWindow.Word,WinData.CheckWindow.WordTable[index].Word);
    MoveWindow(WinData.CheckWindow.Word,WinData.WindowWidth/2-(WinData.TextData.tmAveCharWidth*lstrlen(WinData.CheckWindow.WordTable[index].Word)/2),WinData.TextData.tmHeight,WinData.TextData.tmAveCharWidth*(2+lstrlen(WinData.CheckWindow.WordTable[index].Word)),WinData.TextData.tmHeight+15,true);
    SetWindowText(WinData.CheckWindow.Speech,WinData.CheckWindow.WordTable[index].Part_of_speech);
    MoveWindow(WinData.CheckWindow.Speech,WinData.WindowWidth/2-(WinData.TextData.tmAveCharWidth*lstrlen(WinData.CheckWindow.WordTable[index].Word)/2)+WinData.TextData.tmAveCharWidth*(2+lstrlen(WinData.CheckWindow.WordTable[index].Word))+1,WinData.TextData.tmHeight,WinData.TextData.tmHeight/2+WinData.TextData.tmAveCharWidth*(2+lstrlen(WinData.CheckWindow.WordTable[index].Part_of_speech)),WinData.TextData.tmHeight+15,true);
    
    int RightAnswerIndex = WinData.CheckWindow.NowIndex = 1+rand()%4;
    
    WinData.CheckWindow.AnalyseData.All = WinData.CheckWindow.MaxNum;
        
    for(int i = 1;i < 5;i++)
    {
        int Fourofindex;//四个选项的index变量 
        if(RightAnswerIndex == i)
        {
            //为正确的选项装填字符串 
            switch(i)
            {
                case 1:Tmp = WinData.CheckWindow.ChoiceA;SetWindowText(WinData.CheckWindow.ChoiceA,WinData.CheckWindow.WordTable[index].Meaning);break;
                case 2:Tmp = WinData.CheckWindow.ChoiceB;SetWindowText(WinData.CheckWindow.ChoiceB,WinData.CheckWindow.WordTable[index].Meaning);break;
                case 3:Tmp = WinData.CheckWindow.ChoiceC;SetWindowText(WinData.CheckWindow.ChoiceC,WinData.CheckWindow.WordTable[index].Meaning);break;
                case 4:Tmp = WinData.CheckWindow.ChoiceD;SetWindowText(WinData.CheckWindow.ChoiceD,WinData.CheckWindow.WordTable[index].Meaning);break;
            }
            MoveWindow(Tmp,0,i*(WinData.TextData.tmHeight/2),WinData.WindowWidth,WinData.TextData.tmHeight/2,true);
            continue;
        }
            
        do{
            Fourofindex = rand()%WinData.CheckWindow.MaxNum;
        }while(std::string(WinData.CheckWindow.WordTable[Fourofindex].Word) == std::string(WinData.CheckWindow.WordTable[index].Word) || WinData.CheckWindow.Displayed[0] == Fourofindex ||WinData.CheckWindow.Displayed[1] == Fourofindex ||WinData.CheckWindow.Displayed[2] == Fourofindex ||WinData.CheckWindow.Displayed[3] == Fourofindex);
        
        WinData.CheckWindow.Displayed[i-1] = Fourofindex;
        switch(i)
        {
            case 1:Tmp = WinData.CheckWindow.ChoiceA;SetWindowText(WinData.CheckWindow.ChoiceA,WinData.CheckWindow.WordTable[Fourofindex].Meaning);break;
            case 2:Tmp = WinData.CheckWindow.ChoiceB;SetWindowText(WinData.CheckWindow.ChoiceB,WinData.CheckWindow.WordTable[Fourofindex].Meaning);break;
            case 3:Tmp = WinData.CheckWindow.ChoiceC;SetWindowText(WinData.CheckWindow.ChoiceC,WinData.CheckWindow.WordTable[Fourofindex].Meaning);break;
            case 4:Tmp = WinData.CheckWindow.ChoiceD;SetWindowText(WinData.CheckWindow.ChoiceD,WinData.CheckWindow.WordTable[Fourofindex].Meaning);break;
        }
        MoveWindow(Tmp,0,i*(WinData.TextData.tmHeight/2),WinData.WindowWidth,WinData.TextData.tmHeight/2,true);   
    }
    
    std::string Caption = std::string("单词记忆 ") + std::string("单词总数:") + std::to_string(WinData.CheckWindow.AnalyseData.All) + std::string(" 剩余:") + std::to_string(WinData.CheckWindow.AnalyseData.All-(WinData.CheckWindow.AnalyseData.RightNum+WinData.CheckWindow.AnalyseData.WrongNum));
    SetWindowText(WinData.Window,Caption.c_str());
    
    //复位显示位. 
    WinData.CheckWindow.Displayed[0] = -1;
    WinData.CheckWindow.Displayed[1] = -1;
    WinData.CheckWindow.Displayed[2] = -1;
    WinData.CheckWindow.Displayed[3] = -1;
    MoveWindow(WinData.CheckWindow.ChoiceOK,0,WinData.WindowHeight-WinData.TextData.tmHeight*4/2,WinData.WindowWidth,WinData.TextData.tmHeight+20,true);
    MoveWindow(WinData.CheckWindow.Choice,0,WinData.WindowHeight/2,WinData.WindowWidth,4*(WinData.TextData.tmHeight/1.5),true);
    return;
}
