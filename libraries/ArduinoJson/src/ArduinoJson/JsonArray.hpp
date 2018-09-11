// ArduinoJson - arduinojson.org
// Copyright Benoit Blanchon 2014-2018
// MIT License

#pragma once

#include "JsonArrayData.hpp"
#include "JsonArrayIterator.hpp"

namespace ArduinoJson {

class JsonObject;

namespace Internals {
class JsonArraySubscript;
}

class JsonArray {
  friend class JsonVariant;

 public:
  typedef JsonArrayIterator iterator;

  FORCE_INLINE JsonArray() : _buffer(0), _data(0) {}
  FORCE_INLINE JsonArray(Internals::JsonBuffer* buf,
                         Internals::JsonArrayData* arr)
      : _buffer(buf), _data(arr) {}
  FORCE_INLINE explicit JsonArray(Internals::JsonBuffer* buf)
      : _buffer(buf), _data(new (buf) Internals::JsonArrayData()) {}

  // Adds the specified value at the end of the array.
  //
  // bool add(TValue);
  // TValue = bool, long, int, short, float, double, serialized, JsonVariant,
  //          std::string, String, JsonArrayData, JsonObject
  template <typename T>
  FORCE_INLINE bool add(const T& value) {
    return add_impl<const T&>(value);
  }
  //
  // bool add(TValue);
  // TValue = char*, const char*, const FlashStringHelper*
  template <typename T>
  FORCE_INLINE bool add(T* value) {
    return add_impl<T*>(value);
  }

  FORCE_INLINE iterator begin() const {
    if (!_data) return iterator();
    return iterator(_buffer, _data->begin());
  }

  FORCE_INLINE iterator end() const {
    return iterator();
  }

  // Imports a 1D array
  template <typename T, size_t N>
  FORCE_INLINE bool copyFrom(T (&array)[N]) {
    return copyFrom(array, N);
  }

  // Imports a 1D array
  template <typename T>
  bool copyFrom(T* array, size_t len) {
    bool ok = true;
    for (size_t i = 0; i < len; i++) {
      ok &= add(array[i]);
    }
    return ok;
  }

  // Imports a 2D array
  template <typename T, size_t N1, size_t N2>
  bool copyFrom(T (&array)[N1][N2]) {
    bool ok = true;
    for (size_t i = 0; i < N1; i++) {
      JsonArray nestedArray = createNestedArray();
      for (size_t j = 0; j < N2; j++) {
        ok &= nestedArray.add(array[i][j]);
      }
    }
    return ok;
  }

  // Exports a 1D array
  template <typename T, size_t N>
  FORCE_INLINE size_t copyTo(T (&array)[N]) const {
    return copyTo(array, N);
  }

  // Exports a 1D array
  template <typename T>
  size_t copyTo(T* array, size_t len) const {
    size_t i = 0;
    for (iterator it = begin(); it != end() && i < len; ++it) array[i++] = *it;
    return i;
  }

  // Exports a 2D array
  template <typename T, size_t N1, size_t N2>
  void copyTo(T (&array)[N1][N2]) const {
    if (!_data) return;
    size_t i = 0;
    for (iterator it = begin(); it != end() && i < N1; ++it) {
      it->as<JsonArray>().copyTo(array[i++]);
    }
  }

  FORCE_INLINE JsonArray createNestedArray();
  FORCE_INLINE JsonObject createNestedObject();

  FORCE_INLINE Internals::JsonArraySubscript operator[](size_t index);

  FORCE_INLINE const Internals::JsonArraySubscript operator[](
      size_t index) const;

  FORCE_INLINE bool operator==(const JsonArray& rhs) const {
    return _data == rhs._data;
  }

  // Gets the value at the specified index.
  template <typename T>
  FORCE_INLINE typename Internals::JsonVariantAs<T>::type get(
      size_t index) const {
    iterator it = begin() += index;
    return it != end() ? it->as<T>() : T();
  }

  // Check the type of the value at specified index.
  template <typename T>
  FORCE_INLINE bool is(size_t index) const {
    iterator it = begin() += index;
    return it != end() ? it->is<T>() : false;
  }

  // Removes element at specified position.
  FORCE_INLINE void remove(iterator it) {
    if (!_data) return;
    _data->remove(it.internal());
  }

  // Removes element at specified index.
  FORCE_INLINE void remove(size_t index) {
    remove(begin() += index);
  }

  // Sets the value at specified index.
  //
  // bool add(size_t index, const TValue&);
  // TValue = bool, long, int, short, float, double, serialized, JsonVariant,
  //          std::string, String, JsonArrayData, JsonObject
  template <typename T>
  FORCE_INLINE bool set(size_t index, const T& value) {
    if (!_data) return false;
    return set_impl<const T&>(index, value);
  }
  //
  // bool add(size_t index, TValue);
  // TValue = char*, const char*, const FlashStringHelper*
  template <typename T>
  FORCE_INLINE bool set(size_t index, T* value) {
    if (!_data) return false;
    return set_impl<T*>(index, value);
  }

  FORCE_INLINE size_t size() const {
    if (!_data) return 0;
    return _data->size();
  }

  FORCE_INLINE bool isNull() const {
    return _data == 0;
  }

  template <typename Visitor>
  FORCE_INLINE void visit(Visitor& visitor) const {
    if (_data)
      visitor.acceptArray(*_data);
    else
      visitor.acceptNull();
  }

 private:
  template <typename TValueRef>
  FORCE_INLINE bool set_impl(size_t index, TValueRef value) {
    iterator it = begin() += index;
    if (it == end()) return false;
    return it->set(value);
  }

  template <typename TValueRef>
  FORCE_INLINE bool add_impl(TValueRef value) {
    if (!_data) return false;
    iterator it = iterator(_buffer, _data->add(_buffer));
    if (it == end()) return false;
    return it->set(value);
  }

  Internals::JsonBuffer* _buffer;
  Internals::JsonArrayData* _data;
};
}  // namespace ArduinoJson
