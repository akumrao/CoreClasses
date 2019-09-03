#ifndef _Time_hxx
#define _Time_hxx 1


#include "cif_stdc.h"



#include <inttypes.h>                            /*### @+prefix ************************************/
#include <time.h>
#include <sys/time.h>
#include "Thread.hxx"
                            /*### @-prefix ************************************/

// Classes defined here
class Time;
class MonoTime;
class MonoTimer;
class Date;
class LocalDate;
class UtcDate;
                            /*### @class Time {{{2 ****************************/

/*!
** Wallclock time with a 0.25 nanosecond precision.  Has roughly the same range
** and starting point as UNIX time.
*/
class Time
{
	public:

		// Classes defined here
		struct Precision;
		struct Ntp;
		class Elapsed;
		class Scale;
                            /*### @struct Time::Precision {{{3 ****************/

		/*!
		** A clock precision.  Used primarily to convert time durations from
		** one timescale to another.
		*/
		struct Precision {
			static const Precision HOUR;
			static const Precision MIN;
			static const Precision SEC;
			static const Precision MSEC;
			static const Precision USEC;
			static const Precision NSEC;
			static const Precision STD;
			static const Precision NTP;
			static const Precision NTSC;
			static const Precision PAL;

			/*! Tics per usec for our standard precision. */
			static const int64_t TIME_TPS;

			/*! Number of (10us) jiffies in a period. */
			int64_t mJiffiesPerPeriod;

			/*! Number of tics in a period. */
			int64_t mTicsPerPeriod;

			/*! Jiffies per usec. */
			static const int64_t JIFFIES_PER_USEC;

			/*! Default ctor. */
			Precision(): mJiffiesPerPeriod(1), mTicsPerPeriod(1) {}

			/*! Standard ctor. */
			Precision(int64_t ticsPerPeriod, int64_t usecsPerPeriod = 1) {
				assign(ticsPerPeriod, usecsPerPeriod);
			}

			/*! Copy ctor. */
			Precision(const Precision& rhs): mJiffiesPerPeriod(rhs.mJiffiesPerPeriod), mTicsPerPeriod(rhs.mTicsPerPeriod) {}

			/*! Dtor. */
			~Precision() {}

			/*! Assignment. */
			Precision& assign(
				int64_t ticsPerPeriod,
				int64_t usecsPerPeriod = 1);

			Precision& operator=(const Precision& rhs);

			/*! Comparison. */
			bool operator<(const Precision& rhs) const {
				return ( (mTicsPerPeriod * rhs.mJiffiesPerPeriod) < (rhs.mTicsPerPeriod * mJiffiesPerPeriod) );
			}

			bool operator<=(const Precision& rhs) const {
				return ( (mTicsPerPeriod * rhs.mJiffiesPerPeriod) <= (rhs.mTicsPerPeriod * mJiffiesPerPeriod) );
			}

			bool operator==(const Precision& rhs) const {
				return ( (mTicsPerPeriod * rhs.mJiffiesPerPeriod) == (rhs.mTicsPerPeriod * mJiffiesPerPeriod) );
			}

			bool operator!=(const Precision& rhs) const {
				return ( (mTicsPerPeriod * rhs.mJiffiesPerPeriod) != (rhs.mTicsPerPeriod * mJiffiesPerPeriod) );
			}

			bool operator>=(const Precision& rhs) const {
				return ( (mTicsPerPeriod * rhs.mJiffiesPerPeriod) >= (rhs.mTicsPerPeriod * mJiffiesPerPeriod) );
			}

			bool operator>(const Precision& rhs) const {
				return ( (mTicsPerPeriod * rhs.mJiffiesPerPeriod) > (rhs.mTicsPerPeriod * mJiffiesPerPeriod) );
			}

			/*! Reduce these numbers by common factors until the smallest is <= maxVal */
			static void factor(
				int64_t& num1,
				int64_t& num2,
				int64_t maxVal);
		};
                            /*### @struct Time::Precision }}}3 ****************/
                            /*### @struct Time::Ntp {{{3 **********************/

		/*!
		** 64-bit NTP time representation
		*/
		struct Ntp {
			uint64_t mTics;
		};
                            /*### @struct Time::Ntp }}}3 **********************/
                            /*### @class Time::Elapsed {{{3 *******************/

		/*!
		** An elapsed period of time.  Can be positive or negative.
		*/
		class Elapsed
		{
			public:

				/*! Construct from a given number of tics. */
				explicit Elapsed(int64_t tics = 0): mTics(tics) {}

