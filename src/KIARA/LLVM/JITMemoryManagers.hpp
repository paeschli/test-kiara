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
 * JITMemoryManagers.hpp
 *
 *  Created on: Jul 22, 2013
 *      Author: Simon Moll
 */

#ifndef KIARA_LLVM_JITMEMORYMANAGERS_HPP_INCLUDED
#define KIARA_LLVM_JITMEMORYMANAGERS_HPP_INCLUDED

#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/ExecutionEngine/JITEventListener.h"
#include "SymbolMangler.hpp"
#include <map>

// Internal classes
namespace KIARA
{
namespace Compiler
{
	class CompilerUnit;

	// maps to external symbols
	class ExternalSymbolManager : public llvm::SectionMemoryManager
	{
		SymbolMangler mangler;
		ExternalSymbolManager(const ExternalSymbolManager&) LLVM_DELETED_FUNCTION;
		void operator=(const ExternalSymbolManager&) LLVM_DELETED_FUNCTION;

		typedef std::map<std::string, void*> SymbolMap;
		SymbolMap externFunctions;
		SymbolMap externGlobals;
	public:
		ExternalSymbolManager();

		virtual ~ExternalSymbolManager();

		void registerFunctionSymbol(const std::string &name, void* ptr);

		void registerGlobalSymbol(const std::string &name, void* ptr);

		void* getPointerToNamedGlobal(const std::string & Name, bool AbortOnFailure = false);

		// MemoryManager-interface
		virtual void * getPointerToNamedFunction(const std::string &Name, bool AbortOnFailure = true);

		void dump() const;
	};

	// queries the current stage for the symbol
	class StagingMemoryManager : public llvm::SectionMemoryManager
	{
		SymbolMangler mangler;
		StagingMemoryManager(const StagingMemoryManager&) LLVM_DELETED_FUNCTION;
		void operator=(const StagingMemoryManager&) LLVM_DELETED_FUNCTION;

		CompilerUnit & unit;

	public:
		StagingMemoryManager(CompilerUnit & compilerUnit);

		virtual ~StagingMemoryManager();

		virtual void *getPointerToNamedFunction(const std::string &Name, bool AbortOnFailure = true);
	};

	class MyJITEventListener: public llvm::JITEventListener
	{
	public:
	    MyJITEventListener();

	    virtual void NotifyObjectEmitted(const llvm::ObjectImage &Obj);

	    virtual void NotifyFreeingObject(const llvm::ObjectImage &Obj);

	    static MyJITEventListener *create();
	};

}
}




#endif /* JITMEMORYMANAGERS_HPP_ */
