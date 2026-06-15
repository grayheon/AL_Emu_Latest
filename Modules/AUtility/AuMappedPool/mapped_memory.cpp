#include "mapped_memory.h"

#if defined( _WIN32 ) || defined( _WIN64 )

namespace umtl
{
	//////////////////////////////////////////////////////////////////////////
	//

	mapped_memory::mapped_memory( mapped_file & mFile, size_t startOffset, size_t access_size )
		: offset_(startOffset)
		, size_(access_size)
		, data_(0)
	{
		data_ = mFile.mapview( startOffset, size_ );
	}

	//////////////////////////////////////////////////////////////////////////
	//

	mapped_memory::mapped_memory( mapped_memory & other ) : offset_(other.offset_)
		, size_(other.size_)
		, data_ (other.data_)
	{
		other.data_ = 0;
		other.close();
	}

	//////////////////////////////////////////////////////////////////////////
	//

	mapped_memory::~mapped_memory()
	{
		close();
	}
	
	//////////////////////////////////////////////////////////////////////////
	//

	void mapped_memory::close()
	{
		if( data_ )
			UnmapViewOfFile( data_ );

		data_ = 0;
		size_ = 0;
		offset_ = 0;
	}

	//////////////////////////////////////////////////////////////////////////
	//

	mapped_memory mapped_memory::access( mapped_file & mFile, size_t offset, size_t accessSize )
	{
		return mapped_memory( mFile, offset, accessSize );
	}

	//////////////////////////////////////////////////////////////////////////
}

#endif