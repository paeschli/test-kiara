/*  KIARA - Middleware for efficient and QoS/Security-aware invocation of services and exchange of messages
 *
 *  Copyright (C) 2013  German Research Center for Artificial Intelligence (DFKI)
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
 * codegentest.cpp
 *
 *  Created on: Sep 10, 2013
 *      Author: Dmitri Rubinstein
 */

// KIARA
#include <KIARA/Core/LibraryInit.hpp>
#include <DFC/Utils/StrUtils.hpp>
#include <KIARA/LLVM/Utils.hpp>
#include <KIARA/DB/Module.hpp>
#include <KIARA/Impl/Core.hpp>

// Lang
#include <KIARA/Compiler/Lexer.hpp>
#include <KIARA/Compiler/LangParser.hpp>
#include <KIARA/Compiler/LLVM/Evaluator.hpp>
#include <KIARA/Compiler/IRUtils.hpp>

// IDL
#include <KIARA/IDL/IDLParserContext.hpp>
#include <KIARA/IDL/IDLWriter.hpp>

#include <KIARA/IRGen/IRGen.hpp>

// stdlib
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>
#include <map>
#include <vector>
#include <fstream>

#define DFC_DO_DEBUG
#include <DFC/Utils/Debug.hpp>

struct KIARA_TestEnv : public KIARA::World
{
    KIARA_TestEnv();

    virtual ~KIARA_TestEnv();

    KIARA::World & world() { return *this; }

    const KIARA::Module::Ptr & getModule() const { return module_; }

    KIARA::Impl::Error & getError() { return error_; }

    const KIARA::Impl::Error & getError() const { return error_; }

    bool isError() const { return error_.isError(); }

    const char * getErrorMessage() const { return error_.getMessage(); }

    KIARA_Result getErrorCode() const { return error_.getErrorCode(); }

    void setError(KIARA_Result errorCode, const std::string &errorMessage)
    {
        error_.set(errorCode, errorMessage);
    }

    void clearError() { error_.clear(); }

    KIARA::PtrType::Ptr getContextPtrType() const { return contextPtrType_; }
    KIARA::PtrType::Ptr getConnectionPtrType() const { return connectionPtrType_; }
    KIARA::PtrType::Ptr getMessagePtrType() const { return messagePtrType_; }
    KIARA::PtrType::Ptr getFuncObjPtrType() const { return funcObjPtrType_; }
    KIARA::PtrType::Ptr getDBufferPtrType() const { return dbufferPtrType_; }
    KIARA::PtrType::Ptr getBinaryStreamPtrType() const { return binaryStreamPtrType_; }

    bool loadIDL(const std::string &fileName);
    bool loadIDL(std::istream &in, const std::string &fileName);
    bool loadIDLFromURL(const std::string &url);
    bool loadIDLFromURL(KIARA::URLLoader::Connection * handle, const std::string &url);

    KIARA::Compiler::Evaluator & getEvaluator()
    {
        return evaluator_;
    }

    bool includeFile(const std::string &fileName);

    bool loadModule(const std::string &fileName);

    bool loadPluginLibrary(const std::string &libName);

    void runTest();

private:
    KIARA::PathFinder pathFinder_;
    KIARA::Module::Ptr module_;
    KIARA::Impl::Error error_;
    KIARA::TypeSet types_;
    KIARA::PtrType::Ptr contextPtrType_;
    KIARA::PtrType::Ptr connectionPtrType_;
    KIARA::PtrType::Ptr messagePtrType_;
    KIARA::PtrType::Ptr funcObjPtrType_;
    KIARA::PtrType::Ptr userTypePtrType_;
    KIARA::PtrType::Ptr dbufferPtrType_;
    KIARA::PtrType::Ptr binaryStreamPtrType_;
    KIARA::Compiler::Evaluator evaluator_;
};

// TODO declare only basic types and use aliases ?

