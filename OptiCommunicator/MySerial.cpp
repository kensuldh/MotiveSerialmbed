/*
* MySerial.cpp
* Visual Studio環境でシリアル通信をサポートするクラスです。
* VC++のCLRプロジェクトやC#やVBから利用可能です。
* 作っておいてなんですが、シリアル通信クラスはC#で作った方が良かったなぁ。
* 
* K. Morishita @ Kumamoto-Univ. 2012
* 
* 201212/22 バイナリのやり取りができるように改造した。FIFOバッファを付けたのでずいぶんとやりやすくなった。
*           string型への型変換では、デフォルトではeuc-jpでエンコーディングしています。
*           ユーザの都合に合わせてフォーマットは変更してください。
*           なお、ASCII文字（アルファベットと最低限の記号の集合）の通信であれば意識する必要がありません。
*           その内フォーマットを指定できるようにするつもりですが、対応時期は未定です。
*/
#include "stdafx.h"
#include "MySerial.h"

// コンストラクタ
// ポートの初期化を実行します
MySerial::MySerial(String^ com, int baudrate)
{
	this->port = gcnew SerialPort();
	this->port->PortName = com;
	this->port->BaudRate = baudrate;
	this->port->DataReceived += gcnew SerialDataReceivedEventHandler(this, &MySerial::DataReceivedHandler);// 呼び出す関数を登録する。インスタンスの区別と関数の区別の両方がいる。static関数なら関数名だけでいいけど。
	this->port->Open();
	this->size  = 300;
	this->buff  = gcnew array <Byte>(this->size);
	this->wp    = 0;
	this->rp    = 0;
}
// デスコンストラクタ
MySerial::~MySerial(void)
{

}
// 受信データ数を返す
int MySerial::GetLength(void)
{
	int t = this->wp - this->rp;
	if(t < 0) t += this->size;
	return t;
}
// stringからbyteのアレーへの変換
array<System::Byte>^ MySerial::StringToByteArray(String^ src, String^ name)
{
    if ( name == "" ) name = "euc-jp";

    System::Text::Encoding^ enc = System::Text::Encoding::GetEncoding(name);
    return enc->GetBytes(src);
}
// byteのアレーからstringへの変換
String^ MySerial::ByteArrayToString(array<System::Byte>^ src, String^ name)
{
    if ( name == "" ) name = "euc-jp";

    System::Text::Encoding^ enc = System::Text::Encoding::GetEncoding(name);
    return enc->GetString(src);
}
// 受信データを読み込む
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
				break;	// ここが実行されることがあるようだと、非常にまずい
		}
		copybuff = this->ByteArrayToString(copy, "");
	}
	return copybuff;
}
// 受信データを読み込む
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
				break;	// ここが実行されることがあるようだと、非常にまずい
		}
	}
	return copy;
}
// 送信する
void MySerial::Write(String^ msg)
{
	this->port->Write(msg);
	return;
}
// 送信する
void MySerial::Write(array <Byte>^ sendBuff)
{
	this->port->Write(sendBuff, 0, sendBuff->Length);
	return;
}
// 送信する 改行コード付き
void MySerial::WriteLine(String^ msg)
{
	this->port->WriteLine(msg);
	return;
}
// ポートを閉じる
void MySerial::Close()
{
	this->port->Close();
}
// データ受信処理
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
