
#include "Time.hxx"

                            /*### @+prefix ************************************/
#include <math.h>

                            /*### @-prefix ************************************/
const Time::Precision Time::Precision::HOUR(1, 3600000000LL);
const Time::Precision Time::Precision::MIN(1, 60000000LL);
const Time::Precision Time::Precision::SEC(1, 1000000);
const Time::Precision Time::Precision::MSEC(1, 1000);
const Time::Precision Time::Precision::USEC(1, 1);
const Time::Precision Time::Precision::NSEC(1000, 1);
const Time::Precision Time::Precision::STD(Precision::TIME_TPS, 1000000);
const Time::Precision Time::Precision::NTP(1LL << 32, 1000000);
const Time::Precision Time::Precision::NTSC(3, 100100);
const Time::Precision Time::Precision::PAL(25, 1000000);
const int64_t Time::Precision::TIME_TPS = 4000000000LL;
const int64_t Time::Precision::JIFFIES_PER_USEC = 10LL;
                            /*### @+Time::Precision::assign {{{2 **************/
Time::Precision& Time::Precision::assign(
	int64_t ticsPerPeriod,
	int64_t usecsPerPeriod)
{
//fprintf(stderr, "\n>>> Time::Precision::assign(tics=%lld, usecs=%lld)\n", ticsPerPeriod, usecsPerPeriod);

	mTicsPerPeriod = (ticsPerPeriod < 1) ? 1 : ticsPerPeriod;
	mJiffiesPerPeriod = (usecsPerPeriod < 1) ? JIFFIES_PER_USEC : (usecsPerPeriod * JIFFIES_PER_USEC);
	factor(mTicsPerPeriod, mJiffiesPerPeriod, 100000);

//fprintf(stderr, "\ttics=%lld, jiffies=%lld\n", mTicsPerPeriod, mJiffiesPerPeriod);
	return *this;
}
                            /*### @-Time::Precision::assign }}}2 **************/
                            /*### @+Time::Precision::operator= {{{2 ***********/
Time::Precision& Time::Precision::operator=(
	const Time::Precision& rhs)
{
	if ( &rhs != this ) {
		mTicsPerPeriod = rhs.mTicsPerPeriod;
		mJiffiesPerPeriod = rhs.mJiffiesPerPeriod;
	}

	return *this;
}
                            /*### @-Time::Precision::operator= }}}2 ***********/
                            /*### @+Time::Precision::factor {{{2 **************/
void Time::Precision::factor(
	int64_t& num1,
	int64_t& num2,
	int64_t maxVal)
{
	if ( num1 == 1 || num2 == 1 )
		return;
	else if ( num1 == num2 ) {
		num1 = num2 = 1;
		return;
	}

	int64_t& smaller = (num1 < num2) ? num1 : num2;
	int64_t& larger = (smaller == num1) ? num2 : num1;

	for ( ;; ) {
		if ( !(larger % smaller) ) {
			larger /= smaller;
			smaller = 1;
			return;
		}

		int64_t dmax = 1 + static_cast<int64_t>(sqrt(1.0 * smaller));
		int64_t d = 2;
		while ( d <= dmax ) {
			if ( ((d != 2) && (d % 2) == 0) || ((d != 3) && (d % 3) == 0) )
				++d;
			else if ( !(num1 % d) && !(num2 % d) ) {
				num1 /= d;
				num2 /= d;
				dmax = 1 + static_cast<int64_t>(sqrt(1.0 * smaller));
			}
			else
				++d;
		}

		if ( smaller > maxVal ) {
			double f = (1.0 * maxVal) / smaller;
			smaller = static_cast<int64_t>(0.5 + f * smaller);
			larger = static_cast<int64_t>(0.5 + f * larger);
		}
		else
			return;
	}
}
                            /*### @-Time::Precision::factor }}}2 **************/
                            /*### @+Time::Scale::assign {{{2 ******************/
Time::Scale& Time::Scale::assign(
	const Precision& p)
{
	mPrecision = p;

	mTop = Precision::STD.mTicsPerPeriod * mPrecision.mJiffiesPerPeriod;
	mBottom = mPrecision.mTicsPerPeriod * Precision::STD.mJiffiesPerPeriod;

//fprintf(stderr, "\n>>> Time::Scale::assign(n=%lld, d=%lld)\n", mTop, mBottom);

	Precision::factor(mTop, mBottom, 10000000);
	mBitsB = bits(mBottom);
	mBitsT = bits(mTop);

//fprintf(stderr, "\tn=%lld, d=%lld, b(n)=%d, b(d)=%d\n", mTop, mBottom, mBitsT, mBitsB);
	return *this;
}
                            /*### @-Time::Scale::assign }}}2 ******************/
                            /*### @+Time::Scale::bits {{{2 ********************/
