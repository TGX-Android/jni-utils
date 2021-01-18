//
// Copyright © Vyacheslav Krylov (slavone@protonmail.ch) 2014-2021
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// File created on 04/19/2018
//

#ifndef JNI_METHOD_H
#define JNI_METHOD_H

#include <jni.h>
#include <string>

namespace jni_method {

jmethodID get (JNIEnv *env, const std::string &class_name, const std::string &name,
               const std::string &sig, jclass *class_result = nullptr);

} // namespace jni_method

#endif //JNI_METHOD_H
