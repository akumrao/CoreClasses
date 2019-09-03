#include "String.hxx"



                            /*### @+prefix ************************************/
#include <assert.h>
#include <stdarg.h>
#include <cerrno>

#ifndef SHIFT_
# define SHIFT_(_to, _from, _base, _len, _esize) memmove(_base + _to, _base + _from, (_len - (_from)) * _esize)
#endif

#include "Profiler.hxx"
                            /*### @-prefix ************************************/
const CharClass CharClass::ALPHA(CharClass::_alpha);
const CharClass CharClass::ASCII(CharClass::_ascii);
const CharClass CharClass::CNTRL(CharClass::_cntrl);
const CharClass CharClass::DIGIT(CharClass::_digit);
const CharClass CharClass::EOL(CharClass::_eol);
const CharClass CharClass::IDENT(CharClass::_ident);
const CharClass CharClass::LOWER(CharClass::_lower);
const CharClass CharClass::ODIGIT(CharClass::_odigit);
const CharClass CharClass::PRINT(CharClass::_print);
const CharClass CharClass::PUNCT(CharClass::_punct);
const CharClass CharClass::SPACE(CharClass::_space);
const CharClass CharClass::UPPER(CharClass::_upper);
const CharClass CharClass::XDIGIT(CharClass::_xdigit);
                            /*### @+CharClass::sCharMap ***********************/
