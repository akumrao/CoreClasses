#include "Thread.hxx"



                            /*### @+prefix ************************************/
#include <errno.h>
#include <signal.h>

/*
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
 #include <string.h>
 */
#include "Time.hxx"
                            /*### @-prefix ************************************/
                            /*### @+thread_entry {{{2 *************************/
void* thread_entry(
	void* thr_v)
{
	// Block all sigs so that only main will receive them.
	sigset_t blockedSigs;
	sigfillset(&blockedSigs);
	pthread_sigmask(SIG_SETMASK, &blockedSigs, NULL);

	Thread* t = reinterpret_cast<Thread*>(thr_v);
	t->mNum = ++Thread::sThreadCounter;
	Thread::sThreads.set(t);

	pthread_cleanup_push(thread_cleanup, NULL);
	t->run();
	pthread_cleanup_pop(1);
	t->mRunning = false;

	return NULL;
}
                            /*### @-thread_entry }}}2 *************************/
                            /*### @+thread_cleanup {{{2 ***********************/
void thread_cleanup(
	void* /*ignored*/)
{
	Thread* t = Thread::self();
	if(t == NULL)
		return;
	for ( int32_t i = static_cast<int32_t>(t->mNumCleanupFuncs - 1); i >= 0; --i )
		(*t->mCleanupFuncs)();
	Thread::sThreads.set(NULL);
	t->mRunning = false;
}
                            /*### @-thread_cleanup }}}2 ***********************/
Mutex::Attrs* Mutex::sAttrs = new Mutex::Attrs();
CondVar::Attrs* CondVar::sAttrs = new CondVar::Attrs();
                            /*### @+CondVar::wait {{{2 ************************/
int CondVar::wait(
	int32_t msecs)
{
	if ( !mMutex )
		return EINVAL;

	int rv = 0;

	if ( msecs >= 0 ) {
		timespec ts = Time::now() + Time::MSEC.toElapsed(msecs);

		for ( ;; ) {
			rv = pthread_cond_timedwait(&mData, &mMutex->mData, &ts);
			if ( rv == EINTR || rv == EAGAIN )
				continue;
			else
				break;
		}
	}
	else {
		for ( ;; ) {
			rv = pthread_cond_wait(&mData, &mMutex->mData);
			if ( rv == EINTR || rv == EAGAIN )
				continue;
			else
				break;
		}
	}

	return rv;
}
                            /*### @-CondVar::wait }}}2 ************************/
                            /*### @+Guard::lock {{{2 **************************/
int Guard::lock()
{
	if ( mLocked )
		return 0;

	int rv = mMutex.lock();
	if ( rv == 0 )
		mLocked = true;

	return rv;
}
                            /*### @-Guard::lock }}}2 **************************/
                            /*### @+Guard::unlock {{{2 ************************/
int Guard::unlock()
{
	if ( !mLocked )
		return 0;

	int rv = mMutex.unlock();
	if ( rv == 0 )
		mLocked = false;

	return rv;
}
                            /*### @-Guard::unlock }}}2 ************************/
Mutex* AtomicCounter::sMutex = new Mutex();
Thread::Attrs* Thread::sAttrsDetached = new Thread::Attrs(PTHREAD_CREATE_DETACHED);
Thread::Attrs* Thread::sAttrsJoinable = new Thread::Attrs(PTHREAD_CREATE_JOINABLE);
AtomicCounter Thread::sThreadCounter;
TSD<Thread> Thread::sThreads;
                            /*### @+Thread::~Thread {{{2 **********************/
Thread::~Thread()
{
	cancel(true);
}
                            /*### @-Thread::~Thread }}}2 **********************/
                            /*### @+Thread::num {{{2 **************************/
uint32_t Thread::num()
{
	Thread* t = self();
	if ( t )
		return t->mNum;
	else
		return 0;
}
                            /*### @-Thread::num }}}2 **************************/
                            /*### @+Thread::self {{{2 *************************/
Thread* Thread::self()
{
	return sThreads.get();
}
                            /*### @-Thread::self }}}2 *************************/
                            /*### @+Thread::addCleanup {{{2 *******************/
