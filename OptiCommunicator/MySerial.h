#pragma once

#using <System.dll>

using namespace System;
using namespace System::IO::Ports;

public ref class MySerial
{
private:
	SerialPort^   port;		// �V���A���|�[�g�������Q�ƌ^�N���X
	array <Byte>^ buff;		// ��M�f�[�^���i�[���镶����ϐ�
	int	          rp;
	int           wp;
	int	          size;
public:
	MySerial(String^, int);
	~MySerial(void);
	String^ Read();
	array <Byte>^ ReadByte();
	void Write(String^);
	void Write(array <Byte>^ sendBuff);
	void WriteLine(String^);
	void Close();
private:
	int                  GetLength();
	String^              ByteArrayToString(array<System::Byte>^ src, String^ name);
	array<System::Byte>^ StringToByteArray(String^ src, String^ name);
	void                 DataReceivedHandler(Object^ sender, SerialDataReceivedEventArgs^ e);
};