const uint32_t CharClass::sCharMap[256] =
{
	/* ^@ */ _ascii | _cntrl,
	/* ^A */ _ascii | _cntrl,
	/* ^B */ _ascii | _cntrl,
	/* ^C */ _ascii | _cntrl,
	/* ^D */ _ascii | _cntrl,
	/* ^E */ _ascii | _cntrl,
	/* ^F */ _ascii | _cntrl,
	/* ^G */ _ascii | _cntrl,
	/* ^H */ _ascii | _cntrl,
	/* ^I */ _ascii | _cntrl | _space,
	/* ^J */ _ascii | _cntrl | _space | _eol,
	/* ^K */ _ascii | _cntrl | _space,
	/* ^L */ _ascii | _cntrl | _space,
	/* ^M */ _ascii | _cntrl | _space | _eol,
	/* ^N */ _ascii | _cntrl,
	/* ^O */ _ascii | _cntrl,
	/* ^P */ _ascii | _cntrl,
	/* ^Q */ _ascii | _cntrl,
	/* ^R */ _ascii | _cntrl,
	/* ^S */ _ascii | _cntrl,
	/* ^T */ _ascii | _cntrl,
	/* ^U */ _ascii | _cntrl,
	/* ^V */ _ascii | _cntrl,
	/* ^W */ _ascii | _cntrl,
	/* ^X */ _ascii | _cntrl,
	/* ^Y */ _ascii | _cntrl,
	/* ^Z */ _ascii | _cntrl,
	/* ^[ */ _ascii | _cntrl,
	/* ^\ */ _ascii | _cntrl,
	/* ^] */ _ascii | _cntrl,
	/* ^^ */ _ascii | _cntrl,
	/* ^_ */ _ascii | _cntrl,
	/*    */ _ascii | _print | _space,
	/*  ! */ _ascii | _print | _punct,
	/*  " */ _ascii | _print | _punct,
	/*  # */ _ascii | _print | _punct,
	/*  $ */ _ascii | _print | _punct,
	/*  % */ _ascii | _print | _punct,
	/*  & */ _ascii | _print | _punct,
	/*  ' */ _ascii | _print | _punct,
	/*  ( */ _ascii | _print | _punct,
	/*  ) */ _ascii | _print | _punct,
	/*  * */ _ascii | _print | _punct,
	/*  + */ _ascii | _print | _punct,
	/*  , */ _ascii | _print | _punct,
	/*  - */ _ascii | _print | _punct,
	/*  . */ _ascii | _print | _punct,
	/*  / */ _ascii | _print | _punct,
	/*  0 */ _ascii | _digit | _print | _xdigit | _ident,
	/*  1 */ _ascii | _digit | _print | _xdigit | _ident,
	/*  2 */ _ascii | _digit | _print | _xdigit | _ident,
	/*  3 */ _ascii | _digit | _print | _xdigit | _ident,
	/*  4 */ _ascii | _digit | _print | _xdigit | _ident,
	/*  5 */ _ascii | _digit | _print | _xdigit | _ident,
	/*  6 */ _ascii | _digit | _print | _xdigit | _ident,
	/*  7 */ _ascii | _digit | _print | _xdigit | _ident,
	/*  8 */ _ascii | _digit | _print | _xdigit | _ident,
	/*  9 */ _ascii | _digit | _print | _xdigit | _ident,
	/*  : */ _ascii | _print | _punct,
	/*  ; */ _ascii | _print | _punct,
	/*  < */ _ascii | _print | _punct,
	/*  = */ _ascii | _print | _punct,
	/*  > */ _ascii | _print | _punct,
	/*  ? */ _ascii | _print | _punct,
	/*  @ */ _ascii | _print | _punct,
	/*  A */ _ascii | _alpha | _print | _upper | _xdigit | _ident,
	/*  B */ _ascii | _alpha | _print | _upper | _xdigit | _ident,
	/*  C */ _ascii | _alpha | _print | _upper | _xdigit | _ident,
	/*  D */ _ascii | _alpha | _print | _upper | _xdigit | _ident,
	/*  E */ _ascii | _alpha | _print | _upper | _xdigit | _ident,
	/*  F */ _ascii | _alpha | _print | _upper | _xdigit | _ident,
	/*  G */ _ascii | _alpha | _print | _upper | _ident,
	/*  H */ _ascii | _alpha | _print | _upper | _ident,
	/*  I */ _ascii | _alpha | _print | _upper | _ident,
	/*  J */ _ascii | _alpha | _print | _upper | _ident,
	/*  K */ _ascii | _alpha | _print | _upper | _ident,
	/*  L */ _ascii | _alpha | _print | _upper | _ident,
	/*  M */ _ascii | _alpha | _print | _upper | _ident,
	/*  N */ _ascii | _alpha | _print | _upper | _ident,
	/*  O */ _ascii | _alpha | _print | _upper | _ident,
	/*  P */ _ascii | _alpha | _print | _upper | _ident,
	/*  Q */ _ascii | _alpha | _print | _upper | _ident,
	/*  R */ _ascii | _alpha | _print | _upper | _ident,
	/*  S */ _ascii | _alpha | _print | _upper | _ident,
	/*  T */ _ascii | _alpha | _print | _upper | _ident,
	/*  U */ _ascii | _alpha | _print | _upper | _ident,
	/*  V */ _ascii | _alpha | _print | _upper | _ident,
	/*  W */ _ascii | _alpha | _print | _upper | _ident,
	/*  X */ _ascii | _alpha | _print | _upper | _ident,
	/*  Y */ _ascii | _alpha | _print | _upper | _ident,
	/*  Z */ _ascii | _alpha | _print | _upper | _ident,
	/*  [ */ _ascii | _print | _punct,
	/*  \ */ _ascii | _print | _punct,
	/*  ] */ _ascii | _print | _punct,
	/*  ^ */ _ascii | _print | _punct,
	/*  _ */ _ascii | _print | _punct | _ident,
	/*  ` */ _ascii | _print | _punct,
	/*  a */ _ascii | _alpha | _lower | _print | _xdigit | _ident,
	/*  b */ _ascii | _alpha | _lower | _print | _xdigit | _ident,
	/*  c */ _ascii | _alpha | _lower | _print | _xdigit | _ident,
	/*  d */ _ascii | _alpha | _lower | _print | _xdigit | _ident,
	/*  e */ _ascii | _alpha | _lower | _print | _xdigit | _ident,
	/*  f */ _ascii | _alpha | _lower | _print | _xdigit | _ident,
	/*  g */ _ascii | _alpha | _lower | _print | _ident,
	/*  h */ _ascii | _alpha | _lower | _print | _ident,
	/*  i */ _ascii | _alpha | _lower | _print | _ident,
	/*  j */ _ascii | _alpha | _lower | _print | _ident,
	/*  k */ _ascii | _alpha | _lower | _print | _ident,
	/*  l */ _ascii | _alpha | _lower | _print | _ident,
	/*  m */ _ascii | _alpha | _lower | _print | _ident,
	/*  n */ _ascii | _alpha | _lower | _print | _ident,
	/*  o */ _ascii | _alpha | _lower | _print | _ident,
	/*  p */ _ascii | _alpha | _lower | _print | _ident,
	/*  q */ _ascii | _alpha | _lower | _print | _ident,
	/*  r */ _ascii | _alpha | _lower | _print | _ident,
	/*  s */ _ascii | _alpha | _lower | _print | _ident,
	/*  t */ _ascii | _alpha | _lower | _print | _ident,
	/*  u */ _ascii | _alpha | _lower | _print | _ident,
	/*  v */ _ascii | _alpha | _lower | _print | _ident,
	/*  w */ _ascii | _alpha | _lower | _print | _ident,
	/*  x */ _ascii | _alpha | _lower | _print | _ident,
	/*  y */ _ascii | _alpha | _lower | _print | _ident,
	/*  z */ _ascii | _alpha | _lower | _print | _ident,
	/*  { */ _ascii | _print | _punct,
	/*  | */ _ascii | _print | _punct,
	/*  } */ _ascii | _print | _punct,
	/*  ~ */ _ascii | _print | _punct,
	/* ^? */ _ascii | _cntrl,
	/* 128 */ 0,
	/* 129 */ 0,
	/* 130 */ 0,
	/* 131 */ 0,
	/* 132 */ 0,
	/* 133 */ 0,
	/* 134 */ 0,
	/* 135 */ 0,
	/* 136 */ 0,
	/* 137 */ 0,
	/* 138 */ 0,
	/* 139 */ 0,
	/* 140 */ 0,
	/* 141 */ 0,
	/* 142 */ 0,
	/* 143 */ 0,
	/* 144 */ 0,
	/* 145 */ 0,
	/* 146 */ 0,
	/* 147 */ 0,
	/* 148 */ 0,
	/* 149 */ 0,
	/* 150 */ 0,
	/* 151 */ 0,
	/* 152 */ 0,
	/* 153 */ 0,
	/* 154 */ 0,
	/* 155 */ 0,
	/* 156 */ 0,
	/* 157 */ 0,
	/* 158 */ 0,
	/* 159 */ 0,
	/* 160 */ 0,
	/* 161 */ 0,
	/* 162 */ 0,
	/* 163 */ 0,
	/* 164 */ 0,
	/* 165 */ 0,
	/* 166 */ 0,
	/* 167 */ 0,
	/* 168 */ 0,
	/* 169 */ 0,
	/* 170 */ 0,
	/* 171 */ 0,
	/* 172 */ 0,
	/* 173 */ 0,
	/* 174 */ 0,
	/* 175 */ 0,
	/* 176 */ 0,
	/* 177 */ 0,
	/* 178 */ 0,
	/* 179 */ 0,
	/* 180 */ 0,
	/* 181 */ 0,
	/* 182 */ 0,
	/* 183 */ 0,
	/* 184 */ 0,
	/* 185 */ 0,
	/* 186 */ 0,
	/* 187 */ 0,
	/* 188 */ 0,
	/* 189 */ 0,
	/* 190 */ 0,
	/* 191 */ 0,
	/* 192 */ 0,
	/* 193 */ 0,
	/* 194 */ 0,
	/* 195 */ 0,
	/* 196 */ 0,
	/* 197 */ 0,
	/* 198 */ 0,
	/* 199 */ 0,
	/* 200 */ 0,
	/* 201 */ 0,
	/* 202 */ 0,
	/* 203 */ 0,
	/* 204 */ 0,
	/* 205 */ 0,
	/* 206 */ 0,
	/* 207 */ 0,
	/* 208 */ 0,
	/* 209 */ 0,
	/* 210 */ 0,
	/* 211 */ 0,
	/* 212 */ 0,
	/* 213 */ 0,
	/* 214 */ 0,
	/* 215 */ 0,
	/* 216 */ 0,
	/* 217 */ 0,
	/* 218 */ 0,
	/* 219 */ 0,
	/* 220 */ 0,
	/* 221 */ 0,
	/* 222 */ 0,
	/* 223 */ 0,
	/* 224 */ 0,
	/* 225 */ 0,
	/* 226 */ 0,
	/* 227 */ 0,
	/* 228 */ 0,
	/* 229 */ 0,
	/* 230 */ 0,
	/* 231 */ 0,
	/* 232 */ 0,
	/* 233 */ 0,
	/* 234 */ 0,
	/* 235 */ 0,
	/* 236 */ 0,
	/* 237 */ 0,
	/* 238 */ 0,
	/* 239 */ 0,
	/* 240 */ 0,
	/* 241 */ 0,
	/* 242 */ 0,
	/* 243 */ 0,
	/* 244 */ 0,
	/* 245 */ 0,
	/* 246 */ 0,
	/* 247 */ 0,
	/* 248 */ 0,
	/* 249 */ 0,
	/* 250 */ 0,
	/* 251 */ 0,
	/* 252 */ 0,
	/* 253 */ 0,
	/* 254 */ 0,
	/* 255 */ 0
};
                            /*### @-CharClass::sCharMap ***********************/

                            /*### @+String::Rep::calculateSize {{{2 ************/
