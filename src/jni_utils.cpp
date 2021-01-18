//
// Copyright © Vyacheslav Krylov (slavone@protonmail.ch) 2014-2021
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// File created on 04/19/2018
//

#include <jni.h>
#include <string>
#include <jni_utils.h>

static void utf16_to_utf8 (const jchar *p, jsize len, char *res) {
  for (jsize i = 0; i < len; i++) {
    unsigned int cur = p[i];
    // TODO conversion unsigned int -> signed char is implementation defined
    if (cur <= 0x7f) {
      *res++ = static_cast<char>(cur);
    } else if (cur <= 0x7ff) {
      *res++ = static_cast<char>(0xc0 | (cur >> 6));
      *res++ = static_cast<char>(0x80 | (cur & 0x3f));
    } else if ((cur & 0xF800) != 0xD800) {
      *res++ = static_cast<char>(0xe0 | (cur >> 12));
      *res++ = static_cast<char>(0x80 | ((cur >> 6) & 0x3f));
      *res++ = static_cast<char>(0x80 | (cur & 0x3f));
    } else {
      // correctness is already checked
      unsigned int next = p[++i];
      unsigned int val = ((cur - 0xD800) << 10) + next - 0xDC00 + 0x10000;

      *res++ = static_cast<char>(0xf0 | (val >> 18));
      *res++ = static_cast<char>(0x80 | ((val >> 12) & 0x3f));
      *res++ = static_cast<char>(0x80 | ((val >> 6) & 0x3f));
      *res++ = static_cast<char>(0x80 | (val & 0x3f));
    }
  }
}

static jsize get_utf16_from_utf8_length(const char *p, size_t len, jsize *surrogates) {
  // UTF-8 correctness is supposed
  jsize result = 0;
  for (size_t i = 0; i < len; i++) {
    result += ((p[i] & 0xc0) != 0x80);
    *surrogates += ((p[i] & 0xf8) == 0xf0);
  }
  return result;
}

static void utf8_to_utf16(const char *p, size_t len, jchar *res) {
  // UTF-8 correctness is supposed
  for (size_t i = 0; i < len;) {
    unsigned int a = static_cast<unsigned char>(p[i++]);
    if (a >= 0x80) {
      unsigned int b = static_cast<unsigned char>(p[i++]);
      if (a >= 0xe0) {
        unsigned int c = static_cast<unsigned char>(p[i++]);
        if (a >= 0xf0) {
          unsigned int d = static_cast<unsigned char>(p[i++]);
          unsigned int val = ((a & 0x07) << 18) + ((b & 0x3f) << 12) + ((c & 0x3f) << 6) + (d & 0x3f) - 0x10000;
          *res++ = static_cast<jchar>(0xD800 + (val >> 10));
          *res++ = static_cast<jchar>(0xDC00 + (val & 0x3ff));
        } else {
          *res++ = static_cast<jchar>(((a & 0x0f) << 12) + ((b & 0x3f) << 6) + (c & 0x3f));
        }
      } else {
        *res++ = static_cast<jchar>(((a & 0x1f) << 6) + (b & 0x3f));
      }
    } else {
      *res++ = static_cast<jchar>(a);
    }
  }
}

static size_t get_utf8_from_utf16_length(const jchar *p, jsize len) {
  size_t result = 0;
  for (jsize i = 0; i < len; i++) {
    unsigned int cur = p[i];
    if ((cur & 0xF800) == 0xD800) {
      if (i < len) {
        unsigned int next = p[++i];
        if ((next & 0xFC00) == 0xDC00 && (cur & 0x400) == 0) {
          result += 4;
          continue;
        }
      }

      // TODO wrong UTF-16, it is possible
      return 0;
    }
    result += 1 + (cur >= 0x80) + (cur >= 0x800);
  }
  return result;
}

// API

jstring jni::to_jstring (JNIEnv *env, const std::string &s) {
  jsize surrogates = 0;
  jsize unicode_len = get_utf16_from_utf8_length(s.c_str(), s.size(), &surrogates);
  if (surrogates == 0) {
    // TODO '\0'
    return env->NewStringUTF(s.c_str());
  }
  jsize result_len = surrogates + unicode_len;
  if (result_len <= 256) {
    jchar result[256];
    utf8_to_utf16(s.c_str(), s.size(), result);
    return env->NewString(result, result_len);
  }

  auto result = std::make_unique<jchar[]>(result_len);
  utf8_to_utf16(s.c_str(), s.size(), result.get());
  return env->NewString(result.get(), result_len);
}

std::string jni::from_jstring (JNIEnv *env, jstring s) {
  if (!s) {
    return "";
  }
  jsize s_len = env->GetStringLength(s);
  const jchar *p = env->GetStringChars(s, nullptr);
  if (p == nullptr) {
    // parse_error = true;
    return std::string();
  }
  size_t len = get_utf8_from_utf16_length(p, s_len);
  std::string res(len, '\0');
  if (len) {
    utf16_to_utf8(p, s_len, &res[0]);
  }
  env->ReleaseStringChars(s, p);
  return res;
}

bool jni::throw_new (JNIEnv *env, const std::string &message, jclass clazz) {
  if (clazz != nullptr) {
    jstring jmessage = jni::to_jstring(env, message);
    jmethodID jctor;
    if (clazz == jni_class::AssertionError(env)) {
      jctor = env->GetMethodID(clazz, "<init>", "(Ljava/lang/Object;)V");
    } else {
      jctor = env->GetMethodID(clazz, "<init>", "(Ljava/lang/String;)V");
    }
    auto jexceptionObject = (jthrowable) env->NewObject(clazz, jctor, jmessage);
    env->Throw(jexceptionObject);
    return true;
  }
  return false;
}

bool throw_new (JNIEnv *env, const std::string &message, const std::string &exception = "java/lang/RuntimeException") {
  return jni::throw_new(env, message, jni_class::get(env, exception));
}