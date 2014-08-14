// Copyright (c) 2010, Object Computing, Inc.
// All rights reserved.

#ifndef MIDDLEWARENEWSBRIEF_FUNCTIONS_T_H
#define MIDDLEWARENEWSBRIEF_FUNCTIONS_T_H

#include <iostream>

namespace MiddlewareNewsBrief
{

template <typename T, typename A>
void stream(T& obj, A& archive, size_t i)
{
  obj.isEcho = false;
  obj.counter = i;

  archive << obj;
}

template <typename T, typename A>
int check(T& obj, A& archive, size_t i)
{
  archive >> obj;

  if (obj.isEcho == false || obj.counter != i)
  {
    std::cerr << "Message reply isEcho or counter mismatch" << std::endl;
    return -1;
  }
  return 0;
}


template <typename T, typename IA, typename OA>
void check_and_restream(T& obj, IA& in_archive, OA& out_archive, size_t counter)
{
  in_archive >> obj;

  if (obj.isEcho == true || obj.counter != counter)
  {
    std::cerr << "Subscriber message isEcho or counter mismatch" << std::endl;
  }
  obj.isEcho = true;
  out_archive << obj;
}

} // namespace

#endif