size_t String::Rep::calculateSize(size_t size) 
{
	//
	// The goal here is to have the entire memory block (malloc header + rep
	// + data) land on a decent boundary.
	//
	const size_t _page_size = 4096;
    const size_t _region_size = 128;
    const size_t _hdr_size = 4 * sizeof(void*);
    if ( (size + _hdr_size) > _page_size )
        size += ( _page_size - ((size + _hdr_size) % _page_size) );
    else if ( (size + _hdr_size) > _region_size )
        size += ( _region_size - ((size + _hdr_size) % _region_size) );

	return size;
}
                            /*### @-String::Rep::calculateSize }}}2 ************/
                            /*### @+String::Rep::operator new {{{2 ************/
void* String::Rep::operator new(
	size_t rep_size,
	size_t capacity)
{
	if ( capacity < 20 )
		capacity = 20;

	size_t size = calculateSize(rep_size + capacity);

	void* buf = malloc(size);
	Rep* r = NULL;
	if (buf) { 
		r = reinterpret_cast<Rep*>(buf);
		r->mCapacity = size - rep_size;
	}
	return r;
}
                            /*### @-String::Rep::operator new }}}2 ************/
                            /*### @+String::Rep::resize {{{2 ******************/
int realloc_fail_cnt = 0;
String::Rep* String::Rep::resize(size_type n, bool up)
{
	if ( n < 20 )
			n = 20;
	int savedCapacity = mCapacity;
	mCapacity = n;
	size_t newSize = calculateSize(mCapacity + sizeof(Rep));

	void* buf = realloc(static_cast<void*>(this), newSize);
	if (buf)  {
		Rep* r = reinterpret_cast<Rep*>(buf);
		r->mCapacity = newSize - sizeof(Rep);
		return r;
	}  
	mCapacity = savedCapacity;
	return NULL;
}
                            /*### @-String::Rep::resize }}}2 ******************/
                            /*### @+String::Rep::operator delete {{{2 *********/
void String::Rep::operator delete(
	void* ptr)
{
	free(ptr);
}
                            /*### @-String::Rep::operator delete }}}2 *********/
                            /*### @+String::Rep::release {{{2 *****************/
bool String::Rep::release()
{
	return ( --mReferences == 0 );
}
                            /*### @-String::Rep::release }}}2 *****************/
const String::size_type String::npos = static_cast<size_type>(-1);
                            /*### @+String::String.copy {{{2 ******************/
String::String(
	const String& s,
	size_type pos,
	size_type n)
:
	mData(NULL),
	mLength(0),
	mOffset(String::npos),
	mRep(NULL)
{
	if ( chkPosFixNum(pos, s.length(), n) >= 0 )
		setStr(s, pos, n);
}
                            /*### @-String::String.copy }}}2 ******************/
                            /*### @+String::String.cstr1 {{{2 *****************/
String::String(
	const char* s,
	size_type n)
:
	mData(NULL),
	mLength(0),
	mOffset(String::npos),
	mRep(NULL)
{

	if ( !s ) {
		setExtern(NULL, 0);
	}
	else {
		if ( n == npos )
			n = strlen(s);
		setNormal(n, n, s);
	}
}
                            /*### @-String::String.cstr1 }}}2 *****************/
                            /*### @+String::String.ch {{{2 ********************/
String::String(
	size_type n,
	char c)
:
	mData(NULL),
	mLength(0),
	mOffset(String::npos),
	mRep(NULL)
{
	if ( n != 0 )
		setNormal(n, n, NULL, c);
}
                            /*### @-String::String.ch }}}2 ********************/
                            /*### @+String::String.const {{{2 *****************/
String::String(
	bool /*dummy*/,
	const char* s,
	size_type n)
:
	mData(NULL),
	mLength(0),
	mOffset(String::npos),
	mRep(NULL)
{
	setExtern(s, n);
}
                            /*### @-String::String.const }}}2 *****************/
                            /*### @+String::~String {{{2 **********************/
String::~String()
{
	if ( !is_extern() ) {
		if ( mRep->release() )
			delete mRep;
	}
}
                            /*### @-String::~String }}}2 **********************/
                            /*### @+String::from_date {{{2 ********************/
String String::from_date(
	const Date& d,
	bool humanFormat,
	bool includeDate,
	bool includeTime,
	bool includeTZ,
	int fracDigits)
{
	String s;

	if ( includeDate ) {
		if ( humanFormat )
			s.appendf("%04u-%02u-%02u", d.year(), d.mon(), d.mday());
		else
			s.appendf("%04u%02u%02u", d.year(), d.mon(), d.mday());

		if ( includeTime )
			s.append(1, humanFormat ? ' ' : 'T');
	}

	if ( includeTime ) {
		if ( humanFormat )
			s.appendf("%02u:%02u:", d.hour(), d.min());
		else
			s.appendf("%02u%02u", d.hour(), d.min());

		double secs = d.sec() + d.usec() / 1.0e6;
		if ( fracDigits < 0 )
			fracDigits = 0;
		if ( fracDigits > 6 )
			fracDigits = 6;
		s.appendf("%0*.*f", fracDigits ? (fracDigits + 3) : 2, fracDigits, secs);

		if ( includeTZ ) {
			if ( d.tz() )
				s.appendf("%+03d:%02d", (d.tz() / 3600), abs((d.tz() % 3600) / 60));
			else
				s.append(1, 'Z');
		}
	}

	return s;
}
                            /*### @-String::from_date }}}2 ********************/
                            /*### @+String::from_elapsed {{{2 *****************/
