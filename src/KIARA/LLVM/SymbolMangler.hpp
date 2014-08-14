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
 * SymbolMangler.hpp
 *
 *  Created on: Oct 21, 2013
 *      Author: Simon Moll
 */

#ifndef KIARA_LLVM_SYMBOLMANGLER_HPP_INCLUDED
#define KIARA_LLVM_SYMBOLMANGLER_HPP_INCLUDED

#include <sstream>

namespace KIARA
{

class SymbolMangler
{
	const bool AllowNameToStartWithDigits;
	const bool AllowQuotesInName;
	const bool AllowPeriod;
	const bool AllowUTF8;

	bool isAcceptableChar(char C) const;
	char HexDigit(int V) const;
	void MangleLetter(std::stringstream & out, unsigned char C) const;
	/// NameNeedsEscaping - Return true if the identifier \p Str needs quotes
	/// for this assembler.
	bool NameNeedsEscaping(const std::string & Str) const;
	void appendMangledName(std::stringstream & out, std::string Str) const;
	void appendMangledQuotedName(std::stringstream & out, std::string Str) const;


public:
	SymbolMangler()
	: AllowNameToStartWithDigits(false)
	, AllowQuotesInName(false)
	, AllowPeriod(true)
	, AllowUTF8(false)
	{}

	~SymbolMangler()
	{}

	std::string getNameWithPrefix(std::string Name) const;
};

}

#endif /* SYMBOLMANGLER_HPP_ */
