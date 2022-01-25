#include "pch.h"
#include <iostream>
#include <algorithm>
#include <string>
#include "json.hpp"
#include <regex>

#define WAV_HEADER_SIZE 44
#define BUF_LEN 216282
#define IR_LEN (353236-44)/2
//125044
#define VALUES_PER_SEC 44100;
#define PI 3.14159265358979323846

using json = nlohmann::json;

char* readFullFile(char* fileName, int* len)
{
	FILE* f;
	fopen_s(&f, fileName, "r");
	fseek(f, 0, SEEK_END);	
	int fileSize=ftell(f);
	fseek(f, 0, SEEK_SET);

	char* buf=new char[fileSize];
	fread(buf, 1, fileSize, f);

	*len=fileSize;
	return buf;
}

std::string charArrayToString(char* buf, int len)
{
	char* buf_=new char[len+1];
	memcpy(buf_, buf, len);
	buf_[len]=0;

	std::string s=std::string(buf_);
	free(buf_);
	return s;
}

void writeFillFile(const char* fileName, std::string s)
{
	FILE* f;
	fopen_s(&f, fileName, "w+");
	fwrite(s.c_str(), 1, s.length(), f);
}

std::string readFullFile(char* fileName)
{
	int len;
	char* buf=readFullFile(fileName, &len);
	std::string res=charArrayToString(buf, len);
	free(buf);
	return res;
}

template <typename type> void wav_read(const char* file_name, type* buf, int len)
{
	FILE* f=fopen(file_name, "rb");
	fseek(f, WAV_HEADER_SIZE, SEEK_SET);
	fread(buf, sizeof(type), len, f);
	fclose(f);
}

char* wav_read(const char* file_name, int* len)
{
	FILE* f=fopen(file_name, "rb");
	fseek(f, 0, SEEK_END);
	*len=ftell(f)-WAV_HEADER_SIZE;
	char* buf=new char[*len];
	fseek(f, WAV_HEADER_SIZE, SEEK_SET);
	fread(buf, 1, *len, f);
	fclose(f);
	return buf;
}

void wav_read(const char* file_name, void* buf, int* len)
{
	FILE* f=fopen(file_name, "rb");
	fseek(f, 0, SEEK_END);
	*len=ftell(f)-WAV_HEADER_SIZE;
	fseek(f, WAV_HEADER_SIZE, SEEK_SET);
	fread(buf, 1, *len, f);
	fclose(f);
}

char* wav_header_create(int len)
{
	char* buf=new char[WAV_HEADER_SIZE];	
	
	memcpy(buf, (char*) "RIFF", 4);
	buf+=4;

	*((int*) buf)=len+36;
	buf+=4;

	memcpy(buf, (char*) "WAVEfmt ", 8);
	buf+=8;

	*((int*) buf)=16;
	buf+=4;

	*((short*) buf)=1;
	buf+=2;

	*((short*) buf)=1;
	buf+=2;

	*((int*) buf)=44100;
	buf+=4;

	*((int*) buf)=44100*3;
	buf+=4;

	*((short*) buf)=3;
	buf+=2;

	*((short*) buf)=24;
	buf+=2;

	memcpy(buf, (char*) "data", 4);
	buf+=4;

	*((int*) buf)=len;
	buf+=4;

	return buf-WAV_HEADER_SIZE;
}

