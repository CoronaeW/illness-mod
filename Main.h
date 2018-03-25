//---------------------------------------------------------------------------

#ifndef MainH
#define MainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <Menus.hpp>
#include <ToolWin.hpp>

#include <assert.h>
//数字摄像机SDK的HVVideo模块头文件
#include "HVDAILT.h"
#include "Winbase.h"
#include "Raw2Rgb.h"
#include <Buttons.hpp>
#include <Dialogs.hpp>
#include <stdio.h>
#include <time.h>
#include <windows.h>
#include <dos.h>
#include <math.hpp>
#include "Math.h"
#include "MyLib.h"
#include <ExtDlgs.hpp>
//#define  NUM_SMOOTH  5

//定义彩色图像还是黑白图像
typedef enum  tagHV_IMAGE_MODE
{
	HV_COLOR = 0,
		HV_BW = 1
}HV_IMAGE_MODE;

//自定义消息ID号
#define WM_SNAP_CHANGE		(WM_USER + 100)


//---------------------------------------------------------------------------
class TMainFrm : public TForm
{
__published:	// IDE-managed Components
    TMainMenu *MainMenu1;
    TMenuItem *Snap1;
    TMenuItem *Open1;
    TMenuItem *Start1;
    TMenuItem *Stop1;
    TMenuItem *Close1;
    TMenuItem *N1;
    TMenuItem *Negative1;
        TSaveDialog *SaveDialog1;
        TBitBtn *BitBtn1;
        TStatusBar *StatusBar;
        TMenuItem *N2;
        TMenuItem *StartSave1;
        TMenuItem *StopSave1;
        TOpenPictureDialog *OpenPictureDialog1;
        TBitBtn *Btn_BmpProb;
	TEdit *Edit_Time_Delay;
        TLabel *Label1;
        TEdit *Edit_NUM_SMOOTH;
        TLabel *Label2;
	TEdit *Edit_Time_Length;
	TLabel *Label3;
	TEdit *Edit2;
	TLabel *Label4;
	TEdit *Edit3;
	TEdit *Edit1;
	TEdit *Edit4;
	TLabel *Label5;
    void __fastcall Open1Click(TObject *Sender);
    void __fastcall Start1Click(TObject *Sender);
    void __fastcall Stop1Click(TObject *Sender);
    void __fastcall Close1Click(TObject *Sender);
    void __fastcall Negative1Click(TObject *Sender);
    void __fastcall FormDestroy(TObject *Sender);
    void __fastcall FormCreate(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall SetExposureTime(int nWindWidth,long lTintUpper,long lTintLower);
	void __fastcall BitBtn1Click(TObject *Sender);
	void __fastcall FormMouseDown(TObject *Sender, TMouseButton Button,
	  TShiftState Shift, int X, int Y);
	void __fastcall StartSave1Click(TObject *Sender);
	void __fastcall StopSave1Click(TObject *Sender);
	void __fastcall Btn_BmpProbClick(TObject *Sender);
 //	void __fastcall Edit_NUM_SMOOTHChange(TObject *Sender);
private:	// User declarations
public:		// User declarations
    __fastcall TMainFrm(TComponent* Owner);
//    void __fastcall GetBMPFileName(int status); //创建目录获取文件名
    AnsiString   __fastcall ComputerName();
    AnsiString   __fastcall UserName();
    String ServerIPStr;
    String LoacalIPStr;
    String ReceiveText;
    String ServerError;
    SYSTEMTIME ServerTime;
    SYSTEMTIME LocalTime;
    int IntServerTime ;
    int IntLocalTime ;
//    wchar_t * BMPFileName;  //每次保存的文件名
    int PLeft1,PTop1,PRight1,PBottom1;
    int PLeft2,PTop2,PRight2,PBottom2;
	bool CentrFile_Save;
	int Width_min,Width_max,Height_min,Height_max,Height_,Width_;

	int WFileHandle;
	int I_Threshold1,I_Threshold2;
    TDateTime dDate0;
    unsigned short hour0,min0, sec0, msec0;
	int NUM_SMOOTH;
	double Time_Length;
	double TimeDelay;
	double Time_Delay;


private:	// User declarations

  //  LRESULT OnSnapChange(char *filename);
	LRESULT OnSnapChange(TMessage &Msg);
    int nn;

protected:
    BEGIN_MESSAGE_MAP
        MESSAGE_HANDLER(WM_SNAP_CHANGE,TMessage,OnSnapChange)
    END_MESSAGE_MAP(TForm)

private:
	HHV	m_hhv;			//数字摄像机句柄

	BOOL m_bOpen;		//初始化标志
	BOOL m_bStart;		//启动标志
	BOOL m_bStartSaveBmp;		//启动标志

	BITMAPINFO *m_pBmpInfo;		//BITMAPINFO 结构指针，显示图像时使用
	BYTE *m_pRawBuffer;			//采集图像原始数据缓冲区
	BYTE *m_pImageBuffer;		//Bayer转换后缓冲区
        char m_chBmpBuf[2048];		//BIMTAPINFO 存储缓冲区，m_pBmpInfo即指向此缓冲区

	/*
	 *	Snap 回调函数，用户也可以定义为全局函数，
	 *	如果作为类的成员函数，必须为静态成员函数。
	 */
	static int CALLBACK SnapThreadCallback(HV_SNAP_INFO *pInfo);

	BOOL m_bNegative;

	//颜色查找表
	BYTE m_pLutR[256];
	BYTE m_pLutG[256];
	BYTE m_pLutB[256];


}                 ;
#endif
