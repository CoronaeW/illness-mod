//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "Main.h"

//�����Լ���0
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define  MY_ZERO 0.000000001
#define My_Gray 1.0/1000**(m_pImageBuffer+ii*Bmp->Width*3+jj*3)*(*(m_pImageBuffer+(Height_max-ii-1)*Bmp->Width*3+jj*3)*299* *(m_pImageBuffer+(Height_max-ii-1)*Bmp->Width*3+jj*3+1)*587* *(m_pImageBuffer+(Height_max-ii-1)*Bmp->Width*3+jj*3+2)*114)
//const
const int DeviceNum = 1;
const HV_RESOLUTION Resolution = RES_MODE0;
const HV_SNAP_MODE SnapMode = CONTINUATION;
const HV_BAYER_LAYOUT Layout = BAYER_GR;
const HV_BAYER_CONVERT_TYPE ConvertType = BAYER2RGB_NEIGHBOUR;
const long Gain = 5;
const long ExposureTint_Upper = 20;
const long ExposureTint_Lower = 1000;

const long ShutterDelay = 0;
const long ADCLevel = ADC_LEVEL2;
const int XStart = 0;
const int YStart = 0;
const int HV_Width = 800;
const int HV_Height = 600;

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TMainFrm *MainFrm;
//---------------------------------------------------------------------------
__fastcall TMainFrm::TMainFrm(TComponent* Owner)
    : TForm(Owner)
{
	/*
	 *	��ʼ�����г�Ա������ͬʱ�����������
	 */

	HVSTATUS status = STATUS_OK;

	m_bOpen			= FALSE;
	m_bStart		= FALSE;

	m_bNegative		= FALSE;

	m_pBmpInfo		= NULL;
	m_pRawBuffer	= NULL;
	m_pImageBuffer	= NULL;

	for(int i=0;i<256;i++)
	{
		m_pLutR[i] = i;
		m_pLutG[i] = i;
		m_pLutB[i] = i;

	}

	//	����������� 1
	status = BeginHVDevice(1, &m_hhv);
	//	���麯��ִ��״̬�����ʧ�ܣ��򷵻ش���״̬��Ϣ��
	HV_VERIFY(status);

        //char *p = (char *)268472420;
        //long temp = (long)p;
        //���ò˵���״̬
        Open1->Enabled = true;
        Start1->Enabled = false;
        Stop1->Enabled = false;
        Close1->Enabled = false;
        Negative1->Checked = m_bNegative;
    //    BMPFileName=new wchar_t[100];
    //---------------------------------��ʾĿ¼���ǲ�����Ŀ¼
	AnsiString AppPath = ExtractFilePath(Application->ExeName);
	AnsiString dirPath=AppPath+Now().FormatString("yyyymmdd")+"\\";
    MainFrm->Caption="CCD1303UM::"+dirPath;
//    AppPath.WideChar(BMPFileName,dirPath.WideCharBufSize());

}
//---------------------------------------------------------------------------
void __fastcall TMainFrm::FormClose(TObject *Sender, TCloseAction &Action)
{
	/*
	 *	�û���û��ͨ���˵��������ر�����������ɼ���
	 *	��ֱ�ӹر�Ӧ�ó���ʱ��Ӧ��֤����������ɼ����ر�
	 */
	if (m_bOpen)
        {
		HVCloseSnap(m_hhv);
	}

}
//---------------------------------------------------------------------------

 void __fastcall TMainFrm::FormDestroy(TObject *Sender)
{
	HVSTATUS status = STATUS_OK;

	//	�ر�������������ͷ�����������ڲ���Դ
	status = EndHVDevice(m_hhv);
	HV_VERIFY(status);

	//	����ͼ�񻺳���
	delete []m_pRawBuffer;
	delete []m_pImageBuffer;

}
//---------------------------------------------------------------------------
void __fastcall TMainFrm::FormCreate(TObject *Sender)
{
	HVSetSnapMode(m_hhv, SnapMode);
	HVSetSnapSpeed(m_hhv,1);
	//  ���ø�������������
	for (int i = 0; i < 4; i++){
		HVAGCControl(m_hhv, RED_CHANNEL + i, Gain);
	}

	//	�����ع�ʱ��
    SetExposureTime(HV_Width,ExposureTint_Upper,ExposureTint_Lower);
	//  ����ADC�ļ���
	HVADCControl(m_hhv, ADC_BITS, ADCLevel);

	/*
	*	��Ƶ������ڣ�����Ƶ�����Χ���������ȡֵ��Χ���������봰�ڷ�Χ���ڣ�
	*  ��Ƶ�������Ͻ�X����ʹ��ڿ���ӦΪ4�ı��������Ͻ�Y����ʹ��ڸ߶�ӦΪ2�ı���
	*	������ڵ���ʼλ��һ������Ϊ(0, 0)���ɡ�
	*/
	HVSetOutputWindow(m_hhv, XStart, YStart, HV_Width, HV_Height);

	//	m_pBmpInfo��ָ��m_chBmpBuf���������û������Լ�����BTIMAPINFO������
	m_pBmpInfo								= (BITMAPINFO *)m_chBmpBuf;
	//	��ʼ��BITMAPINFO �ṹ���˽ṹ�ڱ���bmp�ļ�����ʾ�ɼ�ͼ��ʱʹ��
	m_pBmpInfo->bmiHeader.biSize			= sizeof(BITMAPINFOHEADER);
	//	ͼ����ȣ�һ��Ϊ������ڿ���
	m_pBmpInfo->bmiHeader.biWidth			= HV_Width;
	//	ͼ����ȣ�һ��Ϊ������ڸ߶�
	m_pBmpInfo->bmiHeader.biHeight			= HV_Height;

	/*
	*	��������һ����ͬ��
	*	���ڵ���8λ��λͼ����Ӧ������Ӧ��λͼ��ɫ��
	*/
	m_pBmpInfo->bmiHeader.biPlanes			= 1;
	m_pBmpInfo->bmiHeader.biBitCount		= 24;
	m_pBmpInfo->bmiHeader.biCompression		= BI_RGB;
	m_pBmpInfo->bmiHeader.biSizeImage		= 0;
	m_pBmpInfo->bmiHeader.biXPelsPerMeter	= 0;
	m_pBmpInfo->bmiHeader.biYPelsPerMeter	= 0;
	m_pBmpInfo->bmiHeader.biClrUsed			= 0;
	m_pBmpInfo->bmiHeader.biClrImportant	= 0;

	/*
	*	����ԭʼͼ�񻺳�����һ�������洢�ɼ�ͼ��ԭʼ����
	*  һ��ͼ�񻺳�����С��������ڴ�С����Ƶ��ʽȷ����
	*/
	m_pRawBuffer = new BYTE[HV_Width * HV_Height];
	assert(m_pRawBuffer);

	/*
	����Bayerת����ͼ�����ݻ���
	*/
	m_pImageBuffer = new BYTE[HV_Width * HV_Height * 3];
	assert(m_pImageBuffer);
        nn=0;
        m_bStartSaveBmp=0;
        PLeft1=0;PTop1=0;PRight1=HV_Width;PBottom1=HV_Height;
        PLeft2=0;PTop2=0;PRight2=HV_Width;PBottom2=HV_Height;
        CentrFile_Save=0;
		I_Threshold1=0;
        I_Threshold2=0;
}

