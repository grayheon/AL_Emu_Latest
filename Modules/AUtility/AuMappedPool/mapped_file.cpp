#include "mapped_file.h"

#if defined( _WIN32 ) || defined( _WIN64 )

namespace umtl
{
	//////////////////////////////////////////////////////////////////////////
	//

	mapped_file::mapped_file( TCHAR const * filename, bool readOnly, size_t initSize )
		: file_	(0)
		, map_	(0)
		, size_	(0)
		, filename_( filename ? filename : _T("") )
		, isReadOnly_( readOnly )
		, offsetPageSize_(0)
	{
		SYSTEM_INFO sysInfo;

		GetSystemInfo( &sysInfo );

		offsetPageSize_ = sysInfo.dwAllocationGranularity;

		open( filename, readOnly, initSize );
	}

	//////////////////////////////////////////////////////////////////////////
	//

	void mapped_file::open( TCHAR const * filename, bool readOnly /* = false */, size_t initSize /* = 0 */ )
	{
		close();

		filename_ = filename ? filename : _T("");
		isReadOnly_ = readOnly;

		if( !filename_.empty() )
		{
			file_ = CreateFile( filename
				, readOnly ? (GENERIC_READ) : (GENERIC_WRITE | GENERIC_READ)
				, readOnly ? (FILE_SHARE_READ) : (FILE_SHARE_READ | FILE_SHARE_WRITE)
				, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0 );
		}
		else
		{
			file_ = INVALID_HANDLE_VALUE;
		}

		if( filename_.empty() || file_ != INVALID_HANDLE_VALUE )
		{
			if( file_ != INVALID_HANDLE_VALUE )
			{
				size_ = (size_type)GetFileSize( file_, 0 );

				if( size_ < (size_type)initSize )
					size_ = (size_type)initSize;

				if( size_ == 0 ) // 빈파일인 경우 임의의 크기로 생성
					size_ = 1024;
			}
			else // 파일 이름이 없는경우 무조건 생성
			{
				size_ = (size_type)initSize;

				if( size_ == 0 )
					size_ = 1024;
			}

			map_ = CreateFileMapping( file_, 0, (isReadOnly_ ? PAGE_READONLY : PAGE_READWRITE), 0, size_, /*(filename_.empty() ? 0 : filename_.c_str()*/0 );

			if( map_ == 0 )
			{
				CloseHandle( file_ );
				file_ = 0;
				size_ = 0;
			}		
		}		
	}

	//////////////////////////////////////////////////////////////////////////
	//

	unsigned char * mapped_file::mapview( size_t startOffset, size_t size )
	{
		unsigned char * data = 0;

		if( map_ )
		{
			size_t realOffset = startOffset / offsetPageSize_ * offsetPageSize_;
			size_t delem = startOffset % offsetPageSize_;

			data = (unsigned char *)MapViewOfFile( map_
				, isReadOnly_ ? (FILE_MAP_READ) : (FILE_MAP_ALL_ACCESS)
				, 0, (DWORD)realOffset, size+delem );

			if( (!data) && size_ < (size+realOffset+delem) && !filename_.empty() )
			{
				// resize
				open( filename_.c_str(), isReadOnly_, (size+realOffset+delem) );

				if( map_ )
				{
					data = (unsigned char *)MapViewOfFile( map_
						, isReadOnly_ ? (FILE_MAP_READ) : (FILE_MAP_ALL_ACCESS)
						, 0, (DWORD)realOffset, size+delem );
				}
			}

			if( data )
				data += delem;
		}

		return data;
	}

	//////////////////////////////////////////////////////////////////////////
	//

	void mapped_file::close()
	{
		if( map_ )
			CloseHandle(map_);

		if( file_ && file_ != INVALID_HANDLE_VALUE )
			CloseHandle(file_);

		map_ = 0;
		file_ = 0;
	}

	//////////////////////////////////////////////////////////////////////////
	//

	mapped_file::~mapped_file() 
	{
		close();
	}

	//////////////////////////////////////////////////////////////////////////
}

#endif