int Time::Scale::bits(
	int64_t num)
{
	uint64_t mask = 0x7800000000000000ULL;
	uint64_t unum = llabs(num);

	for ( int i = 63; i > 0; i -= 4 ) {
		if ( (unum & mask) )
			return i;
		mask >>= 4;
	}

	return 1;
}
                            /*### @-Time::Scale::bits }}}2 ********************/
                            /*### @+Time::Scale::fromElapsed {{{2 *************/
int64_t Time::Scale::fromElapsed(
	const Time::Elapsed& elapsed) const
{
	int64_t tics = elapsed.getTics();
	if ( tics == 0 )
		return 0;

	uint64_t extraTics = 0;
	int nb;
	if ( tics >= Precision::STD.mTicsPerPeriod && (nb = bits(tics)) + mBitsB > 63 ) {
#if 0
double fs = (Precision::STD.mJiffiesPerPeriod * (tics * 1.0e-6)) / (Precision::STD.mTicsPerPeriod * Precision::JIFFIES_PER_USEC);
fprintf(stderr, ">>> fromElapsed():\n");
fprintf(stderr, "\t%.3f secs: bits(tics=%lld) = %d, bits(%lld) = %d\n", fs, tics, nb, mTop, mBitsT);
#endif
		int64_t jiffies = (tics / Precision::STD.mTicsPerPeriod) * Precision::STD.mJiffiesPerPeriod;
		extraTics = (jiffies / mPrecision.mJiffiesPerPeriod) * mPrecision.mTicsPerPeriod;
		tics %= Precision::STD.mTicsPerPeriod;
//fprintf(stderr, "\tjiffies=%lld, extraTics=%lld, tics=%lld\n", jiffies, extraTics, tics);
	}

	return extraTics + ((tics * mBottom) / mTop);
}
                            /*### @-Time::Scale::fromElapsed }}}2 *************/
                            /*### @+Time::Scale::toElapsed {{{2 ***************/
Time::Elapsed Time::Scale::toElapsed(
	int64_t tics) const
{
	if ( tics == 0 )
		return Elapsed(0);

	uint64_t extraTics = 0;
	int nb;
	if ( tics >= mPrecision.mTicsPerPeriod && (nb = bits(tics)) + mBitsT > 63 ) {
#if 0
double fs = (mPrecision.mJiffiesPerPeriod * (tics * 1.0e-6)) / (mPrecision.mTicsPerPeriod * Precision::JIFFIES_PER_USEC);
fprintf(stderr, ">>> toElapsed():\n");
fprintf(stderr, "\t%.3f secs: bits(tics=%lld) = %d, bits(%lld) = %d\n", fs, tics, nb, mTop, mBitsT);
#endif
		int64_t jiffies = ((tics / mPrecision.mTicsPerPeriod) * mPrecision.mJiffiesPerPeriod);
		extraTics = (jiffies / Precision::STD.mJiffiesPerPeriod) * Precision::STD.mTicsPerPeriod;
		tics %= mPrecision.mTicsPerPeriod;
//fprintf(stderr, "\tjiffies=%lld, extraTics=%lld, tics=%lld\n", jiffies, extraTics, tics);
	}

	return Elapsed(extraTics + ((tics * mTop) / mBottom));
}
                            /*### @-Time::Scale::toElapsed }}}2 ***************/
const Time::Scale Time::HOUR(Time::Precision::HOUR);
const Time::Scale Time::MIN(Time::Precision::MIN);
const Time::Scale Time::SEC(Time::Precision::SEC);
const Time::Scale Time::MSEC(Time::Precision::MSEC);
const Time::Scale Time::USEC(Time::Precision::USEC);
const Time::Scale Time::NSEC(Time::Precision::NSEC);
const Time::Scale Time::NTP(Time::Precision::NTP);
int64_t Time::NTP_OFFSET = 2208988800LL;
                            /*### @+Time::assign.ntp {{{2 *********************/
Time& Time::assign(
	const Time::Ntp& ntp)
{
	mTics = SEC.toElapsed(static_cast<int64_t>((ntp.mTics >> 32) - NTP_OFFSET)).getTics();
	mTics += NTP.toElapsed(ntp.mTics & UINT32_MAX).getTics();
	return *this;
}
                            /*### @-Time::assign.ntp }}}2 *********************/
                            /*### @+Time::operator Time::Ntp {{{2 *************/
