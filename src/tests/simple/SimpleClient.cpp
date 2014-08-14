/*  KIARA - Middleware for efficient and QoS/Security-aware invocation of services and exchange of messages
 *
 *  Copyright (C) 2012  German Research Center for Artificial Intelligence (DFKI)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/*
 * SimpleClient.cpp
 *
 *  Created on: 12.10.2012
 *      Author: Dmitri Rubinstein
 */

#include "SimpleClient.hpp"
#include <boost/assert.hpp>
#include <iostream>
#include <cstdio>

SimpleClient::SimpleClient(FileSystem *fileSystem)
    : fileSystem_(fileSystem)
    , file_(INVALID_FILE)
    , pos_(0)
{
    BOOST_ASSERT(fileSystem_ != 0);
}

SimpleClient::~SimpleClient()
{
    if (fileSystem_ && (file_ != INVALID_FILE))
        fileSystem_->closeFile(file_);
}

void SimpleClient::run()
{
    // 1. Open stream
    file_ = fileSystem_->openFile();

    // 2. Send some data
    fileSystem_->writeFloat(file_, 42.0);
    fileSystem_->writeInt(file_, 123);
    fileSystem_->writeInt(file_, 321);
    Data data = {34, 12.0};
    fileSystem_->writeData(file_, data);

    FilePos pos = fileSystem_->getFPos(file_);
    data.i = pos;
    data.d = pos;
    fileSystem_->writeData(file_, data);

    // 3. Close stream

    fileSystem_->closeFile(file_);
    file_ = static_cast<FileHandle>(-1);

    fileSystem_ = 0;
}
