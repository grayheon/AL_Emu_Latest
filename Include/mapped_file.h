#pragma once

#if defined( _WIN32 ) || defined( _WIN64 )

#include <windows.h>
#include <tchar.h>
#include <string>

namespace umtl
{
	//////////////////////////////////////////////////////////////////////////
	//

	class mapped_file
	{
	public:
		typedef unsigned int size_type;

		size_type		size	()	{ return size_; }
		TCHAR const *	name	()	{ return filename_.empty() ? 0 : filename_.c_str(); }
		bool			isReadOnly() { return isReadOnly_; }

		void			open	( TCHAR const * filename, bool readOnly = false, size_t initSize = 0 );
		void			close	();

		unsigned char * mapview	( size_t startOffset, size_t size );

		// filename 이 null이면 임시 파일을 생성하여 가상메모리풀 처럼 사용됨
		mapped_file( TCHAR const * filename=0, bool readOnly = false, size_t initSize = 0 );
		~mapped_file();

	private:
		HANDLE	file_;
		HANDLE	map_;
		size_type size_;
		bool isReadOnly_;
		unsigned int offsetPageSize_;

		typedef std::basic_string<TCHAR, std::char_traits<TCHAR>, std::allocator<TCHAR> > string;
		string filename_;

		// 복사생성 차단
		mapped_file( mapped_file const & other );
		void operator=( mapped_file const & other );
	};

	//////////////////////////////////////////////////////////////////////////
}

#endif