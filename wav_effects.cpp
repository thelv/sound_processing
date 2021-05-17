#include "pch.h"
#include <iostream>

#define WAV_HEADER_SIZE 44
#define BUF_LEN 216282
#define VALUES_PER_SEC 44100;
# define PI 3.14159265358979323846

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

template <typename type> void distortion_log(type* buf, int len, type* buf_, type max_val)
{
	float max_val_log=-log(max_val);
	for(int i=0; i<len; i++)
	{
		type v=buf[i];
		if(v>max_val)
		{
			buf_[i]=max_val+(log(v)+max_val_log)*max_val;
		}
		else if(v<-max_val)
		{
			buf_[i]=-max_val-(log(-v)+max_val_log)*max_val;
		}
		else
		{
			buf_[i]=buf[i];
		}		
	}
}

template <typename type> void distortion_RC(type* buf, int len, type* buf_, type max_val)
{
	float max_val_log=-log(max_val);
	float q=0;
	for(int i=0; i<len; i++)
	{
		type v=buf[i];
		if(v>max_val)
		{
			buf_[i]=max_val+(log(v)+max_val_log)*max_val;
		}
		else if(v<-max_val)
		{
			buf_[i]=-max_val-(log(-v)+max_val_log)*max_val;
		}
		else
		{
			buf_[i]=buf[i];
		}		
	}
}

template <typename type> void distortion_hard(type* buf, int len, type* buf_, type max_val)
{
	float max_val_log=-log(max_val);
	float q=0;
	for(int i=0; i<len; i++)
	{
		type v=buf[i];
		if(v>max_val)
		{
			buf_[i]=max_val;
		}
		else if(v<-max_val)
		{
			buf_[i]=-max_val;
		}
		else
		{
			buf_[i]=buf[i];
		}		
	}
}

template <typename type> void distortion_inverse(type* buf, int len, type* buf_, type max_val)
{
	float max_val_float=max_val;
	float q=0;
	for(int i=0; i<len; i++)
	{
		type v=buf[i];
		if(v>max_val)
		{
			buf_[i]=max_val*(2-max_val_float/v);
		}
		else if(v<-max_val)
		{
			buf_[i]=-max_val*(2+max_val_float/v);
		}
		else
		{
			buf_[i]=buf[i];
		}		
	}
}

template <typename type> void gen_sin(type* buf, int len, float freq, float scale)
{	
	type max_val;
	memset(&max_val, 0xffffffff, sizeof(type));
	*((char*) &max_val+sizeof(type)-1)=0x7f;
	max_val=max_val*scale;

	float w=2*PI*freq/VALUES_PER_SEC;

	for(int i=0; i<len; i++)
	{
			buf[i]=max_val*sin(w*i);
	}
}

int main()
{	
	short buf[BUF_LEN], buf_[BUF_LEN];
	memset(buf_, 0, BUF_LEN*sizeof(*buf_));

	wav_read("sample.wav", buf, BUF_LEN);

	amp(buf, BUF_LEN, buf, 10);
	RC_low(buf, BUF_LEN, buf, 0.1);
	RC_high(buf, BUF_LEN, buf, 0.1);
	//pitch(buf, BUF_LEN, buf_, 0.5);
	//RC_low(buf, BUF_LEN, buf, 0.1);
	//RC_high(buf, BUF_LEN, buf, 0.1);
	//amp(buf_, BUF_LEN, buf_, 1);

	//distortion_log(buf, BUF_LEN, buf, (short) (SHRT_MAX/202));
	distortion_inverse(buf, BUF_LEN, buf, (short) (SHRT_MAX/60));
	//distortion_log(buf, BUF_LEN, buf, (short) (SHRT_MAX/52));
	//RC_low(buf, BUF_LEN, buf, 0.2);
	//RC_high(buf, BUF_LEN, buf, 0.05);
	amp(buf, BUF_LEN, buf, 24);

	//gen_sin(buf, BUF_LEN, 1000, 0.5);

	//distortion_inverse(buf, BUF_LEN, buf, (short) (SHRT_MAX/4));

	wav_write("sample_.wav", buf, BUF_LEN);

	/*std::cout << 0;

	float s=0;
	for(int i=0; i<=44100*1000; i++)
	{
		s+=log(i);
	}

	std::cout << s;

	
	float s=0;
	float s1=0;
	float s2=0;
	float s3=0;
	for(int i=0; i<=44100*250; i++)
	{
		s+=log(i);
		s1+=log(i+1);
		s2+=log(i+2);
		s3+=log(i+3);
	}

	std::cout << s;
	std::cout << s1;
	std::cout << s2;
	std::cout << s3;*/
}