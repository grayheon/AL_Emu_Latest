#pragma once

#if defined( _WIN32 ) || defined( _WIN64 )

#include "mapped_file.h"

namespace umtl
{
	//////////////////////////////////////////////////////////////////////////
	//

	class mapped_memory
	{
	public:
		mapped_memory( mapped_memory & other );
		~mapped_memory();

		unsigned char *			get()		{ return data_; }
		size_t					size()		{ return size_; }
		size_t					offset()	{ return offset_; }

		static mapped_memory	access( mapped_file & mFile, size_t offset, size_t accessSize );

	private:
		mapped_memory( mapped_file & mFile, size_t startOffset, size_t access_size);

		size_t			offset_;
		size_t			size_;
		unsigned char *	data_;

		void close();

		void operator=( mapped_memory const & other );
	};

	//////////////////////////////////////////////////////////////////////////
	//

	class mapped_cache
	{
	public:
		mapped_cache( size_t needSize ) : size_(needSize), mappedFile_( 0, false, needSize )
			, mappedMemory_( mapped_memory::access( mappedFile_, 0, needSize ) )
		{}

		unsigned char * get()
		{
			return mappedMemory_.get();
		}

		size_t size() {
			return mappedMemory_.size();
		}

		~mapped_cache() {}

	private:
		mapped_file mappedFile_;
		mapped_memory mappedMemory_;
		size_t size_;
	};

	//////////////////////////////////////////////////////////////////////////
}

#endif