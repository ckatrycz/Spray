/*******************************************************************************
    Copyright (c) The Taichi Authors (2016- ). All Rights Reserved.
    The use of this software is governed by the LICENSE file.
*******************************************************************************/

#pragma once

#include <map>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <memory>
#include <cstring>
#include <cassert>
#include <iostream>
#include <type_traits>

#ifdef TC_INCLUDED
TC_NAMESPACE_BEGIN
#else
#define TC_NAMESPACE_BEGIN
#define TC_NAMESPACE_END
#define TC_EXPORT
#define TC_TRACE
#define TC_CRITICAL
#define TC_ASSERT assert
#endif

template <typename T>
TC_EXPORT std::unique_ptr<T> create_instance_unique(const std::string &alias);

////////////////////////////////////////////////////////////////////////////////
//                   A Minimalist Serializer for Taichi                       //
//                           (C++11 Compatible)                               //
////////////////////////////////////////////////////////////////////////////////

class Unit;

namespace type {

template <typename T>
using remove_cvref =
    typename std::remove_cv<typename std::remove_reference<T>::type>;

template <typename T>
using remove_cvref_t = typename remove_cvref<T>::type;

template <typename T>
using is_unit = typename std::is_base_of<Unit, remove_cvref_t<T>>;

template <typename T>
using is_unit_t = typename is_unit<T>::type;
}  // namespace type

#define TC_IO_DECL_INST                               \
  void binary_io(BinaryOutputSerializer &ser) const { \
    ser(*this);                                       \
  }                                                   \
  void binary_io(BinaryInputSerializer &ser) const {  \
    ser(*this);                                       \
  }

#define TC_IO_DECL_INST_VIRT                                  \
  virtual void binary_io(BinaryOutputSerializer &ser) const { \
    ser(*this);                                               \
  }                                                           \
  virtual void binary_io(BinaryInputSerializer &ser) const {  \
    ser(*this);                                               \
  }

#define TC_IO_DECL_INST_VIRT_OVERRIDE                                  \
  virtual void binary_io(BinaryOutputSerializer &ser) const override { \
    ser(*this);                                                        \
  }                                                                    \
  virtual void binary_io(BinaryInputSerializer &ser) const override {  \
    ser(*this);                                                        \
  }

#define TC_IO_DECL      \
  TC_IO_DECL_INST       \
  template <typename S> \
  void io(S &serializer) const

#define TC_IO_DECL_VIRT \
  TC_IO_DECL_INST_VIRT  \
  template <typename S> \
  void io(S &serializer) const

#define TC_IO_DEF(...)           \
  TC_IO_DECL_INST                \
  template <typename S>          \
  void io(S &serializer) const { \
    TC_IO(__VA_ARGS__)           \
  }

#define TC_IO_DEF_VIRT(...)      \
  TC_IO_DECL_INST_VIRT           \
  template <typename S>          \
  void io(S &serializer) const { \
    TC_IO(__VA_ARGS__)           \
  }

#define TC_IO_DEF_WITH_BASE(...) \
  TC_IO_DECL_INST_VIRT_OVERRIDE  \
  template <typename S>          \
  void io(S &serializer) const { \
    Base::io(serializer);        \
    TC_IO(__VA_ARGS__)           \
  }

