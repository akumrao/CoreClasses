#ifndef _Thread_hxx
#define _Thread_hxx 1


#include "cif_stdc.h"


                            /*### @+prefix ************************************/
#include <inttypes.h>   
#include <pthread.h>
#define Synchronize(_m) for ( Guard _m##GUARD(_m); _m##GUARD.active(); )

#define THREAD_MAX_CLEANUP_FUNCS	4
                            /*### @-prefix ************************************/

// Classes defined here
class Mutex;
class CondVar;
class Guard;
template<class _T> class Future;
class AtomicCounter;
template<class T> class TSD;
class Thread;
class ThreadPool;

/*! Thread main function. */
extern "C" {
	extern void* thread_entry(void* thr_v);
}

/*! Thread cleanup function. */
extern "C" {
	extern void thread_cleanup(void* ignored);
}
                            /*### @class Mutex {{{2 ***************************/

/*!
** Wrapper over pthread_mutex_t.
*/
class Mutex
{
	public:

		// Classes defined here
		struct Attrs;

		/*! Needs direct access to low-level mutex. */
		friend class CondVar;
                            /*### @struct Mutex::Attrs {{{3 *******************/

		/*! Mutex attributes. */
		struct Attrs {

			/*! Underlying data */
			pthread_mutexattr_t mData;

			/*! Init the data. */
			Attrs() {
				pthread_mutexattr_init(&mData);
			}

			/*! Destroy the data. */
			~Attrs() {
				pthread_mutexattr_destroy(&mData);
			}

			/*!
			** Set pshared to thread-level (the default).
			*/
			int processPrivate() {
				return pthread_mutexattr_setpshared(&mData, PTHREAD_PROCESS_PRIVATE);
			}

			/*!
			** Set pshared to process-level.
			*/
			int processShared() {
				return pthread_mutexattr_setpshared(&mData, PTHREAD_PROCESS_SHARED);
			}
		};
                            /*### @struct Mutex::Attrs }}}3 *******************/

		/*!
		** Initialize from default static attrs.
		*/
		Mutex() {
			pthread_mutex_init(&mData, &sAttrs->mData);
		}

		/*! Destroy underlying mutex. */
		~Mutex() {
			pthread_mutex_destroy(&mData);
		}

		/*!
		** Re-initialize with given attributes.
		*/
		void init(const Attrs& attrs) {
			pthread_mutex_init(&mData, &attrs.mData);
		}

		/*! Lock the mutex (blocking) */
		int lock() {
			return pthread_mutex_lock(&mData);
		}

		/*! Lock the mutex (non-blocking). */
		int trylock() {
			return pthread_mutex_trylock(&mData);
		}

		/*! Unlock the mutex. */
		int unlock() {
			return pthread_mutex_unlock(&mData);
		}

	private:
		/*! No copying */
		Mutex(const Mutex&);
		Mutex& operator=(const Mutex&);

		/*! Same attrs used to init all mutexes. */
		static Attrs* sAttrs;

		/*! Underlying mutex. */
		pthread_mutex_t mData;
};
                            /*### @class Mutex }}}2 ***************************/
                            /*### @class CondVar {{{2 *************************/

/*!
** Wrapper over pthread_cond_t.
*/
class CondVar
{
	public:

		// Classes defined here
		struct Attrs;
                            /*### @struct CondVar::Attrs {{{3 *****************/

		/*! Condition variable attributes. */
		struct Attrs {

			/*! Underlying data */
			pthread_condattr_t mData;

			/*! Init the data. */
			Attrs() {
				pthread_condattr_init(&mData);
			}

			/*! Destroy the data. */
			~Attrs() {
				pthread_condattr_destroy(&mData);
			}

			/*!
			** Set pshared to thread-level (the default).
			*/
			int processPrivate() {
				return pthread_condattr_setpshared(&mData, PTHREAD_PROCESS_PRIVATE);
			}

			/*!
			** Set pshared to process-level.
			*/
			int processShared() {
				return pthread_condattr_setpshared(&mData, PTHREAD_PROCESS_SHARED);
			}
		};
                            /*### @struct CondVar::Attrs }}}3 *****************/

		/*!
		** Initialize our underlying CV from static attrs and a mutex.
		*/
		CondVar(Mutex& mutex): mMutex(&mutex) {
			pthread_cond_init(&mData, &sAttrs->mData);
		}

