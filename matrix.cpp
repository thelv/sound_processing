#include <iostream>
#include "matrix.h"

template  int** matrix_create<int>(int* m, int h, int w);
template  int** matrix_create<int>(int h, int w);
template  void matrix_free<int>(int** m, int h, int w);
template  int*** matrix_create<int>(int* m, int h, int w, int l);
template  void matrix_free<int>(int*** m, int h, int w, int l);
template int*** matrix_create<int>(int h, int w, int l);
template  int**** matrix_create<int>(int* m, int p, int h, int w, int l);
template int**** matrix_create<int>(int p, int h, int w, int l);
template void matrix_print<int>(int** d, int h, int w);
template void matrix_print<int>(int* d, int h);
template void matrix_print<int>(int*** d, int h, int w, int l);
//template  int* matrix_column<int>(int** d, int j, int h);
template  int** matrix_transpose<int>(int** d, int h, int w);

template  float** matrix_create<float>(float* m, int h, int w);
template  float** matrix_create<float>(int h, int w);
template  void matrix_free<float>(float** m, int h, int w);
template  float*** matrix_create<float>(float* m, int h, int w, int l);
template  void matrix_free<float>(float*** m, int h, int w, int l);
template float*** matrix_create<float>(int h, int w, int l);
template  float**** matrix_create<float>(float* m, int p, int h, int w, int l);
template float**** matrix_create<float>(int p, int h, int w, int l);
template void matrix_print<float>(float** d, int h, int w);
template void matrix_print<float>(float* d, int h);
template void matrix_print<float>(float*** d, int h, int w, int l);
//template  float* matrix_column<float>(float** d, int j, int h);
template  float** matrix_transpose<float>(float** d, int h, int w);

template /*__device__*/  char** matrix_create<char>(char* m, int h, int w);
template /*__device__*/  char** matrix_create<char>(int h, int w);
template /*__device__*/  void matrix_free<char>(char** m, int h, int w);
template /*__device__*/  char*** matrix_create<char>(char* m, int h, int w, int l);
template /*__device__*/  void matrix_free<char>(char*** m, int h, int w, int l);
template /*__device__*/ char*** matrix_create<char>(int h, int w, int l);
template /*__device__*/  char**** matrix_create<char>(char* m, int p, int h, int w, int l);
template /*__device__*/ char**** matrix_create<char>(int p, int h, int w, int l);
template void matrix_print<char>(char** d, int h, int w);
template void matrix_print<char>(char* d, int h);
template void matrix_print<char>(char*** d, int h, int w, int l);
//template /*__device__*/  char* matrix_column<char>(char** d, int j, int h);
template /*__device__*/  char** matrix_transpose<char>(char** d, int h, int w);

template /*__device__*/  void array_free<void*>(void*** m, int h);
template void array_free<int>(int** a, int h);
template void array_free<float>(float** a, int h);

template  int** matrix_defragment(int** a, int h, int w);

int* array_natural_create(int l);

template string toJson<float>(float* a, int h);
template string toJson<int>(int* a, int h);
template string toJson<float>(float** a, int h, int w);
template string toJson<int>(int** a, int h, int w);


template <typename type> void matrix_print(type** d, int h, int w)
{
	for(int i=0; i<h; i++)
	{
		for(int j=0; j<w; j++)
		{
			cout << d[i][j];
			cout << " ";
		}
		cout << "\n";
	}
}

template <typename type>  type** matrix_create(type* m, int h, int w)
{
	type** d=new type*[h];
	for(int i=0; i<h; i++)
	{
		d[i]=m+i*w;
	}

	return d;
}

template <typename type>  type** matrix_create(int h, int w)
{
	type* m=(type*) malloc(h*w*sizeof(type));
	return matrix_create(m, h, w);
}


template <typename type>  void matrix_free(type** m, int h, int w)
{
	free((void*) m[0]);
	delete[] m;
	return;
}

template <typename type>  type*** matrix_create(type* m, int h, int w, int l)
{
	type*** d=new type**[h];
	for(int i=0; i<h; i++)
	{
		d[i]=matrix_create<type>(m+i*w*l, w, l);
	}

	return d;
}

template <typename type>  void matrix_free(type*** m, int h, int w, int l)
{
	free((void*) m[0][0]);

	for(int i=0; i<=h; i++)
	{
		delete[] m[i];
	}	

	delete[] m;

	return;	
}

template <typename type> type*** matrix_create(int h, int w, int l)
{
	type* m=(type*) malloc(h*w*l*sizeof(type));
	return matrix_create(m, h, w, l);
}

template <typename type>  type**** matrix_create(type* m, int p, int h, int w, int l)
{
	type**** d=new type***[p];
	for(int i=0; i<p; i++)
	{
		d[i]=matrix_create<type>(m+i*h*w*l, h, w, l);
	}

	return d;
}

template <typename type> type**** matrix_create(int p, int h, int w, int l)
{
	type* m=(type*) malloc(p*h*w*l*sizeof(type));
	return matrix_create(m, p, h, w, l);
}

/*template <typename type> void matrix_print(type** d, int h, int w)
{
	for(int i=0; i<h; i++)
	{
		for(int j=0; j<w; j++)
		{
			cout << d[i][j];
			cout << " ";
		}
		cout << "\n";
	}
}*/

template <typename type> void matrix_print(type* d, int h)
{
	for(int i=0; i<h; i++)
	{		
		cout << d[i];
		cout << " ";
	}
}


template <typename type> void matrix_print(type*** d, int h, int w, int l)
{
	for(int i=0; i<h; i++)
	{
		for(int j=0; j<w; j++)
		{
			cout << "{ ";
			for(int k=0; k<l; k++)
			{
				cout << d[i][j][k];
				cout << " ";
			}			
			cout << "} ";
		}
		cout << "\n";
	}
}

template <typename type>  type* matrix_column(type** d, int j, int h)
{
	float* c=new type[h];
	for(int i=0; i<h; i++)
	{
		c[i]=d[i][j];
	}

	return c;
}

template <typename type>  type** matrix_transpose(type** d, int h, int w)
{
	type** dT=matrix_create<type>(w, h);
	for(int i=0; i<h; i++)
	{
		for(int j=0; j<w; j++)
		{
			dT[j][i]=d[i][j];
		}
	}

	return dT;
}

int* array_natural_create(int l)
{
	int* d=new int[l];
	for(int i=0; i<l; i++)
	{
		d[i]=i;
	}
	return d;
}

template <typename type>  type** matrix_defragment(type** a, int h, int w)
{
	type** r=matrix_create<type>(h, w);
	for(int i=0; i<h; i++)
	{
		memcpy(r[i], a[i], w*sizeof(type));
	}
	return r;
}

template <typename type> string toJson(type* a, int h)
{
	string s="";
	for(int i=0; i<h; i++)
	{
		s+=std::to_string(a[i])+", ";
	}

	return "["+s.substr(0, s.length()-2)+"]";
}

template <typename type> string toJson(type** a, int h, int w)
{
	string s="";
	for(int i=0; i<h; i++)
	{
		s+=toJson(a[i], w)+", ";
	}

	return (string)"["+s.substr(0, s.length()-2)+"]";
}

template <typename type> void array_free(type** a, int h)
{
	for(int i=0; i<h; i++)
	{
		delete[] a[i];
	}
	delete[] a;
}