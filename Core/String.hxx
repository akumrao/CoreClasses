#ifndef _String_hxx
#define _String_hxx 1

#include "cif_stdc.h"

   /////////////////////////////////////////////////////////////////////////
   // Copyright(c) 2005 Broadware Technologies, Inc. All rights reserved. //
   //                                                                     //
   //        THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF               //
   //                   Broadware Technologies, Inc.                      //
   //       The copyright notice above does not evidence any              //
   //       actual or intended publication of such source code.           //
   /////////////////////////////////////////////////////////////////////////

                            /*### @+prefix ************************************/
#include "Thread.hxx"
#include "Time.hxx"
                            /*### @-prefix ************************************/

// Classes defined here
class CharClass;
class String;
class Compare;



#include <iomanip>
#include <map>
#include <algorithm>

//#include "Object.hxx"
                            /*### @class CharClass {{{2 ***********************/

/*!
** Provides ctype-style functionality for use with String.
*/
class CharClass
{
	public:
		enum Attr {
			_alpha= 0x0001,
			_ascii = 0x0002,
			_cntrl = 0x0004,
			_digit = 0x0008,
			_eol = 0x0010,
			_ident = 0x0020,
			_lower = 0x0040,
			_odigit = 0x0080,
			_print = 0x0100,
			_punct = 0x0200,
			_space = 0x0400,
			_upper = 0x0800,
			_xdigit = 0x1000
		};

		/*! Constants for each attribute. */
		static const CharClass ALPHA;
		static const CharClass ASCII;
		static const CharClass CNTRL;
		static const CharClass DIGIT;
		static const CharClass EOL;
		static const CharClass IDENT;
		static const CharClass LOWER;
		static const CharClass ODIGIT;
		static const CharClass PRINT;
		static const CharClass PUNCT;
		static const CharClass SPACE;
		static const CharClass UPPER;
		static const CharClass XDIGIT;

		/*! Copy ctor (this is standard ctor) */
		CharClass(const CharClass& rhs)
		: mAttrs(rhs.mAttrs) {}

		/*! Basic dtor */
		~CharClass() {}

		/*! Clear all our attrs. */
		CharClass& clear() {
			mAttrs = 0;
			return *this;
		}

		/*! Does c have any of the attrs of this? */
		bool match(uint8_t c) const {
			return ( (sCharMap[static_cast<uint32_t>(c)] & mAttrs) != 0 );
		}

		/*! Assignment */
		CharClass& operator=(const CharClass& rhs) {
			mAttrs = rhs.mAttrs;
			return *this;
		}

		/*! Create by combining two */
		CharClass operator|(const CharClass& rhs) const {
			return CharClass(mAttrs | rhs.mAttrs);
		}

		/*! Add another class into this one. */
		CharClass& operator|=(const CharClass& rhs) {
			mAttrs |= rhs.mAttrs;
			return *this;
		}

	private:

		/*! Only for internal use */
		CharClass(uint32_t attrs = 0)
		: mAttrs(attrs) {}

		/*! Attributes of this class. */
		uint32_t mAttrs;

		/*! Static character attributes. */
		static const uint32_t sCharMap[256];
};
                            /*### @class CharClass }}}2 ***********************/
                            /*### @class String {{{2 **************************/

/*!
** todo
*/
class String
{
	public:

		// Classes defined here
		struct Hash;
		struct Rep;

		/*! this is a uint32_t */
		typedef size_t size_type;

		/*! Illegal position, also used as synonym for end */
		static const size_type npos;
                            /*### @struct String::Hash {{{3 *******************/

		/*!
		** For generating hash keys.
		*/
		struct Hash {
			Hash() {}

			uint32_t operator()(const String& key) const {
				const uint32_t len = key.length();
				register const uint8_t* p = reinterpret_cast<const uint8_t*>(key.data());
				register uint32_t i, u = 0;
				
				for ( i = 0; i < len; ++i )
					u += (u << 5) + *p++;
				return u;
			}
		};
                            /*### @struct String::Hash }}}3 *******************/
                            /*### @struct String::Rep {{{3 ********************/