		/*!
		** Initialize attrs, but defer assignment of mutex
		*/
		CondVar(): mMutex(NULL) {
			pthread_cond_init(&mData, &sAttrs->mData);
		}

		/*! Destroy underlying CV. */
		~CondVar() {
			pthread_cond_destroy(&mData);
		}

		/*! Broadcast to all waiters */
		int broadcast() {
			return pthread_cond_broadcast(&mData);
		}

		/*!
		** Re-initialize against given attributes and mutex.
		*/
		void init(Mutex& mutex, const Attrs& attrs) {
			mMutex = &mutex;
			pthread_cond_init(&mData, &attrs.mData);
		}

		/*! Signal a single waiter */
		int signal() {
			return pthread_cond_signal(&mData);
		}

		/*! Wait for the given number of milliseconds (-1 = forever) */
		int wait(int32_t msecs = -1);

	private:
		/*! No copying */
		CondVar(const CondVar&);
		CondVar& operator=(const CondVar&);

		/*! Same attrs used to init all CVs. */
		static Attrs* sAttrs;

		/*! Underlying condition variable. */
		pthread_cond_t mData;

		/*! Our companion mutex. */
		Mutex* mMutex;
};
                            /*### @class CondVar }}}2 *************************/
                            /*### @class Guard {{{2 ***************************/

/*!
** Used for restricting the scope of a mutex lock to a basic block.  Must be
** allocated off the stack.
*/
class Guard
{
	public:

		/*! Assign the mutex we are guarding and lock it. */
		Guard(Mutex& mutex): mActive(true), mLocked(false), mMutex(mutex) {
			lock();
		}

		/*! Unlock the mutex. */
		~Guard() {
			unlock();
		}

		/*! Returns false only one time (see Synchronize() macro). */
		bool active() {
			if ( mActive ) {
				mActive = false;
				return true;
			}
			else
				return false;
		}

		/*! Manually lock the mutex */
		int lock();

		/*! Manually unlock the mutex */
		int unlock();

	private:
		/*! No copying */
		Guard(const Guard&);
		Guard& operator=(const Guard&);

		/*! Are we currently active? */
		bool mActive;

		/*! Is our underlying mutex currently locked by us? */
		bool mLocked;

		/*! The Mutex we're wrapped over */
		Mutex& mMutex;
};
                            /*### @class Guard }}}2 ***************************/
                            /*### @class Future {{{2 **************************/

/*!
** A handle to an object that will be populated asynchronously by a separate
** thread.
*/
template<class _T>
class Future
{
	public:

		/*! Wrap ourselves around the given object. */
		Future(_T& obj): mCondVar(mMutex), mObj(obj), mPopulated(false) {}

		/*! Empty dtor */
		~Future() {}

		/*! Get immediate access to our underlying object */
		_T& getObj() {
			return mObj;
		}

		/*! Return a reference to our populated object. */
		operator _T&() {
			Synchronize( mMutex ) {
				while ( !mPopulated )
					mCondVar.wait();
			}
			return mObj;
		}

		/*! Has our object been populated? */
		bool populated() const {
			return mPopulated;
		}

		/*! Set the populated flag on our object. */
		void setPopulated() {
			Synchronize( mMutex ) {
				mPopulated = true;
				mCondVar.broadcast();
			}
		}

	private:
		/*! No copying */
		Future(const Future&);
		Future& operator=(const Future&);

		/*! Protects access to the object. */
		Mutex mMutex;

		/*! Protects access to the object. */
		CondVar mCondVar;

		/*! The object to which we provide access. */
		_T& mObj;

		/*! Has the object been populated? */
		bool mPopulated;
};
                            /*### @class Future }}}2 **************************/
                            /*### @class AtomicCounter {{{2 *******************/

/*!
** A mutex-protected integer, suitable for use as a reference counter.  Uses
** static mutex, since lock duration is so short.
*/
class AtomicCounter
{
	public:

		/*! Initialize counter. */
		AtomicCounter(): mNum(0) {}

		/*! Dtor */
		~AtomicCounter() {}

		/*! Pre-increment. */
		int operator++() {
			if ( mNum ) {
				Guard g(*sMutex);
				return ++mNum;
			}
			else
				return ++mNum;
		}

