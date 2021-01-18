//
// Copyright © Vyacheslav Krylov (slavone@protonmail.ch) 2014-2021
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// File created on 04/19/2018
//

#ifndef JNI_CLASS_H
#define JNI_CLASS_H

#include <jni.h>
#include <string>

namespace jni_class {

jclass get (JNIEnv *env, const std::string &path);

inline jclass ByteArray (JNIEnv *env) {
  return get(env, "[B");
}
inline jclass String (JNIEnv *env) {
  return get(env, "java/lang/String");
}
inline jclass RuntimeException (JNIEnv *env) {
  return get(env, "java/lang/RuntimeException");
}
inline jclass IOException (JNIEnv *env) {
  return get(env, "java/io/IOException");
}
inline jclass FileNotFoundException (JNIEnv *env) {
  return get(env, "java/io/FileNotFoundException");
}
inline jclass IllegalArgumentException (JNIEnv *env) {
  return get(env, "java/lang/IllegalArgumentException");
}
inline jclass IllegalStateException (JNIEnv *env) {
  return get(env, "java/lang/IllegalStateException");
}
inline jclass AssertionError (JNIEnv *env) {
  return get(env, "java/lang/AssertionError");
}

} // namespace jni_class

#endif //JNI_CLASS_H