KIARA_TestEnv::KIARA_TestEnv()
    : KIARA::World()
    , pathFinder_()
    , module_()
    , evaluator_(*this, KIARA::llvmGetGlobalContext()) // FIXME separate context per thread
{
    char *mpath = getenv("KIARA_MODULE_PATH");
    pathFinder_.setSearchPathsFromPathList(mpath);

    module_ = new KIARA::Module(world(), "context");
    KIARA::StructType::Ptr contextType = KIARA::StructType::create(world(), "KIARA_Context");
    KIARA::StructType::Ptr connectionType = KIARA::StructType::create(world(), "KIARA_Connection");
    KIARA::StructType::Ptr messageType = KIARA::StructType::create(world(), "KIARA_Message");
    KIARA::StructType::Ptr funcObjType = KIARA::StructType::create(world(), "KIARA_FuncObj");
    KIARA::StructType::Ptr userTypeType = KIARA::StructType::create(world(), "KIARA_UserType");
    KIARA::StructType::Ptr dbufferType = KIARA::StructType::create(world(), "kr_dbuffer_t");
    KIARA::StructType::Ptr binaryStreamType = KIARA::StructType::create(world(), "KIARA_BinaryStream");

    contextPtrType_ = KIARA::PtrType::get(contextType);
    connectionPtrType_ = KIARA::PtrType::get(connectionType);
    messagePtrType_ = KIARA::PtrType::get(messageType);
    funcObjPtrType_ = KIARA::PtrType::get(funcObjType);
    userTypePtrType_ = KIARA::PtrType::get(userTypeType);
    dbufferPtrType_ = KIARA::PtrType::get(dbufferType);
    binaryStreamPtrType_ = KIARA::PtrType::get(binaryStreamType);

    // All types that are also defined in KL files must be added to the top scope, otherwise they will be defined twice
    KIARA::IR::IRUtils::addObjectToScope(contextType->getTypeName(), contextType, evaluator_.getTopScope());
    KIARA::IR::IRUtils::addObjectToScope(connectionType->getTypeName(), connectionType, evaluator_.getTopScope());
    KIARA::IR::IRUtils::addObjectToScope(messageType->getTypeName(), messageType, evaluator_.getTopScope());
    KIARA::IR::IRUtils::addObjectToScope(funcObjType->getTypeName(), funcObjType, evaluator_.getTopScope());
    KIARA::IR::IRUtils::addObjectToScope(userTypeType->getTypeName(), userTypeType, evaluator_.getTopScope());
    KIARA::IR::IRUtils::addObjectToScope(dbufferType->getTypeName(), dbufferType, evaluator_.getTopScope());
    KIARA::IR::IRUtils::addObjectToScope(binaryStreamType->getTypeName(), binaryStreamType, evaluator_.getTopScope());

    includeFile("stdlib.kl");
    includeFile("api.kl");
    loadModule("message.bc");

    //evaluator_.linkNativeFunc("getConnection", (void*)&kiara_getConnection);
    //evaluator_.linkNativeFunc("sendData", (void*)&kiara_sendData);

    // For CURL :
    // loadPluginLibrary("warnless");
    // loadPluginLibrary("curl_shared");
}

KIARA_TestEnv::~KIARA_TestEnv()
{
}

bool KIARA_TestEnv::loadIDL(std::istream &in, const std::string &fileName)
{
    KIARA::IDLParserContext ctx(module_, in, fileName);
    return ctx.parse();
}

bool KIARA_TestEnv::loadIDLFromURL(const std::string &url)
{
    std::string idlContents;
    bool result = KIARA::URLLoader::load(url, idlContents);
    if (!result)
        return false;
    std::istringstream iss(idlContents);
    return loadIDL(iss, url);
}

bool KIARA_TestEnv::loadIDLFromURL(KIARA::URLLoader::Connection * handle, const std::string &url)
{
    std::string idlContents;
    bool result = KIARA::URLLoader::load(handle, url, idlContents);
    if (!result)
        return false;
    std::istringstream iss(idlContents);
    return loadIDL(iss, url);
}

bool KIARA_TestEnv::loadIDL(const std::string &fileName)
{
    std::ifstream inf(fileName.c_str());
    return loadIDL(inf, fileName);
}

bool KIARA_TestEnv::includeFile(const std::string &fileName)
{
    if (isError())
        return false;

    std::string errorMsg;
    std::string path = pathFinder_.findPath(fileName, &errorMsg);
    if (path.empty())
    {
        setError(KIARA_CONFIG_ERROR, errorMsg);
        return false;
    }

    evaluator_.includeFile(path);
    return true;
}

bool KIARA_TestEnv::loadModule(const std::string &fileName)
{
    if (isError())
        return false;

    std::string errorMsg;
    std::string path = pathFinder_.findPath(fileName, &errorMsg);
    if (path.empty())
    {
        setError(KIARA_CONFIG_ERROR, errorMsg);
        return false;
    }

    if (!evaluator_.loadModule(path, errorMsg))
    {
        setError(KIARA_CONFIG_ERROR, errorMsg);
    }
    return true;
}

bool KIARA_TestEnv::loadPluginLibrary(const std::string &libName)
{
    if (isError())
        return false;

    std::string fileName;
#ifdef _WIN32
    fileName = libName + ".dll";
#else
    fileName = "lib" + libName + ".so";
#endif

    std::string errorMsg;
    std::string path = pathFinder_.findPath(fileName, &errorMsg);
    if (path.empty())
    {
        setError(KIARA_CONFIG_ERROR, errorMsg);
        return false;
    }

    evaluator_.loadPlugin(path);
    return true;
}

