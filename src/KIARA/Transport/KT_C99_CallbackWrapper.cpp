/**
 * @file KT_C99_CallbackWrapper.cpp
 * @author Mathias Hablützel <habl@zhaw.ch>
 * @version 1.0
 * @license TBD
 *
 * @brief C99 callback wrapper for C++ poller in KT_Zeromq.cpp
 * 
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
 */

#include "KT_C99_CallbackWrapper.hpp"

namespace KIARA
{
namespace Transport
{

/**
 * @brief Instantiate the wrapper with the callback handle from C99
 * @param handle Callback handle
 */
KT_C99_CallbackWrapper::KT_C99_CallbackWrapper (kt_handle_t handle) :
        _c99_cb (handle)
{

}

KT_C99_CallbackWrapper::~KT_C99_CallbackWrapper ()
{
    // TODO Auto-generated destructor stub
}

/**
 * @brief Generate the callback function for the poller or more generally the
 *   implementations of the transport stacks.
 * @return Function pointer.
 */
std::function<void (KT_Msg&, KT_Session*, KT_Connection*)> KT_C99_CallbackWrapper::make_function ()
{
    using namespace std::placeholders;
    return std::bind(&KT_C99_CallbackWrapper::callback_wrapper, this, _1, _2, _3);
}

/**
 * @brief The callback function called by the poller which wraps the objects to
 *   C structs which are used by the developer code.
 * @param msg Message object received by the transport stack.
 * @param sess Session object indentifying the current connection.
 * @param conn Connection object containing the connection information.
 */
void KT_C99_CallbackWrapper::callback_wrapper (KT_Msg& msg, KT_Session* sess,
        KT_Connection* conn)
{
    // Because calling a nullptr is a stupid idea. BTDT
    if (nullptr == _c99_cb) return;

    // Mangle the objects to structs.
    kt_conn_session_t* conn_sess = new kt_conn_session {conn, sess};
    kt_msg_t* msg_t = reinterpret_cast<kt_msg_t*>(&msg);
    // Call the C function.
    _c99_cb(conn_sess, msg_t);
}

} /* namespace Transport */
} /* namespace KIARA */