				/*! Copy ctor. */
				Elapsed(const Elapsed& rhs): mTics(rhs.mTics) {}

				/*! Dtor. */
				~Elapsed() {}

				/*! Assignment */
				Elapsed& assign(int64_t tics) {
					mTics = tics;
					return *this;
				}

				Elapsed& clear() {
					return assign(0);
				}

				Elapsed& operator=(const Elapsed& rhs) {
					mTics = rhs.mTics;
					return *this;
				}

				Elapsed& operator+=(const Elapsed& rhs) {
					mTics += rhs.mTics;
					return *this;
				}

				Elapsed& operator-=(const Elapsed& rhs) {
					mTics -= rhs.mTics;
					return *this;
				}

				/*! Binary ops */
				Elapsed operator*(int32_t rhs) const {
					return Elapsed(mTics * rhs);
				}

				/*! Binary ops */
				Elapsed operator*(double rhs) const {
					return Elapsed((int64_t)(mTics * rhs));
				}

				Elapsed operator+(const Elapsed& rhs) const {
					return Elapsed(mTics + rhs.mTics);
				}

				Elapsed operator-(const Elapsed& rhs) const {
					return Elapsed(mTics - rhs.mTics);
				}

				Elapsed operator/(int32_t rhs) const {
					return Elapsed(mTics / rhs);
				}

				Elapsed operator/(double rhs) const {
					return Elapsed((int64_t)(mTics / rhs));
				}

				/*! Comparison. */
				bool operator<(const Elapsed& rhs) const {
					return ( mTics < rhs.mTics );
				}

				bool operator<=(const Elapsed& rhs) const {
					return ( mTics <= rhs.mTics );
				}

				bool operator==(const Elapsed& rhs) const {
					return ( mTics == rhs.mTics );
				}

				bool operator!=(const Elapsed& rhs) const {
					return ( mTics != rhs.mTics );
				}

				bool operator>=(const Elapsed& rhs) const {
					return ( mTics >= rhs.mTics );
				}

				bool operator>(const Elapsed& rhs) const {
					return ( mTics > rhs.mTics );
				}

				/*! Return our tics */
				int64_t getTics() const {
					return mTics;
				}

			private:

				/*! How many tics of standard precision in this period. */
				int64_t mTics;
		};
                            /*### @class Time::Elapsed }}}3 *******************/
                            /*### @class Time::Scale {{{3 *********************/

		/*!
		** Used to convert values to/from standard precision.
		*/
		class Scale
		{
			public:

				/*! Empty ctor */
				Scale(): mBitsB(0), mBitsT(0), mBottom(0), mTop(0) {}

				/*! Set precision. */
				Scale(const Precision& p) {
					assign(p);
				}

				/*! Copy ctor. */
				Scale(const Scale& rhs): mBitsB(rhs.mBitsB), mBitsT(rhs.mBitsT), mBottom(rhs.mBottom), mPrecision(rhs.mPrecision), mTop(rhs.mTop) {}

				/*! Dtor */
				~Scale() {}

				/*! Set precision. */
				Scale& assign(const Precision& p);

				/*! How many bits needed for this num? */
				static int bits(int64_t num);

				/*! Convert given value to alternate precision. */
				int64_t fromElapsed(const Time::Elapsed& elapsed) const;

				/*! Convert given value to standard precision. */
				Time::Elapsed toElapsed(int64_t tics) const;

			private:

				/*! Number of bits needed for denominator. */
				int mBitsB;

				/*! Number of bits needed for numerator. */
				int mBitsT;

				/*! Bottom when converting to standard precision. */
				int64_t mBottom;

				/*! The precision we are scaling to/from */
				Precision mPrecision;

				/*! Top when converting to standard precision. */
				int64_t mTop;
		};
                            /*### @class Time::Scale }}}3 *********************/
		static const Scale HOUR;
		static const Scale MIN;
		static const Scale SEC;
		static const Scale MSEC;
		static const Scale USEC;
		static const Scale NSEC;
		static const Scale NTP;

		/*! Default ctor. */
		Time(): mTics(0) {}

		/*! Copy ctor. */
		Time(const Time& rhs): mTics(rhs.mTics) {}

		/*! From 64-bit NTP time. */
		Time(const Ntp& ntp) {
			assign(ntp);
		}

		/*! From secs and usecs. */
		Time(time_t secs, uint32_t usecs) {
			assign(secs, usecs);
		}

		/*! From unix time. */
		explicit Time(time_t secs) {
			assign(secs);
		}

