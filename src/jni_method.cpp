//
// Copyright Vyacheslav Krylov (slavone@protonmail.ch) 2014-2021
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// File created on 04/19/2018
//

#include <jni_method.h>
#include <jni_class.h>
#include <unordered_map>

std::unordered_map<std::string, jmethodID> jni_methods;

jmethodID jni_method::get (JNIEnv *env, const std::string &class_path, const std::string &name, const std::string &sig, jclass *class_result) {
  std::string key(class_path + "_" + name + sig);
  auto const &itr = jni_methods.find(key);
  if (itr != jni_methods.end()) {
    if (class_result != NULL && *class_result == NULL) {
      *class_result = jni_class::get(env, class_path);
    }
    return itr->second;
  }
  jclass clazz = nullptr;
  if (class_result != nullptr) {
    clazz = *class_result;
  }
  if (clazz == NULL) {
    clazz = jni_class::get(env, class_path);
    if (clazz == nullptr) {
      return NULL;
    }
    if (class_result != NULL) {
      *class_result = clazz;
    }
  }
  jmethodID method = env->GetMethodID(clazz, name.c_str(), sig.c_str());
  if (method == nullptr) {
    return nullptr;
  }
  jni_methods.insert({key, method});
  return method;
}