Time::operator Time::Ntp() const
{
	Ntp ntp;
	ntp.mTics = (static_cast<uint64_t>(SEC.fromElapsed(Elapsed(mTics))) + NTP_OFFSET) << 32;
	ntp.mTics |= NTP.fromElapsed(Elapsed(mTics % Precision::TIME_TPS));
	return ntp;
}
                            /*### @-Time::operator Time::Ntp }}}2 *************/
Mutex* MonoTime::sMutex = new Mutex();
CondVar* MonoTime::sCondVar = new CondVar(*sMutex);
Time::Scale* MonoTime::sScale = NULL;
MonoTime::State MonoTime::sState = MonoTime::eNeedsInit;
                            /*### @+MonoTime::init {{{2 ***********************/
void MonoTime::init(bool fast)
{
	Mutex& m = *sMutex;
	Synchronize( m ) {
		if ( sState == eReady )
			return;
		else if ( sState == eInitializing ) {
			while ( sState == eInitializing )
				sCondVar->wait();
			return;
		}
		else
			sState = eInitializing;
	}
	sScale = new Time::Scale(Time::NSEC);
	sState = eReady;
	sCondVar->broadcast();
}
                            /*### @-MonoTime::init }}}2 ***********************/
                            /*### @+MonoTime::now {{{2 ************************/
MonoTime MonoTime::now(bool fast)
{
	if ( sState != eReady )
		init(fast);

	timespec ts;
#ifdef CLOCK_MONOTONIC
	clock_gettime(CLOCK_MONOTONIC, &ts);
#else
	clock_gettime(CLOCK_REALTIME, &ts);
#endif
	return MonoTime((1000000000ULL * ts.tv_sec) + ts.tv_nsec);
}
                            /*### @-MonoTime::now }}}2 ************************/
Mutex MonoTimer::sInitMutex;
bool MonoTimer::sInitialized = false;
const Time::Elapsed MonoTimer::sMinElapsed = Time::NSEC.toElapsed(1);
Time::Elapsed MonoTimer::sOverhead;
                            /*### @+MonoTimer::init {{{2 **********************/
void MonoTimer::init()
{
	if ( !sInitialized ) {
		Synchronize( sInitMutex ) {
			if ( !sInitialized ) {
				MonoTime::init();

				MonoTime then(MonoTime::now());
				sOverhead = MonoTime::now() - then;
				sInitialized = true;
			}
		}
	}
}
                            /*### @-MonoTimer::init }}}2 **********************/
                            /*### @+Date::Info::Info.default {{{2 *************/
Date::Info::Info()
:
	mDay(0),
	mHour(0),
	mMinute(0),
	mMonth(0),
	mSecond(0),
	mTimezone(0),
	mUsec(0),
	mYear(0)
{
}
                            /*### @-Date::Info::Info.default }}}2 *************/
                            /*### @+Date::Info::Info.copy {{{2 ****************/
Date::Info::Info(
	const Date::Info& rhs)
:
	mDay(rhs.mDay),
	mHour(rhs.mHour),
	mMinute(rhs.mMinute),
	mMonth(rhs.mMonth),
	mSecond(rhs.mSecond),
	mTimezone(rhs.mTimezone),
	mUsec(rhs.mUsec),
	mYear(rhs.mYear)
{
}
                            /*### @-Date::Info::Info.copy }}}2 ****************/
                            /*### @+Date::Info::~Info {{{2 ********************/
Date::Info::~Info()
{
}
                            /*### @-Date::Info::~Info }}}2 ********************/
                            /*### @+Date::Info::assign {{{2 *******************/
Date::Info& Date::Info::assign(
	const struct tm& ltm,
	long tz,
	const struct timeval& tv)
{
	mYear = ltm.tm_year + 1900;
	mMonth = ltm.tm_mon + 1;
	mDay = ltm.tm_mday;
	mHour = ltm.tm_hour;
	mMinute = ltm.tm_min;
	mSecond = ltm.tm_sec;
	mTimezone = static_cast<int32_t>(tz);
	mUsec = tv.tv_usec;

	return *this;
}
                            /*### @-Date::Info::assign }}}2 *******************/
                            /*### @+Date::Info::operator= {{{2 ****************/
Date::Info& Date::Info::operator=(
	const Date::Info& rhs)
{
	if ( &rhs != this ) {
		mDay = rhs.mDay;
		mHour = rhs.mHour;
		mMinute = rhs.mMinute;
		mMonth = rhs.mMonth;
		mSecond = rhs.mSecond;
		mTimezone = rhs.mTimezone;
		mUsec = rhs.mUsec;
		mYear = rhs.mYear;
	}

	return *this;
}
                            /*### @-Date::Info::operator= }}}2 ****************/
                            /*### @+Date::Info::operator+= {{{2 ***************/
