#ifndef _Profiler_hxx
#define _Profiler_hxx 1



#include "cif_stdc.h"



                            /*### @+prefix ************************************/
#include "Thread.hxx"
#include "Time.hxx"

#ifdef CXPROFILE_ENABLE
# if CXPROFILE_ENABLE == 3
#  define CXPROF(v, fn) ProfiledCall v(fn)
#  define CXPROF_UTIL(v, fn) ProfiledCall v(fn)
# else
#  if CXPROFILE_ENABLE == 2
#   define CXPROF(v, fn)
#   define CXPROF_UTIL(v, fn) ProfiledCall v(fn)
#  else
#   define CXPROF(v, fn) ProfiledCall v(fn)
#   define CXPROF_UTIL(v, fn)
#  endif
# endif
#else
# define CXPROF(v, fn)
# define CXPROF_UTIL(v, fn)
#endif

#define PROF_FUNC_HASHSIZE	127
#define PROF_FUNC_HEAPSIZE	127	// one memory page per heap
#define PROF_MAX_FUNCHEAPS	16	// ~2K unique function names
#define PROF_MAX_THREADS	64
#define PROF_MAX_TREEHEAPS	256	// ~20K unique trees
#define PROF_TREE_HEAPSIZE	78	// one memory page per heap
                            /*### @-prefix ************************************/

// Classes defined here
class Profiler;
class ProfilerMgr;
class ProfiledCall;
                            /*### @class Profiler {{{2 ************************/

/*!
** Holder of thread-specific data collection.
*/
class Profiler
{
	public:

		// Classes defined here
		class Function;
		class CallTree;
                            /*### @class Profiler::Function {{{3 **************/

		/*!
		** The collection of information associated with a profiled basic block
		*/
		class Function
		{
			public:

				/*! The only constructor (inits fields). */
				Function(): mName(NULL), mNext(NULL), mNumCalls(0), mNumTrees(0) {}

				/*! Empty dtor. */
				~Function() {}

				/*! Add one to our number of calls */
				void addCall() {
					mNumCalls += 1;
				}

				/*! Add to our elapsed time */
				void addTime(const Time::Elapsed& elapsed) {
					mTotalElapsed += elapsed;
				}

				/*! Add one to our number of trees, and return the value before hand */
				uint32_t addTree() {
					return mNumTrees++;
				}

				/*! Dump our elapsed time */
				void dumpElapsed(double usecsTotal) const;

				/*! Return the function in our list with this name */
				Function* findFunc(const char* name) {
					for ( Function* f = this; f; f = f->mNext ) {
						if ( f->mName == name )
							return f;
					}
					return NULL;
				}

				/*! Return elapsed wallclock time */
				const Time::Elapsed& getElapsed() const {
					return mTotalElapsed;
				}

				/*! Return name */
				const char* getName() const {
					return mName;
				}

				/*! Return next function in list */
				Function* getNext() {
					return mNext;
				}

				/*! Return num calls */
				uint32_t getNumCalls() const {
					return mNumCalls;
				}

				/*! Return num trees */
				uint32_t getNumTrees() const {
					return mNumTrees;
				}

				/*! Merge this function's data into ours. */
				Function& operator+=(const Function& rhs) {
					if ( &rhs != this ) {
						mNumCalls += rhs.mNumCalls;
						mTotalElapsed += rhs.mTotalElapsed;
					}
					return *this;
				}

				/*! Set the name for this function. */
				void setName(const char* name) {
					mName = name;
				}

				/*! Add this function into our list. */
				void storeFunc(Function* func) {
					func->mNext = mNext;
					mNext = func;
				}

			private:
				/*! No copying */
				Function(const Function&);
				Function& operator=(const Function&);

				/*! The name of this function. */
				const char* mName;

				/*! The next function in the hash table at this slot */
				Function* mNext;

				/*! How many times has this function been called? */
				uint32_t mNumCalls;

				/*! How many call trees have me at the top? */
				uint32_t mNumTrees;

				/*! Total elapsed wallclock time for this function */
				Time::Elapsed mTotalElapsed;
		};
                            /*### @class Profiler::Function }}}3 **************/
                            /*### @class Profiler::CallTree {{{3 **************/

		/*!
		** The collection of information associated with a particular calling
		** tree, rooted at a single function
		*/
		class CallTree
		{
			public:

				/*! The only constructor (inits fields). */
				CallTree(Profiler* p = NULL): mFunction(NULL), mFunctionCaller(0), mNext(NULL), mNumCalls(0), mProfiler(p), mTrees(NULL) {}

				/*! Empty dtor. */
				~CallTree() {}

				/*! Add one to our number of calls */
				void addCall() {
					mNumCalls += 1;
				}

				/*! Add to our self elapsed time */
				void addSelfTime(const Time::Elapsed& elapsed) {
					mSelfElapsed += elapsed;
				}

				/*! Add to our total elapsed time */
				void addTotalTime(const Time::Elapsed& elapsed) {
					mTotalElapsed += elapsed;
				}

				/*! Dump our elapsed time */
				void dumpElapsed(double usecsTotal) const;

				/*! Return num calls */
				uint32_t getNumCalls() const {
					return mNumCalls;
				}

				/*! Return self elapsed time */
				const Time::Elapsed& getSelfElapsed() const {
					return mSelfElapsed;
				}

				/*! Return total elapsed time */
				const Time::Elapsed& getTotalElapsed() const {
					return mTotalElapsed;
				}

				/*! Return the tree in our list with this function (or allocate one) */
				CallTree* getTree(Function* func) {
					for ( CallTree* t = mTrees; t; t = t->mNext ) {
						if ( t->mFunction == func )
							return t;
					}
					return newTree(func);
				}

				/*! Record pointers to all of our trees (and their trees) and return the index after us */
				uint32_t listTrees(
					CallTree** list,
					uint32_t idx);

				/*! Write our name to the log */
				void logName() const;

				/*! Set our profiler. */
				void setProfiler(Profiler* p) {
					mProfiler = p;
				}

				/*! Recursively sort our children by total elapsed time */
				void sortElapsed();

				/*! Set our times to the sum of our children. */
				void sumTrees();

				/*! Merge this tree's data into ours. */
				CallTree& operator+=(const CallTree& rhs);

			private:
				/*! No copying */
				CallTree(const CallTree&);
				CallTree& operator=(const CallTree&);

				/*! Get a new tree from the profiler and add it to our list */
				CallTree* newTree(Function* func);

				/*! The function at the root of our tree */
				Function* mFunction;

				/*! How many trees are rooted at our func before us? */
				uint32_t mFunctionCaller;

				/*! The sibling in the list of trees below a single tree. */
				CallTree* mNext;

				/*! How many times has this call tree been called? */
				uint32_t mNumCalls;

				/*! Profiler that owns us. */
				Profiler* mProfiler;

				/*! Elapsed wallclock time for just the head of this call tree */
				Time::Elapsed mSelfElapsed;

				/*! Total elapsed wallclock time for this call tree */
				Time::Elapsed mTotalElapsed;

				/*! Linked list of trees called directly from us */
				CallTree* mTrees;
		};
                            /*### @class Profiler::CallTree }}}3 **************/

		/*! Initialize all our data structures. */
		Profiler();

		/*! Free any memory we allocated. */
		~Profiler();

		/*! For qsort */
		static int compareFuncsElapsed(
			const void* lvp,
			const void* rvp);

		/*! For qsort */
		static int compareTreesElapsed(
			const void* lvp,
			const void* rvp);

		/*! Log our data */
		void dump();

		/*! Sum up my top-level trees */
		void finish();

		/*! Return the current call */
		ProfiledCall* getCall() {
			return mCurrentCall;
		}

		/*! Return the function with this name */
		Function* getFunc(const char* name);

		/*! Return a top-level tree */
		CallTree* getTree(Function* func) {
			return mTreeTop.getTree(func);
		}

		/*! Allocate and return a new tree */
		CallTree* newTree();

		/*! Merge the data from this profile into us. */
		Profiler& operator+=(const Profiler& rhs);

		/*! Set the current call */
		void setCall(ProfiledCall* call) {
			mCurrentCall = call;
		}

		/*! Cleanup function invoked when the current thread goes away. */
		static void threadCleanup();

	private:

		/*! Calculate a hash ID for this name */
		uint32_t hash(const char* name) {
			register const uint8_t* p = reinterpret_cast<const uint8_t*>(name);
			register uint32_t u = 0;
			while ( *p )
				u += (u << 5) + *p++;
			return (u % PROF_FUNC_HASHSIZE);
		}

		/*! The currently active call */
		ProfiledCall* mCurrentCall;

		/*! List of heaps we've allocated */
		Function* mFuncAllocHeaps[PROF_MAX_FUNCHEAPS];

