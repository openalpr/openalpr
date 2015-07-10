#include "stdafx.h"
#include "bitmapmat-net.h"

using namespace openalprnet;

BitmapMat::BitmapMat(array<Byte>^ byteArray)
{
	this->m_bitmap = ByteArrayToMat(byteArray);
}

BitmapMat::BitmapMat(Bitmap^ bitmap)
{
	this->m_bitmap = BitmapToMat(bitmap);
}

BitmapMat::BitmapMat(MemoryStream^ memoryStream)
{
	this->m_bitmap = MemoryStreamBitmapToMat(memoryStream);
}

BitmapMat::BitmapMat(String^ filename)
{
	Bitmap^ bitmap = gcnew Bitmap(filename);
	this->m_bitmap = BitmapToMat(bitmap);
	delete bitmap;
}