Date::Info& Date::Info::operator+=(
	const Time::Elapsed& amt)
{
	//
	// We know that:
	//    0 <= amt <= 1 hour
	// so the max number of usecs is 3.6B
	//
	uint32_t usecs = static_cast<uint32_t>(Time::USEC.fromElapsed(amt) + mUsec);
	mUsec = usecs % 1000000;
	if ( usecs > 1000000 ) {
		uint32_t secs = mSecond + usecs / 1000000;
		mSecond = secs % 60;
		mMinute += secs / 60;
		// We know the hour can't rollover here
	}

	return *this;
}
                            /*### @-Date::Info::operator+= }}}2 ***************/
                            /*### @+Date::~Date {{{2 **************************/
Date::~Date()
{
}
                            /*### @-Date::~Date }}}2 **************************/
bool LocalDate::sFetching = false;
Date::Info LocalDate::sInfo;
Mutex* LocalDate::sMutex = new Mutex();
Time LocalDate::sTimeFetch;
Time LocalDate::sTimeNextFetch;
                            /*### @+LocalDate::~LocalDate {{{2 ****************/
LocalDate::~LocalDate()
{
}
                            /*### @-LocalDate::~LocalDate }}}2 ****************/
                            /*### @+LocalDate::update {{{2 ********************/
void LocalDate::update()
{
	static const Time::Elapsed slop(Time::SEC.toElapsed(1));

	if ( sFetching || mTime < sTimeFetch || (mTime + slop) >= sTimeNextFetch ) {
		Mutex& m = *sMutex;
		Synchronize( m ) {
			if ( mTime < sTimeFetch || mTime >= sTimeNextFetch ) {
				//
				// This is our "fetch": call the library functions and update our
				// static members.
				//
				sFetching = true;

				extern long timezone;
				extern int daylight;
				struct timeval tv;
				struct tm ltm;

				tv = (sTimeFetch = mTime);
				if ( sInfo.mYear == 0 )
					tzset();
				localtime_r(&tv.tv_sec, &ltm);
				sInfo.assign(ltm, -(timezone - (daylight ? 3600 : 0)), tv);

				int64_t usecsTillNext = 1000000LL * (60 * (60 - ltm.tm_min) - ltm.tm_sec) - tv.tv_usec;
				sTimeNextFetch = sTimeFetch + Time::USEC.toElapsed(usecsTillNext);

				mInfo = sInfo;
				sFetching = false;
				return;
			}
			else {
				mInfo = sInfo;
				mInfo += (mTime - sTimeFetch);
			}
		}
	}
	else {
		mInfo = sInfo;
		mInfo += (mTime - sTimeFetch);
	}
}
                            /*### @-LocalDate::update }}}2 ********************/
bool UtcDate::sFetching = false;
Date::Info UtcDate::sInfo;
Mutex* UtcDate::sMutex = new Mutex();
Time UtcDate::sTimeFetch;
Time UtcDate::sTimeNextFetch;
                            /*### @+UtcDate::~UtcDate {{{2 ********************/
UtcDate::~UtcDate()
{
}
                            /*### @-UtcDate::~UtcDate }}}2 ********************/
                            /*### @+UtcDate::update {{{2 **********************/
void UtcDate::update()
{
	static const Time::Elapsed slop(Time::SEC.toElapsed(1));

	if ( sFetching || mTime < sTimeFetch || (mTime + slop) >= sTimeNextFetch ) {
		Mutex& m = *sMutex;
		Synchronize( m ) {
			if ( mTime < sTimeFetch || mTime >= sTimeNextFetch ) {
				//
				// This is our "fetch": call the library functions and update our
				// static members.
				//
				sFetching = true;

				struct timeval tv;
				struct tm ltm;

				tv = (sTimeFetch = mTime);
				gmtime_r(&tv.tv_sec, &ltm);
				sInfo.assign(ltm, 0, tv);

				int64_t usecsTillNext = 1000000LL * (60 * (60 - ltm.tm_min) - ltm.tm_sec) - tv.tv_usec;
				sTimeNextFetch = sTimeFetch + Time::USEC.toElapsed(usecsTillNext);

				mInfo = sInfo;
				sFetching = false;
				return;
			}
			else {
				mInfo = sInfo;
				mInfo += (mTime - sTimeFetch);
			}
		}
	}
	else {
		mInfo = sInfo;
		mInfo += (mTime - sTimeFetch);
	}
}
                            /*### @-UtcDate::update }}}2 **********************/


/* ==================== editors ====================== */

