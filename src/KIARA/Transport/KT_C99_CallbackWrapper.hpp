/**
 * @file KT_C99_CallbackWrapper.hpp
 * @author Mathias Hablützel <habl@zhaw.ch>
 * @version 1.0
 * @license TBD
 *
 * @brief C99 callback wrapper for C++ poller in KT_Zeromq.cpp
 * 
 */

#ifndef C99CALLBACKWRAPPER_HPP_
#define C99CALLBACKWRAPPER_HPP_

#include "k_transport.h"
#include <functional>
#include <iostream>

#include "KT_Msg.hpp"
#include "KT_Connection.hpp"
#include "KT_Session.hpp"

namespace KIARA
{
namespace Transport
{
/**
 * @class KT_C99_CallbackWrapper
 * Since the callback function is written in C++ and expects objects it is not
 * directly compatible to call C99 functions. This class creates a C++ to C
 * function callback wrapper.
 * 
 * This works in three steps:
 * -# Instantiate the wrapper an pass the C callback.
 * -# Call make_function as this will create the a function pointer for the C++
 *    code used in e.g. KT_Zeromq.cpp which is then called.
 * -# The actually called function is callback_wrapper which mangles the C++
 *    objects to C structs that can be used in the C99 callback function written
 *    by the developer.
 * 
 * @see k_transport.cpp
 * @warning This class requires C++11 and std::thread support.
 */
class KT_C99_CallbackWrapper
{
public:
    KT_C99_CallbackWrapper(kt_handle_t);
    std::function<void(KT_Msg&, KT_Session*, KT_Connection*)> make_function();
    virtual ~KT_C99_CallbackWrapper ();
    void set_callback (kt_handle_t);
    void callback_wrapper (KT_Msg&, KT_Session*, KT_Connection*);

private:
    kt_handle_t _c99_cb = nullptr;
};

} /* namespace Transport */
} /* namespace KIARA */

#endif /* C99CALLBACKWRAPPER_HPP_ */
