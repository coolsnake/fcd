//
// capstone_wrapper.h
// Copyright (C) 2015 Félix Cloutier.
// All Rights Reserved.
//
// This file is part of fcd.
// 
// fcd is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// fcd is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with fcd.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef fcd__capstone_wrapper_h
#define fcd__capstone_wrapper_h

#include "llvm_warnings.h"

SILENCE_LLVM_WARNINGS_BEGIN()
#include <llvm/Support/ErrorOr.h>
SILENCE_LLVM_WARNINGS_END()

#include <capstone/capstone.h>
#include <memory>

class capstone_error_category : public std::error_category
{
public:
	virtual const char* name() const noexcept override;
	virtual std::string message(int ev) const override;
};

class capstone_iter
{
	csh borrowed_handle;
	cs_insn* memory;
	const uint8_t* code;
	size_t remaining;
	uint64_t nextAddress;
	
	bool is_end();
	
	capstone_iter();
	
public:
	enum operation_result
	{
		success,
		no_more_data,
		invalid_data,
	};
	
	capstone_iter(csh handle, const uint8_t* code, size_t remaining, uint64_t next_address);
	capstone_iter(const capstone_iter& that);
	capstone_iter(capstone_iter&& that);
	~capstone_iter();
	
	static const capstone_iter end;
	
	inline uint64_t next_address() { return nextAddress; }
	
	inline cs_insn& operator*() { return *memory; }
	inline cs_insn* operator->() { return memory; }
	
	operation_result next();
};

struct cs_free_deleter
{
	void operator()(cs_insn* that) const
	{
		cs_free(that, 1);
	}
};

class capstone
{
	csh handle;
	
	capstone(csh handle);
	
public:
	typedef std::unique_ptr<cs_insn, cs_free_deleter> inst_ptr;
	static llvm::ErrorOr<capstone> create(cs_arch arch, unsigned mode);
	
	capstone(capstone&& that);
	~capstone();
	
	inst_ptr alloc();
	
	bool disassemble(cs_insn* into, const uint8_t* begin, const uint8_t* end, uint64_t virtual_address);
	capstone_iter begin(const uint8_t* begin, const uint8_t* end, uint64_t virtual_address = 0);
};

#endif /* defined(fcd__capstone_wrapper_h) */
