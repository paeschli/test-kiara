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
 * SimpleClient.hpp
 *
 *  Created on: 12.10.2012
 *      Author: Dmitri Rubinstein
 */

#ifndef SIMPLECLIENT_HPP_INCLUDED
#define SIMPLECLIENT_HPP_INCLUDED

#include <string>
#include <iostream>
#include <boost/cstdint.hpp>

typedef boost::uint64_t FilePos;
typedef boost::int64_t FileHandle;

const FileHandle INVALID_FILE = -1;
const FilePos INVALID_POSITION = static_cast<FilePos>(-1);

struct Data
{
    boost::int32_t i;
    double d;
};

class FileSystem
{
public:

    // Mapped to StreamIO.openStream
    virtual FileHandle openFile() = 0;

    // Mapped to StreamIO.closeStream
    virtual bool closeFile(FileHandle file) = 0;

    // Mapped to StreamIO.write_i32
    virtual bool writeInt(FileHandle file, boost::int32_t data) = 0;

    // Mapped to StreamIO.write_float
    virtual bool writeFloat(FileHandle file, float data) = 0;

    // Mapped to StreamIO.write_Data
    virtual bool writeData(FileHandle file, const Data &data) = 0;

    // Mapped to StreamIO.getFPos
    virtual FilePos getFPos(FileHandle file) = 0;

    // Mapped to StreamIO.setFPos
    virtual bool setFPos(FileHandle file, FilePos position) = 0;

    virtual ~FileSystem() { }
};

class SimpleClient
{
public:

    SimpleClient(FileSystem *fileSystem);

    ~SimpleClient();

    void run();

private:
    FileSystem *fileSystem_;
    FileHandle file_;
    FilePos pos_;
};

#endif /* SIMPLECLIENT_HPP_INCLUDED */
