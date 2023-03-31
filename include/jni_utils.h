//
// Copyright © Vyacheslav Krylov (slavone@protonmail.ch) 2014
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// File created on 04/19/2018
//

#ifndef JNI_UTILS_H
#define JNI_UTILS_H

#include <jni.h>
#include <string>
#include <map>
#include <jni_as.h>
#include <jni_class.h>
#include <jni_method.h>

namespace jni {
  inline bool validate_environment (JNIEnv *env) {
    return jni_class::ByteArray(env) &&
      jni_class::String(env) &&
      jni_class::RuntimeException(env) &&
      jni_class::IOException(env) &&
      jni_class::FileNotFoundException(env);
  }

  template <typename PTR_T, typename std::enable_if_t<std::is_pointer<PTR_T>::value, int> = 0> PTR_T jlong_to_ptr (jlong ptr) {
    return reinterpret_cast<PTR_T>(static_cast<std::uintptr_t>(ptr));
  }

  template <typename PTR_T, typename std::enable_if_t<std::is_pointer<PTR_T>::value, int> = 0> jlong ptr_to_jlong (PTR_T ptr) {
    return static_cast<jlong>(reinterpret_cast<std::uintptr_t>(ptr));
  }

  jstring to_jstring (JNIEnv *env, const std::string &s);
  std::string from_jstring (JNIEnv *env, jstring s);

  bool throw_new (JNIEnv *env, const std::string &message, jclass clazz);
  bool throw_new (JNIEnv *env, const std::string &message, const std::string &exception = "java/lang/RuntimeException");

  // Array stuff

  template <typename ARRAY_T> inline ARRAY_T array_new (JNIEnv *env, jsize length);
  template <typename T> inline void array_set (JNIEnv *env, T array, jsize offset, jsize length, const char *data);
  template <typename T, typename ARRAY_T> inline T * array_get (JNIEnv *env, ARRAY_T array);
  template <typename T, typename ARRAY_T> inline void array_release (JNIEnv *env, ARRAY_T array, T *elements);

  #define JNI_ARRAY_RELEASE(TYPE,ARRAY) \
    template <> inline void array_release<TYPE,TYPE##Array> (JNIEnv *env, TYPE##Array array, TYPE *elements) { env->Release##ARRAY##ArrayElements(array, elements, JNI_ABORT); }

