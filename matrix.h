#pragma once
#include <string>

using namespace std;

template <typename type>  type** matrix_create(type* m, int h, int w);
template <typename type>  type** matrix_create(int h, int w);
template <typename type>  void matrix_free(type** m, int h, int w);
template <typename type>  type*** matrix_create(type* m, int h, int w, int l);
template <typename type>  void matrix_free(type*** m, int h, int w, int l);
template <typename type> type*** matrix_create(int h, int w, int l);
template <typename type>  type**** matrix_create(type* m, int p, int h, int w, int l);
template <typename type> type**** matrix_create(int p, int h, int w, int l);
template <typename type> void matrix_print(type** d, int h, int w);
template <typename type> void matrix_print(type* d, int h);
template <typename type> void matrix_print(type*** d, int h, int w, int l);
template <typename type>  type* matrix_column(type** d, int j, int h);
template <typename type>  type** matrix_transpose(type** d, int h, int w);
template <typename type>  type** matrix_defragment(type** a, int h, int w);
template <typename type> void array_free(type** a, int h);
int* array_natural_create(int l);

template <typename type> string toJson(type* a, int h);
template <typename type> string toJson(type** a, int h, int w);
