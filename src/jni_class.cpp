//
// Copyright © Vyacheslav Krylov (slavone@protonmail.ch) 2014-2021
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// File created on 04/19/2018
//

#include <jni_class.h>
#include <unordered_map>

std::unordered_map<std::string, jclass> jni_classes;

jclass createGlobalRef (JNIEnv *env, const char *class_name) {
  jclass clazz = env->FindClass(class_name);
  if (clazz == nullptr) {
    return nullptr;
  }
  auto clazz_global = (jclass) env->NewGlobalRef(clazz);
  env->DeleteLocalRef(clazz);
  if (clazz_global == nullptr) {
    return nullptr;
  }
  return clazz_global;
}

jclass jni_class::get (JNIEnv *env, const std::string &path) {
  auto const &itr = jni_classes.find(path);
  if (itr != jni_classes.end()) {
    return itr->second;
  }
  jclass clazz = createGlobalRef(env, path.c_str());
  if (clazz == nullptr) {
    return nullptr;
  }
  jni_classes.insert({path, clazz});
  return clazz;
}