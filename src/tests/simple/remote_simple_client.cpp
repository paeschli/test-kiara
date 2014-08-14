/*  KIARA - Middleware for efficient and QoS/Security-aware invocation of services and exchange of messages
 *
 *  Copyright (C) 2012, 2013  German Research Center for Artificial Intelligence (DFKI)
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
 * remote_simple_client.cpp
 *
 *  Created on: 12.10.2012
 *      Author: Dmitri Rubinstein
 */
#include "SimpleClient.hpp"
#include <cstring>
#include <cstdlib>
#include <queue>
#include <iostream>
#include <sstream>
#include <cassert>
#include <KIARA/kiara.h>
#include <KIARA/kiara_cxx_stdlib.hpp>

KIARA_CXX_DECL_STRUCT(Data,
    KIARA_CXX_STRUCT_MEMBER(i)
    KIARA_CXX_STRUCT_MEMBER(d))

KIARA_CXX_DECL_FUNC(OpenStream,
    KIARA_CXX_FUNC_RESULT(FileHandle &, result))

KIARA_CXX_DECL_FUNC(CloseStream,
    KIARA_CXX_FUNC_ARG(FileHandle, stream)
    KIARA_CXX_FUNC_RESULT(bool &, result))

KIARA_CXX_DECL_FUNC(Write_i32,
    KIARA_CXX_FUNC_ARG(FileHandle, stream)
    KIARA_CXX_FUNC_ARG(boost::int32_t, data)
    KIARA_CXX_FUNC_RESULT(bool &, result))

KIARA_CXX_DECL_FUNC(Write_float,
    KIARA_CXX_FUNC_ARG(FileHandle, stream)
    KIARA_CXX_FUNC_ARG(float, data)
    KIARA_CXX_FUNC_RESULT(bool &, result))

KIARA_CXX_DECL_FUNC(Write_Data,
    KIARA_CXX_FUNC_ARG(FileHandle, stream)
    KIARA_CXX_FUNC_ARG(Data, data)
    KIARA_CXX_FUNC_RESULT(bool &, result))

KIARA_CXX_DECL_FUNC(GetStreamPosition,
    KIARA_CXX_FUNC_ARG(FileHandle, stream)
    KIARA_CXX_FUNC_RESULT(FilePos &, result))

KIARA_CXX_DECL_FUNC(SetStreamPosition,
    KIARA_CXX_FUNC_ARG(FileHandle, stream)
    KIARA_CXX_FUNC_ARG(FilePos, position)
    KIARA_CXX_FUNC_RESULT(bool &, result))

class KiaraFileSystem : public FileSystem
{
public:

    KiaraFileSystem(KIARA_Context *ctx, const char *host, const int port)
        : ctx_(ctx)
        , conn_(0)
    {
        std::cout<<"Connecting with "<<host<<":"<<port<<std::endl;

        std::ostringstream ost;
        ost << "http://"<<host<<":"<<port<<"/kiara";

        std::cout<<"Connecting with "<<ost.str()<<std::endl;

        KIARA_Connection *conn = kiaraOpenConnection(ctx, ost.str().c_str());
        if (!conn)
        {
            std::cerr<<"Could not open connection : "<<kiaraGetContextError(ctx)<<std::endl;
            exit(1);
        }

#define CHECK_STUB(s)                                                                           \
        if (!s)                                                                                 \
        {                                                                                       \
            std::cerr<<"Could not generate stub: "<<kiaraGetConnectionError(conn_)<<std::endl;  \
            exit(1);                                                                            \
        }

        std::cout<<"Generating stubs..."<<std::endl;

        // Generate stubs for all service methods associated with the opened connection
        openStreamStub_   = KIARA_GENERATE_CLIENT_FUNC(conn_, "StreamIO.openStream", OpenStream, "");
        CHECK_STUB(openStreamStub_);
        closeStreamStub_  = KIARA_GENERATE_CLIENT_FUNC(conn, "StreamIO.closeStream", CloseStream, "");
        CHECK_STUB(closeStreamStub_);
        write_i32Stub_    = KIARA_GENERATE_CLIENT_FUNC(conn, "StreamIO.write_i32", Write_i32, "");
        CHECK_STUB(write_i32Stub_);
        write_floatStub_  = KIARA_GENERATE_CLIENT_FUNC(conn, "StreamIO.write_float", Write_float, "");
        CHECK_STUB(write_floatStub_);
        write_DataStub_   = KIARA_GENERATE_CLIENT_FUNC(conn, "StreamIO.write_Data", Write_Data, "");
        CHECK_STUB(write_DataStub_);
        getStreamPositionStub_  = KIARA_GENERATE_CLIENT_FUNC(conn, "StreamIO.getStreamPosition", GetStreamPosition, "");
        CHECK_STUB(getStreamPositionStub_);
        setStreamPositionStub_  = KIARA_GENERATE_CLIENT_FUNC(conn, "StreamIO.setStreamPosition", SetStreamPosition, "");
        CHECK_STUB(setStreamPositionStub_);
    }