String String::from_elapsed(
	const Time::Elapsed& e,
	bool iso8601,
	int fracDigits)
{
	String str;
	if ( fracDigits < 0 )
		fracDigits = 0;
	else if ( fracDigits > 9 )
		fracDigits = 9;

	if ( iso8601 ) {
		int32_t secs = static_cast<int32_t>(Time::SEC.fromElapsed(e));

		if ( secs < 0 ) {
			str.append(1, '-');
			secs =  -secs;
		}
		str.appendf("%02d", secs / 3600);
		secs %= 3600;

		str.appendf(":%02d", secs / 60);
		secs %= 60;

		if ( fracDigits ) {
			double f = llabs(Time::NSEC.fromElapsed(e) % 60000000000LL) / 1.0e9;
			str.appendf(":%0*.*f", fracDigits + 3, fracDigits, f);
		}
		else
			str.appendf(":%02d", secs);
	}
	else {
		double f = Time::NSEC.fromElapsed(e) / 1.0e9;
		str.appendf("%.*f", fracDigits, f);
	}

	return str;
}
                            /*### @-String::from_elapsed }}}2 *****************/
                            /*### @+String::adjust_length {{{2 ****************/
void String::adjust_length(
	size_type n,
	size_type pos)
{
	if (!modify(0, true)) return;

	fixNum(pos, length());

	size_type newLen = pos + n;
	if ( newLen <= capacity() ) {
		if ( newLen > length() )
			mData[mRep->mLength = newLen] = '\0';
	}
}
                            /*### @-String::adjust_length }}}2 ****************/
                            /*### @+String::append.str1 {{{2 ******************/
String& String::append(
	const String& s,
	size_type pos,
	size_type n)
{

	if ( chkPosFixNum(pos, s.length(), n) != 0 )
		return *this;
	else
		return append(s.data() + pos, n);
}
                            /*### @-String::append.str1 }}}2 ******************/
                            /*### @+String::append.cstr1 {{{2 *****************/
String& String::append(
	const char* s,
	size_type n)
{
	if ( n == npos )
		n = strlen(s);
	if ( n == 0 )
		return *this;

	if (modify(n, true)) {
		memcpy(mData + mRep->mLength, s, n);
		mData[(mRep->mLength += n)] = '\0';
	}

	return *this;
}
                            /*### @-String::append.cstr1 }}}2 *****************/
                            /*### @+String::append.ch {{{2 ********************/
String& String::append(
	size_type n,
	char c)
{
	if ( n == 0 )
		return *this;

	if ( modify(n, true)) {
		memset(mData + mRep->mLength, c, n);
		mData[(mRep->mLength += n)] = '\0';
	}
	return *this;
}
                            /*### @-String::append.ch }}}2 ********************/
                            /*### @+String::appendf {{{2 **********************/
String& String::appendf(
	const char* format,
	...)
{
	va_list ap;
	va_start(ap, format);
	vappendf(format, ap);
	va_end(ap);

	return *this;
}
                            /*### @-String::appendf }}}2 **********************/
                            /*### @+String::vappendf {{{2 *********************/
String& String::vappendf(
	const char* format,
	va_list ap)
{
	char lbuf[128];
	int n = vsnprintf(lbuf, sizeof(lbuf), format, ap);

	if ( modify(n, true)) {

		//
		// If we got the whole thing into our local buffer, just append that.
		// Otherwise, write it directly into the rep's buffer (which we've just
		// grown big enough to handle it).
		//
		if ( n < static_cast<int>(sizeof(lbuf)) )
			memcpy(mData + mRep->mLength, lbuf, n + 1);
		else
			vsnprintf(mData + mRep->mLength, n + 1, format, ap);
		mRep->mLength += n;
	}

	return *this;
}
                            /*### @-String::vappendf }}}2 *********************/
                            /*### @+String::assign.copy {{{2 ******************/
String& String::assign(
	const String& s)
{
	if ( &s != this ) {
		if ( !is_extern() && mRep->release() )
			delete mRep;
		setStr(s, 0, s.length());
	}
	return *this;
}
                            /*### @-String::assign.copy }}}2 ******************/
                            /*### @+String::assign.str1 {{{2 ******************/
String& String::assign(
	const String& s,
	size_type pos,
	size_type n)
{

	if ( chkPosFixNum(pos, s.length(), n) )
		return *this;

	if ( &s == this ) {
		String s1(*this, pos, n); // get right slice or ptr

		if ( !is_extern() )
			mRep->release(); // always have at least s1

		setStr(s1, 0, s1.length());
	}
	else {
		if ( !is_extern() && mRep->release() )
			delete mRep;

		setStr(s, pos, n);
	}

	return *this;
}
                            /*### @-String::assign.str1 }}}2 ******************/
                            /*### @+String::assign.cstr1 {{{2 *****************/
String& String::assign(
	const char* s,
	size_type n)
{

	if ( n == npos )
		n = strlen(s);

	if ( n ) {
		if (modify(n, false)) {
			if ( s < mData || s > (mData + mRep->mCapacity) )
				memcpy(mData, s, n);
			else
				memmove(mData, s, n);
			mData[(mRep->mLength = n)] = '\0';
		}
	}
	else
		clear();
	return *this;
}
                            /*### @-String::assign.cstr1 }}}2 *****************/
                            /*### @+String::assign.ch {{{2 ********************/
String& String::assign(
	size_type n,
	char c)
{
	if ( n ) {
		if ( modify(n, false)) {
			memset(mData, c, n);
			mData[(mRep->mLength = n)] = '\0';
		}
	}
	else
		clear();
	return *this;
}
                            /*### @-String::assign.ch }}}2 ********************/
                            /*### @+String::assign.const {{{2 *****************/
String& String::assign(
	bool /*dummy*/,
	const char* s,
	size_type n)
{
	if ( !is_extern() && mRep->release() )
		delete mRep;

	setExtern(s, n);
	return *this;
}
                            /*### @-String::assign.const }}}2 *****************/
                            /*### @+String::assignf {{{2 **********************/
String& String::assignf(
	const char* format,
	...)
{
	va_list ap;
	va_start(ap, format);
	vassignf(format, ap);
	va_end(ap);

	return *this;
}
                            /*### @-String::assignf }}}2 **********************/
                            /*### @+String::vassignf {{{2 *********************/
String& String::vassignf(
	const char* format,
	va_list ap)
{
	char lbuf[128];
	int n = vsnprintf(lbuf, sizeof(lbuf), format, ap);

	//
	// If we got the whole thing into our local buffer, just append that.
	// Otherwise, write it directly into the rep's buffer (which we've just
	// grown big enough to handle it).
	//
	if ( modify(n, false)) {

		if ( n < static_cast<int>(sizeof(lbuf)) )
			memcpy(mData, lbuf, n + 1);
		else
			vsnprintf(mData, n + 1, format, ap);
		mRep->mLength = n;
	}
	return *this;
}
                            /*### @-String::vassignf }}}2 *********************/
                            /*### @+String::clear {{{2 ************************/