void Thread::addCleanup(
	Thread::CleanupFunc func)
{
	if ( mNumCleanupFuncs <= THREAD_MAX_CLEANUP_FUNCS )
		mCleanupFuncs[mNumCleanupFuncs++] = func;
}
                            /*### @-Thread::addCleanup }}}2 *******************/
                            /*### @+Thread::cancel {{{2 ***********************/
int Thread::cancel(
	bool doJoin)
{
	int rv = 0;

	if ( mRunning && mId ) {
		if ( (rv = pthread_cancel(mId)) != 0 )
			return rv;
	}

	if ( doJoin )
		return join();
	else
		return 0;
}
                            /*### @-Thread::cancel }}}2 ***********************/
                            /*### @+Thread::join {{{2 *************************/
int Thread::join()
{
	if ( needsJoin() ) {
		void* rp = NULL;
		int rv = pthread_join(mId, &rp);
		mId = 0;
		mNum = 0;
		return rv;
	}
	else
		return 0;
}
                            /*### @-Thread::join }}}2 *************************/
                            /*### @+Thread::start {{{2 ************************/
int Thread::start()
{
	int rv;

	if ( mDetached )
		rv = pthread_create(&mId, &sAttrsDetached->mData, thread_entry, this);
	else
		rv = pthread_create(&mId, &sAttrsJoinable->mData, thread_entry, this);

	return rv;
}
                            /*### @-Thread::start }}}2 ************************/
                            /*### @+ThreadPool::Scheduler::Scheduler {{{2 *****/
ThreadPool::Scheduler::Scheduler()
:
	mAtBat(NULL),
	mEnabled(true),
	mDeckCV(mMutex),
	mDugoutCV(mMutex),
	mOnDeck(NULL)
{
}
                            /*### @-ThreadPool::Scheduler::Scheduler }}}2 *****/
                            /*### @+ThreadPool::Scheduler::~Scheduler {{{2 ****/
ThreadPool::Scheduler::~Scheduler()
{
}
                            /*### @-ThreadPool::Scheduler::~Scheduler }}}2 ****/
                            /*### @+ThreadPool::Scheduler::add {{{2 ***********/
bool ThreadPool::Scheduler::add(
	ThreadPool::Thread* t)
{
//fprintf(stderr, "[%u] >>> ThreadPool::Scheduler::add(%u)\n", pthread_self(), t->id());
	if ( !mEnabled ) {
//fprintf(stderr, "[%u] <<< ThreadPool::Scheduler::add(%u): false\n", pthread_self(), t->id());
		return false;
	}

	Synchronize( mMutex ) {
		for ( ;; ) {
//fprintf(stderr, "[%u] ThreadPool::Scheduler::add(%u): top of loop\n", pthread_self(), t->id());
			if ( !mEnabled )
				return false;
			else if ( !mAtBat ) {
				mAtBat = t;
//fprintf(stderr, "[%u] ThreadPool::Scheduler::add(%u): true\n", pthread_self(), t->id());
				return true;
			}
			else if ( !mOnDeck ) {
//fprintf(stderr, "[%u] ThreadPool::Scheduler::add(%u): on deck\n", pthread_self(), t->id());
				mOnDeck = t;
				mDeckCV.wait();
				mOnDeck = NULL;
			}
			else {
//fprintf(stderr, "[%u] ThreadPool::Scheduler::add(%u): dugout\n", pthread_self(), t->id());
				mDugoutCV.wait();
			}
		}
	}

	/* not reached */
	return false;
}
                            /*### @-ThreadPool::Scheduler::add }}}2 ***********/
                            /*### @+ThreadPool::Scheduler::remove {{{2 ********/
void ThreadPool::Scheduler::remove(
	Thread* t)
{
	if ( mAtBat != t )
		return;

	Synchronize( mMutex ) {
		mAtBat = NULL;
		if ( mOnDeck )
			mDeckCV.signal();
		else
			mDugoutCV.signal();
	}
}
                            /*### @-ThreadPool::Scheduler::remove }}}2 ********/
                            /*### @+ThreadPool::Thread::Thread {{{2 ***********/
