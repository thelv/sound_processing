#include "pch.h"
#include <iostream>

#define WAV_HEADER_SIZE 44
#define BUF_LEN 216282

template <typename type> void wav_read(const char* file_name, type* buf, int len)
{
	FILE* f=fopen(file_name, "rb");
	fseek(f, WAV_HEADER_SIZE, SEEK_SET);
	fread(buf, sizeof(type), len, f);
	fclose(f);
}

template <typename type> void wav_write(const char* file_name, type* buf, int len)
{
	FILE* f=fopen(file_name, "rb+");
	fseek(f, WAV_HEADER_SIZE, SEEK_SET);
	fwrite(buf, sizeof(type), len, f);
	fclose(f);
}

template <typename type> void amp(type* buf, int len, type* buf_, float scale)
{
	for(int i=0; i<len; i++)
	{
		buf_[i]=(float) buf[i]*scale;
	}
}

template <typename type> void pitch(type* buf, int len, type* buf_, float scale)
{
	int len_=len*scale;
	int scale_=1/scale;
	for(int i_=0; i_<len_; i_++)
	{
		float i=i_*scale_;
		int iCeil=i;
		if(i==iCeil)
		{
			buf_[i_]=buf[iCeil];
		}
		else
		{
			float k=i-iCeil;
			buf_[i_]=buf[iCeil]*(1-k)+buf[iCeil+1]*k;
		}
	}
}

template <typename type> void RC_low(type* buf, int len, type* buf_, float p)
{
	double q=0;
	for(int i=0; i<len; i++)
	{
		q+=(buf[i]-q)*p;
		buf_[i]=q;
	}
}

template <typename type> void RC_high(type* buf, int len, type* buf_, float p)
{
	double q=0;
	for(int i=0; i<len; i++)
	{
		int cur=buf[i]-q;
		q+=cur*p;
		buf_[i]=cur;
	}
}

template <typename type> void convolusion(type* buf, int len, type* IR, int IRlen, type* buf_)
{

}

int main()
{	
	short buf[BUF_LEN], buf_[BUF_LEN];
	memset(buf_, 0, BUF_LEN*sizeof(*buf_));

	wav_read("sample.wav", buf, BUF_LEN);

	amp(buf, BUF_LEN, buf, 2);
	pitch(buf, BUF_LEN, buf_, 0.5);
	RC_high(buf_, BUF_LEN, buf_, 0.5);
	amp(buf_, BUF_LEN, buf_, 1);

	wav_write("sample_.wav", buf_, BUF_LEN);
}