void String::clear()
{
	if ( is_extern() )
		setExtern(NULL, 0);
	else if ( mRep->mReferences > 1 ) {
		if ( mRep->release() )
			delete mRep;
		setExtern(NULL, 0);
	}
	else {
		if (modify(0, false))
			mData[(mRep->mLength = 0)] = '\0';
	}
}
                            /*### @-String::clear }}}2 ************************/
                            /*### @+String::erase {{{2 ************************/
String& String::erase(
	size_type pos,
	size_type n)
{
	if ( chkPosFixNum(pos, length(), n) )
		return *this;

	if ( modify(0, true)) {
		if ( (pos + n) < mRep->mLength )
			SHIFT_(pos, pos + n, mData, mRep->mLength, 1);
		mData[mRep->mLength -= n] = '\0';
	}
	return *this;
}
                            /*### @-String::erase }}}2 ************************/
                            /*### @+String::get_raw {{{2 **********************/
char* String::get_raw(
	size_type n,
	size_type pos)
{
	fixNum(pos, length());

	bool res;
	if ( (pos + n) > length() )
		res = modify((pos + n) - length(), true);
	else
		res = modify(0, true);

	return (res? mData + pos: NULL);
}
                            /*### @-String::get_raw }}}2 **********************/
                            /*### @+String::insert.str1 {{{2 ******************/
String& String::insert(
	size_type pos,
	const String& s,
	size_type pos1,
	size_type n)
{
	CXPROF_UTIL(v0, "String::insert().str1");

	if ( chkPosFixNum(pos1, s.length(), n) )
		return *this;

	insert(pos, s.data() + pos1, n);

	return *this;
}
                            /*### @-String::insert.str1 }}}2 ******************/
                            /*### @+String::insert.cstr {{{2 ******************/
String& String::insert(
	size_type pos,
	const char* s,
	size_type n)
{
	if ( pos >= length() )
		return append(s, n);

	if ( n == npos )
		n = strlen(s);
	if ( n == 0 )
		return *this;

	if (modify(n, true)) {
		SHIFT_(pos + n, pos, mData, mRep->mLength, 1);
		memcpy(mData + pos, s, n);
		mData[(mRep->mLength += n)] = '\0';
	}

	return *this;
}
                            /*### @-String::insert.cstr }}}2 ******************/
                            /*### @+String::insert.ch {{{2 ********************/
String& String::insert(
	size_type pos,
	size_type n,
	char c)
{
	if ( n == 0 )
		return *this;

	if ( pos >= length() )
		return append(n, c);

	if( modify(n, true)) {
		SHIFT_(pos + n, pos, mData, mRep->mLength, 1);
		memset(mData + pos, c, n);
		mData[(mRep->mLength += n)] = '\0';
	}

	return *this;
}
                            /*### @-String::insert.ch }}}2 ********************/
                            /*### @+String::operator[].lval {{{2 **************/
char& String::operator[](
	size_type n)
{
	assert( n < length());
	modify(0, true);
	return mData[n];
	
}
                            /*### @-String::operator[].lval }}}2 **************/
                            /*### @+String::replace.str {{{2 ******************/
String& String::replace(
	size_type pos,
	size_type n,
	const String& s,
	size_type pos1,
	size_type n1)
{
	if ( chkPosFixNum(pos, length(), n)
			|| chkPosFixNum(pos1, s.length(), n1) )
		return *this;

	replace(pos, n, s.data() + pos1, n1);

	return *this;
}
                            /*### @-String::replace.str }}}2 ******************/
                            /*### @+String::replace.cstr {{{2 *****************/
String& String::replace(
	size_type pos,
	size_type n,
	const char* s,
	size_type n1)
{
	if ( chkPosFixNum(pos, length(), n) )
		return *this;

	if ( n1 == npos )
		n1 = strlen(s);
	if ( n1 == 0 )
		return *this;

	if (modify(n1 > n ? (n1 - n) : 0, true)) {

		if ( n != n1 ) {
			SHIFT_(pos + n1, pos + n, mData, mRep->mLength, 1);
			mData[(mRep->mLength += (n1 - n))] = '\0';
		}
		memcpy(mData + pos, s, n1);
	}

	return *this;
}
                            /*### @-String::replace.cstr }}}2 *****************/
                            /*### @+String::replace.ch {{{2 *******************/
String& String::replace(
	size_type pos,
	size_type n,
	size_type n1,
	char c)
{
	if ( chkPosFixNum(pos, length(), n) || n1 == 0 )
		return *this;

	if ( modify(n1 > n ? (n1 - n) : 1, true) ) {

		if ( n != n1 ) {
			SHIFT_(pos + n1, pos + n, mData, mRep->mLength, 1);
			mData[(mRep->mLength += (n1 - n))] = '\0';
		}
		memset(mData + pos, c, n1);
	}
	return *this;
}
                            /*### @-String::replace.ch }}}2 *******************/
                            /*### @+String::reserve {{{2 **********************/
bool String::reserve(
	size_type n)
{
	if ( capacity() < n )
		return modify(n - capacity(), true);
	else if ( capacity() > n + 4096 ) {
		if ( !modify(0, true)) return false;
		if ( capacity() > n + 4096 ) {
			Rep* rep = mRep->resize(n + 1, false);
			if (!rep) return false;
			else {
				mRep = rep;
				return true;
			}
		} 
	}
	return true;
}
                            /*### @-String::reserve }}}2 **********************/
                            /*### @+String::swap {{{2 *************************/
void String::swap(
	String& s)
{
	String tmp(s);
	s.assign(*this);
	assign(tmp);
}
                            /*### @-String::swap }}}2 *************************/
                            /*### @+String::to_lower {{{2 *********************/
String& String::to_lower(
	size_type pos,
	size_type n)
{
	if ( chkPosFixNum(pos, length(), n) )
		return *this;

	const size_type epos = pos + n;
	if ( find_first_of(CharClass::UPPER, pos) < epos ) {
		if (modify(0, true)) {
			for ( size_type i = pos; i < epos; ++i ) {
				if ( CharClass::UPPER.match(mData[i]) )
					mData[i] |= 0x20;
			}
		}
	}

	return *this;
}
                            /*### @-String::to_lower }}}2 *********************/
                            /*### @+String::to_upper {{{2 *********************/
