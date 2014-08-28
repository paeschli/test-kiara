/**
 * @file KT_Connection.cpp
 * @author Mathias Hablützel <habl@zhaw.ch>
 * @version 1.0
 * @license TBD
 *
 * @brief Connection interface.
 */
#include "KT_Connection.hpp"

KIARA::Transport::KT_Connection::KT_Connection ( ) {
    _sessions = new std::map<std::string, KIARA::Transport::KT_Session*>;
}

KIARA::Transport::KT_Connection::~KT_Connection ( ) { }