		/*! Post-increment. */
		int operator++(int) {
			if ( mNum ) {
				Guard g(*sMutex);
				return mNum++;
			}
			else
				return mNum++;
		}

		/*! Pre-increment. */
		int operator--() {
			Guard g(*sMutex);
			return --mNum;
		}

		/*! Post-increment. */
		int operator--(int) {
			Guard g(*sMutex);
			return mNum--;
		}

		/*! Return current value. */
		operator int() const {
			return mNum;
		}

	private:
		/*! No copying */
		AtomicCounter(const AtomicCounter&);
		AtomicCounter& operator=(const AtomicCounter&);

		/*! Mutex used for all atomic counters. */
		static Mutex* sMutex;

		/*! The counter value. */
		int mNum;
};
                            /*### @class AtomicCounter }}}2 *******************/
                            /*### @class TSD {{{2 *****************************/

/*!
** Thread-specific storage.
*/
template<class T>
class TSD
{
	public:

		/*! Function pointer for optional destructor. */
		typedef void (*Destructor)(void*);

		/*! Initializes storage */
		TSD(Destructor destructor = NULL) {
			pthread_key_create(&mKey, destructor);
		}

		/*! Nukes storage */
		~TSD() {
			pthread_key_delete(mKey);
		}

		/*! Fetch our data. */
		T* get() {
			return static_cast<T*>(pthread_getspecific(mKey));
		}

		/*! Store our data. */
		void set(const T* data) {
			pthread_setspecific(mKey, static_cast<const void*>(data));
		}

	private:
		/*! No copying */
		TSD(const TSD&);
		TSD& operator=(const TSD&);

		/*! Handle to our data. */
		pthread_key_t mKey;
};
                            /*### @class TSD }}}2 *****************************/
                            /*### @class Thread {{{2 **************************/

/*!
** Wrapper over posix thread.  Subclasses provide 'run' method
*/
class Thread
{
	public:

		// Classes defined here
		struct Attrs;

		/*! So they can muck with mId and mName */
		friend void* thread_entry(void*);
		friend void thread_cleanup(void*);

		/*! Function to be invoked at thread termination */
		typedef void(*CleanupFunc)(void);
                            /*### @struct Thread::Attrs {{{3 ******************/

		/*! Thread attributes. */
		struct Attrs {

			/*! Underlying data */
			pthread_attr_t mData;

			/*! Init the data. */
			Attrs(int detachState) {
				pthread_attr_init(&mData);
				pthread_attr_setscope(&mData, PTHREAD_SCOPE_SYSTEM);
				pthread_attr_setdetachstate(&mData, detachState);
				pthread_attr_setstacksize(&mData, 1*1024*1024);
			}

			/*! Destroy the data. */
			~Attrs() {
				pthread_attr_destroy(&mData);
			}
		};
                            /*### @struct Thread::Attrs }}}3 ******************/

		/*! Minimal initialization. */
		Thread(bool detached = false): mDetached(detached), mId(0), mNum(0), mNumCleanupFuncs(0), mRunning(true) {}

		/*! Cancel and join. */
		virtual ~Thread();

		/*! Get the number of the currently running thread. */
		static uint32_t num();

		/*! Get the Thread object for the current thread. */
		static Thread* self();

		/*! Register an additional cleanup function for this thread. */
		void addCleanup(CleanupFunc func);

		/*! Send a cancellation message to this thread (and possibly join). */
		int cancel(bool doJoin = true);

		/*! Return our thread id */
		pthread_t getId() const {
			return mId;
		}

		/*! Return our thread number */
		uint32_t getNum() const {
			return mNum;
		}

		/*! Are we running? */
		bool isRunning() const {
			return mRunning;
		}

		/*! Wait for thread to exit (if not detached). */
		int join();

		/*! Are we waiting on a join? */
		bool needsJoin() const {
			return (!mDetached && mId);
		}

		/*! Start thread (if not running). */
		int start();

	protected:

		/*! Derived classes implement this. */
		virtual void run() = 0;


	private:
		/*! No copying */
		Thread(const Thread&);
		Thread& operator=(const Thread&);

		/*! Attrs we init with if we're detached. */
		static Attrs* sAttrsDetached;

		/*! Attrs we init with if we're not detached. */
		static Attrs* sAttrsJoinable;

		/*! For assigning mNum */
		static AtomicCounter sThreadCounter;