String& String::to_upper(
	size_type pos,
	size_type n)
{

	if ( chkPosFixNum(pos, length(), n) )
		return *this;

	const size_type epos = pos + n;
	if ( find_first_of(CharClass::LOWER, pos) < epos ) {
		if ( modify(0, true)) {
			for ( size_type i = pos; i < epos; ++i ) {
				if ( CharClass::LOWER.match(mData[i]) )
					mData[i] &= ~0x20;
			}
		}
	}

	return *this;
}
                            /*### @-String::to_upper }}}2 *********************/
                            /*### @+String::trim_left {{{2 ********************/
String& String::trim_left()
{
	if ( length() && CharClass::SPACE.match(mData[0]) ) {
		if ( modify(0, true)) {
			size_type n = find_first_not_of(CharClass::SPACE);
			if ( n == npos )
				clear();
			else
				erase(0, n);
		}
	}

	return *this;
}
                            /*### @-String::trim_left }}}2 ********************/
                            /*### @+String::trim_right {{{2 *******************/
String& String::trim_right()
{
	if ( length() && CharClass::SPACE.match(mData[length() - 1]) ) {
		if ( modify(0, true)) {
			size_type n = find_last_not_of(CharClass::SPACE);
			if ( n == npos )
				clear();
			else
				erase(n + 1);
		}
	}

	return *this;
}
                            /*### @-String::trim_right }}}2 *******************/
                            /*### @+String::compare.str2 {{{2 *****************/
int String::compare(
	size_type pos,
	size_type n,
	const String& s,
	size_type pos1,
	size_type n1,
	bool ignoreCase) const
{
	CXPROF_UTIL(v0, "String::compare().str2");

	switch ( chkPosFixNum(pos1, s.length(), n1) ) {
		case -1:
			return length();

		case 1:
			return -1;

		default:
			return compare(pos, n, s.data() + pos1, n1, ignoreCase);
	}
}
                            /*### @-String::compare.str2 }}}2 *****************/
                            /*### @+String::compare.cstr1 {{{2 ****************/
int String::compare(
	size_type pos,
	size_type n,
	const char* s,
	size_type len,
	bool ignoreCase) const
{
	CXPROF_UTIL(v0, "String::compare().cstr1");
	switch ( chkPosFixNum(pos, length(), n) ) {
		case -1:
			return -len;

		case 1:
			return -1;

		default:
			break;
	}

	if ( len == npos )
		len = strlen(s);
	if ( len == 0 )
		return 1;

	int zero_value = 0;
	if ( len < n )
		zero_value = 1;
	else if ( len > n ) {
		zero_value = -1;
		len = n;
	}

	int cmp = ignoreCase
			? strncasecmp(data() + pos, s, len)
			: memcmp(data() + pos, s, len);

	return ( (cmp == 0) ? zero_value : cmp );
}
                            /*### @-String::compare.cstr1 }}}2 ****************/
                            /*### @+String::find.cstr {{{2 ********************/
String::size_type String::find(
	const char* s,
	size_type pos,
	size_type n) const
{
	CXPROF_UTIL(v0, "String::find().cstr");
	if ( !s || pos >= length() )
		return npos;

	if ( n == npos )
		n = strlen(s);
	if ( n == 0 )
		return npos;

	size_type rem = length() - pos;
	while ( rem >= n ) {
		if ( mData[pos] != *s || memcmp(data() + pos, s, n) != 0 ) {
			--rem;
			++pos;
		}
		else
			return pos;
	}

	return npos;
}
                            /*### @-String::find.cstr }}}2 ********************/
                            /*### @+String::find.ch {{{2 **********************/
String::size_type String::find(
	char c,
	size_type pos) const
{
	CXPROF_UTIL(v0, "String::find().ch");
	if ( chkPos(pos, length()) )
		return npos;

	const char* p = static_cast<const char*>(memchr(data() + pos, c, length() - pos));
	return ( p ? (p - data()) : npos );
}
                            /*### @-String::find.ch }}}2 **********************/
                            /*### @+String::find_first_of.cstr {{{2 ***********/
String::size_type String::find_first_of(
	const char* s,
	size_type pos,
	size_type n) const
{
	CXPROF_UTIL(v0, "String::find_first_of().cstr");
	if ( !s ) return npos;
	size_type len = length();

	if ( chkPos(pos, len) )
		return npos;

	if ( n == npos )
		n = strlen(s);
	if ( n == 0 )
		return npos;

	size_type clen = len - pos;
	size_type fpos = npos;
	const char* p;

	do {
		p = reinterpret_cast<const char*>(memchr(mData + pos, s[--n], clen));
		if ( p ) {
			if ( (fpos = p - mData) == pos )
				return pos;
			clen = fpos + 1 - pos;
		}
	} while ( n > 0 );

	return fpos;
}
                            /*### @-String::find_first_of.cstr }}}2 ***********/
                            /*### @+String::find_first_of.match {{{2 **********/
String::size_type String::find_first_of(
	const CharClass& m,
	size_type pos) const
{
	CXPROF_UTIL(v0, "String::find_first_of().match");
	size_type len = length();
	if ( chkPos(pos, len) )
		return npos;

	while ( pos < len && !m.match(mData[pos]) )
		++pos;
	return ( (pos == len) ? npos : pos );
}
                            /*### @-String::find_first_of.match }}}2 **********/
                            /*### @+String::find_first_not_of.cstr {{{2 *******/
String::size_type String::find_first_not_of(
	const char* s,
	size_type pos,
	size_type n) const
{
	CXPROF_UTIL(v0, "String::find_first_not_of().cstr");
	if ( !s ) return npos;
	size_type len = length();

	if ( chkPos(pos, len) )
		return npos;

	if ( n == npos )
		n = strlen(s);
	if ( n == 0 )
		return npos;

	while ( pos < len && memchr(s, mData[pos], n) )
		++pos;
	return ( (pos == len) ? npos : pos );
}
                            /*### @-String::find_first_not_of.cstr }}}2 *******/
                            /*### @+String::find_first_not_of.ch {{{2 *********/
String::size_type String::find_first_not_of(
	char c,
	size_type pos) const
{
	CXPROF_UTIL(v0, "String::find_first_not_of().ch");
	size_type len = length();
	if ( chkPos(pos, len) )
		return npos;

	while ( pos < len && mData[pos] == c )
		++pos;

	return ( (pos == len) ? npos : pos );
}
                            /*### @-String::find_first_not_of.ch }}}2 *********/
                            /*### @+String::find_first_not_of.match {{{2 ******/
