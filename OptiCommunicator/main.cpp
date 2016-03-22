/*------------/

NatNet API���g����Motive�Ƃ̒ʐM�T���v��

�y���Ӂz���Motive��DataStreaming���J�n���Ă�������

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

//--------------�v���g�^�C�v�錾
void __cdecl DataHandler(sFrameOfMocapData* data, void* pUserData);		// �f�[�^���󂯎�����ۂɌĂ΂��R�[���o�b�N
void __cdecl MessageHandler(int msgType, char* msg);		            // NatNetAPI���G���[���͂����ۂɌĂ΂��R�[���o�b�N
int CreateClient(int iConnectionType); //�N���C�A���g�쐬�֐��i�������j

//--------------�O���[�o���ϐ�
unsigned int MyServersDataPort = 1511;
unsigned int MyServersCommandPort = 1510;
int iConnectionType = ConnectionType_Multicast;
NatNetClient* theClient;
char szMyIPAddress[128] = "";//�g�p���鎩���̃C���^�t�F�[�X�ɑΉ�����IP�A�h���X
char szServerIPAddress[128] = "";//�T�[�o��IP�A�h���X�iMotive�́ADataStreaming View�ɂ���܂��j

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



char buffer[2048] = {0};         // �f�[�^��ǂݍ��ވׂ̃o�b�t�@
//BYTE* RecData = &buffer;
DWORD toReadBytes = 2048;  // �Ǎ��ݎ��̎w��o�C�g�� 
DWORD readBytes;           // ���ۂɓǂݍ��񂾃o�C�g�� 
char* szStr = {0};
char* ctx = { 0 };

BYTE Rbuffer;
BYTE* pbuffer = &Rbuffer;			// ���M�f�[�^���i�[����o�b�t�@

DWORD toWriteBytes = 10;      // ���M�������o�C�g�� 
DWORD writeBytes;       // ���ۂɑ��M���ꂽ�o�C�g�� 

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
	// �|�[�g�̃{�[���[�g�A�p���e�B����ݒ� 
	DCB config;
	config.DCBlength = sizeof(DCB);
	config.BaudRate = CBR_57600;
	config.ByteSize = 8;
	config.fNull = TRUE;
	config.fParity = TRUE;	
	config.Parity = NOPARITY;
	config.StopBits = TWOSTOPBITS;
	config.EofChar = 'N';
	//�n�[�h�E�F�A�t���[����
	config.fOutxCtsFlow = FALSE;       // �@CTS�n�[�h�E�F�A�t���[����FCTS������g�p���Ȃ��ꍇ��FLASE���w��
	//   �@�@�@�@�@�@CTS���������ꍇ��TRUE���w�肵��CTS�M�����Ď����܂��B
	config.fOutxDsrFlow = FALSE;       //  DSR�n�[�h�E�F�A�t���[����F�g�p���Ȃ��ꍇ��FALSE���w��
	config.fDtrControl = DTR_CONTROL_DISABLE;// DTR�L��/�����F�@�����Ȃ�@DTR_CONTROL_DISABLE;ISABLE
	config.fRtsControl = RTS_CONTROL_DISABLE;  // RTS����F�@RTS��������Ȃ��ꍇ��RTS_CONTROL_DISABLE���w��
	//�@�@�@�@�@�@�@�@ RTS���������ꍇ��RTS_CONTROL_ENABLE���w��@�@��

	// �\�t�g�E�F�A�t���[����
	config.fOutX = FALSE;              // ���M��XON/OFF����̗L���F�@�Ȃ���FLALSE
	config.fInX = FALSE;               // ��M��XON/XOFF����̗L���F�Ȃ���FALSE
	config.fTXContinueOnXoff = TRUE;   // ��M�o�b�t�@�[���t��XOFF��M��̌p�����M�ہF���M��TRUE
	config.XonLim = 512;               // XON��������܂łɊi�[�ł���ŏ��o�C�g���F
	config.XoffLim = 512;              // XOFF��������܂łɊi�[�ł���ŏ��o�C�g���F
	config.XonChar = 0x11;             // ���M��XON���� ( ���M�F�r�W�B���� ) �̎w��F
	//�@��ʂɁAXON�����Ƃ���11H ( �f�o�C�X����P�FDC1 )���悭�g���܂�
	config.XoffChar = 0x13;            // XOFF�����i���M�s�F�r�W�[�ʍ��j�̎w��F�Ȃ���FALSE
	//�@��ʂɁAXOFF�����Ƃ���13H ( �f�o�C�X����3�FDC3 )���悭�g���܂�

	// Parity�AStopBits�ADataBits�����l�ɐݒ� 
	SetCommState(myHComPort, &config);
	SetupComm(	//�ݒ�
		myHComPort,	// �@�ʐM�f�o�C�X�̃n���h���FCreateFile()�Ŏ擾�����n���h�����w��
		512,   //   ��M�o�b�t�@�[�T�C�Y�F�@��M�̃o�b�t�@�[�T�C�Y���o�C�g�P�ʂŎw��
		512    // �@���M�o�b�t�@�[�T�C�Y�F�@���M�̃o�b�t�@�[�T�C�Y���o�C�g�P�ʂŎw��
		);
	COMMTIMEOUTS timeout;
	timeout.ReadIntervalTimeout = 5; // �����Ǎ����̂Q�����Ԃ̑S�̑҂����ԁimsec�j

	timeout.ReadTotalTimeoutMultiplier = 10; //�Ǎ��̂P����������̎���
	timeout.ReadTotalTimeoutConstant = 100; //�Ǎ��G���[���o�p�̃^�C���A�E�g����
	//(��M�g�[�^���^�C���A�E�g) = ReadTotalTimeoutMultiplier * (��M�\��o�C�g��) + ReadTotalTimeoutConstant 

	timeout.WriteTotalTimeoutMultiplier = 0; //�������݂P����������̑҂�����
	timeout.WriteTotalTimeoutConstant = 500;//�������݃G���[���o�p�̃^�C���A�E�g����
	SetCommTimeouts(myHComPort, &timeout);//SetCommTimeOut()�֐��Ƀ|�[�g�n���h�������COMMTIMEOUTS�\���̂�
	//�A�h���X�������܂��B
	//(���M�g�[�^���^�C���A�E�g) = WriteTotalTimeoutMultiplier * (���M�\��o�C�g��) + WriteTotalTimeoutConstant 
	/*
	BYTE buffer[1024];         // �f�[�^��ǂݍ��ވׂ̃o�b�t�@ 
	DWORD toReadBytes = 1024;  // �Ǎ��ݎ��̎w��o�C�g�� 
	DWORD readBytes;           // ���ۂɓǂݍ��񂾃o�C�g�� 
	
	BYTE Rbuffer;
	BYTE* pbuffer = &Rbuffer;			// ���M�f�[�^���i�[����o�b�t�@
	
	DWORD toWriteBytes = 2;      // ���M�������o�C�g�� 
	DWORD writeBytes;       // ���ۂɑ��M���ꂽ�o�C�g�� 
	*/
	
	EscapeCommFunction(
		myHComPort,    // �@�ʐM�f�o�C�X�̃n���h���FCreateFile()�Ŏ擾�����n���h�����w��
		SETRTS // ��M�\�ł��邱�Ƃ𑊎葤�Ɏ����FRTS���A�N�e�B�u�ɂ��遨SETRTS
		//�@�@�@�@�@�@�i�Q�l�j   RTS���A�N�e�B�u�ɂ��遨CLRRTS�@
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
					myHComPort   // �@�ʐM�f�o�C�X�̃n���h���F�@CreateFile()�Ŏ擾�����n���h�����w��
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

// Motive�ɐڑ�
int CreateClient(int iConnectionType)
{
	// �������łɐڑ����Ă����ꍇ�͐V���ɐڑ�����蒼�����߁A�O�񕪂�j������
	if (theClient)
	{
		theClient->Uninitialize();
		delete theClient;
	}

	// �N���C�A���g��new����
	theClient = new NatNetClient(iConnectionType);
	
	// �T�[�o���烁�b�Z�[�W���󂯂��Ƃ��ɌĂяo�����֐����Z�b�g
	theClient->SetMessageCallback(MessageHandler);//NatNetAPI�̒��ŋN�����G���[���b�Z�[�W���󂯂��Ƃ��̎󂯎��֐�
	theClient->SetVerbosityLevel(Verbosity_Debug);//NatNetAPI�̒��ŋN�����G���[�ɂ��Ăǂ̂��炢�ڂ����\�����邩�i�H�j
	theClient->SetDataCallback(DataHandler, theClient);	// �f�[�^�󂯎�莞�̊֐�

	// new����Client�ŁA������Ethernet�C���^�t�F�[�X�iIP�Ŏw��j�ƃT�[�o�̃C���^�t�F�[�X�iIP�j���w�肵�Đڑ�
	int retCode = theClient->Initialize(szMyIPAddress, szServerIPAddress);

	//�ڑ����ɃG���[���������Ƃ��̏���
	if (retCode != ErrorCode_OK)
	{
		printf("Unable to connect to server.  Error code: %d. Exiting", retCode);
		return ErrorCode_Internal;
	}

	return ErrorCode_OK;
}

// NatNet���񂪃f�[�^����M�����ۂɌĂяo�����
// �Ԃ����Ⴏ�A�g��Ȃ������A�s�v�ȃf�[�^�����͑S�������āA�󂯎�����f�[�^��ۑ����镔�����������΂����̂ł�
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

//��������AMotive���瑗���Ă���\���̂̓W�J�̂����������X�Ə����Ă���

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
//�����܂�
}

// NatNet�̃G���[���b�Z�[�W���󂯎�����Ƃ��ǂ����邩
void __cdecl MessageHandler(int msgType, char* msg)
{
	printf("\n%s\n", msg);
}


