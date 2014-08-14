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
 * SymbolMangler.cpp
 *
 *  Created on: Oct 21, 2013
 *      Author: Simon Moll
 */

/*
 * This is mostly taken from llvm/Target/Mangler (LLVM 3.3) and modified to remove dependence on MC-objects
 */

#include <KIARA/LLVM/SymbolMangler.hpp>

#include <llvm/Target/Mangler.h>

#include <llvm/Support/raw_os_ostream.h>
#include <cassert>
#include <sstream>

#ifdef VERBOSE
#define IF_VERBOSE(X) X
#else
#define IF_VERBOSE(X)
#endif

using namespace llvm;

namespace KIARA
{

bool SymbolMangler::isAcceptableChar(char C) const
{
  if ((C < 'a' || C > 'z') &&
      (C < 'A' || C > 'Z') &&
      (C < '0' || C > '9') &&
      C != '_' && C != '$' && C != '@' &&
      !(AllowPeriod && C == '.') &&
      !(AllowUTF8 && (C & 0x80)))
    return false;
  return true;
}

char SymbolMangler::HexDigit(int V) const
{
  return V < 10 ? V+'0' : V+'A'-10;
}

void SymbolMangler::MangleLetter(std::stringstream & out, unsigned char C) const
{
  out
	  << '_'
	  << HexDigit(C >> 4)
	  << HexDigit(C & 15)
	  << '_';
}

/// NameNeedsEscaping - Return true if the identifier \p Str needs quotes
/// for this assembler.
bool SymbolMangler::NameNeedsEscaping(const std::string & Str) const
{
  assert(!Str.empty() && "Cannot create an empty MCSymbol");

  // If the first character is a number and the target does not allow this, we
  // need quote
  const bool AllowNameToStartWithDigits = false;
  if (!AllowNameToStartWithDigits && Str[0] >= '0' && Str[0] <= '9')
    return true;

  // If any of the characters in the string is an unacceptable character, force
  // quotes.

  for (unsigned i = 0, e = Str.size(); i != e; ++i)
    if (!isAcceptableChar(Str[i]))
      return true;
  return false;
}

/// appendMangledName - Add the specified string in mangled form if it uses
/// any unusual characters.
void SymbolMangler::appendMangledName(std::stringstream & out, std::string Str) const
{
  // The first character is not allowed to be a number unless the target
  // explicitly allows it.
  if (!AllowNameToStartWithDigits && Str[0] >= '0' && Str[0] <= '9') {
    MangleLetter(out, Str[0]);
    Str = Str.substr(1);
  }

  for (unsigned i = 0, e = Str.size(); i != e; ++i) {
    if (!isAcceptableChar(Str[i]))
      MangleLetter(out, Str[i]);
    else
	out << Str[i];
  }
}


/// appendMangledQuotedName - On systems that support quoted symbols, we still
/// have to escape some (obscure) characters like " and \n which would break the
/// assembler's lexing.
void SymbolMangler::appendMangledQuotedName(std::stringstream & out, std::string Str) const{
  for (unsigned i = 0, e = Str.size(); i != e; ++i) {
    if (Str[i] == '"' || Str[i] == '\n')
      MangleLetter(out, Str[i]);
    else
      out << Str[i];
  }
}


/// getNameWithPrefix - Fill OutName with the name of the appropriate prefix
/// and the specified name as the global variable name.  GVName must not be
/// empty.
std::string SymbolMangler::getNameWithPrefix(std::string Name) const
{
	 Mangler::ManglerPrefixTy PrefixTy = Mangler::Default;

	std::stringstream buffer;
	std::string oldName = Name;

  assert(!Name.empty() && "getNameWithPrefix requires non-empty name");

  // If the global name is not led with \1, add the appropriate prefixes.
  if (Name[0] == '\1') {
    Name = Name.substr(1);
  } else {
	  // TODO prefixing
#if 0
    if (PrefixTy == Mangler::Private) {
      const char *Prefix = MAI.getPrivateGlobalPrefix();
      OutName.append(Prefix, Prefix+strlen(Prefix));
    } else if (PrefixTy == Mangler::LinkerPrivate) {
      const char *Prefix = MAI.getLinkerPrivateGlobalPrefix();
      OutName.append(Prefix, Prefix+strlen(Prefix));
    }

    const char *Prefix = MAI.getGlobalPrefix();
    if (Prefix[0] == 0)
      ; // Common noop, no prefix.
    else if (Prefix[1] == 0)
      OutName.push_back(Prefix[0]);  // Common, one character prefix.
    else
      OutName.append(Prefix, Prefix+strlen(Prefix)); // Arbitrary length prefix.
#endif
  }

	// If this is a simple string that doesn't need escaping, just append it.
	if (!NameNeedsEscaping(Name) ||
		// If quotes are supported, they can be used unless the string contains
		// a quote or newline.
		(AllowQuotesInName && (Name.find_first_of("\n\"") == std::string::npos)))
	{
		buffer << Name;
	} else if (!AllowQuotesInName) {

		// On systems that do not allow quoted names, we need to mangle most
		// strange characters.
		appendMangledName(buffer, Name);
	} else {

		// Okay, the system allows quoted strings.  We can quote most anything, the
		// only characters that need escaping are " and \n.
		assert(Name.find_first_of("\n\"") != std::string::npos);
		appendMangledQuotedName(buffer, Name);
	}

	std::string newName = buffer.str();
	IF_VERBOSE(llvm::errs() << "[SymbolMangler] Mangled " << Name << " to " << newName << "\n";)
	return newName;
}


#if 0
/// AddFastCallStdCallSuffix - Microsoft fastcall and stdcall functions require
/// a suffix on their name indicating the number of words of arguments they
/// take.
static void AddFastCallStdCallSuffix(SmallVectorImpl<char> &OutName,
                                     const Function *F, const DataLayout &TD) {
  // Calculate arguments size total.
  unsigned ArgWords = 0;
  for (Function::const_arg_iterator AI = F->arg_begin(), AE = F->arg_end();
       AI != AE; ++AI) {
    Type *Ty = AI->getType();
    // 'Dereference' type in case of byval parameter attribute
    if (AI->hasByValAttr())
      Ty = cast<PointerType>(Ty)->getElementType();
    // Size should be aligned to DWORD boundary
    ArgWords += ((TD.getTypeAllocSize(Ty) + 3)/4)*4;
  }

  raw_svector_ostream(OutName) << '@' << ArgWords;
}
#endif

}