String::size_type String::find_first_not_of(
	const CharClass& m,
	size_type pos) const
{
	CXPROF_UTIL(v0, "String::find_first_not_of().match");
	size_type len = length();
	if ( chkPos(pos, len) )
		return npos;

	while ( pos < len && m.match(mData[pos]) )
		++pos;

	return ( (pos == len) ? npos : pos );
}
                            /*### @-String::find_first_not_of.match }}}2 ******/
                            /*### @+String::starts_with.cstr {{{2 *******************/
bool String::starts_with(
	const char* s) const
{
	CXPROF_UTIL(v0, "String::starts_with.cstr");
	if ( !s ) return false;
	size_type pos = find(s);
	return ( pos == 0 );
}
                            /*### @-String::starts_with.cstr }}}2 ******/
                            /*### @+String::starts_with.String {{{2 *******************/
bool String::starts_with(
	const String& s) const
{
	CXPROF_UTIL(v0, "String::starts_with.String");
	return starts_with(s.c_str());
}
                            /*### @-String::starts_with.String }}}2 ******/
                            /*### @+String::ends_with.cstr {{{2 *******************/
bool String::ends_with(
	const char* s) const
{
	CXPROF_UTIL(v0, "String::ends_with.cstr");
	if ( !s ) return false;
	size_type pos = rfind(s, npos, strlen(s));
	return ( pos == length() - strlen(s) );
}
                            /*### @-String::ends_with.cstr }}}2 ******/
                            /*### @+String::ends_with.String {{{2 *******************/
bool String::ends_with(
	const String& s) const
{
	CXPROF_UTIL(v0, "String::ends_with.String");
	return ends_with(s.c_str());
}
                            /*### @-String::ends_with.String }}}2 ******/
                            /*### @+String::rfind.cstr {{{2 *******************/
String::size_type String::rfind(
	const char* s,
	size_type pos,
	size_type n) const
{
	CXPROF_UTIL(v0, "String::rfind().cstr");
	if ( !s ) return npos;
	size_type len = length();
	if ( n == npos )
		n = strlen(s);
	if ( n == 0 || n >= len )
		return npos;

	fixNum(pos, len - n);

	for ( ;; ) {
		if ( mData[pos] == *s && memcmp(data() + pos, s, n) == 0 )
			return pos;
		else if ( pos-- == 0 )
			return npos;
	}
}
                            /*### @-String::rfind.cstr }}}2 *******************/
                            /*### @+String::rfind.ch {{{2 *********************/
String::size_type String::rfind(
	char c,
	size_type pos) const
{
	CXPROF_UTIL(v0, "String::rfind().ch");
	size_type len = length();
	if ( len == 0 )
		return npos;

	fixNum(pos, len - 1);

	for ( ;; ) {
		if ( mData[pos] == c )
			return pos;
		else if ( pos-- == 0 )
			return npos;
	}
}
                            /*### @-String::rfind.ch }}}2 *********************/
                            /*### @+String::find_last_of.cstr {{{2 ************/
String::size_type String::find_last_of(
	const char* s,
	size_type pos,
	size_type n) const
{
	CXPROF_UTIL(v0, "String::find_last_of().cstr");
	if ( !s ) return npos;
	size_type len = length();
	if ( n == npos )
		n = strlen(s);
	if ( n == 0 || n >= len )
		return npos;

	fixNum(pos, len - 1);

	for ( ;; ) {
		if ( memchr(s, mData[pos], n) )
			return pos;
		else if ( pos-- == 0 )
			return npos;
	}
}
                            /*### @-String::find_last_of.cstr }}}2 ************/
                            /*### @+String::find_last_of.match {{{2 ***********/
String::size_type String::find_last_of(
	const CharClass& m,
	size_type pos) const
{
	CXPROF_UTIL(v0, "String::find_last_of().match");
	size_type len = length();
	if ( len == 0 )
		return npos;

	fixNum(pos, len - 1);

	for ( ;; ) {
		if ( m.match(mData[pos]) )
			return pos;
		else if ( pos-- == 0 )
			return npos;
	}
}
                            /*### @-String::find_last_of.match }}}2 ***********/
                            /*### @+String::find_last_not_of.cstr {{{2 ********/
String::size_type String::find_last_not_of(
	const char* s,
	size_type pos,
	size_type n) const
{
	CXPROF_UTIL(v0, "String::find_last_not_of().cstr");
	if ( !s ) return npos;
	size_type len = length();
	if ( n == npos )
		n = strlen(s);
	if ( n == 0 || n >= len )
		return npos;

	fixNum(pos, len - 1);

	for ( ;; ) {
		if ( !memchr(s, mData[pos], n) )
			return pos;
		else if ( pos-- == 0 )
			return npos;
	}
}
                            /*### @-String::find_last_not_of.cstr }}}2 ********/
                            /*### @+String::find_last_not_of.ch {{{2 **********/
String::size_type String::find_last_not_of(
	char c,
	size_type pos) const
{
	CXPROF_UTIL(v0, "String::find_last_not_of().ch");
	size_type len = length();
	if ( len == 0 )
		return pos;

	fixNum(pos, len - 1);

	for ( ;; ) {
		if ( mData[pos] != c )
			return pos;
		else if ( pos-- == 0 )
			return npos;
	}
}
                            /*### @-String::find_last_not_of.ch }}}2 **********/
                            /*### @+String::find_last_not_of.match {{{2 *******/
String::size_type String::find_last_not_of(
	const CharClass& m,
	size_type pos) const
{
	CXPROF_UTIL(v0, "String::find_last_not_of().match");

	size_type len = length();
	if ( len == 0 )
		return pos;

	fixNum(pos, len - 1);

	for ( ;; ) {
		if ( !m.match(mData[pos]) )
			return pos;
		else if ( pos-- == 0 )
			return npos;
	}
}
                            /*### @-String::find_last_not_of.match }}}2 *******/
                            /*### @+String::is_class {{{2 *********************/
bool String::is_class(
	const CharClass& m,
	String::size_type pos,
	String::size_type n) const
{
	if ( pos >= length() )
		return false;

	size_type epos = find_first_not_of(m, pos);
	if ( epos == npos )
		epos = length();

	if ( n == npos ) {
		if ( epos == length() )
			return true;
		else
			return false;
	}
	else if ( n <= (epos - pos) )
		return true;
	else
		return false;
}
                            /*### @-String::is_class }}}2 *********************/
                            /*### @+String::copy {{{2 *************************/