		/*!
		** Holder of actual data with mutex-protected reference counting.  The
		** data follows the fixed fields of the rep, which allows us to avoid
		** double allocations (one for rep, another for data).  If mCapacity >
		** 0, then then data == (rep + 1).  If mCapacity == 0, then data ==
		** *(rep + 1) (e.g., the block contains an address, not the actual
		** data itself).
		*/
		struct Rep {

			/*! Amount of memory allocated for data. */
			size_type mCapacity;

			/*! Number of chars in data. */
			size_type mLength;

			/*! Number of references to us. */
			AtomicCounter mReferences;

			/*! Required for AIX, which otherwise zeros out our members (grrr). */
			Rep(): mLength(0) {}

			/*! Return pointer to data */
			char* data() {
				return reinterpret_cast<char*>(this + 1);
			}

			/*! Takes a parameter the extra data size */
			void* operator new(
				size_t rep_size,
				size_t capacity);

			/*! Reclaim memory */
			void operator delete(void* ptr);

			/*! Release this rep, true if needs a delete. */
			bool release();
		
			/* get the size on a decent boundry */
			static size_t calculateSize(size_t size);

			/*! Resize big enough to handle the given capacity */
			Rep* resize(size_type n, bool up = true);
		};
                            /*### @struct String::Rep }}}3 ********************/

		/*! STL: empty string. */
		String(): mData(NULL), mLength(0), mOffset(npos), mRep(NULL) {}

		/*! STL: generalization of copy ctor (adds ref) */
		String(
			const String& s,
			size_type pos = 0,
			size_type n = npos);

		/*! STL: copy first n chars from s */
		String(
			const char* s,
			size_type n = npos);

		/*! STL: make n copies of c */
		String(
			size_type n,
			char c);

		/*! Non-STL: const assignment (no copy/modify). */
		String(
			bool dummy, /*! unique sig */
			const char* s,
			size_type n = npos);

		/*! STL */
		~String();

		/*! Non-STL: convert this[pos,+] to float. */
		double as_float(size_type pos = 0) const {
			return strtof(c_str() + pos, NULL);
		}

		/*! Non-STL: convert this[pos,+] to double. */
		double as_double(size_type pos = 0) const {
			return strtod(c_str() + pos, NULL);
		}

		/*!Returns true -for success(stored converted digits 
		 * in output param), false -for error(no conversion performed or 
		 * no digits in the string */
		bool as_double(String& str, double& digits);
        
		/*! Non-STL: convert this[pos,+] to int32, in given base */
		int32_t as_int32(size_type pos = 0, int base = 10) const {
			return strtol(c_str() + pos, NULL, base);
		}

		/*!Returns true -for success(stored converted digits 
		 * in output param, false -for error(no conversion performed or 
		 * no digits in the string */
		bool as_int32(String& str, int32_t& digits);

		/*! Non-STL: convert this[pos,+] to uint32, in given base. */
		uint32_t as_uint32(size_type pos = 0, int base = 10) const {
			return strtoul(c_str() + pos, NULL, base);
		}

		/*! Non-STL: convert this[pos,+] to int64. */
		int64_t as_int64(size_type pos = 0, int base = 10) const {
			return strtoll(c_str() + pos, NULL, base);
		}

		/*! Non-STL: convert this[pos,+] to uint64. */
		uint64_t as_uint64(size_type pos = 0, int base = 10) const {
			return strtoull(c_str() + pos, NULL, base);
		}

		/*! Non-STL: create iso8601 date string. */
		static String from_date(
			const Date& d,
			bool humanFormat = false,
			bool includeDate = true,
			bool includeTime = true,
			bool includeTZ = false,
			int fracDigits = 0);

		/*! Non-STL: create from double. */
		static String from_double(double val, const char* fmt = "%f") {
			return String().assignf(fmt, val);
		}

		/*! Non-STL: create from elapsed time. */
		static String from_elapsed(
			const Time::Elapsed& e,
			bool iso8601 = false,
			int fracDigits = 2);

		/*! Non-STL: create from int32. */
		static String from_int32(int32_t val, const char* fmt = "%d") {
			return String().assignf(fmt, val);
		}

		/*! Non-STL: create from uint32. */
		static String from_uint32(uint32_t val, const char* fmt = "%u") {
			return String().assignf(fmt, val);
		}

