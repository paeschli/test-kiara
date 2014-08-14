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
#ifndef KIARA_IDL_IDLPARSERCONTEXT_HPP_INCLUDED
#define KIARA_IDL_IDLPARSERCONTEXT_HPP_INCLUDED

#include <iostream>
#include <vector>
#include "IDLToken.hpp"
#include <KIARA/DB/Module.hpp>

namespace KIARA
{

class KIARA_API IDLParserContext
{
public:
    void *scanner;
    void *parser;

    std::istream *is; // input stream
    std::string fileName; // file name used
    int lineNum;
    IDLToken token;
    std::string scannerError;
    bool parsingFailed;

    IDLParserContext(const Module::Ptr &module, const std::string &fileName = "<stdin>");

    IDLParserContext(const Module::Ptr &module, std::istream &is, const std::string &fileName);

    ~IDLParserContext();

    std::string getText();

    const Module::Ptr & getModule() { return module_; }

    void setScannerError(const std::string &error)
    {
        scannerError = error;
    }

    const std::string & getScannerError() const
    {
        return scannerError;
    }

    void clearScannerError()
    {
        scannerError.clear();
    }

    bool isScannerError() const
    {
        return !scannerError.empty();
    }

    bool isParsingFailed() const
    {
        return parsingFailed;
    }

    void addParserError(const std::string &error)
    {
        parserErrors.push_back(error);
    }

    const std::vector<std::string> & getParserErrors()
    {
        return parserErrors;
    }

    /** Returns token ID or -1 on error.
     */
    int lex();

    /** Returns true if the input was parsed correctly.
     *  If strict argument is false error recovery will be used.
     */
    bool parse(bool strict = true);

private:

    std::vector<std::string> parserErrors;
    Module::Ptr module_;

    void initScanner();
    void initParser();
    void destroyScanner();
    void destroyParser();
};

} // namespace KIARA

#endif