  #define JNI_ARRAY_GET(TYPE,ARRAY) \
    template <> inline TYPE * array_get<TYPE> (JNIEnv *env, TYPE##Array array) { return env->Get##ARRAY##ArrayElements(array, NULL); }

  #define JNI_ARRAY_NEW(TYPE,ARRAY) \
    template <> inline TYPE##Array array_new<TYPE##Array> (JNIEnv *env, jsize length) { return env->New##ARRAY##Array(length); }

  #define JNI_ARRAY_SET(TYPE,ARRAY) \
    template <> inline void array_set<TYPE##Array> (JNIEnv *env, TYPE##Array array, jsize offset, jsize length, const char *data) { env->Set##ARRAY##ArrayRegion(array, offset, length, (const TYPE *) data); }

  #define JNI_ARRAY(JNI_ARRAY_FUNC) \
    JNI_ARRAY_FUNC(jint, Int); \
    JNI_ARRAY_FUNC(jlong, Long); \
    JNI_ARRAY_FUNC(jbyte, Byte); \
    JNI_ARRAY_FUNC(jfloat, Float); \
    JNI_ARRAY_FUNC(jdouble, Double);

  JNI_ARRAY(JNI_ARRAY_NEW);
  JNI_ARRAY(JNI_ARRAY_SET);
  JNI_ARRAY(JNI_ARRAY_GET);
  JNI_ARRAY(JNI_ARRAY_RELEASE);
  #undef JNI_ARRAY_NEW
  #undef JNI_ARRAY_SET
  #undef JNI_ARRAY_GET
  #undef JNI_ARRAY_RELEASE

  #undef JNI_ARRAY

  // Object wrapper

  class Object {
  private:
    JNIEnv *env;
    jobject obj;
    jclass clazz;
    std::map<std::string, jfieldID> cachedFields;
    std::map<std::string, jmethodID> cachedMethods;
    const bool cacheEnabled;

    jfieldID findField (const char *fieldName, const char *sig) {
      if (!cacheEnabled) {
        return env->GetFieldID(clazz, fieldName, sig);
      }
      std::string key = std::string(fieldName) + "_" + std::string(sig);
      auto itr = cachedFields.find(key);
      if (itr != cachedFields.end()) {
        return itr->second;
      } else {
        jfieldID fieldId = env->GetFieldID(clazz, fieldName, sig);
        cachedFields[key] = fieldId;
        return fieldId;
      }
    }

    jmethodID findMethod (const char *methodName, const char *sig) {
      if (!cacheEnabled) {
        return env->GetMethodID(clazz, methodName, sig);
      }
      std::string key = std::string(methodName) + "_" + std::string(sig);
      auto itr = cachedMethods.find(key);
      if (itr != cachedMethods.end()) {
        return itr->second;
      } else {
        jmethodID methodId = env->GetMethodID(clazz, methodName, sig);
        cachedMethods[key] = methodId;
        return methodId;
      }
    }

  public:
    Object (JNIEnv *env, jobject obj,
            bool cacheFields = false) : Object(
      env,
      obj,
      env->GetObjectClass(obj),
      cacheFields
    ) { }

    Object (JNIEnv *env, jobject obj, jclass clazz,
            bool cacheFields = false) :
      env(env), obj(env->NewGlobalRef(obj)), clazz(clazz),
      cacheEnabled(cacheFields) { }
    ~Object () {
      env->DeleteGlobalRef(obj);
    }

    jobject getThis () {
      return obj;
    }

    // Getters

    jint getInt (const char *fieldName) {
      return env->GetIntField(obj, findField(fieldName, "I"));
    }

    jlong getLong (const char *fieldName) {
      return env->GetLongField(obj, findField(fieldName, "J"));
    }

    jboolean getBoolean (const char *fieldName) {
      return env->GetBooleanField(obj, findField(fieldName, "Z"));
    }

    jdouble getDouble (const char *fieldName) {
      return env->GetDoubleField(obj, findField(fieldName, "D"));
    }

    jbyteArray getByteArray (const char *fieldName) {
      return (jbyteArray) env->GetObjectField(obj, findField(fieldName, "[B"));
    }

    jintArray getIntArray (const char *fieldName) {
      return (jintArray) env->GetObjectField(obj, findField(fieldName, "[I"));
    }

    jstring getString (const char *fieldName) {
      return (jstring) env->GetObjectField(obj, findField(fieldName, "Ljava/lang/String;"));
    }

    Object getObject (const char *fieldName, const char *sig = "Ljava/lang/Object;") {
      return Object(env, getRawObject(fieldName, sig));
    }

    jobject getRawObject (const char *fieldName, const char *sig = "Ljava/lang/Object;") {
      return env->GetObjectField(obj, findField(fieldName, sig));
    }

    jobjectArray getObjectArray (const char *fieldName, const char *sig = "[Ljava/lang/Object;") {
      return (jobjectArray) env->GetObjectField(obj, findField(fieldName, sig));
    }

    // Setters

    void setInt (const char *fieldName, jint value) {
      env->SetIntField(obj, findField(fieldName, "I"), value);
    }

    void setLong (const char *fieldName, jlong value) {
      env->SetLongField(obj, findField(fieldName, "J"), value);
    }

    // Methods

    void callVoid (const char *methodName) {
      env->CallVoidMethod(obj, findMethod(methodName, "()V"));
    }

    void callVoid (const char *methodName, jint arg1) {
      env->CallVoidMethod(obj, findMethod(methodName, "(I)V"), arg1);
    }

    void callVoid (const char *methodName, jboolean arg1) {
      env->CallVoidMethod(obj, findMethod(methodName, "(Z)V"), arg1);
    }

    void callVoid (const char *methodName, jfloat arg1) {
      env->CallVoidMethod(obj, findMethod(methodName, "(F)V"), arg1);
    }

    void callVoid (const char *methodName, jbyteArray arg1) {
      env->CallVoidMethod(obj, findMethod(methodName, "([B)V"), arg1);
    }

    jbyteArray callByteArray (const char *methodName, jint arg1) {
      return (jbyteArray) env->CallObjectMethod(obj, findMethod(methodName, "(I)[B"), arg1);
    }
  };
}

#endif //JNI_UTILS_H