template <typename type> void wav_write(const char* file_name, type* buf, int len)
{
	FILE* f=fopen(file_name, "wb");
	//fseek(f, WAV_HEADER_SIZE, SEEK_SET);
	fwrite(wav_header_create(len*sizeof(type)), 1, WAV_HEADER_SIZE, f);
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
	for(int i=0; i<len; i++)
	{
		int s=0;
		for(int j=std::max(0, i-IRlen-1); j<=i; j++)
		{
			s+=buf[j]*IR[i-j];
		}
		//memcpy((type*) &s+sizeof(int)/sizeof(type)-1, buf_+i, sizeof(type));
		buf_[i]=s/32768;
	}
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

struct Samples_cut_sample 
{
	char* data;
	int len;
	int volumeMax;
	int string;
	int fret;
	int pitch;
};

int samples_cut_cmp(const Samples_cut_sample* s1, const Samples_cut_sample* s2)
{
	return s1->volumeMax-s2->volumeMax;
}

int convert_24_to_32_bit(char* data)
{
	int a=0;
	memcpy((char*) &a+1, data, 3);
	return a;
}

int* samples_find_cut_points(const char* file_name, int* len_return)
{
	int* cut_points=new int[1000];
	int cut_points_n=0;

	cut_points[cut_points_n++]=0;
	
	int len;
	char* buf=wav_read(((std::string) file_name+".wav").c_str(), &len);

	int samples_len=len/3;
	int cursor=0;

	while(true)
	{
		//wait silence
		int silence_count=0;
		while(silence_count<44100)
		{
			if(abs(convert_24_to_32_bit(buf+cursor*3))<INT32_MAX/200)
			{
				silence_count++;
			}
			else
			{
				silence_count=0;
			}

			cursor++;
			if(cursor>samples_len) goto end;
		}

		//find sample start 
		while(abs(convert_24_to_32_bit(buf+cursor*3))<INT32_MAX/175)
		{			
			cursor++;
			if(cursor>samples_len) goto end;
		}

		int a=convert_24_to_32_bit(buf+cursor*3);
		bool sign=(a>0);
		while((convert_24_to_32_bit(buf+cursor*3)>0)==sign)
		{
			cursor--;
		}

		cut_points[cut_points_n++]=cursor;
	}

	end:

		cut_points[cut_points_n++]=samples_len;
		*len_return=cut_points_n-1;
		return cut_points;
}

Samples_cut_sample* samples_cut(const char* file_name, int* cut_points, int cut_points_n, json j)
{	
	Samples_cut_sample* samples=new Samples_cut_sample[cut_points_n];

	int len;
	char* buf=wav_read(((std::string) file_name+".wav").c_str(), &len);

	double k=1;//44100./48000;

	int* p=cut_points;/*[16]=
	{
		324575,
		1443320,
		2447035,
		3721047,
		4683372,
		6457727,
		8131853,
		9900124,
		11399520,
		12869071,
		14337975,
		15770952,
		17061161,
		18556342,
		21185738, 
		(int) ((float) len/k)/3
	};*/

	int samplesCount=cut_points_n-1;

	int j_i=0;
	int j_ij=0;
	for(int i=1; i!=samplesCount+1; i++)
	{	
		samples[i-1].data=buf+3*(int) (k*p[i-1]);
		samples[i-1].len=(int) (k*(p[i]-p[i-1]));//-100000;
		samples[i-1].string=j[j_i]["string"].get<int>();
		samples[i-1].fret=j[j_i]["fret"].get<int>();

		j_ij++;
		if(j_ij==j[j_i]["count"].get<int>())
		{
			j_ij=0;
			j_i++;
		}
	}	

	for(int i=0; i<samplesCount; i++)
	{
		Samples_cut_sample& sample=samples[i];

		int volumeMax=0;		
		for(int j=0; j<sample.len; j++)
		{
			int volumeCur=std::abs(convert_24_to_32_bit(sample.data+j*3));
			volumeMax=std::max(volumeMax, volumeCur);
		}

		sample.volumeMax=volumeMax;
	}

	return samples;
}

void samples_organize(Samples_cut_sample* samples, int samples_count, char* file_name)
{
	Samples_cut_sample*** samples_board=new Samples_cut_sample**[6];

	for(int i=0; i<6; i++)
	{
		samples_board[i]=new Samples_cut_sample*[128];
		for(int j=0; j<128; j++)
		{
			samples_board[i][j]=new Samples_cut_sample[100];			
		}
	}
	int samples_board_len[6][128]={0};

	for(int i=0; i<samples_count; i++)
	{
		Samples_cut_sample sample=samples[i];
		samples_board[sample.string][sample.fret][samples_board_len[sample.string][sample.fret]]=sample;
		samples_board_len[sample.string][sample.fret]++;
	}

	json ji;
	for(int i=0; i<6; i++)
	{
		json jj;
		for(int j=0; j<128; j++)
		{
			qsort(samples_board[i][j], samples_board_len[i][j], sizeof(Samples_cut_sample), (_CoreCrtNonSecureSearchSortCompareFunction) samples_cut_cmp);

			if(samples_board_len[i][j]!=0)
			{
				json jk={{"string", i}, {"note", j}};
				jk["samples"]=json::array();	
			
				for(int k=0; k<samples_board_len[i][j]; k++)
				{		
					Samples_cut_sample sample=samples_board[i][j][k];
					wav_write<char>((file_name+(std::string) "-"+std::to_string(sample.string)+"-"+std::to_string(sample.fret)+"-"+std::to_string(k)+".wav").c_str(), sample.data, sample.len*3);
					std::cout << "\n";
					std::cout << (double) sample.volumeMax/INT32_MAX;
					jk["samples"].push_back({{"volume", (float) sample.volumeMax/INT32_MAX}});
				}
				ji.push_back(jk);
			}
		}
		//ji.push_back(jj);
	}
	
	writeFillFile((file_name+(std::string)".json").c_str(), ji.dump());
	//qsort(samples, samplesCount, sizeof(Samples_cut_sample), (_CoreCrtNonSecureSearchSortCompareFunction) samples_cut_cmp);

	/*for(int i=0; i<samplesCount; i++)
	{		
		wav_write<char>((file_name+(std::string) "-"+std::to_string(j["string"].get<int>())+"-"+std::to_string(j["fret"].get<int>())+std::to_string(i)+".wav").c_str(), samples[i].data, samples[i].len*3);
		std::cout << "\n";
		std::cout << (double) samples[i].volumeMax/INT32_MAX;
	}*/
	//fgets()
}

/*void wav_convert_24_to_32_bit(char* buf24, int len, int* buf32)
{
}*/

int main2()
{
	//samples_cut("6x6");
	return 1;
}

int main()
{
	int len;
	int* cut_points=samples_find_cut_points("6x6_2", &len);
	
	std::string s=readFullFile((char*) "6x6_2.json");
	s=std::regex_replace(s, std::regex("[\\r\\n\\t]"), "");
	json j=json::parse(s);

	Samples_cut_sample* samples=samples_cut("6x6_2", cut_points, len, j);

	samples_organize(samples, len-1, (char*) "test");
}

int main1()
{	
	short buf[BUF_LEN], buf_[BUF_LEN];
	short* IRbuf=new short[IR_LEN];
	memset(buf_, 0, BUF_LEN*sizeof(*buf_));

	wav_read("sample.wav", buf, BUF_LEN);
	wav_read("IR3.wav", IRbuf, IR_LEN);
/*
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
	amp(buf, BUF_LEN, buf, 24);*/

	convolusion(buf, BUF_LEN, IRbuf, IR_LEN, buf_);

	//gen_sin(buf, BUF_LEN, 1000, 0.5);

	//distortion_inverse(buf, BUF_LEN, buf, (short) (SHRT_MAX/4));

	wav_write("sample_.wav", buf_, BUF_LEN);

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
	return 0;
}