#define TC_IO(...) \
  { serializer(#__VA_ARGS__, __VA_ARGS__); }

#define TC_SERIALIZER_IS(T)                                                 \
  (std::is_same<typename std::remove_reference<decltype(serializer)>::type, \
                T>())

static_assert(
    sizeof(std::size_t) == sizeof(uint64_t),
    "sizeof(std::size_t) should be 8. Try compiling with 64bit mode.");

template <typename T, typename S>
struct IO {
  using implemented = std::false_type;
};

class Serializer {
 public:
  template <typename T, std::size_t n>
  using TArray = T[n];
  template <typename T, std::size_t n>
  using StdTArray = std::array<T, n>;

  std::unordered_map<std::size_t, void *> assets;

  template <typename T, typename T_ = typename type::remove_cvref_t<T>>
  static T_ &get_writable(T &&t) {
    return *const_cast<T_ *>(&t);
  }

  template <typename T>
  struct has_io {
    template <typename T_>
    static constexpr auto helper(T_ *)
        -> std::is_same<decltype((std::declval<T_>().template io(
                            std::declval<Serializer &>()))),
                        void>;

    template <typename>
    static constexpr auto helper(...) -> std::false_type;

   public:
    using T__ = typename type::remove_cvref_t<T>;
    using type = decltype(helper<T__>(nullptr));
    static constexpr bool value = type::value;
  };

  template <typename T>
  struct has_free_io {
    template <typename T_>
    static constexpr auto helper(T_ *) ->
        typename IO<T_, Serializer>::implemented;

    template <typename>
    static constexpr auto helper(...) -> std::false_type;

   public:
    using T__ = typename type::remove_cvref_t<T>;
    using type = decltype(helper<T__>(nullptr));
    static constexpr bool value = type::value;
  };

  /*

 public:
  template <typename T>
  struct Item {
    using is_array =
        typename std::is_array<typename std::remove_cv<T>::type>::type;
    using is_lref = typename std::is_lvalue_reference<T>::type;

    static_assert(!std::is_pointer<T>(), "");

    using ValueType = type_switch_t<
        std::pair<is_lref, T>,  // Keep l-value references
        std::pair<is_array,
                  typename std::remove_cv<T>::type>,  // Do nothing for arrays
        std::pair<std::true_type, typename type::remove_cvref_t<T>>
        // copy r-value references?
        // is there a better way?
        >;

    Item(const std::string &key, ValueType &&value)
        : key(key), value(std::forward<ValueType>(value)) {
    }

    ValueType value;
    const std::string &key;
  };

  template <typename T>
  auto make_item(const std::string &name, T &&t) -> Item<T> {
    return Item<T>(name, std::forward<T>(t));
  }

  template <typename T>
  auto make_item(T &&t) -> Item<T> {
    return Item<T>("", std::forward<T>(t));
  }
static_assert(
    std::is_same<typename Serializer::Item<int &>::is_array, std::false_type>(),
    "");

static_assert(
    std::is_same<typename Serializer::Item<int &>::ValueType, int &>(),
    "");
static_assert(std::is_same<typename Serializer::Item<int &&>::ValueType, int>(),
              "");

static_assert(std::is_same<typename Serializer::Item<int &&>::ValueType, int>(),
              "");

static_assert(
    std::is_same<typename Serializer::Item<int[32]>::ValueType, int[32]>(),
    "");
   */
};

template <bool writing>
class BinarySerializer : public Serializer {
 public:
  std::vector<uint8_t> data;
  uint8_t *c_data;

  std::size_t head;
  std::size_t preserved;

  using Base = Serializer;
  // using Base::Item;
  using Base::assets;

  void write_to_file(const std::string &file_name) {
    FILE *f = fopen(file_name.c_str(), "wb");
    if (f == nullptr) {
      TC_ERROR(
          "Can not open file [{}] for writing. (Does the directory exist?)",
          file_name);
      assert(f != nullptr);
    }
    void *ptr = c_data;
    if (!ptr) {
      assert(!data.empty());
      ptr = &data[0];
    }
    fwrite(ptr, sizeof(uint8_t), head, f);
    fclose(f);
  }

  template <bool writing_ = writing>
  typename std::enable_if<!writing_, void>::type initialize(
      const std::string &file_name) {
    FILE *f = fopen(file_name.c_str(), "rb");
    if (f == nullptr) {
      TC_ERROR("Cannot open file: {}", file_name);
      return;
    }
    assert(f != nullptr);
    std::size_t length = 0;
    while (true) {
      size_t limit = 1 << 8;
      data.resize(data.size() + limit);
      void *ptr = reinterpret_cast<void *>(&data[length]);
      size_t length_tmp = fread(ptr, sizeof(uint8_t), limit, f);
      length += length_tmp;
      if (length_tmp < limit) {
        break;
      }
    }
    fclose(f);
    data.resize(length);
    c_data = reinterpret_cast<uint8_t *>(&data[0]);
    head = sizeof(std::size_t);
  }

  template <bool writing_ = writing>
  typename std::enable_if<writing_, void>::type initialize(
      std::size_t preserved_ = std::size_t(0),
      void *c_data = nullptr) {
    std::size_t n = 0;
    head = 0;
    if (preserved_ != 0) {
      TC_TRACE("perserved = {}", preserved_);
      // Preserved mode
      this->preserved = preserved_;
      assert(c_data != nullptr);
      this->c_data = (uint8_t *)c_data;
    } else {
      // otherwise use a std::vector<uint8_t>
      this->preserved = 0;
      this->c_data = nullptr;
    }
    this->operator()("", n);
  }

  template <bool writing_ = writing>
  typename std::enable_if<!writing_, void>::type initialize(
      void *raw_data,
      std::size_t preserved_ = std::size_t(0)) {
    if (preserved_ != 0) {
      assert(raw_data == nullptr);
      data.resize(preserved_);
      c_data = &data[0];
    } else {
      assert(raw_data != nullptr);
      c_data = reinterpret_cast<uint8_t *>(raw_data);
    }
    head = sizeof(std::size_t);
    preserved = 0;
  }

  void finalize() {
    if (writing) {
      if (c_data) {
        *reinterpret_cast<std::size_t *>(&c_data[0]) = head;
      } else {
        *reinterpret_cast<std::size_t *>(&data[0]) = head;
      }
    } else {
      assert(head == *reinterpret_cast<std::size_t *>(c_data));
    }
  }

  // std::string
  void operator()(const char *, const std::string &val_) {
    auto &val = get_writable(val_);
    if (writing) {
      std::vector<char> val_vector(val.begin(), val.end());
      this->operator()(nullptr, val_vector);
    } else {
      std::vector<char> val_vector;
      this->operator()(nullptr, val_vector);
      val = std::string(val_vector.begin(), val_vector.end());
    }
  }

  // C-array
  template <typename T, std::size_t n>
  void operator()(const char *, const TArray<T, n> &val) {
    if (writing) {
      for (std::size_t i = 0; i < n; i++) {
        this->operator()("", val[i]);
      }
    } else {
      // TODO: why do I have to let it write to tmp, otherwise I get Sig Fault?
      TArray<typename type::remove_cvref_t<T>, n> tmp;
      for (std::size_t i = 0; i < n; i++) {
        this->operator()("", tmp[i]);
      }
      std::memcpy(const_cast<typename std::remove_cv<T>::type *>(val), tmp,
                  sizeof(tmp));
    }
  }

  // Elementary data types
  template <typename T>
  typename std::enable_if<!has_io<T>::value && !std::is_pointer<T>::value,
                          void>::type
  operator()(const char *, const T &val) {
    static_assert(!std::is_reference<T>::value, "T cannot be reference");
    static_assert(!std::is_const<T>::value, "T cannot be const");
    static_assert(!std::is_volatile<T>::value, "T cannot be volatile");
    static_assert(!std::is_pointer<T>::value, "T cannot be pointer");
    if (writing) {
      std::size_t new_size = head + sizeof(T);
      if (c_data) {
        if (new_size > preserved) {
          TC_CRITICAL("Preserved Buffer (size {}) Overflow.", preserved);
        }
        //*reinterpret_cast<typename type::remove_cvref_t<T> *>(&c_data[head]) =
        //    val;
        std::memcpy(&c_data[head], &val, sizeof(T));
      } else {
        data.resize(new_size);
        //*reinterpret_cast<typename type::remove_cvref_t<T> *>(&data[head]) =
        //    val;
        std::memcpy(&data[head], &val, sizeof(T));
      }
    } else {
      // get_writable(val) =
      //    *reinterpret_cast<typename std::remove_reference<T>::type *>(
      //        &c_data[head]);
      std::memcpy(&get_writable(val), &c_data[head], sizeof(T));
    }
    head += sizeof(T);
  }

  template <typename T>
  typename std::enable_if<has_io<T>::value, void>::type operator()(
      const char *,
      const T &val) {
    val.io(*this);
  }

  // Unique Pointers to non-taichi-unit Types
  template <typename T>
  typename std::enable_if<!type::is_unit<T>::value, void>::type operator()(
      const char *,
      const std::unique_ptr<T> &val_) {
    auto &val = get_writable(val_);
    if (writing) {
      this->operator()(ptr_to_int(val.get()));
      if (val.get() != nullptr) {
        this->operator()("", *val);
        // Just for checking future raw pointers
        assets.insert(std::make_pair(ptr_to_int(val.get()), val.get()));
      }
    } else {
      std::size_t original_addr;
      this->operator()("", original_addr);
      if (original_addr != 0) {
        val = std::make_unique<T>();
        assets.insert(std::make_pair(original_addr, val.get()));
        this->operator()("", *val);
      }
    }
  }

  template <typename T>
  std::size_t ptr_to_int(T *t) {
    return reinterpret_cast<std::size_t>(t);
  }

  // Unique Pointers to taichi-unit Types
  template <typename T>
  typename std::enable_if<type::is_unit<T>::value, void>::type operator()(
      const char *,
      const std::unique_ptr<T> &val_) {
    auto &val = get_writable(val_);
    if (writing) {
      this->operator()(val->get_name());
      this->operator()(ptr_to_int(val.get()));
      if (val.get() != nullptr) {
        val->binary_io(*this);
        // Just for checking future raw pointers
        assets.insert(std::make_pair(ptr_to_int(val.get()), val.get()));
      }
    } else {
      std::string name;
      std::size_t original_addr;
      this->operator()("", name);
      this->operator()("", original_addr);
      if (original_addr != 0) {
        val = create_instance_unique<T>(name);
        assets.insert(std::make_pair(original_addr, val.get()));
        val->binary_io(*this);
      }
    }
  }

  // Raw pointers (no ownership)
  template <typename T>
  typename std::enable_if<std::is_pointer<T>::value, void>::type operator()(
      const char *,
      const T &val_) {
    auto &val = get_writable(val_);
    if (writing) {
      this->operator()("", ptr_to_int(val));
      if (val != nullptr) {
        TC_ASSERT_INFO(assets.find(ptr_to_int(val)) != assets.end(),
                       "Cannot find the address with a smart pointer pointing "
                       "to. Make sure the smart pointer is serialized before "
                       "the raw pointer.");
      }
    } else {
      std::size_t val_ptr;
      this->operator()("", val_ptr);
      if (val_ptr != 0) {
        TC_ASSERT(assets.find(val_ptr) != assets.end());
        val = reinterpret_cast<typename std::remove_pointer<T>::type *>(
            assets[val_ptr]);
      }
    }
  }

  // std::vector
  template <typename T>
  void operator()(const char *, const std::vector<T> &val_) {
    auto &val = get_writable(val_);
    if (writing) {
      this->operator()("", val.size());
    } else {
      std::size_t n;
      this->operator()("", n);
      val.resize(n);
    }
    /*
    for (std::size_t i = 0; i < val.size(); i++) {
      printf("i %d\n", i);
      printf("val %d\n", val[i]);
    }

     */
    for (std::size_t i = 0; i < val.size(); i++) {
      this->operator()("", val[i]);
    }
  }

  // std::pair
  template <typename T, typename G>
  void operator()(const char *, const std::pair<T, G> &val) {
    this->operator()(nullptr, val.first);
    this->operator()(nullptr, val.second);
  }

  // std::map
  template <typename T, typename G>
  void operator()(const char *, const std::map<T, G> &val_) {
    auto &val = get_writable(val_);
    if (writing) {
      this->operator()(nullptr, val.size());
      for (auto iter : val) {
        T first = iter.first;
        this->operator()(nullptr, first);
        this->operator()(nullptr, iter.second);
      }
    } else {
      val.clear();
      std::size_t n;
      this->operator()(nullptr, n);
      for (std::size_t i = 0; i < n; i++) {
        std::pair<T, G> record;
        this->operator()(nullptr, record);
        val.insert(record);
      }
    }
  }

  template <typename T, typename... Args>
  void operator()(const char *, const T &t, Args &&... rest) {
    this->operator()(nullptr, t);
    this->operator()(nullptr, std::forward<Args>(rest)...);
  }

  template <typename T>
  void operator()(const T &val) {
    this->operator()(nullptr, val);
  }
};

using BinaryOutputSerializer = BinarySerializer<true>;
using BinaryInputSerializer = BinarySerializer<false>;

class TextSerializer : public Serializer {
 public:
  std::string data;
  void print() const {
    std::cout << data << std::endl;
  }

  void write_to_file(const std::string &file_name) {
    std::ofstream fs(file_name);
    fs << data;
    fs.close();
  }

 private:
  int indent;
  static constexpr int indent_width = 2;
  bool first_line;

 public:
  TextSerializer() {
    indent = 0;
    first_line = false;
  }

  void add_line(const std::string &str) {
    if (first_line) {
      first_line = false;
    } else {
      data += "\n";
    }
    data += std::string(indent_width * indent, ' ') + str;
  }

  void add_line(const std::string &key, const std::string &value) {
    add_line(key + ": " + value);
  }

  template <typename T>
  static std::string serialize(const char *key, const T &t) {
    TextSerializer ser;
    ser(key, t);
    return ser.data;
  }

  void operator()(const char *key, const std::string &val) {
    add_line(std::string(key) + ": " + val);
  }

  template <typename T, std::size_t n>
  using is_compact =
      typename std::integral_constant<bool,
                                      std::is_arithmetic<T>::value && (n < 7)>;

  // C-array
  template <typename T, std::size_t n>
  typename std::enable_if<is_compact<T, n>::value, void>::type operator()(
      const char *key,
      const TArray<T, n> &val) {
    std::stringstream ss;
    ss << "[";
    for (std::size_t i = 0; i < n; i++) {
      ss << val[i];
      if (i != n - 1) {
        ss << ", ";
      }
    }
    ss << "]";
    add_line(key, ss.str());
  }

  // C-array
  template <typename T, std::size_t n>
  typename std::enable_if<!is_compact<T, n>::value, void>::type operator()(
      const char *key,
      const TArray<T, n> &val) {
    add_line(key, "[");
    indent++;
    for (std::size_t i = 0; i < n; i++) {
      this->operator()(("[" + std::to_string(i) + "]").c_str(), val[i]);
    }
    indent--;
    add_line("]");
  }

  // std::array
  template <typename T, std::size_t n>
  typename std::enable_if<is_compact<T, n>::value, void>::type operator()(
      const char *key,
      const StdTArray<T, n> &val) {
    std::stringstream ss;
    ss << "[";
    for (std::size_t i = 0; i < n; i++) {
      ss << val[i];
      if (i != n - 1) {
        ss << ", ";
      }
    }
    ss << "]";
    add_line(key, ss.str());
  }

  // std::array
  template <typename T, std::size_t n>
  typename std::enable_if<!is_compact<T, n>::value, void>::type operator()(
      const char *key,
      const StdTArray<T, n> &val) {
    add_line(key, "[");
    indent++;
    for (std::size_t i = 0; i < n; i++) {
      this->operator()(("[" + std::to_string(i) + "]").c_str(), val[i]);
    }
    indent--;
    add_line("]");
  }

  // Elementary data types
  template <typename T>
  typename std::enable_if<!has_io<T>::value && !has_free_io<T>::value,
                          void>::type
  operator()(const char *key, const T &val) {
    static_assert(!has_io<T>::value, "");
    std::stringstream ss;
    ss << std::boolalpha << val;
    add_line(key, ss.str());
  }

  template <typename T>
  typename std::enable_if<has_io<T>::value, void>::type operator()(
      const char *key,
      const T &val) {
    add_line(key, "{");
    indent++;
    val.io(*this);
    indent--;
    add_line("}");
  }

  template <typename T>
  typename std::enable_if<has_free_io<T>::value, void>::type operator()(
      const char *key,
      const T &val) {
    add_line(key, "{");
    indent++;
    IO<typename type::remove_cvref_t<T>, decltype(*this)>()(*this, val);
    indent--;
    add_line("}");
  }

  template <typename T>
  void operator()(const char *key, const std::vector<T> &val) {
    add_line(key, "[");
    indent++;
    for (std::size_t i = 0; i < val.size(); i++) {
      this->operator()(("[" + std::to_string(i) + "]").c_str(), val[i]);
    }
    indent--;
    add_line("]");
  }

  template <typename T, typename G>
  void operator()(const char *key, const std::pair<T, G> &val) {
    add_line(key, "(");
    indent++;
    this->operator()("[0]", val.first);
    this->operator()("[1]", val.second);
    indent--;
    add_line(")");
  }

  template <typename T, typename G>
  void operator()(const char *key, const std::map<T, G> &val) {
    add_line(key, "{");
    indent++;
    for (auto iter : val) {
      T first = iter.first;
      this->operator()("key", first);
      this->operator()("value", iter.second);
    }
    indent--;
    add_line("}");
  }

  template <typename T, typename... Args>
  void operator()(const char *key_, const T &t, Args &&... rest) {
    std::string key(key_);
    size_t pos = key.find(",");
    std::string first_name = key.substr(0, pos);
    std::string rest_names =
        key.substr(pos + 2, int(key.size()) - (int)pos - 2);
    this->operator()(first_name.c_str(), t);
    this->operator()(rest_names.c_str(), std::forward<Args>(rest)...);
  }

  /*
  template <typename T>
  void operator()(Item<T> &item) {
    this->operator()(item.key.c_str(), get_writable(item.value));
  }
  */
};

template <typename T>
typename std::enable_if<Serializer::has_io<T>::value, std::ostream &>::type
operator<<(std::ostream &os, const T &t) {
  os << TextSerializer::serialize("value", t);
  return os;
}

template <typename T>
void read_from_binary_file(T &t, const std::string &file_name) {
  BinaryInputSerializer reader;
  reader.initialize(file_name);
  reader(t);
  reader.finalize();
}

template <typename T>
void write_to_binary_file(const T &t, const std::string &file_name) {
  BinaryOutputSerializer writer;
  writer.initialize();
  writer(t);
  writer.finalize();
  writer.write_to_file(file_name);
}

// Compile-Time Tests
static_assert(std::is_same<decltype(Serializer::get_writable(
                               std::declval<const std::vector<int> &>())),
                           std::vector<int> &>(),
              "");

static_assert(
    std::is_same<
        decltype(Serializer::get_writable(
            std::declval<const std::vector<std::unique_ptr<int>> &>())),
        std::vector<std::unique_ptr<int>> &>(),
    "");

TC_NAMESPACE_END
