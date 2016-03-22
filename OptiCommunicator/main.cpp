/*------------/

NatNet APIを使ったMotiveとの通信サンプル

【注意】先にMotiveでDataStreamingを開始してください

/------------*/


#include <iostream>
#include <fstream>
#include <conio.h>//_getch
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
//#include "serial.h"

#include "NatNetTypes.h"
#include "NatNetClient.h"

#pragma comment(lib, "NatNetLib.lib")

//--------------プロトタイプ宣言
void __cdecl DataHandler(sFrameOfMocapData* data, void* pUserData);		// データを受け取った際に呼ばれるコールバック
void __cdecl MessageHandler(int msgType, char* msg);		            // NatNetAPIがエラーをはいた際に呼ばれるコールバック
int CreateClient(int iConnectionType); //クライアント作成関数（初期化）

//--------------グローバル変数
unsigned int MyServersDataPort = 1511;
unsigned int MyServersCommandPort = 1510;
int iConnectionType = ConnectionType_Multicast;
NatNetClient* theClient;
char szMyIPAddress[128] = "";//使用する自分のインタフェースに対応するIPアドレス
char szServerIPAddress[128] = "";//サーバのIPアドレス（Motiveの、DataStreaming Viewにあります）

using namespace std;
string Fname = "Opti_Switch_1.csv";
string Fname2 = "Switch_1.csv";
ofstream fout(Fname);
ofstream fout2(Fname2);
long double t0 = 0.0;
long double t = 0.0;
int sFlag = 0;


LPCSTR portName = "COM5";
HANDLE myHComPort = CreateFile(
	(LPCTSTR)portName,
	GENERIC_READ | GENERIC_WRITE,
	0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);



char buffer[2048] = {0};         // データを読み込む為のバッファ
//BYTE* RecData = &buffer;
DWORD toReadBytes = 2048;  // 読込み時の指定バイト数 
DWORD readBytes;           // 実際に読み込んだバイト数 
char* szStr = {0};
char* ctx = { 0 };

BYTE Rbuffer;
BYTE* pbuffer = &Rbuffer;			// 送信データを格納するバッファ

DWORD toWriteBytes = 10;      // 送信したいバイト数 
DWORD writeBytes;       // 実際に送信されたバイト数 