		/*! From timeval. */
		Time(const timeval& tv) {
			assign(tv);
		}

		/*! From timespec. */
		Time(const timespec& ts) {
			assign(ts);
		}

		/*! Dtor. */
		~Time() {}

		/*! Assignment */
		Time& assign(const Time& rhs) {
			mTics = rhs.mTics;
			return *this;
		}

		Time& assign(const Ntp& ntp);

		Time& assign(time_t secs, uint32_t usecs) {
			mTics = (secs * Precision::TIME_TPS) + (usecs * (Precision::TIME_TPS / 1000000));
			return *this;
		}

		Time& assign(time_t secs) {
			mTics = (secs * Precision::TIME_TPS);
			return *this;
		}

		Time& assign(const timeval& tv) {
			mTics = (tv.tv_sec * Precision::TIME_TPS) + (tv.tv_usec * (Precision::TIME_TPS / 1000000));
			return *this;
		}

		Time& assign(const timespec& ts) {
			mTics = (ts.tv_sec * Precision::TIME_TPS) + (ts.tv_nsec * (Precision::TIME_TPS / 1000000000));
			return *this;
		}

		Time& operator=(const Time& rhs) {
			return assign(rhs);
		}

		Time& operator+=(const Elapsed& e) {
			mTics += e.getTics();
			return *this;
		}

		Time& operator-=(const Elapsed& e) {
			mTics -= e.getTics();
			return *this;
		}

		/*! Various const operators */
		Time operator+(const Elapsed& e) const {
			return (Time(*this) += e);
		}

		Time operator-(const Elapsed& e) const {
			return (Time(*this) -= e);
		}

		Elapsed operator-(const Time& rhs) const {
			return Elapsed(mTics - rhs.mTics);
		}

		bool operator<(const Time& rhs) const {
			return ( mTics < rhs.mTics );
		}

		bool operator<=(const Time& rhs) const {
			return ( mTics <= rhs.mTics );
		}

		bool operator==(const Time& rhs) const {
			return ( mTics == rhs.mTics );
		}

		bool operator!=(const Time& rhs) const {
			return ( mTics != rhs.mTics );
		}

		bool operator>=(const Time& rhs) const {
			return ( mTics >= rhs.mTics );
		}

		bool operator>(const Time& rhs) const {
			return ( mTics > rhs.mTics );
		}

		/*! Conversion to alternate forms */
		operator Ntp() const;

		operator time_t() const {
			return static_cast<time_t>(SEC.fromElapsed(Elapsed(mTics)));
		}

		operator timeval() const {
			timeval tv;
			tv.tv_sec = *this;
			tv.tv_usec = USEC.fromElapsed(Elapsed(mTics % Precision::TIME_TPS));
			return tv;
		}

		operator timespec() const {
			timespec ts;
			ts.tv_sec = *this;
			ts.tv_nsec = NSEC.fromElapsed(Elapsed(mTics % Precision::TIME_TPS));
			return ts;
		}

		/*! Return current time */
		static Time now() {
			timeval tv;
			gettimeofday(&tv, NULL);
			return Time(tv);
		}

		/*! Return tics */
		int64_t getTics() const { return mTics; };

	private:

		/*! Difference between NTP epoch and UNIX epoch in secs. */
		static int64_t NTP_OFFSET;

		/*! How many tics of standard precision since the Unix epoch. */
		int64_t mTics;
};
                            /*### @class Time }}}2 ****************************/
                            /*### @class MonoTime {{{2 ************************/

/*!
** Monotonic high-resolution elapsed time source.  Cannot be mapped to Time,
** but durations are measured in elapsed time.
*/
class MonoTime
{
	public:

		/*! Our initialization state */
		enum State {
			eNeedsInit,
			eInitializing,
			eReady
		};

		/*! Default ctor. */
		MonoTime(uint64_t tics = 0): mTics(tics) {}

		/*! Copy ctor. */
		MonoTime(const MonoTime& rhs): mTics(rhs.mTics) {}

		/*! Dtor. */
		~MonoTime() {}

		/*!
		** Return raw underlying value (for serializing to disk or wire)
		*/
		uint64_t getTics() const {
			return mTics;
		}

		/*! Assignment */
		MonoTime& assign(const MonoTime& rhs) {
			mTics = rhs.mTics;
			return *this;
		}

		MonoTime& operator=(const MonoTime& rhs) {
			return assign(rhs);
		}

		MonoTime& operator+=(const Time::Elapsed& e) {
			mTics += sScale->fromElapsed(e);
			return *this;
		}

