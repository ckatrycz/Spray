/*******************************************************************************
    Copyright (c) The Taichi Authors (2016- ). All Rights Reserved.
    The use of this software is governed by the LICENSE file.
*******************************************************************************/

#pragma once

#include <taichi/math/math.h>
#include <taichi/math/array_fwd.h>

TC_NAMESPACE_BEGIN

template <typename T>
class Array1D {
 private:
 public:
  int size;
  std::vector<T> data;

  Array1D(int size);

  Array1D(int size, T init);

  Array1D(const Array1D<T> &arr);

  Array1D &operator=(const Array1D<T> &);

  Array1D();

  ~Array1D();

  void reset(T a);

  bool same_dim(const Array1D<T> &arr);

  real dot(const Array1D<T> &b);

  Array1D<T> add(real alpha, const Array1D<T> &b);

  Array1D<T> operator-(const Array1D<T> &b);

  T &operator[](int i) {
    return data[i];
  }

  const T &operator[](int i) const {
    return data[i];
  }

  T abs_sum();

  T abs_max();

  void print(std::string name = "");

  size_t get_data_size() const {
    return size * sizeof(T);
  }

  const std::vector<T> &get_data() const {
    return this->data;
  }

  int get_dim() const {
    return 1;
  }

  TC_IO_DECL {
    TC_IO(size);
    TC_IO(data);
  }
};

template <typename T>
Array1D<T>::Array1D(int size) : size(size) {
  data = std::vector<T>(size);
  // memcpy(&data[0], this->data, this->get_data_size());
}

template <typename T>
real Array1D<T>::dot(const Array1D<T> &b) {
  real sum(0);
  assert(same_dim(b));
  for (int i = 0; i < size; i++) {
    sum += dot(this->data[i], b.data[i]);
  }
  return sum;
}

template <typename T>
Array1D<T> Array1D<T>::add(real alpha, const Array1D<T> &b) {
  Array1D o(size);
  assert(same_dim(b));
  for (int i = 0; i < size; i++) {
    o.data[i] = data[i] + alpha * b.data[i];
  }
  return o;
}

template <typename T>
Array1D<T> Array1D<T>::operator-(const Array1D<T> &b) {
  Array1D o(size);
  assert(same_dim(b));
  for (int i = 0; i < size; i++) {
    o.data[i] = data[i] - b.data[i];
  }
  return o;
}

template <typename T>
T Array1D<T>::abs_sum() {
  T ret(0);
  for (int i = 0; i < size; i++) {
    ret += abs(data[i]);
  }
  return ret;
}

template <typename T>
T Array1D<T>::abs_max() {
  T ret(0);
  for (int i = 0; i < size; i++) {
    for (int k = 0; k < 2; k++) {
      ret[k] = max(ret[k], abs(data[i][k]));
    }
  }
  return ret;
}

template <typename T>
void Array1D<T>::print(std::string name) {
  TC_NOT_IMPLEMENTED
}

template <typename T>
void Array1D<T>::reset(T a) {
  for (int i = 0; i < size; i++) {
    data[i] = a;
  }
}

template <typename T>
Array1D<T>::~Array1D() {
}

template <typename T>
Array1D<T>::Array1D(const Array1D<T> &arr) : Array1D(arr.size) {
  data = arr.data;
}

template <typename T>
Array1D<T>::Array1D() {
  size = 0;
}

template <typename T>
Array1D<T>::Array1D(int size, T init) : Array1D<T>(size) {
  if (init == T(0.0_f)) {
    memset(&data[0], 0, get_data_size());
  } else {
    for (int i = 0; i < size; i++)
      data[i] = init;
  }
}

template <typename T>
bool Array1D<T>::same_dim(const Array1D<T> &arr) {
  return size == arr.size;
}

template <typename T>
Array1D<T> &Array1D<T>::operator=(const Array1D<T> &arr) {
  this->size = arr.size;
  data = arr.data;
  return *this;
}

typedef Array1D<Vector2> ArrayVec2;

TC_NAMESPACE_END
