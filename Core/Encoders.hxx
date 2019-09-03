#ifndef _Encoders_hxx
#define _Encoders_hxx 1



#include "cif_stdc.h"


                            /*### @+prefix ************************************/
#include "String.hxx"
                            /*### @-prefix ************************************/

// Classes defined here
class Base64Encoder;
class HexEncoder;
class UrlEncoder;
                            /*### @class Base64Encoder {{{2 *******************/

/*!
** Base64 encoding routines
*/
class Base64Encoder
{
	public:

		/*! HTTP-safe base64 encoding chars. */
		static const String DIGITS_safe;

		/*! Standard base64 encoding chars. */
		static const String DIGITS_std;

		/*! HTTP-safe base64 padding char. */
		static const char PCHAR_safe;

		/*! Standard base64 padding char. */
		static const char PCHAR_std;

		/*! Append base64-decoded version of in[pos,+n] onto out */
		static String& decode(
			String& out,
			const String& in,
			String::size_type pos = 0,
			String::size_type n = String::npos,
			const String& digits = Base64Encoder::DIGITS_std,
			char pad = Base64Encoder::PCHAR_std);

		/*! Append base64-encoded version of in[pos,+n] onto out */
		static String& encode(
			String& out,
			const String& in,
			String::size_type pos = 0,
			String::size_type n = String::npos,
			const String& digits = Base64Encoder::DIGITS_std,
			char pad = Base64Encoder::PCHAR_std);

		/*! Append (url-safe) base64-decoded version of in[pos,+n] onto out */
		static String& decodeUrl(String& out, const String& in, String::size_type pos = 0, String::size_type n = String::npos) {
			return decode(out, in, pos, n, DIGITS_safe, PCHAR_safe);
		}

		/*! Append (url-safe) base64-encoded version of in[pos,+n] onto out */
		static String& encodeUrl(String& out, const String& in, String::size_type pos = 0, String::size_type n = String::npos) {
			return encode(out, in, pos, n, DIGITS_safe, PCHAR_safe);
		}
};
                            /*### @class Base64Encoder }}}2 *******************/
                            /*### @class HexEncoder {{{2 **********************/

/*!
** Hex encoding routines
*/
class HexEncoder
{
	public:

		/*! Append hex-decoded version of in[pos,+n] onto out */
		static String& decode(
			String& out,
			const String& in,
			String::size_type pos = 0,
			String::size_type n = String::npos);

		/*! Decode single hex digit */
		static uint8_t decodeChar(char c) {
			return DIGITS_decode[static_cast<uint8_t>(c)];
		}

		/*! Append hex-encoded version of in[pos,+n] onto out */
		static String& encode(
			String& out,
			const String& in,
			bool lower = true,
			String::size_type pos = 0,
			String::size_type n = String::npos);

	private:

		/*! Mapping of hex digits to nibble value */
		static const uint8_t DIGITS_decode[256];
};
                            /*### @class HexEncoder }}}2 **********************/
                            /*### @class UrlEncoder {{{2 **********************/

/*!
** URL encoding routines
*/
class UrlEncoder
{
	public:

		/*! Append url-decoded version of in[pos,+n] onto out */
		static String& decode(
			String& out,
			const String& in,
			String::size_type pos = 0,
			String::size_type n = String::npos);

		/*! Append url-encoded version of in[pos,+n] onto out */
		static String& encode(
			String& out,
			const String& in,
			String::size_type pos = 0,
			String::size_type n = String::npos);
};
                            /*### @class UrlEncoder }}}2 **********************/

#endif // !_Encoders_hxx


/* ==================== editors ====================== */