		MonoTime& operator-=(const Time::Elapsed& e) {
			mTics -= sScale->fromElapsed(e);
			return *this;
		}

		/*! Various const operators */
		MonoTime operator+(const Time::Elapsed& e) const {
			return (MonoTime(*this) += e);
		}

		MonoTime operator-(const Time::Elapsed& e) const {
			return (MonoTime(*this) -= e);
		}

		Time::Elapsed operator-(const MonoTime& rhs) const {
			return sScale->toElapsed(mTics - rhs.mTics);
		}

		bool operator<(const MonoTime& rhs) const {
			return ( mTics < rhs.mTics );
		}

		bool operator<=(const MonoTime& rhs) const {
			return ( mTics <= rhs.mTics );
		}

		bool operator==(const MonoTime& rhs) const {
			return ( mTics == rhs.mTics );
		}

		bool operator!=(const MonoTime& rhs) const {
			return ( mTics != rhs.mTics );
		}

		bool operator>=(const MonoTime& rhs) const {
			return ( mTics >= rhs.mTics );
		}

		bool operator>(const MonoTime& rhs) const {
			return ( mTics > rhs.mTics );
		}

		/*! Initialize our monotonic time facilities. */
		static void init(bool fast = false);

		/*! Return current time. */
		static MonoTime now(bool fast = false);

	private:

		/*! For protection during initialization */
		static Mutex* sMutex;

		/*! For protection during initialization */
		static CondVar* sCondVar;

		/*! For converting between monotonic and normal precisions */
		static Time::Scale* sScale;

		/*! Our initialization state */
		static State sState;

		/*! Number of tics on the clock (no defined base time) */
		uint64_t mTics;

};
                            /*### @class MonoTime }}}2 ************************/
                            /*### @class MonoTimer {{{2 ***********************/

/*!
** Used to track elapsed time.
*/
class MonoTimer
{
	public:

		/*! Initialize parameters */
		MonoTimer() {
			if ( !sInitialized )
				init();
		}

		/*! Dtor */
		~MonoTimer() {}

		/*! Clear the elapsed time */
		void clear() {
			mElapsed.clear();
		}

		/*! Return the elapsed time */
		const Time::Elapsed& elapsed() const {
			return mElapsed;
		}

		/*! Set the start time for the timer */
		void start(const MonoTime& t = MonoTime::now()) {
			mTimeStart = t;
		}

		/*! Set the stop time for the timer */
		void stop(const MonoTime& t = MonoTime::now()) {
			Time::Elapsed e((t - mTimeStart) - sOverhead);
			if ( e < sMinElapsed )
				mElapsed += sMinElapsed;
			else
				mElapsed += e;
		}

		/*! Initialize our static members. */
		static void init();

	private:
		/*! No copying */
		MonoTimer(const MonoTimer&);
		MonoTimer& operator=(const MonoTimer&);

		/*! Total elapsed time on the timer (updated at stop) */
		Time::Elapsed mElapsed;

		/*! When we last started the timer */
		MonoTime mTimeStart;

		/*! Protects initialization state. */
		static Mutex sInitMutex;

		/*! Have we been initialized? */
		static bool sInitialized;

		/*! Minimum value for an elapsed period. */
		static const Time::Elapsed sMinElapsed;

		/*! Overhead involved in time sampling. */
		static Time::Elapsed sOverhead;
};
                            /*### @class MonoTimer }}}2 ***********************/
                            /*### @class Date {{{2 ****************************/

/*!
** Parent classes for representations of calendar date.
*/
class Date
{
	public:

		// Classes defined here
		struct Info;
                            /*### @struct Date::Info {{{3 *********************/

		/*!
		** The broken-out fields for a date (similar to a 'struct tm' with
		** addition of timezone and usecs).
		*/
		struct Info {

			/*! Day of month [1,31]. */
			uint16_t mDay;

			/*! Hour of day [0,23]. */
			uint16_t mHour;

			/*! Minute of hour [0,59]. */
			uint16_t mMinute;

			/*! Month [1,12]. */
			uint16_t mMonth;

			/*! Second of minute [0,60]. */
			uint16_t mSecond;

			/*! Seconds from UTC. */
			int32_t mTimezone;

			/*! Microsecond within second. */
			uint32_t mUsec;

			/*! Year [1970,2038]. */
			uint16_t mYear;

			/*! Default ctor. */
			Info();

			/*! Copy ctor. */
			Info(const Info& rhs);

