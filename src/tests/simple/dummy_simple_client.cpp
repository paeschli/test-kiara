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
 * dummy_simple_client.cpp
 *
 *  Created on: 12.10.2012
 *      Author: Dmitri Rubinstein
 */
#include "SimpleClient.hpp"
#include <queue>


class DummyFileSystem : public FileSystem
{
public:

    virtual FileHandle openFile()
    {
        // TODO implement this
        return 0;
    }

    virtual bool closeFile(FileHandle stream)
    {
        // TODO implement this
        return false;
    }

    virtual bool writeInt(FileHandle stream, boost::int32_t data)
    {
        // TODO implement this
        return false;
    }

    virtual bool writeFloat(FileHandle stream, float data)
    {
        // TODO implement this
        return false;
    }

    virtual bool writeData(FileHandle stream, const Data &data)
    {
        // TODO implement this
        return false;
    }

    virtual FilePos getFPos(FileHandle stream)
    {
        // TODO implement this
        return 0;
    }

    virtual bool setFPos(FileHandle stream, FilePos position)
    {
        // TODO implement this
        return false;
    }

    virtual ~DummyFileSystem()
    {
    }

private:
};

int main(int argc, char **argv)
{
    DummyFileSystem connection;
    SimpleClient client(&connection);
    client.run();
}