int main(int argc, char**argv)
{
	//serial_t obj = serial_create("COM5", 9600);
	//unsigned char Sbuf[128], Rbuf[128], len;

	/*
	LPCSTR portName = "COM5";
	HANDLE myHComPort = CreateFile(
		(LPCTSTR)portName,
		GENERIC_READ | GENERIC_WRITE,
		0, NULL, OPEN_EXISTING, 0, NULL);
	*/
	// ポートのボーレート、パリティ等を設定 
	DCB config;
	config.DCBlength = sizeof(DCB);
	config.BaudRate = CBR_57600;
	config.ByteSize = 8;
	config.fNull = TRUE;
	config.fParity = TRUE;	
	config.Parity = NOPARITY;
	config.StopBits = TWOSTOPBITS;
	config.EofChar = 'N';
	//ハードウェアフロー制御
	config.fOutxCtsFlow = FALSE;       // 　CTSハードウェアフロー制御：CTS制御を使用しない場合はFLASEを指定
	//   　　　　　　CTS制御をする場合はTRUEを指定してCTS信号を監視します。
	config.fOutxDsrFlow = FALSE;       //  DSRハードウェアフロー制御：使用しない場合はFALSEを指定
	config.fDtrControl = DTR_CONTROL_DISABLE;// DTR有効/無効：　無効なら　DTR_CONTROL_DISABLE;ISABLE
	config.fRtsControl = RTS_CONTROL_DISABLE;  // RTS制御：　RTS制御をしない場合はRTS_CONTROL_DISABLEを指定
	//　　　　　　　　 RTS制御をする場合はRTS_CONTROL_ENABLEを指定　　他

	// ソフトウェアフロー制御
	config.fOutX = FALSE;              // 送信時XON/OFF制御の有無：　なし→FLALSE
	config.fInX = FALSE;               // 受信時XON/XOFF制御の有無：なし→FALSE
	config.fTXContinueOnXoff = TRUE;   // 受信バッファー満杯＆XOFF受信後の継続送信可否：送信可→TRUE
	config.XonLim = 512;               // XONが送られるまでに格納できる最小バイト数：
	config.XoffLim = 512;              // XOFFが送られるまでに格納できる最小バイト数：
	config.XonChar = 0x11;             // 送信時XON文字 ( 送信可：ビジィ解除 ) の指定：
	//　一般に、XON文字として11H ( デバイス制御１：DC1 )がよく使われます
	config.XoffChar = 0x13;            // XOFF文字（送信不可：ビジー通告）の指定：なし→FALSE
	//　一般に、XOFF文字として13H ( デバイス制御3：DC3 )がよく使われます

	// Parity、StopBits、DataBitsも同様に設定 
	SetCommState(myHComPort, &config);
	SetupComm(	//設定
		myHComPort,	// 　通信デバイスのハンドル：CreateFile()で取得したハンドルを指定
		512,   //   受信バッファーサイズ：　受信のバッファーサイズをバイト単位で指定
		512    // 　送信バッファーサイズ：　送信のバッファーサイズをバイト単位で指定
		);
	COMMTIMEOUTS timeout;
	timeout.ReadIntervalTimeout = 5; // 文字読込時の２も時間の全体待ち時間（msec）

	timeout.ReadTotalTimeoutMultiplier = 10; //読込の１文字あたりの時間
	timeout.ReadTotalTimeoutConstant = 100; //読込エラー検出用のタイムアウト時間
	//(受信トータルタイムアウト) = ReadTotalTimeoutMultiplier * (受信予定バイト数) + ReadTotalTimeoutConstant 

	timeout.WriteTotalTimeoutMultiplier = 0; //書き込み１文字あたりの待ち時間
	timeout.WriteTotalTimeoutConstant = 500;//書き込みエラー検出用のタイムアウト時間
	SetCommTimeouts(myHComPort, &timeout);//SetCommTimeOut()関数にポートハンドルおよびCOMMTIMEOUTS構造体の
	//アドレスを代入します。
	//(送信トータルタイムアウト) = WriteTotalTimeoutMultiplier * (送信予定バイト数) + WriteTotalTimeoutConstant 
	/*
	BYTE buffer[1024];         // データを読み込む為のバッファ 
	DWORD toReadBytes = 1024;  // 読込み時の指定バイト数 
	DWORD readBytes;           // 実際に読み込んだバイト数 
	
	BYTE Rbuffer;
	BYTE* pbuffer = &Rbuffer;			// 送信データを格納するバッファ
	
	DWORD toWriteBytes = 2;      // 送信したいバイト数 
	DWORD writeBytes;       // 実際に送信されたバイト数 
	*/
	
	EscapeCommFunction(
		myHComPort,    // 　通信デバイスのハンドル：CreateFile()で取得したハンドルを指定
		SETRTS // 受信可能であることを相手側に示す：RTSをアクティブにする→SETRTS
		//　　　　　　（参考）   RTSを非アクティブにする→CLRRTS　
		);

	int c;
	
	/*
	Rbuffer = '0';
	WriteFile(myHComPort, pbuffer, toWriteBytes, &writeBytes, NULL);
	Sleep(10);

	while (ReadFile(myHComPort, &buffer, toReadBytes, &readBytes, NULL)){
		buffer[readBytes] = '\0';
		printf("\n%s", buffer);		
		Rbuffer = '0';
		WriteFile(myHComPort, pbuffer, toWriteBytes, &writeBytes, NULL);
		Sleep(10);
		//ReadFile(myHComPort, &buffer, toReadBytes, &readBytes, NULL);
		
		//PurgeComm(myHComPort, PURGE_RXCLEAR);
		//}
		
		
		//printf("%c,%d\n", buffer[i], readBytes);
		
	}
	*/
	/*
	while (c = _getch())
	{
		switch (c)
		{
		case '0':
			Rbuffer = '0';
			WriteFile(myHComPort, pbuffer, toWriteBytes, &writeBytes, NULL);
			//Sleep(100);
			//ReadFile(myHComPort, buffer, toReadBytes, &readBytes, NULL);
			//printf("%s\n", buffer);
			break;
		}
		Rbuffer = '0';
		WriteFile(myHComPort, pbuffer, toWriteBytes, &writeBytes, NULL);
		ReadFile(myHComPort, buffer, toReadBytes, &readBytes, NULL);
		printf("%s\n", buffer);
	}
	
	*/
	// Create NatNet Client
	//CreateClient(iConnectionType);

	// Ready to receive marker stream!
	printf("\nClient is connected to server and listening for data...\n");
	fout << "t0, t, x, y, z, qx, qy, qz, qw" << endl;
	//int c;
	bool bExit = false;
	while (1)
	{
		if (_kbhit()){
			c = _getch();
			switch (c)
			{
			case 's':
				Rbuffer = '0';
				WriteFile(myHComPort, pbuffer, toWriteBytes, &writeBytes, NULL);
				CreateClient(iConnectionType);
				sFlag = 1;
				break;

			case 'q':
				bExit = true;
				fout << "END" << endl;
				CloseHandle(
					myHComPort   // 　通信デバイスのハンドル：　CreateFile()で取得したハンドルを指定
					);
				fout.close();
				fout2.close();
				break;
			}
		}
		
		if (sFlag == 1){
			Rbuffer = '0';
			WriteFile(myHComPort, pbuffer, toWriteBytes, &writeBytes, NULL);
			ReadFile(myHComPort, &buffer, toReadBytes, &readBytes, NULL);
			buffer[readBytes] = '\0';

			fout2 << buffer << endl;
		}

		if (bExit)
			break;
	}
	/*
	if (sFlag == 1){
		while (ReadFile(myHComPort, &buffer, toReadBytes, &readBytes, NULL)){
			buffer[readBytes] = '\0';
			printf("\n%s", buffer);
			Rbuffer = '0';
			WriteFile(myHComPort, pbuffer, toWriteBytes, &writeBytes, NULL);
			Sleep(70);
			//ReadFile(myHComPort, &buffer, toReadBytes, &readBytes, NULL);

			//PurgeComm(myHComPort, PURGE_RXCLEAR);
			//}


			//printf("%c,%d\n", buffer[i], readBytes);

		}
	}
	*/
	// Done - clean up.
	theClient->Uninitialize();

	//return 0;
	return ErrorCode_OK;

}