String::size_type String::copy(
	char* buf,
	size_type n,
	size_type pos) const
{
	if ( chkPosFixNum(pos, length(), n) )
		return 0;
	else {
		memcpy(buf, data() + pos, n);
		return n;
	}
}
                            /*### @-String::copy }}}2 *************************/
                            /*### @+String::substr {{{2 ***********************/
String String::substr(
	size_type pos,
	size_type n) const
{
	String b;

	if ( chkPosFixNum(pos, length(), n) )
		return b;
	else
		return b.assign(*this, pos, n);
}
                            /*### @-String::substr }}}2 ***********************/
                            /*### @+String::chkPos {{{2 ***********************/
int String::chkPos(
	size_type pos,
	size_type len)
{
	return ( (pos >= len) ? -1 : 0 );
}
                            /*### @-String::chkPos }}}2 ***********************/
                            /*### @+String::chkPosFixNum {{{2 *****************/
int String::chkPosFixNum(
	size_type pos,
	size_type len,
	size_type& n)
{
	if ( chkPos(pos, len) )
		return -1;
	else
		return fixNum(n, len - pos);
}
                            /*### @-String::chkPosFixNum }}}2 *****************/
                            /*### @+String::fixNum {{{2 ***********************/
int String::fixNum(
	size_type& n,
	size_type len)
{
	if ( n > len )
		n = len;

	return (n > 0) ? 0 : 1;
}
                            /*### @-String::fixNum }}}2 ***********************/
                            /*### @+String::cstrSafe {{{2 *********************/
bool String::cstrSafe() const
{
	if ( mRep ) {
		if ( mOffset == npos || (mLength + mOffset) == mRep->mLength )
			return true;
		else
			return false;
	}
	else
		return ( mOffset == npos );
}
                            /*### @-String::cstrSafe }}}2 *********************/
                            /*### @+String::modify {{{2 ***********************/
bool String::modify(
	size_type needed,
	bool keepData) const
{
	size_type len = 0;
	if ( keepData ) {
		len = length();
		needed += len;
	}
	if ( is_extern() )
		return setNormal(needed, len, mData);
	else if ( mRep->mReferences > 1 ) {
		String tmp(*this);
		mRep->release();
		return setNormal(needed, len, tmp.data());
	} else if ( needed >= mRep->mCapacity || mOffset != npos ) {
		if ( needed >= mRep->mCapacity ) {
			Rep* rep = mRep->resize(needed + 1);
			if (rep) mRep = rep;
			else return false;
		} 
		if ( mOffset != npos && len )
			SHIFT_(0, mOffset, mRep->data(), len + mOffset, 1);
		mOffset = npos;
		mData = mRep->data();
		mLength = 0;
		mData[mRep->mLength = len] = '\0';
		return true;
	}
	return true;
}
                            /*### @-String::modify }}}2 ***********************/
                            /*### @+String::setExtern {{{2 ********************/
void String::setExtern(
	const char* s,
	size_type n) const
{
	mRep = NULL;
	if ( (mData = const_cast<char*>(s)) ) {
		if ( n == npos ) {
			mOffset = npos;
			mLength = strlen(s);
		}
		else {
			mLength = n;
			mOffset = 0;
		}
	}
	else
		mLength = 0;
}
                            /*### @-String::setExtern }}}2 ********************/
                            /*### @+String::setNormal {{{2 ********************/
bool String::setNormal(
	String::size_type cap,
	String::size_type len,
	const char* s,
	char c) const
{
	mRep = new(cap + 1) Rep();
	if (!mRep)  return false;

	++mRep->mReferences;
	mOffset = npos;
	mData = mRep->data();
	mLength = 0;
	if ( len ) {
		if ( s )
			memcpy(mData, s, len);
		else
			memset(mData, c, len);
	}
	mData[(mRep->mLength = len)] = '\0';
	return true;
}
                            /*### @-String::setNormal }}}2 ********************/
                            /*### @+String::setRef {{{2 ***********************/
void String::setRef(
	const String& s,
	String::size_type pos,
	String::size_type n) const
{
	CXPROF_UTIL(v0, "String::setRef()");

	mRep = s.mRep;
	++mRep->mReferences;

	if ( s.is_normal() ) {
		if ( pos == 0 && n == s.length() ) {
			// normal -> normal
			mOffset = npos;
			mData = mRep->data();
			mLength = 0;
		}
		else {
			// normal -> slice
			mOffset = pos;
			mData = mRep->data() + mOffset;
			mLength = n;
		}
	}
	else {
		// slice -> slice
		mOffset = s.mOffset + pos;
		mData = mRep->data() + mOffset;
		mLength = n;
	}
}
                            /*### @-String::setRef }}}2 ***********************/
                            /*### @+String::setStr {{{2 ***********************/
void String::setStr(
	const String& s,
	String::size_type pos,
	String::size_type n) const
{
	if ( s.is_extern() ) {
		if ( s.data() ) {
			if ( pos + n == s.length() && s.mOffset == npos && mOffset == npos )
				setExtern(s.data() + pos, npos);
			else
				setExtern(s.data() + pos, n);
		}
		else
			setExtern(NULL, 0);
	}
	else
		//setRef(s, pos, n);
		setNormal(n, n, s.c_str() + pos);
}
                            /*### @-String::setStr }}}2 ***********************/
                            /*### @+String::as_int32 {{{2 ***********************/
bool String::as_int32(String& str, int32_t& digits)
{
    if (str.empty()) {
        return false;
    }
    
    int base = 10;
    char *endptr = NULL;
    errno = 0;
    
    int32_t val = strtol(str.c_str(), &endptr, base);
    //Check for various possible errors
    if ((errno != 0) || (endptr == str.c_str()) || (*endptr != '\0')) {
        return false;
    }
    
    digits = val;
    return true;
}
                            /*### @-String::as_int32 }}}2 ***********************/
                            /*### @+String::as_double {{{2 ***********************/
bool String::as_double(String& str, double& digits)
{
    if (str.empty()) {
        return false;
    }
    
    char *endptr = NULL;
    errno = 0;
    
    double val = strtod(str.c_str(), &endptr);
    //Check for various possible errors
    if ((errno != 0) || (endptr == str.c_str()) || (*endptr != '\0')) {
        return false;
    }

    digits = val;
    return true;
}
                            /*### @-String::as_double }}}2 ***********************/
/* ==================== editors ====================== */

