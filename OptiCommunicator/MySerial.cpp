/*
* MySerial.cpp
* Visual Studio���ŃV���A���ʐM���T�|�[�g����N���X�ł��B
* VC++��CLR�v���W�F�N�g��C#��VB���痘�p�\�ł��B
* ����Ă����ĂȂ�ł����A�V���A���ʐM�N���X��C#�ō���������ǂ������Ȃ��B
* 
* K. Morishita @ Kumamoto-Univ. 2012
* 
* 201212/22 �o�C�i���̂���肪�ł���悤�ɉ��������BFIFO�o�b�t�@��t�����̂ł����Ԃ�Ƃ��₷���Ȃ����B
*           string�^�ւ̌^�ϊ��ł́A�f�t�H���g�ł�euc-jp�ŃG���R�[�f�B���O���Ă��܂��B
*           ���[�U�̓s���ɍ��킹�ăt�H�[�}�b�g�͕ύX���Ă��������B
*           �Ȃ��AASCII�����i�A���t�@�x�b�g�ƍŒ���̋L���̏W���j�̒ʐM�ł���Έӎ�����K�v������܂���B
*           ���̓��t�H�[�}�b�g���w��ł���悤�ɂ������ł����A�Ή������͖���ł��B
*/
#include "stdafx.h"
#include "MySerial.h"

// �R���X�g���N�^
// �|�[�g�̏����������s���܂�
MySerial::MySerial(String^ com, int baudrate)
{
	this->port = gcnew SerialPort();
	this->port->PortName = com;
	this->port->BaudRate = baudrate;
	this->port->DataReceived += gcnew SerialDataReceivedEventHandler(this, &MySerial::DataReceivedHandler);// �Ăяo���֐���o�^����B�C���X�^���X�̋�ʂƊ֐��̋�ʂ̗���������Bstatic�֐��Ȃ�֐��������ł������ǁB
	this->port->Open();
	this->size  = 300;
	this->buff  = gcnew array <Byte>(this->size);
	this->wp    = 0;
	this->rp    = 0;
}
// �f�X�R���X�g���N�^
MySerial::~MySerial(void)
{

}
// ��M�f�[�^����Ԃ�
int MySerial::GetLength(void)
{
	int t = this->wp - this->rp;
	if(t < 0) t += this->size;
	return t;
}
// string����byte�̃A���[�ւ̕ϊ�
array<System::Byte>^ MySerial::StringToByteArray(String^ src, String^ name)
{
    if ( name == "" ) name = "euc-jp";

    System::Text::Encoding^ enc = System::Text::Encoding::GetEncoding(name);
    return enc->GetBytes(src);
}
// byte�̃A���[����string�ւ̕ϊ�
String^ MySerial::ByteArrayToString(array<System::Byte>^ src, String^ name)
{
    if ( name == "" ) name = "euc-jp";

    System::Text::Encoding^ enc = System::Text::Encoding::GetEncoding(name);
    return enc->GetString(src);
}
// ��M�f�[�^��ǂݍ���
String^ MySerial::Read()
{
	String^ copybuff = "";
	int size = this->GetLength();
	
	if(size > 0)
	{
		array <Byte>^ copy  = gcnew array <Byte>(size);
		for(int i = 0; i < size; i++)
		{
			int t = (this->rp + 1) % this->size;
			copy[i] = this->buff[this->rp];
			if(t != this->wp)
				this->rp = t;
			else
				break;	// ���������s����邱�Ƃ�����悤���ƁA���ɂ܂���
		}
		copybuff = this->ByteArrayToString(copy, "");
	}
	return copybuff;
}
// ��M�f�[�^��ǂݍ���
array <Byte>^ MySerial::ReadByte()
{
	array <Byte>^ copy = gcnew array <Byte>(0);
	int size = this->GetLength();
	
	if(size > 0)
	{
		copy  = gcnew array <Byte>(size);
		for(int i = 0; i < size; i++)
		{
			int t = (this->rp + 1) % this->size;
			copy[i] = this->buff[this->rp];
			if(t != this->wp)
				this->rp = t;
			else
				break;	// ���������s����邱�Ƃ�����悤���ƁA���ɂ܂���
		}
	}
	return copy;
}
// ���M����
void MySerial::Write(String^ msg)
{
	this->port->Write(msg);
	return;
}
// ���M����
void MySerial::Write(array <Byte>^ sendBuff)
{
	this->port->Write(sendBuff, 0, sendBuff->Length);
	return;
}
// ���M���� ���s�R�[�h�t��
void MySerial::WriteLine(String^ msg)
{
	this->port->WriteLine(msg);
	return;
}
// �|�[�g�����
void MySerial::Close()
{
	this->port->Close();
}
// �f�[�^��M����
void MySerial::DataReceivedHandler( Object^ sender, SerialDataReceivedEventArgs^ e)
{
	SerialPort^ sp = (SerialPort^)sender;
	int size = sp->BytesToRead;

	for(int k = 0; k < size; k++)
	{
		int t = (this->wp + 1) % this->size;
		this->buff[this->wp] = sp->ReadByte();
		if(t != this->rp) 
			this->wp = t;
		else
			break;
	}
}