// Motiveに接続
int CreateClient(int iConnectionType)
{
	// もしすでに接続していた場合は新たに接続を作り直すため、前回分を破棄する
	if (theClient)
	{
		theClient->Uninitialize();
		delete theClient;
	}

	// クライアントをnewする
	theClient = new NatNetClient(iConnectionType);
	
	// サーバからメッセージを受けたときに呼び出される関数をセット
	theClient->SetMessageCallback(MessageHandler);//NatNetAPIの中で起きたエラーメッセージを受けたときの受け取り関数
	theClient->SetVerbosityLevel(Verbosity_Debug);//NatNetAPIの中で起きたエラーについてどのくらい詳しく表示するか（？）
	theClient->SetDataCallback(DataHandler, theClient);	// データ受け取り時の関数

	// newしたClientで、自分のEthernetインタフェース（IPで指定）とサーバのインタフェース（IP）を指定して接続
	int retCode = theClient->Initialize(szMyIPAddress, szServerIPAddress);

	//接続時にエラーがおきたときの処理
	if (retCode != ErrorCode_OK)
	{
		printf("Unable to connect to server.  Error code: %d. Exiting", retCode);
		return ErrorCode_Internal;
	}

	return ErrorCode_OK;
}

// NatNetさんがデータを受信した際に呼び出される
// ぶっちゃけ、使わない部分、不要なデータ部分は全部消して、受け取ったデータを保存する部分だけ書けばいいのでは
void __cdecl DataHandler(sFrameOfMocapData* data, void* pUserData)
{
	/*
	Rbuffer = '0';
	WriteFile(myHComPort, pbuffer, toWriteBytes, &writeBytes, NULL);
	ReadFile(myHComPort, &buffer, toReadBytes, &readBytes, NULL);
	buffer[readBytes] = '\0';

	fout2 << buffer << endl;
	*/
	/*
	ReadFile(myHComPort, &buffer, toReadBytes, &readBytes, NULL);
	buffer[readBytes] = '\0';
	fout2 << buffer << endl;
	//printf("\n%s", buffer);
	Rbuffer = '0';
	WriteFile(myHComPort, pbuffer, toWriteBytes, &writeBytes, NULL);
	//Sleep(70);
	*/

	NatNetClient* pClient = (NatNetClient*)pUserData;

	int i = 0;

//ここから、Motiveから送られてくる構造体の展開のしかたを延々と書いている

	printf("FrameID : %d\n", data->iFrame);
	printf("Timestamp :  %3.2lf\n", data->fTimestamp);
	printf("Latency :  %3.2lf\n", data->fLatency);
	
	// FrameOfMocapData params
	bool bIsRecording = ((data->params & 0x01) != 0);
	bool bTrackedModelsChanged = ((data->params & 0x02) != 0);
	if (bIsRecording)
		printf("RECORDING\n");
	if (bTrackedModelsChanged)
		printf("Models Changed.\n");


	// timecode - for systems with an eSync and SMPTE timecode generator - decode to values
	int hour, minute, second, frame, subframe;
	bool bValid = pClient->DecodeTimecode(data->Timecode, data->TimecodeSubframe, &hour, &minute, &second, &frame, &subframe);
	// decode to friendly string
	char szTimecode[128] = "";
	pClient->TimecodeStringify(data->Timecode, data->TimecodeSubframe, szTimecode, 128);
	printf("Timecode : %s\n", szTimecode);

	// Other Markers
	printf("Other Markers [Count=%d]\n", data->nOtherMarkers);
	for (i = 0; i < data->nOtherMarkers; i++)
	{
		printf("Other Marker %d : %3.2f\t%3.2f\t%3.2f\n",
			i,
			data->OtherMarkers[i][0],
			data->OtherMarkers[i][1],
			data->OtherMarkers[i][2]);
	}
	// Rigid Bodies
	printf("Rigid Bodies [Count=%d]\n", data->nRigidBodies);
	for (i = 0; i < data->nRigidBodies; i++)
	{
		// params
		// 0x01 : bool, rigid body was successfully tracked in this frame
		bool bTrackingValid = data->RigidBodies[i].params & 0x01;

		printf("Rigid Body [ID=%d  Error=%3.2f  Valid=%d]\n", data->RigidBodies[i].ID, data->RigidBodies[i].MeanError, bTrackingValid);
		printf("\tx\ty\tz\tqx\tqy\tqz\tqw\n");
		printf("\t%3.2f\t%3.2f\t%3.2f\t%3.2f\t%3.2f\t%3.2f\t%3.2f\n",
			data->RigidBodies[i].x,
			data->RigidBodies[i].y,
			data->RigidBodies[i].z,
			data->RigidBodies[i].qx,
			data->RigidBodies[i].qy,
			data->RigidBodies[i].qz,
			data->RigidBodies[i].qw);
		fout << data->fTimestamp << ",," << data->RigidBodies[0].x << "," << data->RigidBodies[0].y << "," << data->RigidBodies[0].z << "," << data->RigidBodies[0].qx << "," << data->RigidBodies[0].qy << "," << data->RigidBodies[0].qz << "," << data->RigidBodies[0].qw << endl;
		printf("\tRigid body markers [Count=%d]\n", data->RigidBodies[i].nMarkers);
		for (int iMarker = 0; iMarker < data->RigidBodies[i].nMarkers; iMarker++)
		{
			printf("\t\t");
			if (data->RigidBodies[i].MarkerIDs)
				printf("MarkerID:%d", data->RigidBodies[i].MarkerIDs[iMarker]);
			if (data->RigidBodies[i].MarkerSizes)
				printf("\tMarkerSize:%3.2f", data->RigidBodies[i].MarkerSizes[iMarker]);
			if (data->RigidBodies[i].Markers)
				printf("\tMarkerPos:%3.2f,%3.2f,%3.2f\n",
				data->RigidBodies[i].Markers[iMarker][0],
				data->RigidBodies[i].Markers[iMarker][1],
				data->RigidBodies[i].Markers[iMarker][2]);
		}
	}

	// skeletons
	printf("Skeletons [Count=%d]\n", data->nSkeletons);
	for (i = 0; i < data->nSkeletons; i++)
	{
		sSkeletonData skData = data->Skeletons[i];
		printf("Skeleton [ID=%d  Bone count=%d]\n", skData.skeletonID, skData.nRigidBodies);
		for (int j = 0; j< skData.nRigidBodies; j++)
		{
			sRigidBodyData rbData = skData.RigidBodyData[j];
			printf("Bone %d\t%3.2f\t%3.2f\t%3.2f\t%3.2f\t%3.2f\t%3.2f\t%3.2f\n",
				rbData.ID, rbData.x, rbData.y, rbData.z, rbData.qx, rbData.qy, rbData.qz, rbData.qw);

			printf("\tRigid body markers [Count=%d]\n", rbData.nMarkers);
			for (int iMarker = 0; iMarker < rbData.nMarkers; iMarker++)
			{
				printf("\t\t");
				if (rbData.MarkerIDs)
					printf("MarkerID:%d", rbData.MarkerIDs[iMarker]);
				if (rbData.MarkerSizes)
					printf("\tMarkerSize:%3.2f", rbData.MarkerSizes[iMarker]);
				if (rbData.Markers)
					printf("\tMarkerPos:%3.2f,%3.2f,%3.2f\n",
					data->RigidBodies[i].Markers[iMarker][0],
					data->RigidBodies[i].Markers[iMarker][1],
					data->RigidBodies[i].Markers[iMarker][2]);
			}
		}
	}

	// labeled markers
	bool bOccluded;     // marker was not visible (occluded) in this frame
	bool bPCSolved;     // reported position provided by point cloud solve
	bool bModelSolved;  // reported position provided by model solve
	printf("Labeled Markers [Count=%d]\n", data->nLabeledMarkers);
	for (i = 0; i < data->nLabeledMarkers; i++)
	{
		bOccluded = ((data->LabeledMarkers[i].params & 0x01) != 0);
		bPCSolved = ((data->LabeledMarkers[i].params & 0x02) != 0);
		bModelSolved = ((data->LabeledMarkers[i].params & 0x04) != 0);
		sMarker marker = data->LabeledMarkers[i];
		int modelID, markerID;
		theClient->DecodeID(marker.ID, &modelID, &markerID);
		printf("Labeled Marker [ModelID=%d, MarkerID=%d, Occluded=%d, PCSolved=%d, ModelSolved=%d] [size=%3.2f] [pos=%3.2f,%3.2f,%3.2f]\n",
			modelID, markerID, bOccluded, bPCSolved, bModelSolved, marker.size, marker.x, marker.y, marker.z);
	}
	//Sleep(100);
//ここまで
}

// NatNetのエラーメッセージを受け取ったときどうするか
void __cdecl MessageHandler(int msgType, char* msg)
{
	printf("\n%s\n", msg);
}