		/*! Non-STL: create from int64. */
		static String from_int64(int64_t val, const char* fmt = "%lld") {
			return String().assignf(fmt, val);
		}

		/*! Non-STL: create from uint64. */
		static String from_uint64(uint64_t val, const char* fmt = "%llu") {
			return String().assignf(fmt, val);
		}

		/*! Non-STL: set length = pos + n (does not change capacity) */
		void adjust_length(
			size_type n,
			size_type pos = npos);

		/*! STL: append s[pos,+n] to this */
		String& append(
			const String& s,
			size_type pos = 0,
			size_type n = npos);

		/*! STL: append s[0,+n] to this */
		String& append(
			const char* s,
			size_type n = npos);

		/*! STL: append n copies of c to this */
		String& append(
			size_type n,
			char c);

		/*! Non-STL: append text formatted in sprintf style */
		String& appendf(
			const char* format,
			...) __A_FORMAT__(printf,2,3);

		/*! Non-STL: append text formatted in vsprintf style */
		String& vappendf(
			const char* format,
			va_list ap);

		/*! STL: assign s to this */
		String& assign(const String& s);

		/*! STL: assign s[pos,+n] to this */
		String& assign(
			const String& s,
			size_type pos,
			size_type n);

		/*! STL: assign s[0,+n] to this */
		String& assign(
			const char* s,
			size_type n = npos);

		/*! STL: assign n copies of c to this */
		String& assign(
			size_type n,
			char c);

		/*! Non-STL: const assignment (no copy/modify) */
		String& assign(
			bool dummy, /*! unique sig */
			const char* s,
			size_type n = npos);

		/*! Non-STL: assign text formatted in sprintf style */
		String& assignf(
			const char* format,
			...) __A_FORMAT__(printf,2,3);

		/*! Non-STL: assign text formatted in vsprintf style */
		String& vassignf(
			const char* format,
			va_list ap);

		/*! STL: clear contents */
		void clear();

		/*! STL: erase this[pos,+n] */
		String& erase(
			size_type pos = 0,
			size_type n = npos);

		/*! Non-STL: return raw pointer to buffer at this[pos,+n]. */
		char* get_raw(
			size_type n,
			size_type pos = npos);

		/*! STL: insert s[pos1,+n] before this[pos] */
		String& insert(
			size_type pos,
			const String& s,
			size_type pos1 = 0,
			size_type n = npos);

		/*! STL: insert s[0,+n] before this[pos] */
		String& insert(
			size_type pos,
			const char* s,
			size_type n = npos);

		/*! STL: insert n copies of c before this[pos] */
		String& insert(
			size_type pos,
			size_type n,
			char c);

		/*! STL */
		String& operator+=(char c) {
			return append(1, c);
		}

		/*! STL */
		String& operator+=(const char* s) {
			return append(s);
		}

		/*! STL */
		String& operator+=(const String& s) {
			return append(s);
		}

		/*! STL */
		String& operator=(const char* s) {
			return assign(s);
		}

		/*! STL */
		String& operator=(const String& s) {
			return assign(s);
		}

		/*! STL */
		String& operator=(char c) {
			return assign(1, c);
		}

		/*! STL */
		char& operator[](size_type n);

		/*! STL */
		char operator[](size_type n) const {
			return ( (n < length()) ? mData[n] : '\0' );
		}

		/*! STL: replace this[pos,+n] with s[pos1,+n1] */
		String& replace(
			size_type pos,
			size_type n,
			const String& s,
			size_type pos1 = 0,
			size_type n1 = npos);

		/*! STL: replace this[pos,+n] with s[0,+n1] */
		String& replace(
			size_type pos,
			size_type n,
			const char* s,
			size_type n1 = npos);

		/*! STL: replace this[pos,+n] with n1 copies of c */
		String& replace(
			size_type pos,
			size_type n,
			size_type n1,
			char c);

		bool reserve( size_type n);

		/*! STL: swap reps with str */
		void swap(String& s);

		/*! Non-STL: convert this[pos,+n] to lower-case. */
		String& to_lower(
			size_type pos = 0,
			size_type n = npos);