//---------------------------------------------------------------------------
void __fastcall TMainFrm::Open1Click(TObject *Sender)
{
	HVSTATUS status = STATUS_OK;

	/*
	 *	��ʼ������������ɼ�ͼ���ڴ�Ŀ��ƣ�
	 *	ָ���ص�����SnapThreadCallback���û�����m_hWnd
	 */
	status = HVOpenSnap(m_hhv, SnapThreadCallback, this->Handle);
	HV_VERIFY(status);
	if (HV_SUCCESS(status))
        {
		m_bOpen = TRUE;		//��־�Ѿ���SnapEx����

                //���ò˵���״̬
                Open1->Enabled = false;
                Start1->Enabled = true;
                Stop1->Enabled = false;
                Close1->Enabled = true;

                Open1->Checked = true;
                Start1->Checked = false;
                Stop1->Checked = false;
                Close1->Checked = false;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMainFrm::Start1Click(TObject *Sender)
{
	HVSTATUS status = STATUS_OK;

	/*
	 *	��������������ɼ�ͼ���ڴ�
	 */
	BYTE *ppBuf[1];
	ppBuf[0] = m_pRawBuffer;
	status = HVStartSnap(m_hhv, ppBuf,1);
	HV_VERIFY(status);
	if (HV_SUCCESS(status))
		{
		m_bStart = TRUE;

        //���ò˵���״̬
        Open1->Enabled = false;
        Start1->Enabled = false;
        Stop1->Enabled = true;
        Close1->Enabled = true;

        Open1->Checked = true;
        Start1->Checked = true;
        Stop1->Checked = false;
        Close1->Checked = false;

		//OnSnapChange("1");





	}
}
//---------------------------------------------------------------------------
void __fastcall TMainFrm::Stop1Click(TObject *Sender)
{
	HVSTATUS status =STATUS_OK;

	//	ֹͣ�ɼ�ͼ���ڴ棬�����ٴε���HVStartSnapEx��������������ɼ�
	status = HVStopSnap(m_hhv);
	HV_VERIFY(status);
	if (HV_SUCCESS(status))
        {
		m_bStart = FALSE;

        //���ò˵���״̬
        Open1->Enabled = false;
        Start1->Enabled = true;
        Stop1->Enabled = false;
        Close1->Enabled = true;

        Open1->Checked = true;
        Start1->Checked = false;
        Stop1->Checked = false;
        Close1->Checked = false;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMainFrm::Close1Click(TObject *Sender)
{
	HVSTATUS status = STATUS_OK;

	/*
	 *	��ֹ����������ɼ�ͼ���ڴ棬ͬʱ�ͷ����вɼ�������
	 *	�ٴ���������������ɼ����������³�ʼ��
	 */
	status = HVCloseSnap(m_hhv);
	HV_VERIFY(status);

	if (HV_SUCCESS(status))
        {
		m_bOpen		= FALSE;
		m_bStart	= FALSE;

        //���ò˵���״̬
        Open1->Enabled = true;
        Start1->Enabled = false;
        Stop1->Enabled = false;
        Close1->Enabled = false;

        Open1->Checked = false;
        Start1->Checked = false;
        Stop1->Checked = false;
        Close1->Checked = false;

	}
}
//----------------------------------------------------------------------------
/*
	����:
		SnapThreadCallback
	�������:
		SNAP_INFO *pInfo		SNAP_INFO�ṹ������ǰ���������SNAPִ��״̬
	�������:
		int
	˵��:
		����������ɼ����ڴ�ص����������û�һ�㲻�õ��ã����û��ṩ��SDKʹ�ã�
		�û��ڻص�������ʵ�ֶԲɼ����ݵĴ�������ʾ����
 */
int CALLBACK TMainFrm::SnapThreadCallback(HV_SNAP_INFO *pInfo)
{
	HWND hwnd = (HWND)(pInfo->pParam);

	/*
	 *	�����Զ�����ϢWM_SNAP_EX_CHANGE�������ڣ�
	 *	ͬʱ���뵱ǰ���Դ�����ͼ�����
	 *	ע�⣺��SendMessage������Ϣ������ȴ���Ϣ������Ϻ󣬲����˳�����SendMessage����
	 */
	::SendMessage(hwnd, WM_SNAP_CHANGE, 0, 0);

	return 1;
}

//-----------------------------------------------------------------------------


/*
	����:
		OnSnapChange
	�������:
		TMessage &Msg			�ֲ���������Ϣ��Ϊ��ǰ���Դ�����ͼ�����
	�������:
		LRESULT
	˵��:
		ʵ�ֶԲɼ����ݵĴ�������ʾ
 */
LRESULT TMainFrm::OnSnapChange(TMessage &Msg)
//LRESULT TMainFrm::OnSnapChange(char *filename)
{
	HVSTATUS status = STATUS_OK;
		char MYMSG[255];
        double X_centr1,Y_centr1,S_I1,sum_Mul_IX1,sum_Mul_IY1,II2;
		double X_centr2,Y_centr2,S_I2,sum_Mul_IX2,sum_Mul_IY2;
        char CH1=13,CH2=10;
		int prob1[256],prob2[256];
        double probs1[256],probs2[256];
		TDateTime dDate2;
        int PFileHandle;
//        AnsiString fn;
//        struct date day;
//        struct time t;
        int ii,jj;
        int mm;

	   unsigned short hour2,min2, sec2, msec2;
       double Time_Delay;//s
       AnsiString dirPathFileName;
       char BMPFileName[25],TXTFileName[25];

    Graphics::TBitmap *Bmp = new Graphics::TBitmap();
    Bmp->Width = HV_Width;
    Bmp->Height = HV_Height;
	Bmp->PixelFormat = pf24bit;


	HDC DC			= GetDC(this->Handle);;		//�õ�VIEW��DC

		Width_min = Edit1->Text.ToDouble();
		Width_max = Edit2->Text.ToDouble();
		Height_min = Edit3->Text.ToDouble();
		Height_max = Edit4->Text.ToDouble();
		TPoint points[5];
		Canvas->Pen->Color = clWhite;
		points[0].x = Width_min;
		points[0].y = Height_min;
		points[1].x = Width_max;
		points[1].y = Height_min;
		points[2].x = Width_max;
		points[2].y = Height_max;
		points[3].x = Width_min;
		points[3].y = Height_max;
		points[4].x = Width_min;
		points[4].y = Height_min;
		Canvas->Polyline(points,4);

	//	��ԭʼͼ�����ݽ���Bayerת����ת����Ϊ24λ��
         //ͬʱ��ԭʼ���ݽ������·�ת
	ConvertBayer2Rgb(m_pImageBuffer,m_pRawBuffer,HV_Width,HV_Height,ConvertType,m_pLutR,m_pLutG,m_pLutB,true,Layout);
//       ShowMessage(sizeof(m_pImageBuffer));


				 //��ɫ���ұ�
				BYTE pLutR[256];
				BYTE pLutG[256];
				BYTE pLutB[256];
				for(int i=0;i<256;i++)
				{
						pLutR[i] = i;
						pLutG[i] = i;
						pLutB[i] = i;

				}
						SetStretchBltMode(Bmp->Canvas->Handle, COLORONCOLOR);
						//��ͼ�����ݱ��浽λͼ�ṹ
						SetDIBitsToDevice(Bmp->Canvas->Handle, 0, 0, Width_, Height_,
								Width_min, Height_min, 0, m_pBmpInfo->bmiHeader.biHeight,
                        m_pImageBuffer, m_pBmpInfo, DIB_RGB_COLORS);

						//��ͼ�����ݱ��浽�ļ�
//                 sprintf(MYMSG,"0%d.bmp",nn);
  /*                      if(nn<9&&m_bStartSaveBmp==1)
						{


								//ShowMessage(BMPFileName);
						   Bmp->SaveToFile(BMPFileName);
						   nn++;
						}
						*/

                        if(m_bStartSaveBmp==1&&PRight1-PLeft1>0&&PBottom1-PTop1>0)
                        {

					/*  ����С��ͼ��
							   Graphics::TBitmap *TmpBmp = new Graphics::TBitmap();

							   TmpBmp->PixelFormat = pf24bit;
							TmpBmp->Width = PRight1-PLeft1;
							TmpBmp->Height = PBottom1-PTop1;
						   for(int ii=0;ii< PBottom1-PTop1;ii++)
								for(int jj=0;jj<PRight1-PLeft1;jj++)
								   TmpBmp->Canvas->Pixels[jj][TmpBmp->Height-ii-1]=*(m_pImageBuffer+(ii+Bmp->Height-PBottom1)*Bmp->Width*3+(jj+PLeft1)*3);
							  TmpBmp->SaveToFile(BMPFileName);
					*/



                     //�����һ��ͼ��

						   S_I1=0;sum_Mul_IX1=0;sum_Mul_IY1=0;
						   S_I2=0;sum_Mul_IX2=0;sum_Mul_IY2=0;
						   for(int ii=PTop1;ii< PBottom1;ii++)
								for(int jj=PLeft1;jj<PRight1;jj++)
								  {
									if( *(m_pImageBuffer+(Bmp->Height-ii-1)*Bmp->Width*3+jj*3)<I_Threshold1)
									  II2=0;
									else
									  II2=1.0* *(m_pImageBuffer+(Bmp->Height-ii-1)*Bmp->Width*3+jj*3)* *(m_pImageBuffer+(Bmp->Height-ii-1)*Bmp->Width*3+jj*3)* *(m_pImageBuffer+(Bmp->Height-ii-1)*Bmp->Width*3+jj*3)* *(m_pImageBuffer+ii*Bmp->Width*3+jj*3);
									S_I1+=II2;
									sum_Mul_IX1+=II2*jj;
									sum_Mul_IY1+=II2*ii;
								   }

						   X_centr1=sum_Mul_IX1/S_I1;
						   Y_centr1=sum_Mul_IY1/S_I1;

                     //����ڶ���ͼ��

                           for( ii=PTop2;ii< PBottom2;ii++)
                                for(jj=PLeft2;jj<PRight2;jj++)
                                  {
									if( *(m_pImageBuffer+(Bmp->Height-ii-1)*Bmp->Width*3+jj*3)<I_Threshold2)
                                      II2=0;
                                    else
                                      II2=1.0* *(m_pImageBuffer+(Bmp->Height-ii-1)*Bmp->Width*3+jj*3)* *(m_pImageBuffer+(Bmp->Height-ii-1)*Bmp->Width*3+jj*3)* *(m_pImageBuffer+(Bmp->Height-ii-1)*Bmp->Width*3+jj*3)* *(m_pImageBuffer+ii*Bmp->Width*3+jj*3);
                                    S_I2+=II2;
                                    sum_Mul_IX2+=II2*jj;
                                    sum_Mul_IY2+=II2*ii;
                                   }

                           X_centr2=sum_Mul_IX2/S_I2;
                           Y_centr2=sum_Mul_IY2/S_I2;


							  sprintf(MYMSG,"��������(%g,%g)(%g,%g)%d",X_centr1,Bmp->Height-Y_centr1,X_centr2,Bmp->Height-Y_centr2,nn);
							  StatusBar->Panels->Items[4]->Text =MYMSG;
						  dDate2=dDate2.CurrentDateTime();
						  dDate2.DecodeTime(&hour2, &min2,&sec2, &msec2);

						sprintf(MYMSG,"%d:%d:%g,%g,%g,%g,%g%c%c",hour2, min2,sec2+msec2/1000.0,X_centr1,Y_centr1,X_centr2,Y_centr2,CH1,CH2);
						FileWrite(WFileHandle, MYMSG, strlen(MYMSG));
//--------------------------------------------------------------------------------------------------------
							 //ʱ�䵽�������ļ�
                              //   TmpBmp->SaveToFile(BMPFileName);
							  Time_Delay=(hour2-hour0)*3600.0+(min2-min0)*60.0+sec2-sec0+(msec2-msec0)*0.001;
							  //Time_Delay = -Time_Delay;
							  if(Time_Delay<0) Time_Delay+=24.0*3600;
//                              sprintf(MYMSG,"%d:%d:%d.%d-%d:%d:%d.%d,%g",hour2, min2,sec2, msec2,hour0, min0,sec0, msec0, Time_Delay);
//                              ShowMessage(MYMSG);
                              if(CentrFile_Save==1&&Time_Delay>=Edit_Time_Delay->Text.ToDouble())
//                              if(CentrFile_Save==1&&nn>200)
                                {
                                  FileClose(WFileHandle);


								   //������ʷֲ���һ�����
								for(ii=0;ii<256;ii++)
								  prob1[ii]=0;
								for(int ii=PTop1;ii< PBottom1;ii++)
										for(int jj=PLeft1;jj<PRight1;jj++)
											prob1[ *(m_pImageBuffer+(Bmp->Height-ii-1)*Bmp->Width*3+jj*3)]++;
								for(ii=0;ii<256;ii++)
								  {
									probs1[ii]=0;mm=0;
									for(jj=-NUM_SMOOTH;jj<=NUM_SMOOTH;jj++)
									  {
										if(jj+ii<0||jj+ii>255) continue;
										probs1[ii]=probs1[ii]*mm/(mm+1.0)+prob1[ii+jj]/(mm+1.0);
										mm++;
									  }
								  }
								for(ii=4;ii<252;ii++)
								   if((probs1[ii]<(probs1[ii-1]+probs1[ii-2]+probs1[ii-3]+probs1[ii-4])/4.0&&probs1[ii]<(probs1[ii+1]+probs1[ii+2]+probs1[ii+3]+probs1[ii+4])/4.0)
								   ||(probs1[ii-1]==0&&probs1[ii]<(probs1[ii+1]+probs1[ii+2]+probs1[ii+3]+probs1[ii+4])/4.0))
									  I_Threshold1=ii;
                                //������ʷֲ��ڶ������
                                for(ii=0;ii<256;ii++)
                                  prob2[ii]=0;
                                for(int ii=PTop2;ii< PBottom2;ii++)
                                        for(int jj=PLeft2;jj<PRight2;jj++)
                                            prob2[ *(m_pImageBuffer+(Bmp->Height-ii-1)*Bmp->Width*3+jj*3)]++;
                                for(ii=0;ii<256;ii++)
                                  {
                                    probs2[ii]=0;mm=0;
                                    for(jj=-NUM_SMOOTH;jj<=NUM_SMOOTH;jj++)
                                      {
                                        if(jj+ii<0||jj+ii>255) continue;
                                        probs2[ii]=probs2[ii]*mm/(mm+1.0)+prob2[ii+jj]/(mm+1.0);
                                        mm++;
                                      }
                                  }
                                for(ii=4;ii<252;ii++)
                                   if((probs2[ii]<(probs2[ii-1]+probs2[ii-2]+probs2[ii-3]+probs2[ii-4])/4.0&&probs2[ii]<(probs2[ii+1]+probs2[ii+2]+probs2[ii+3]+probs2[ii+4])/4.0)
                                   ||(probs2[ii-1]==0&&probs2[ii]<(probs2[ii+1]+probs2[ii+2]+probs2[ii+3]+probs2[ii+4])/4.0))
                                      I_Threshold2=ii;


                                  dDate0=Now();
                                  dDate0.DecodeTime(&hour0, &min0,&sec0, &msec0);
                                ::GetLocalTime(&LocalTime);
                                   dirPathFileName=Now().FormatString("yyyymmdd");
                                   dirPathFileName+= FormatFloat("00", LocalTime.wHour)+FormatFloat("00", LocalTime.wMinute)+FormatFloat("00", LocalTime.wSecond)+FormatFloat("000", LocalTime.wMilliseconds);

                                 sprintf(BMPFileName,"%s.BMP",dirPathFileName);
                                 Bmp->SaveToFile(BMPFileName);

                                 sprintf(TXTFileName,"%s.ARR",dirPathFileName);
                                WFileHandle = FileCreate(TXTFileName);
                                nn=0;
                                }
//--------------------------------------------------------------------------------------------------------
                           if(CentrFile_Save==0)
                              {
                                  dDate0=Now();
                                  dDate0.DecodeTime(&hour0, &min0,&sec0, &msec0);
                                ::GetLocalTime(&LocalTime);
                                   dirPathFileName=Now().FormatString("yyyymmdd");
                                   dirPathFileName+= FormatFloat("00", LocalTime.wHour)+FormatFloat("00", LocalTime.wMinute)+FormatFloat("00", LocalTime.wSecond)+FormatFloat("000", LocalTime.wMilliseconds);

                                 sprintf(BMPFileName,"%s.BMP",dirPathFileName);
                                 Bmp->SaveToFile(BMPFileName);
                                CentrFile_Save=1;
                                //������ʷֲ���һ�����
                                for(ii=0;ii<256;ii++)
                                  prob1[ii]=0;
                                for(int ii=PTop1;ii< PBottom1;ii++)
                                        for(int jj=PLeft1;jj<PRight1;jj++)
                                            prob1[ *(m_pImageBuffer+(Bmp->Height-ii-1)*Bmp->Width*3+jj*3)]++;
                                for(ii=0;ii<256;ii++)
                                  {
                                    probs1[ii]=0;mm=0;
                                    for(jj=-NUM_SMOOTH;jj<=NUM_SMOOTH;jj++)
                                      {
                                        if(jj+ii<0||jj+ii>255) continue;
                                        probs1[ii]=probs1[ii]*mm/(mm+1.0)+prob1[ii+jj]/(mm+1.0);
                                        mm++;
                                      }
                                  }
                                for(ii=4;ii<252;ii++)
                                   if((probs1[ii]<(probs1[ii-1]+probs1[ii-2]+probs1[ii-3]+probs1[ii-4])/4.0&&probs1[ii]<(probs1[ii+1]+probs1[ii+2]+probs1[ii+3]+probs1[ii+4])/4.0)
                                   ||(probs1[ii-1]==0&&probs1[ii]<(probs1[ii+1]+probs1[ii+2]+probs1[ii+3]+probs1[ii+4])/4.0))
									  I_Threshold1=ii;
                                //������ʷֲ��ڶ������
                                for(ii=0;ii<256;ii++)
                                  prob2[ii]=0;
                                for(int ii=PTop2;ii< PBottom2;ii++)
                                        for(int jj=PLeft2;jj<PRight2;jj++)
                                            prob2[ *(m_pImageBuffer+(Bmp->Height-ii-1)*Bmp->Width*3+jj*3)]++;
                                for(ii=0;ii<256;ii++)
                                  {
                                    probs2[ii]=0;mm=0;
                                    for(jj=-NUM_SMOOTH;jj<=NUM_SMOOTH;jj++)
                                      {
                                        if(jj+ii<0||jj+ii>255) continue;
                                        probs2[ii]=probs2[ii]*mm/(mm+1.0)+prob2[ii+jj]/(mm+1.0);
										mm++;
                                      }
                                  }
                                for(ii=4;ii<252;ii++)
                                   if((probs2[ii]<(probs2[ii-1]+probs2[ii-2]+probs2[ii-3]+probs2[ii-4])/4.0&&probs2[ii]<(probs2[ii+1]+probs2[ii+2]+probs2[ii+3]+probs2[ii+4])/4.0)
                                   ||(probs2[ii-1]==0&&probs2[ii]<(probs2[ii+1]+probs2[ii+2]+probs2[ii+3]+probs2[ii+4])/4.0))
                                      I_Threshold2=ii;
                                //������ʷֲ�����

                                 sprintf(TXTFileName,"%s_p.TXT",dirPathFileName);
                                 PFileHandle = FileCreate(TXTFileName);

                                  sprintf(MYMSG,"I_Threshold1=%d,I_Threshold2=%d%c%c",I_Threshold1,I_Threshold2,CH1,CH2);
                                  FileWrite(PFileHandle, MYMSG, strlen(MYMSG));
                                for(ii=0;ii<256;ii++)
                                {
                                  sprintf(MYMSG,"%d,%d,%g,%d,%g%c%c",ii,prob1[ii],probs1[ii],prob2[ii],probs2[ii],CH1,CH2);
                                  FileWrite(PFileHandle, MYMSG, strlen(MYMSG));
                                }
                                  FileClose(PFileHandle);

                                 sprintf(TXTFileName,"%s.ARR",dirPathFileName);
                                 WFileHandle = FileCreate(TXTFileName);
                                  dDate0=Now();
                                  dDate0.DecodeTime(&hour0, &min0,&sec0, &msec0);

                              }
                              nn++;
                        }




	BYTE *p = NULL, * q = NULL;
	//	����ͼ��ɫ
	if (m_bNegative) {
		for (int i = 0; i < HV_Height; i++){
			p = m_pImageBuffer + i * HV_Width * 3;
			for(int j = 0; j < HV_Width; j++){
				q = p + j * 3;
				*(q + 0) = ~(*(q + 0));
				*(q + 1) = ~(*(q + 1));
				*(q + 2) = ~(*(q + 2));
			}
		}
	}

	//����ͼ�ͻ�����ʾͼ��
	StretchDIBits(DC,
					0,
					0,
					HV_Width,					//��ʾ���ڿ���
					HV_Height,					//��ʾ���ڸ߶�
					0,
					0,
					HV_Width,					//ͼ�����
					HV_Height,					//ͼ��߶�
					m_pImageBuffer,			//ͼ�񻺳���
					m_pBmpInfo,				//BMPͼ��������Ϣ
					DIB_RGB_COLORS,
					SRCCOPY
					);

	ReleaseDC(this->Handle,DC);
	   delete Bmp;
//       delete TmpBmp;
	return 1;
}

//---------------------------------------------------------------------------
void __fastcall TMainFrm::Negative1Click(TObject *Sender)
{
	m_bNegative = !m_bNegative;			//����ͼ��ɫ��־

    Negative1->Checked = m_bNegative;
}

//���ݿ����������������ع�ʱ��
//�����Ĳ����������ʱ��Ƶ�ʣ�����ֵ��ȡĬ��ֵ��
//������
//nWindWidth:��ǰͼ�����
//lTintUpper:�ع�ʱ��ķ���, lTintUpper/lTintLower ���ʵ�ʵ��ع�ʱ��
//lTintLower:�ع�ʱ��ķ�ĸ��lTintUpper/lTintLower ���ʵ�ʵ��ع�ʱ��
void __fastcall TMainFrm::SetExposureTime(int nWindWidth,long lTintUpper,long lTintLower)
{
	int size = sizeof(HVTYPE);
	HVTYPE type;
	HVGetDeviceInfo(m_hhv,DESC_DEVICE_TYPE, &type, &size);

	//When outputwindow changes, change the exposure
	//��ο��ع�ϵ��ת����ʽ
	long lClockFreq = 24000000;
	int nOutputWid = nWindWidth;
	double dExposure = 0.0;
	double dTint = max((double)lTintUpper/(double)lTintLower,MY_ZERO);
	if(type == HV1300UCTYPE || type == HV1301UCTYPE)
	{
		long lTb = 0;
		dExposure = (dTint* lClockFreq + 180.0)/((double)nOutputWid + 244.0 + lTb);
	}
	else
	{
		long lTb = 0;
		dExposure = (dTint* lClockFreq + 180.0)/((double)nOutputWid + 305.0 + lTb) + 1 ;
	}

	if (dExposure > 16383)
		dExposure = 16383;
	HVAECControl(m_hhv, AEC_EXPOSURE_TIME, (long)dExposure);

}

//---------------------------------------------------------------------------
/*
void __fastcall TMainFrm::GetBMPFileName(int status)
{
    //---------------------------------����Ŀ¼
    AnsiString AppPath = ExtractFilePath(Application->ExeName);
    AnsiString dirPath=AppPath+Now().FormatString("yyyymmdd")+"\\";
    if(!DirectoryExists(dirPath))//���·��������
    {
        CreateDir(dirPath);  //������Ŀ¼
    }
    //---------------------------------------------------------------------

    if(status==1)
    {//���Ųɼ�
        dirPath=dirPath+"�ɼ���֡"+UserName()+"\\";
        if(!DirectoryExists(dirPath))//���·��������
        {
            CreateDir(dirPath);  //������Ŀ¼
        }
    }

    //---------------------------------------------------------------------
    if(status==2)
    { //�����ɼ�
        dirPath=dirPath+"Continue"+UserName()+"\\";
        if(!DirectoryExists(dirPath))//���·��������
        {
            CreateDir(dirPath);  //������Ŀ¼
        }
    }
    //---------------------------------------------------------------------
    if(status==3)
    { //�Զ���ɼ���Ŀ¼
        dirPath=dirPath+"SelfHz"+UserName()+"\\";
        if(!DirectoryExists(dirPath))//���·��������
        {
            CreateDir(dirPath);  //������Ŀ¼
        }
    }
    //---------------------------------------------------------------------
    if(status==4)
    { //��ʱ�ɼ���Ŀ¼
        dirPath=dirPath+"LocalTime"+UserName()+"\\";
        if(!DirectoryExists(dirPath))//���·��������
        {
            CreateDir(dirPath);//������Ŀ¼
        }
    }
    //---------------------------------------------------------------------
    if(status==5)
    { //�������������Ʋɼ���Ŀ¼
        dirPath=dirPath+"CCD"+UserName()+"\\";
        if(!DirectoryExists(dirPath))//���·��������
        {
            CreateDir(dirPath);//������Ŀ¼
        }
    }

    //-----------�����ļ���BMPFileName
    ::GetLocalTime(&LocalTime);
    AnsiString fn =dirPath+ FormatFloat("00", LocalTime.wHour)+FormatFloat("00", LocalTime.wMinute)+FormatFloat("00", LocalTime.wSecond)+FormatFloat("000", LocalTime.wMilliseconds);
    fn=fn+".bmp";
    fn.WideChar(BMPFileName,fn.WideCharBufSize());
}
*/
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
AnsiString   __fastcall TMainFrm::ComputerName()
{  // ��õ�ǰ���������
    char   ComputerName[80];
    unsigned long ulsize=80;
    ::GetComputerName(ComputerName,&ulsize);
    return  AnsiString(ComputerName).UpperCase();
}
//---------------------------------------------------------------------------

AnsiString   __fastcall TMainFrm::UserName()
{  // ��õ�ǰ�û���
    char   UserName[80];
    unsigned long ulsize=80;
	::GetUserName(UserName,&ulsize);
    return  AnsiString(UserName).UpperCase();
}
//---------------------------------------------------------------------------
void __fastcall TMainFrm::BitBtn1Click(TObject *Sender)
{
AnsiString   dirPathFileName;
double num;
unsigned short hour,min, sec, msec;
int WFileHandle;
char MYMSG[256],TXTFileName[256];
TDateTime dDate;
char BMPFileName[255];
int prob1[256],prob2[256];
double probs1[256],probs2[256];
	Time_Length=Edit_Time_Length->Text.ToDouble();
	NUM_SMOOTH=Edit_NUM_SMOOTH->Text.ToDouble();
		Time_Delay=Edit_Time_Delay->Text.ToDouble();
		num=Time_Length/Time_Delay;

	dirPathFileName=Now().FormatString("yyyymmdd");
			sprintf(TXTFileName,"%s.TXT",dirPathFileName);
			WFileHandle = FileCreate(TXTFileName);
		for(int kk=0;kk<num;kk++)
		{
			DWORD oldtimer = GetTickCount();
			dirPathFileName=Now().FormatString("yyyymmdd");
			::GetLocalTime(&LocalTime);
			dirPathFileName+= FormatFloat("00", LocalTime.wHour)+FormatFloat("00", LocalTime.wMinute)+FormatFloat("00", LocalTime.wSecond)+FormatFloat("000", LocalTime.wMilliseconds);
			sprintf(BMPFileName,"%s.BMP",dirPathFileName);

//ShowMessage(UserName());
//GetBMPFileName(3);
//dirPathFileName=Now().FormatString("yyyymmdd");
////                                 AnsiString dirPath=AppPath+Now().FormatString("yyyymmdd")
//sprintf(BMPFileName,"%s.BMP",dirPathFileName);


//ShowMessage(BMPFileName);
Graphics::TBitmap *Bmp = new Graphics::TBitmap();
	HDC DC			= GetDC(this->Handle);;		//�õ�VIEW��DC
ConvertBayer2Rgb(m_pImageBuffer,m_pRawBuffer,HV_Width,HV_Height,ConvertType,m_pLutR,m_pLutG,m_pLutB,true,Layout);
//       ShowMessage(sizeof(m_pImageBuffer));
//
//
//				 ��ɫ���ұ�
				BYTE pLutR[256];
				BYTE pLutG[256];
				BYTE pLutB[256];
				for(int i=0;i<256;i++)
				{
						pLutR[i] = i;
						pLutG[i] = i;
						pLutB[i] = i;

				}
Width_ = Width_max-Width_min;
Height_ = Height_max-Height_min;
Bmp->Width = Width_;
Bmp->Height = Height_;
Bmp->PixelFormat = pf24bit;

SetStretchBltMode(Bmp->Canvas->Handle, COLORONCOLOR);
						//��ͼ�����ݱ��浽λͼ�ṹ
						SetDIBitsToDevice(Bmp->Canvas->Handle, 0, 0, Width_, Height_,
								Width_min,HV_Height-Height_max,0,HV_Height,
						m_pImageBuffer, m_pBmpInfo, DIB_RGB_COLORS);
double mm=0;
						for(int ii=0;ii<256;ii++)
								  prob1[ii]=0;
								for(int ii=Height_min;ii< Height_max;ii++)
										for(int jj=Width_min;jj<Width_max;jj++)
											prob1[ *(m_pImageBuffer+(Height_max-ii-1)*Bmp->Width*3+jj*3)]++;
								for(int ii=0;ii<256;ii++)
								  {
									probs1[ii]=0;mm=0;
									for(int jj=-NUM_SMOOTH;jj<=NUM_SMOOTH;jj++)
									  {
										if(jj+ii<0||jj+ii>255) continue;
										probs1[ii]=probs1[ii]*mm/(mm+1.0)+prob1[ii+jj]/(mm+1.0);
										mm++;
									  }
								  }
								for(int ii=4;ii<252;ii++)
								   if((probs1[ii]<(probs1[ii-1]+probs1[ii-2]+probs1[ii-3]+probs1[ii-4])/4.0&&probs1[ii]<(probs1[ii+1]+probs1[ii+2]+probs1[ii+3]+probs1[ii+4])/4.0)
								   ||(probs1[ii-1]==0&&probs1[ii]<(probs1[ii+1]+probs1[ii+2]+probs1[ii+3]+probs1[ii+4])/4.0))
									  I_Threshold1=ii;
double max=0;
double S_I1=0,sum_Mul_IX1=0,sum_Mul_IY1=0,II1=0;
double X_centr1,Y_centr1;
for(int ii=Height_min;ii< Height_max;ii++)
	for(int jj=Width_min;jj<Width_max;jj++)
	  {
		if( *(m_pImageBuffer+(Height_max-ii-1)*Bmp->Width*3+jj*3)<I_Threshold1)
		  II1=0;
		else
		  II1=1.0* *(m_pImageBuffer+(Height_max-ii-1)*Bmp->Width*3+jj*3)* *(m_pImageBuffer+(Height_max-ii-1)*Bmp->Width*3+jj*3)* *(m_pImageBuffer+(Height_max-ii-1)*Bmp->Width*3+jj*3)* *(m_pImageBuffer+ii*Bmp->Width*3+jj*3);
		S_I1+=II1;
		sum_Mul_IX1+=II1*jj;
		sum_Mul_IY1+=II1*ii;
	   }

X_centr1=sum_Mul_IX1/S_I1;
Y_centr1=sum_Mul_IY1/S_I1;
sprintf(MYMSG,"��������(%g,%g)",X_centr1,Y_centr1);
StatusBar->Panels->Items[4]->Text =MYMSG;
dDate=dDate.CurrentDateTime();
dDate.DecodeTime(&hour, &min,&sec, &msec);
sprintf(MYMSG,"%d:%d:%g  %.2f  %.2f \r\n",hour, min,sec+msec/1000.0,X_centr1,Y_centr1);
FileWrite(WFileHandle, MYMSG, strlen(MYMSG));
if(kk%30==0)	Bmp->SaveToFile(BMPFileName);

while ( ( GetTickCount() - oldtimer ) < 1000*Time_Delay)
   {
	Application->ProcessMessages();
   }
   ReleaseDC(this->Handle,DC);
			}
   ShowMessage("Complete!");
FileClose(WFileHandle);

nn=0;

		}
//---------------------------------------------------------------------------


void __fastcall TMainFrm::FormMouseDown(TObject *Sender,
	  TMouseButton Button, TShiftState Shift, int X, int Y)
{
  char MYMSG[255];
   if(Shift.Contains(ssShift)&&Button==mbLeft)
   {
      PLeft1=X;
	  PTop1=Y;
	  sprintf(MYMSG,"LT1(%d,%d)",X,Y);
	  StatusBar->Panels->Items[0]->Text =MYMSG;
   }
  if(Shift.Contains(ssShift)&&Button==mbRight)
   {
	  PRight1=X;
	  PBottom1=Y;
      sprintf(MYMSG,"RB1(%d,%d)",X,Y);
      StatusBar->Panels->Items[1]->Text =MYMSG;
   }
   if(Shift.Contains(ssCtrl)&&Button==mbLeft)
   {
      PLeft2=X;
      PTop2=Y;
      sprintf(MYMSG,"LT2(%d,%d)",X,Y);
      StatusBar->Panels->Items[2]->Text =MYMSG;
   }
  if(Shift.Contains(ssCtrl)&&Button==mbRight)
   {
      PRight2=X;
      PBottom2=Y;
      sprintf(MYMSG,"RB2(%d,%d)",X,Y);
      StatusBar->Panels->Items[3]->Text =MYMSG;
   }

}
//---------------------------------------------------------------------------

void __fastcall TMainFrm::StartSave1Click(TObject *Sender)
{
int ii;
double num;
StartSave1->Enabled = false;

Time_Length=Edit_Time_Length->Text.ToDouble();
TimeDelay=Edit_Time_Delay->Text.ToDouble();
num=Time_Length/TimeDelay;
ShowMessage(num);
for(ii=0;ii<num;ii++)
{
   m_bStartSaveBmp=1;
//   OnSnapChange("1");
   sleep(TimeDelay);

}

}
//---------------------------------------------------------------------------

void __fastcall TMainFrm::StopSave1Click(TObject *Sender)
{
 if(CentrFile_Save==1)
  FileClose(WFileHandle);
 m_bStartSaveBmp=0;
}
//---------------------------------------------------------------------------

void __fastcall TMainFrm::Btn_BmpProbClick(TObject *Sender)
{
          int prob[256];
          double probs[256];
          int ii,jj,nn;
          char MYMSG[255];
               char CH1=13,CH2=10;
          char BMPFileName[25],TXTFileName[25];
          AnsiString dirPathFileName;
//   GetBMPFileName(3);
   PLeft1=524;PTop1=338;PRight1=885;PBottom1=659;
//   PLeft1=597;PTop1=392;PRight1=786;PBottom1=539;
//   PLeft1=550;PTop1=348;PRight1=835;PBottom1=515;

  Graphics::TBitmap *Bmp = new Graphics::TBitmap();
  try
  {
    Bmp->LoadFromFile("D:\\CCD\\HVRealtime\\123011187.bmp");



                                //������ʷֲ�s
                                for(ii=0;ii<256;ii++)
                                  prob[ii]=0;
                            for(ii=Bmp->Height-PBottom1;ii< Bmp->Height-PTop1;ii++)
                                  for( jj=PLeft1;jj<PRight1;jj++)
                                          {
//                                            ShowMessage(GetRValue(Bmp->Canvas->Pixels[jj][Bmp->Height-ii-1]));
                                            prob[ GetRValue(Bmp->Canvas->Pixels[jj][ii])]++;
                                          }
//                                 TmpBmp->Canvas->Pixels[jj][Bmp->Height-ii-1]

                        /*        for(ii=0;ii<256;ii++)
                                  if(I_max<prob[ii]){I_max=prob[ii];Ind=ii;}
                                for(ii=ind;ii>=0;ii--)
                                  if(prob  */
                                   dirPathFileName=Now().FormatString("yyyymmdd");
//                                 AnsiString dirPath=AppPath+Now().FormatString("yyyymmdd")
                                 sprintf(BMPFileName,"%s.BMP",dirPathFileName);
                                 sprintf(TXTFileName,"%s.TXT",dirPathFileName);

//                                sprintf(MYMSG,"%s_p.txt",WcharToChar(TXTFileName));
//                                ShowMessage(MYMSG);
                                WFileHandle = FileCreate(TXTFileName);
                                for(ii=0;ii<256;ii++)
                                  {
                                    probs[ii]=0;nn=0;
                                    for(jj=-15;jj<=15;jj++)
                                      {
                                        if(jj+ii<0||jj+ii>255) continue;
                                        probs[ii]=probs[ii]*nn/(nn+1.0)+prob[ii+jj]/(nn+1.0);
                                        nn++;
                                      }

                                  }
                                for(ii=0;ii<256;ii++)
                                {
                                  sprintf(MYMSG,"%d,%d,%g%c%c",ii,prob[ii],probs[ii],CH1,CH2);
                                  FileWrite(WFileHandle, MYMSG, strlen(MYMSG));
                                }
                                  FileClose(WFileHandle);


//                  /*  ����С��ͼ��
                         Graphics::TBitmap *TmpBmp = new Graphics::TBitmap();
                               TmpBmp->PixelFormat = pf24bit;
                            TmpBmp->Width = PRight1-PLeft1;
                            TmpBmp->Height = PBottom1-PTop1;
//                          for(ii=Bmp->Height-PBottom1;ii< Bmp->Height-PTop1;ii++)
                          for(ii=PTop1;ii<PBottom1;ii++)
                               for( jj=PLeft1;jj<PRight1;jj++)
                                  if(GetRValue(Bmp->Canvas->Pixels[jj][ii])<0)
                                     TmpBmp->Canvas->Pixels[jj-PLeft1][ii-PTop1]=0;
                                  else
                                    TmpBmp->Canvas->Pixels[jj-PLeft1][ii-PTop1]=Bmp->Canvas->Pixels[jj][ii];

//                          for(int ii=0;ii< PBottom1-PTop1;ii++)
  //                              for(int jj=0;jj<PRight1-PLeft1;jj++)
    //                               TmpBmp->Canvas->Pixels[jj][TmpBmp->Height-ii-1]=Bmp->Canvas->Pixels[jj+PLeft1][(ii+Bmp->Height-PBottom1)];
//                                   TmpBmp->Canvas->Pixels[jj][TmpBmp->Height-ii-1]=*(m_pImageBuffer+(ii+Bmp->Height-PBottom1)*Bmp->Width*3+(jj+PLeft1)*3);

                              TmpBmp->SaveToFile(BMPFileName);
//                    */



  }
  catch (...)
  {
    MessageBeep(0);
  }
  delete Bmp;


}
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