			/*! Empty dtor. */
			~Info();

			/*! Assign from given sources of information. */
			Info& assign(
				const struct tm& ltm,
				long tz,
				const struct timeval& tv);

			/*! Assignment. */
			Info& operator=(const Info& rhs);

			/*! Increment by an amount of time. */
			Info& operator+=(const Time::Elapsed& amt);
		};
                            /*### @struct Date::Info }}}3 *********************/

		/*! Standard ctor. */
		Date(const Time& t): mTime(t) {}

		/*! Copy ctor. */
		Date(const Date& rhs): mInfo(rhs.mInfo), mTime(rhs.mTime) {}

		/*! Virtual dtor. */
		virtual ~Date();

		/*! Assign from other date. */
		Date& assign(const Date& rhs) {
			if ( &rhs != this ) {
				mTime = rhs.mTime;
				mInfo = rhs.mInfo;
			}
			return *this;
		}

		/*! Assign from time and update our info. */
		Date& assign(const Time& t) {
			mTime = t;
			update();
			return *this;
		}

		/*! Hour of day. */
		uint16_t hour() const {
			return mInfo.mHour;
		}

		/*! Day of month. */
		uint16_t mday() const {
			return mInfo.mDay;
		}

		/*! Minute of hour. */
		uint16_t min() const {
			return mInfo.mMinute;
		}

		/*! Month of year. */
		uint16_t mon() const {
			return mInfo.mMonth;
		}

		/*! Second of minute. */
		uint16_t sec() const {
			return mInfo.mSecond;
		}

		/*! Assignment */
		Date& operator=(const Date& rhs) {
			return assign(rhs);
		}

		/*! Return underlying time for this date. */
		const Time& time() const {
			return mTime;
		}

		/*! Seconds from UTC. */
		int32_t tz() const {
			return mInfo.mTimezone;
		}

		/*! Microseconds within second. */
		int32_t usec() const {
			return mInfo.mUsec;
		}

		/*! Year (4-digit). */
		uint16_t year() const {
			return mInfo.mYear;
		}

	protected:

		/*! Update the date data. */
		virtual void update() = 0;

		/*! Components of our date. */
		Info mInfo;

		/*! The time associated with our date pieces. */
		Time mTime;
};
                            /*### @class Date }}}2 ****************************/
                            /*### @class LocalDate {{{2 ***********************/

/*!
** Date in local timezone.
*/
class LocalDate:
	public Date
{
	public:

		/*! Standard ctor. */
		LocalDate(const Time& t = Time()): Date(t) {
			update();
		}

		/*! Copy ctor. */
		LocalDate(const LocalDate& rhs): Date(rhs) {}

		/*! Virtual dtor. */
		virtual ~LocalDate();

		/*! Return time of next lookup for this date (debug). */
		const Time& timeNextFetch() const {
			return sTimeNextFetch;
		}

	protected:

		/*! Update the date data. */
		virtual void update();

	private:

		/*! Are we currently in the middle of a fetch? */
		static bool sFetching;

		/*! The info we got at our last fetch. */
		static Date::Info sInfo;

		/*! Held when interacting with static members */
		static Mutex* sMutex;

		/*! The time when we did our last fetch. */
		static Time sTimeFetch;

		/*! The point at which we need to do another fetch. */
		static Time sTimeNextFetch;
};
                            /*### @class LocalDate }}}2 ***********************/
                            /*### @class UtcDate {{{2 *************************/

/*!
** Date in UTC timezone.
*/
class UtcDate:
	public Date
{
	public:

		/*! Standard ctor. */
		UtcDate(const Time& t = Time()): Date(t) {
			update();
		}

		/*! Copy ctor. */
		UtcDate(const UtcDate& rhs): Date(rhs) {}

		/*! Virtual dtor. */
		virtual ~UtcDate();

		/*! Return time of next lookup for this date (debug). */
		const Time& timeNextFetch() const {
			return sTimeNextFetch;
		}

	protected:

		/*! Update the date data. */
		virtual void update();

	private:

		/*! Are we currently in the middle of a fetch? */
		static bool sFetching;

		/*! The info we got at our last fetch. */
		static Date::Info sInfo;

		/*! Held when interacting with static members */
		static Mutex* sMutex;

		/*! The time when we did our last fetch. */
		static Time sTimeFetch;

		/*! The point at which we need to do another fetch. */
		static Time sTimeNextFetch;
};
                            /*### @class UtcDate }}}2 *************************/

#endif // !_Time_hxx


/* ==================== editors ====================== */