		/*! Non-STL: convert this[pos,+n] to upper-case. */
		String& to_upper(
			size_type pos = 0,
			size_type n = npos);

		/*! Non-STL: trim whitespace at both beginning and end. */
		String& trim() {
			trim_left();
			trim_right();
			return *this;
		}

		/*! Non-STL: trim whitespace at beginning. */
		String& trim_left();

		/*! Non-STL: trim whitespace at end. */
		String& trim_right();

		/*! STL (enhanced): compare this to s */
		int compare(const String& s, bool ignoreCase = false) const {
			if ( mData == s.mData && length() == s.length() )
			return 0;
			else
			return compare(0, length(), s.c_str(), s.length(), ignoreCase);
		}

		/*! STL (enhanced): compare this[pos,+n] to s */
		int compare(size_type pos, size_type n, const String& s, bool ignoreCase = false) const {
			return compare(pos, n, s.c_str(), s.length(), ignoreCase);
		}

		/*! STL (enhanced): compare this[pos,+n] to s[pos1,+n1] */
		int compare(
			size_type pos,
			size_type n,
			const String& s,
			size_type pos1,
			size_type n1,
			bool ignoreCase = false) const;

		/*! STL (enhanced): compare this to s */
		int compare(const char* s, bool ignoreCase = false) const {
			return compare(0, length(), s, strlen(s), ignoreCase);
		}

		/*! STL (enhanced): compare this[pos,+n] to s[0,+len] */
		int compare(
			size_type pos,
			size_type n,
			const char* s,
			size_type len = npos,
			bool ignoreCase = false) const;

		/*! STL: return first position within this[pos,$] of string s */
		size_type find(const String& s, size_type pos = 0) const {
			return find(s.c_str(), pos, s.length());
		}

		/*! STL: return first position within this[pos,$] of string s[0-n] */
		size_type find(
			const char* s,
			size_type pos = 0,
			size_type n = npos) const;

		/*! STL: return first position within this[pos,$] of char c */
		size_type find(
			char c,
			size_type pos = 0) const;

		/*! STL: return first position within this[pos,$] of any char in s[0-n] */
		size_type find_first_of(
			const char* s,
			size_type pos = 0,
			size_type n = npos) const;

		/*! STL: return first position within this[pos,$] of char c */
		size_type find_first_of(char c, size_type pos = 0) const {
			return find(c, pos);
		}

		/*! Non-STL: return first position within this[pos,$] of any char of class m */
		size_type find_first_of(
			const CharClass& m,
			size_type pos = 0) const;

		/*! STL: return first position within this[pos,$] of any char NOT in s[0-n] */
		size_type find_first_not_of(
			const char* s,
			size_type pos = 0,
			size_type n = npos) const;

		/*! STL: return first position within this[pos,$] of any char EXCEPT c */
		size_type find_first_not_of(
			char c,
			size_type pos = 0) const;

		/*! Non-STL: return first position within this[pos,$] of any char NOT of class m */
		size_type find_first_not_of(
			const CharClass& m,
			size_type pos = 0) const;

		/*! Non-STL: returns true if string starts with char string s */
		bool starts_with(
			const char* s) const;

		/*! Non-STL: returns true if string starts with string s */
		bool starts_with(
			const String& s) const;

		/*! Non-STL: returns true if string ends with char string s */
		bool ends_with(
			const char* s) const;

		/*! Non-STL: returns true if string ends with string s */
		bool ends_with(
			const String& s) const;

		/*! STL: return last position within this[0,pos] of string s */
		size_type rfind(const String& s, size_type pos = npos) const {
			return rfind(s.c_str(), pos, s.length());
		}

		/*! STL: return last position within this[0,pos] of string s[0-n] */
		size_type rfind(
			const char* s,
			size_type pos = npos,
			size_type n = npos) const;

		/*! STL: return last position within this[0,pos] of char c */
		size_type rfind(
			char c,
			size_type pos = npos) const;

		/*! STL: return last position within this[0,pos] of any char in s[0-n] */
		size_type find_last_of(
			const char* s,
			size_type pos = npos,
			size_type n = npos) const;

		/*! STL: return last position within this[0,pos] of char c */
		size_type find_last_of(char c, size_type pos = npos) const {
			return rfind(c, pos);
		}