void KIARA_TestEnv::runTest()
{
    using namespace KIARA::Compiler;

    std::cerr<<"running tests..."<<std::endl;

    World &world = *this;

    std::string serviceMethodName = "service_method";

    //KIARA::IRGenContext genCtx(this, getEvaluator().getTopScope());
    KIARA::Compiler::IRBuilder builder(getEvaluator().getTopScope());

    KIARA::Type::Ptr tyArgs[] = {
        type_c_int(),
        type_c_char_ptr()
    };

    KIARA::FunctionType::Ptr fty = KIARA::FunctionType::get(world.type_c_int(), tyArgs);

    std::vector<KIARA::IR::Prototype::Arg> args;

    args.push_back(
            KIARA::IR::Prototype::Arg(
                    "$closure",
                    getFuncObjPtrType()));

    for (size_t i = 0; i < fty->getNumParams(); ++i)
    {
        args.push_back(
                KIARA::IR::Prototype::Arg(
                        fty->getParamName(i),
                        fty->getParamType(i)));
    }

    KIARA::IR::Prototype::Ptr proto = KIARA::Compiler::createCFuncProto(
            fty->getFullTypeName(),
            world.type_c_int(),
            args,
            world);
    TFunction func = Function(proto);

    {
        KIARA::Compiler::IRBuilder::ScopeGuard g("func", builder);
        builder.initFunctionScope(func);

        TLiteral successVal = Literal<KIARA_Result>(KIARA_SUCCESS, builder);
        TLiteral exceptionVal = Literal<KIARA_Result>(KIARA_EXCEPTION, builder);

        TVar statusVar = Var("$status", world.type_c_int(), builder);

        TLiteral resultVal = successVal;

        TVar connVar = Var("$connection", getConnectionPtrType(), builder);

        Callee getConnection("getConnection", builder);
        Callee createRequestMessage("createRequestMessage", builder);
        Callee assign("=", builder);
        Callee notEqual("!=", builder);
        Callee equal("==", builder);

        TCall connVal = getConnection(Arg(func, 0));
        TVar msgVar = Var("$msg", getMessagePtrType(), builder);

        TCall msgVal = createRequestMessage(
                connVar,
                Literal<std::string>(serviceMethodName, builder),
                Literal<size_t>(serviceMethodName.length(), builder));

        TBlock msgBlock = Block(world, "msgBlock");
        TBlock serBlock = Block(world, "serBlock");
        TBlock deserBlock = Block(world, "deserBlock");

        TBlock expr = Block(
            assign(statusVar, Literal<int>(12, builder)),
            If(notEqual(statusVar, successVal), Break(msgBlock)),
            If(equal(statusVar, Literal<int>(33, builder)), Break(serBlock))
        );

        msgBlock->addExpr(expr);

        msgBlock->addExpr(statusVar);

        TLet body = Let(statusVar, resultVal,
                         Let(connVar, connVal,
                              Let(msgVar, msgVal, msgBlock)));
        func->setBody(body);
    }

    std::cout<<func->toString()<<std::endl;
}

void showUsage()
{
    std::cout<<"Run language compiler\n"
        "Usage:\n"
        "kiara_codegentest [options] files\n"
        "\n"
        "  -h    | -help | --help         : prints this and exit\n"
        "  -o filename                    : output LLVM module to file"
        <<std::endl;
}

#define ARG(str) (!strcmp(argv[i], str))
#define ARG_STARTS_WITH(str,len) (!strncmp(argv[i], str, len))
#define ARG_CONTAINS(str) strstr(argv[i], str)
#define APP_ERROR(msg) { std::cerr<<msg<<std::endl; exit(1); }

int main(int argc, char **argv)
{
    const char *outputFile = 0;
    std::vector<const char *> files;
    typedef std::vector<const char *>::iterator Iter;

    // parse command line
    int i;
    for (i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            if (ARG("-help") || ARG("--help") || ARG("-h"))
            {
                showUsage();
                exit(0);
            }
            else if (ARG("-o"))
            {
                if (++i < argc)
                {
                    outputFile = argv[i];
                }
                else
                {
                    APP_ERROR("-o option require argument");
                }
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
            // no '-' prefix, assume this is a file name
            files.push_back(argv[i]);
        }
    }

    // read rest files
    for (; i < argc; i++)
    {
        files.push_back(argv[i]);
    }

    KIARA::LibraryInit init;

    KIARA::World world;

#if 0
    KIARA::Compiler::Lexer lexer("=<>!+-*&|/%^", "=<>&|");
    lexer.setCXXComment(false);
    std::string fileName("stdin");
    std::ifstream fin;
    if (files.size())
    {
        fileName = files[0];
        fin.open(files[0]);
        if (!fin)
        {
            std::cerr<<"Error: Could not open file: "<<files[0]<<std::endl;
            return 1;
        }
        lexer.reset(fin);
    }
#endif

    // Initialize LLVM and JIT
    KIARA::llvmInitialize();
    KIARA::llvmInitializeJIT();

#if 1 // set to 0 in order to output tokens

    // check exception only on Windows for better error reporting
#ifdef _WIN32
    try
#endif
    {
        KIARA_TestEnv testEnv;
        testEnv.runTest();
    }
#ifdef _WIN32
    catch (const std::exception &ex)
    {
        std::cerr<<"!!! Exception: "<<ex.what()<<std::endl;
        throw;
    }
#endif

    KIARA::llvmShutdown();

#else
    KIARA::Compiler::Token token;
    while (lexer.next(token) != KIARA::Compiler::TOK_EOF)
    {
        std::cout<<token<<" at "<<token.loc.line<<":"<<token.loc.col<<std::endl;
    }
#endif

    return 0;
}