		/*! How many heaps we've allocated */
		uint32_t mFuncAllocNum;

		/*! The current function heap */
		Function* mFuncCurrentHeap;

		/*! Which function is the next one to be returned? */
		uint32_t mFuncCurrentNum;

		/*! The statically allocated initial function heap */
		Function mFuncStaticHeap[PROF_FUNC_HEAPSIZE];

		/*! The hash table of functions. */
		Function* mFuncTable[PROF_FUNC_HASHSIZE];

		/*! List of heaps we've allocated */
		CallTree* mTreeAllocHeaps[PROF_MAX_TREEHEAPS];

		/*! How many heaps we've allocated */
		uint32_t mTreeAllocNum;

		/*! The current tree heap */
		CallTree* mTreeCurrentHeap;

		/*! Which tree is the next one to be returned? */
		uint32_t mTreeCurrentNum;

		/*! The statically allocated initial tree heap */
		CallTree mTreeStaticHeap[PROF_TREE_HEAPSIZE];

		/*! The parent of all top-level call trees. */
		CallTree mTreeTop;
};
                            /*### @class Profiler }}}2 ************************/
                            /*### @class ProfilerMgr {{{2 *********************/

/*!
** Manages profiler lifecycle and data reporting/summary.
*/
class ProfilerMgr
{
	public:

		/*! Log this text and dump/exit */
		static void error(
			const char* fmt,
			...) __A_NORETURN__ __A_FORMAT__(printf,1,2);

		/*! Return this thread's profiler. */
		static Profiler* getProfiler() {
			if ( sMgr ) {
				Profiler* p = sMgr->mProfilerTSD.get();
				return ( p ? p : sMgr->newProfiler() );
			}
			else
				return NULL;
		}

		/*! Merge this profiler into the main and nuke/remove it. */
		void merge(Profiler* p);

		/*! Merge this profiler into the main and nuke/remove it. */
		static void mergeProfiler(Profiler* p) {
			if ( sMgr )
				sMgr->merge(p);
		}

		/*! Log this text. */
		static void log(
			const char* fmt,
			...) __A_FORMAT__(printf,1,2);

		/*! Mandatory startup init. */
		static void start(const char* arg0) {
			if ( !sMgr )
				sMgr = new ProfilerMgr(arg0);
		}

		/*! Stop profiling and dump our data. */
		static void stop() {
			if ( sMgr )
				sMgr->dump();
		}

		/*! Dtor. */
		~ProfilerMgr();

	private:

		/*! Only via start() */
		ProfilerMgr(const char* arg0);

		/*! Stop profiling and dump our data. */
		void dump();

		/*! Allocate, register, and return a new profiler. */
		Profiler* newProfiler();

		/*! Handle to output logfile. */
		FILE* mLogFp;

		/*! Profiler for main thread. */
		Profiler* mMainProfiler;

		/*! Protects state. */
		Mutex mMutex;

		/*! Number of profilers. */
		uint32_t mNumProfilers;

		/*! Pointers to all profilers created. */
		Profiler* mProfilers[PROF_MAX_THREADS];

		/*! Access to current thread's profiler. */
		TSD<Profiler> mProfilerTSD;

		/*! Have we stopped profiling? */
		bool mStopped;

		/*! Our singleton. */
		static ProfilerMgr* sMgr;
};
                            /*### @class ProfilerMgr }}}2 *********************/
                            /*### @class ProfiledCall {{{2 ********************/

/*!
** Information associated with a particular call of a function
*/
class ProfiledCall
{
	public:

		/*! Called when block is entered. */
		ProfiledCall(const char* name);

		/*! Called when block is exited. */
		~ProfiledCall();

	private:
		/*! No copying */
		ProfiledCall(const ProfiledCall&);
		ProfiledCall& operator=(const ProfiledCall&);

		/*! The function we are calling */
		Profiler::Function* mFunction;

		/*! Where we were called from. */
		ProfiledCall* mParent;

		/*! The profiler to which we belong. */
		Profiler* mProfiler;

		/*! Tracks our elapsed time */
		MonoTimer mTimer;

		/*! Total wallclock time for this call */
		Time::Elapsed mTotalElapsed;

		/*! The tree to which we belong. */
		Profiler::CallTree* mTree;
};
                            /*### @class ProfiledCall }}}2 ********************/

#endif // !_Profiler_hxx


/* ==================== editors ====================== */

