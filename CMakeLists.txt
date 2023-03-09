#
# Copyright © Vyacheslav Krylov (slavone@protonmail.ch) 2014
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
# File created on 18/01/2021
#

project(jni-utils)

add_library(jni-utils STATIC
  src/jni_utils.cpp
  src/jni_method.cpp
  src/jni_class.cpp
)
target_include_directories(jni-utils PUBLIC
  include
)