ThreadPool::Thread::Thread()
:
	::Thread(false),
	mScheduler(NULL)
{
}
                            /*### @-ThreadPool::Thread::Thread }}}2 ***********/
                            /*### @+ThreadPool::Thread::~Thread {{{2 **********/
ThreadPool::Thread::~Thread()
{
}
                            /*### @-ThreadPool::Thread::~Thread }}}2 **********/
                            /*### @+ThreadPool::Thread::run {{{2 **************/
void ThreadPool::Thread::run()
{
//fprintf(stderr, "[%u] >>> ThreadPool::Thread::run()\n", pthread_self());
	if ( !mScheduler ) {
//fprintf(stderr, "[%u] <<< ThreadPool::Thread::run(): no scheduler\n", pthread_self());
		return;
	}

	threadInit();

	while ( mScheduler->add(this) )
		threadActivate();

	threadCleanup();
//fprintf(stderr, "[%u] <<< ThreadPool::Thread::run(): shutdown\n", pthread_self());
}
                            /*### @-ThreadPool::Thread::run }}}2 **************/
                            /*### @+ThreadPool::Thread::threadCleanup {{{2 ****/
void ThreadPool::Thread::threadCleanup()
{
}
                            /*### @-ThreadPool::Thread::threadCleanup }}}2 ****/
                            /*### @+ThreadPool::Thread::threadInit {{{2 *******/
void ThreadPool::Thread::threadInit()
{
}
                            /*### @-ThreadPool::Thread::threadInit }}}2 *******/
                            /*### @+ThreadPool::ThreadPool {{{2 ***************/
ThreadPool::ThreadPool()
:
	mCapacityThreads(0),
	mNumThreads(0),
	mRunning(false),
	mThreads(NULL)
{
	mCapacityThreads = 16;
	mThreads = static_cast<Thread**>(malloc(mCapacityThreads * sizeof(Thread*)));
}
                            /*### @-ThreadPool::ThreadPool }}}2 ***************/
                            /*### @+ThreadPool::~ThreadPool {{{2 **************/
ThreadPool::~ThreadPool()
{
	stop();

	for ( uint32_t i = 0; i < mNumThreads; ++i )
		delete mThreads[i];

	if ( mThreads )
		delete mThreads;
}
                            /*### @-ThreadPool::~ThreadPool }}}2 **************/
                            /*### @+ThreadPool::add {{{2 **********************/
void ThreadPool::add(
	Thread* t)
{
	if ( mNumThreads == mCapacityThreads ) {
		mCapacityThreads += 16;
		mThreads = static_cast<Thread**>(realloc(mThreads, mCapacityThreads * sizeof(Thread*)));
	}
	mThreads[mNumThreads++] = t;

	t->schedSet(&mScheduler);
	if ( mRunning )
		t->start();
}
                            /*### @-ThreadPool::add }}}2 **********************/
                            /*### @+ThreadPool::start {{{2 ********************/
void ThreadPool::start()
{
//fprintf(stderr, "[%u] >>> ThreadPool::start()\n", pthread_self());
	if ( mRunning ) {
//fprintf(stderr, "[%u] <<< ThreadPool::start(): already running\n", pthread_self());
		return;
	}

	mScheduler.enable();
	for ( uint32_t i = 0; i < mNumThreads; ++i ) {
		mThreads[i]->start();
//fprintf(stderr, "[%u] ThreadPool::start(): started thread %u\n", pthread_self(), mThreads[i]->id());
	}

	mRunning = true;
}
                            /*### @-ThreadPool::start }}}2 ********************/
                            /*### @+ThreadPool::stop {{{2 *********************/
void ThreadPool::stop()
{
	if ( !mRunning )
		return;

	mScheduler.disable();
	for ( uint32_t i = 0; i < mNumThreads; ++i )
		mThreads[i]->join();

	mRunning = false;
}
                            /*### @-ThreadPool::stop }}}2 *********************/


/* ==================== editors ====================== */