		/*! So that we can always find our own thread. */
		static TSD<Thread> sThreads;

		/*! Set of functions to be invoked at thread termination. */
		CleanupFunc mCleanupFuncs[THREAD_MAX_CLEANUP_FUNCS];

		/*! Is this a detached thread? */
		bool mDetached;

		/*! pthread identifier. */
		pthread_t mId;

		/*! Increments by 1 for each running thread, starting at 1. */
		uint32_t mNum;

		/*! Number of registered cleanup funcs. */
		uint32_t mNumCleanupFuncs;

		/*! Is this thread currently running? */
		bool mRunning;
};
                            /*### @class Thread }}}2 **************************/
                            /*### @class ThreadPool {{{2 **********************/

/*!
** Contains a pool of threads dedicated to servicing some incoming set of
** events.  As an event arrives, a thread is assigned from the pool to service
** it.  When the thread is done servicing the event, it adds itself back into
** the pool.  Event detection and handling is provided by derived classes.
*/
class ThreadPool
{
	public:

		// Classes defined here
		class Scheduler;
		class Thread;
                            /*### @class ThreadPool::Scheduler {{{3 ***********/

		/*!
		** Handles scheduling of event assignments to threads.
		*/
		class Scheduler
		{
			public:

				/*! Default ctor. */
				Scheduler();

				/*! Empty dtor. */
				~Scheduler();

				/*! Add this thread to schedule.  Returns false if we're disabled. */
				bool add(ThreadPool::Thread* t);

				/*! Disable this scheduler. */
				void disable() {
					mEnabled = false;
					mDeckCV.signal();
					mDugoutCV.broadcast();
				}

				/*! Enable this scheduler. */
				void enable() {
					mEnabled = true;
				}

				/*! Remove this thread from the schedule */
				void remove(Thread* t);

			private:
				/*! No copying */
				Scheduler(const Scheduler&);
				Scheduler& operator=(const Scheduler&);

				/*! The thread currently at bat. */
				Thread* mAtBat;

				/*! Is scheduling enabled? */
				bool mEnabled;

				/*! Protects access to scheduling state. */
				Mutex mMutex;

				/*! Condition variable for 'on deck' position. */
				CondVar mDeckCV;

				/*! Condition variable for all threads in the dugout. */
				CondVar mDugoutCV;

				/*! The thread currently on deck. */
				Thread* mOnDeck;
		};
                            /*### @class ThreadPool::Scheduler }}}3 ***********/
                            /*### @class ThreadPool::Thread {{{3 **************/

		/*!
		** A thread in a thread pool.
		*/
		class Thread:
			public ::Thread
		{
			public:

				/*! Default ctor. */
				Thread();

				/*! Virtual dtor. */
				virtual ~Thread();

				/*! Set our scheduler */
				void schedSet(Scheduler* s) {
					mScheduler = s;
				}

			protected:

				/*! Thread's main() */
				virtual void run();

				/*! Remove ourselves from our scheduler */
				void schedRemove() {
					mScheduler->remove(this);
				}

				/*! Go get and process an event. */
				virtual void threadActivate() = 0;

				/*! Cleanup before thread exits. */
				virtual void threadCleanup();

				/*! Init after thread starts. */
				virtual void threadInit();

			private:

				/*! Our scheduler */
				Scheduler* mScheduler;
		};
                            /*### @class ThreadPool::Thread }}}3 **************/

		/*! Default ctor. */
		ThreadPool();

		/*! Stop and nuke our threads. */
		~ThreadPool();

		/*! Add a thread to the pool */
		void add(Thread* t);

		/*! Enable the scheduler and start all the threads in the pool. */
		void start();

		/*! Disable the scheduler and join on all our threads. */
		void stop();

	private:
		/*! No copying */
		ThreadPool(const ThreadPool&);
		ThreadPool& operator=(const ThreadPool&);

		/*! Capacity of the pool */
		uint32_t mCapacityThreads;

		/*! Number of threads in the pool */
		uint32_t mNumThreads;

		/*! Are we currently running? */
		bool mRunning;

		/*! Our scheduler. */
		Scheduler mScheduler;

		/*! The threads in the pool */
		Thread** mThreads;
};
                            /*### @class ThreadPool }}}2 **********************/

#endif // !_Thread_hxx


/* ==================== editors ====================== */

