/**
 * @file Debug.hpp
 * @author Mathias Hablützel <habl@zhaw.ch>
 * @version 1.0
 * @license TBD
 *
 * @brief Debug functions go here.
 */

#ifndef DEBUG_HPP_
#define DEBUG_HPP_

#include <iostream>
#include <iomanip>

/** Dumps the memory region into hex format to std::cout
 *
 * @param mem A pointer to the memory region to dump
 * @param n Number of bytes to dump to
 */
void Dump( const void * mem, unsigned int n ) {
  const char * p = reinterpret_cast< const char *>( mem );
  for ( unsigned int i = 0; i < n; i++ ) {
     std::cout << std::hex << int(p[i]) << " ";
  }
  std::cout << std::endl;
}

#endif /* DEBUG_HPP_ */