		/*! Non-STL: return last position within this[0,pos] of any char of class m */
		size_type find_last_of(
			const CharClass& m,
			size_type pos = npos) const;

		/*! STL: return last position within this[0,pos] of any char NOT in s[0-n] */
		size_type find_last_not_of(
			const char* s,
			size_type pos = npos,
			size_type n = npos) const;

		/*! STL: return last position within this[0,pos] of any char EXCEPT c */
		size_type find_last_not_of(
			char c,
			size_type pos = npos) const;

		/*! Non-STL: return last position within this[0,pos] of any char NOT of class m */
		size_type find_last_not_of(
			const CharClass& m,
			size_type pos = npos) const;

		/*! Non-STL: Is this this[pos,+n] of given class (min = min matching chars). */
		bool is_class(
			const CharClass& m,
			size_type pos = 0,
			size_type n = npos) const;

		/*! STL: nul-terminated representation of data (never NULL). */
		const char* c_str() const {
			if ( !cstrSafe() )
			modify(0, true);
			return data();
		}

		/*! STL: how many chars can we hold? */
		size_type capacity() const {
			return ( is_extern() ? 0 : (mRep->mCapacity - 1) );
		}

		/*! STL: copy this[pos,+n] to buf */
		size_type copy(
			char* buf,
			size_type n,
			size_type pos = 0) const;

		/*! STL: return pointer to underlying data */
		const char* data() const {
			return mData ? mData : "";
		}

		/*! STL: does this string have 0 length? */
		bool empty() const {
			return ( length() == 0 );
		}

		/*! STL: number of chars in string */
		size_type length() const {
			return ( is_normal() ? mRep->mLength : mLength );
		}

		/*! STL: synonym for length() */
		size_type size() const {
			return length();
		}

		/*! STL: return this[pos,+n] */
		String substr(
			size_type pos = 0,
			size_type n = npos) const;

		/*! Non-STL (debug): Is this an external string? */
		bool is_extern() const {
			return !mRep;
		}

		/*! Non-STL (debug): Is this a 'normal' string? */
		bool is_normal() const {
			return (mRep && mOffset == npos);
		}

		/*! Non-STL (debug): how many references to our rep? */
		int ref_count() const {
			return ( is_extern() ? 0 : mRep->mReferences );
		}

	private:

		/*! Check standard args, -1 if pos fail. */
		static int chkPos(
			size_type pos,
			size_type len);

		/*! Check (and maybe fix) standard args, -1 if pos fail, +1 if n fail. */
		static int chkPosFixNum(
			size_type pos,
			size_type len,
			size_type& n);

		/*! Check (and fix) n, 1 if n == 0. */
		static int fixNum(
			size_type& n,
			size_type len);

		/*! Can a cstr safely be done on this string? */
		bool cstrSafe() const;

		/*! Make sure we have a rep to modify (private) */
		bool modify(
			size_type needed,
			bool keepData) const;

		/*! Set to external ptr. */
		void setExtern(
			const char* s,
			size_type n) const;

		/*! Create a rep and copy in the given data. */
		bool setNormal(
			size_type cap,
			size_type len,
			const char* s = NULL,
			char c = '\0') const;

		/*! Set reference to given string (possibly as slice). */
		void setRef(
			const String& s,
			size_type pos,
			size_type n) const;

		/*! Assign from this string (wrapper over others). */
		void setStr(
			const String& s,
			size_type pos,
			size_type n) const;

		/*! The contents of the string (points into rep or to external buf) */
		mutable char* mData;

		/*! Not used for eNormal */
		mutable size_type mLength;

		/*! Indicates style of string and offset into buffer. */
		mutable size_type mOffset;

		/*! Holder of underlying data; not used for eExternal */
		mutable Rep* mRep;
};
                            /*### @class String }}}2 **************************/

/*! STL */
inline bool operator!=(const String& s1, const String& s2) {
	return ( s1.compare(s2) != 0 );
}

/*! STL */
inline bool operator!=(
	const String& s1,
	const char* s2) {
	return ( s1.compare(s2) != 0 );
}