    virtual FileHandle openFile()
    {
        FileHandle stream;

        int result = openStreamStub_(stream);
        if (result != KIARA_SUCCESS)
            return INVALID_FILE;
        return stream;
    }

    virtual bool closeFile(FileHandle stream)
    {
        bool success;

        int result = closeStreamStub_(stream, success);
        if (result != KIARA_SUCCESS)
            return false;

        return success;
    }

    virtual bool writeInt(FileHandle stream, boost::int32_t data)
    {
        bool success;

        int result = write_i32Stub_(stream, data, success);
        if (result != KIARA_SUCCESS)
            return false;

        return success;
    }

    virtual bool writeFloat(FileHandle stream, float data)
    {
        bool success;

        int result = write_floatStub_(stream, data, success);
        if (result != KIARA_SUCCESS)
            return false;

        return success;
    }

    virtual bool writeData(FileHandle stream, const Data &data)
    {
        bool success;

        int result = write_DataStub_(stream, data, success);
        if (result != KIARA_SUCCESS)
            return false;

        return success;
    }

    virtual FilePos getFPos(FileHandle stream)
    {
        FilePos pos;

        int result = getStreamPositionStub_(stream, pos);
        if (result != KIARA_SUCCESS)
            return INVALID_POSITION;

        return pos;
    }

    virtual bool setFPos(FileHandle stream, FilePos position)
    {
        bool success;

        int result = setStreamPositionStub_(stream, position, success);
        if (result != KIARA_SUCCESS)
            return false;

        return success;
    }

    virtual ~KiaraFileSystem()
    {
        if (conn_)
            kiaraCloseConnection(conn_);
    }

private:
    KIARA_Context *ctx_;
    KIARA_Connection *conn_;

    OpenStream openStreamStub_;
    CloseStream closeStreamStub_;
    Write_i32 write_i32Stub_;
    Write_float write_floatStub_;
    Write_Data write_DataStub_;
    GetStreamPosition getStreamPositionStub_;
    SetStreamPosition setStreamPositionStub_;
};

void printUsageInfo(const char *appName)
{
    std::cout<<
            "KIARA Simple Client\n"
            "Usage:\n"
            << appName << " [options] host port\n\n"
            "  -h    | -help | --help              : prints this and exit\n"
            "  -i    | --info                      : print info\n"
            <<std::endl;
}

#define ARG(str) (!strcmp(argv[i], str))
#define ARG_STARTS_WITH(str,len) (!strncmp(argv[i], str, len))
#define ARG_CONTAINS(str) strstr(argv[i], str)
#define APP_ERROR(msg) { std::cerr<<msg<<std::endl; exit(1); }

int main(int argc, char **argv)
{
    /* Initialize KIARA library */
    kiaraInit(&argc, argv);

    int port = 8080;
    const char *host = "localhost";
    //bool printInfoAndExit = false;
    std::vector<const char *> args;

    // parse command line
    int i;
    for (i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            if (ARG("-help") || ARG("--help") || ARG("-h"))
            {
                printUsageInfo(argv[0]);
                exit(0);
            }
            if (ARG("-i") || ARG("--info"))
            {
                //printInfoAndExit = true;
            }
            else if (ARG("--"))
            {
                i++;
                break;
            }
            else
            {
                APP_ERROR("Unknown option "<<argv[i]);
            }
        }
        else
        {
            // no '-' prefix, assume this is an argument
            args.push_back(argv[i]);
        }
    }

    // read rest files
    if (args.size() >= 1)
        host = args[0];

    if (args.size() >= 2)
        port = atoi(args[1]);

    KIARA_Context *ctx = kiaraNewContext();

    /* Run client */
    KiaraFileSystem connection(ctx, host, port);
    SimpleClient client(&connection);
    client.run();

    /* Finalize context and library */
    kiaraFreeContext(ctx);
    kiaraFinalize();
}