/*! STL */
inline bool operator!=(
	const char* s1,
	const String& s2) {
	return ( s2.compare(s1) != 0 );
}

/*! STL */
inline bool operator<(
	const String& s1,
	const String& s2) {
	return ( s1.compare(s2) < 0 );
}

/*! STL */
inline bool operator<(
	const String& s1,
	const char* s2) {
	return ( s1.compare(s2) < 0 );
}

/*! STL */
inline bool operator<(
	const char* s1,
	const String& s2) {
	return ( s2.compare(s1) >= 0 );
}

/*! STL */
inline bool operator<=(
	const String& s1,
	const String& s2) {
	return ( s1.compare(s2) <= 0 );
}

/*! STL */
inline bool operator<=(
	const String& s1,
	const char* s2) {
	return ( s1.compare(s2) <= 0 );
}

/*! STL */
inline bool operator<=(
	const char* s1,
	const String& s2) {
	return ( s2.compare(s1) > 0 );
}

/*! STL */
inline bool operator==(
	const String& s1,
	const String& s2) {
	return ( s1.compare(s2) == 0 );
}

/*! STL */
inline bool operator==(
	const String& s1,
	const char* s2) {
	return ( s1.compare(s2) == 0 );
}

/*! STL */
inline bool operator==(
	const char* s1,
	const String& s2) {
	return ( s2.compare(s1) == 0 );
}

/*! STL */
inline bool operator>(
	const String& s1,
	const String& s2) {
	return ( s1.compare(s2) > 0 );
}

/*! STL */
inline bool operator>(
	const String& s1,
	const char* s2) {
	return ( s1.compare(s2) > 0 );
}

/*! STL */
inline bool operator>(
	const char* s1,
	const String& s2) {
	return ( s2.compare(s1) <= 0 );
}

/*! STL */
inline bool operator>=(
	const String& s1,
	const String& s2) {
	return ( s1.compare(s2) >= 0 );
}

/*! STL */
inline bool operator>=(
	const String& s1,
	const char* s2) {
	return ( s1.compare(s2) >= 0 );
}

/*! STL */
inline bool operator>=(
	const char* s1,
	const String& s2) {
	return ( s2.compare(s1) < 0 );
}

/*! STL */
inline String operator+(
	const String& s1,
	const String& s2) {
	return String(s1.c_str(), s1.length()).append(s2);
}

/*! STL */
inline String operator+(
	const char* s1,
	const String& s2) {
	return String(s1).append(s2);
}

/*! STL */
inline String operator+(
	const String& s1,
	const char* s2) {
	return String(s1.c_str(), s1.length()).append(s2);
}

/*! STL */
inline String operator+(
	char c1,
	const String& s2) {
	return String(1, c1).append(s2);
}

/*! STL */
inline String operator+(
	const String& s1,
	char c2) {
	return String(s1.c_str(), s1.length()).append(1, c2);
}



 /*
 * (C) Copyright Nicolai M. Josuttis 1999.
 * Permission to copy, use, modify, sell and distribute this software
 * is granted provided this copyright notice appears in all copies.
 * This software is provided "as is" without express or implied
 * warranty, and with no claim as to its suitability for any purpose.
 */
class Compare {
  public:
    // constants for the comparison criterion
    enum cmp_mode {case_, nocase_};
  private:
    // actual comparison mode
    cmp_mode m_mode;

    // auxiliary function to compare case insensitive
    static bool nocase_compare (char c1, char c2) {
        return toupper(c1) < toupper(c2);
    }

  public:
    // constructor: initializes the comparison criterion
    Compare (cmp_mode m = nocase_) : m_mode(m) {
    }

    Compare& operator=(const Compare& rsh) {
		m_mode = rsh.m_mode;
		return *this;
	}
		
    // the comparison
    bool operator() (const String& s1, const String& s2) const {
        if (m_mode == case_) 
            return s1 < s2;
        else 
            return std::lexicographical_compare(s1.c_str(), s1.c_str() + s1.length(),
                                            s2.c_str(), s2.c_str() + s2.length(),
                                        	nocase_compare);
    }
};

typedef std::map<String, String, Compare> StringsMap;

#endif // !_String_hxx


/* ==================== editors ====================== */

