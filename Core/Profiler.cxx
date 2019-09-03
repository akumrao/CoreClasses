

#include "Profiler.hxx"



                            /*### @+prefix ************************************/
#include <stdarg.h>
#include <assert.h>
                            /*### @-prefix ************************************/
                            /*### @+Profiler::Function::dumpElapsed {{{2 ******/
void Profiler::Function::dumpElapsed(
	double usecsTotal) const
{
	double usecs = Time::NSEC.fromElapsed(mTotalElapsed) / 1000.0;

	ProfilerMgr::log("%5.1f%%  %9u  %12.1f  %10.1f  %s\n",
				(100.0 * usecs) / usecsTotal,
				mNumCalls,
				usecs,
				usecs / mNumCalls,
				mName);
}
                            /*### @-Profiler::Function::dumpElapsed }}}2 ******/
                            /*### @+Profiler::CallTree::dumpElapsed {{{2 ******/
void Profiler::CallTree::dumpElapsed(
	double usecsTotal) const
{
	double myUsecsTotal = Time::NSEC.fromElapsed(mTotalElapsed) / 1000.0;
	double myUsecsSelf = Time::NSEC.fromElapsed(mSelfElapsed) / 1000.0;
	double perc = (100.0 * myUsecsTotal) / usecsTotal;
	if ( perc <= 0.1 )
		return;

	ProfilerMgr::log(
			"%5.1f%%  %9u  %12.1f  %10.1f  ",
			perc, mNumCalls, myUsecsTotal, myUsecsTotal / mNumCalls);
	logName();
	ProfilerMgr::log(
			"{{" "{2\n"
			"                   %12.1f  %10.1f    %5.1f%% - [self]\n",
			myUsecsSelf, myUsecsSelf / mNumCalls, (100.0 * myUsecsSelf) / myUsecsTotal);

	for ( CallTree* t = mTrees; t; t = t->mNext ) {
		double usecs = Time::NSEC.fromElapsed(t->mTotalElapsed) / 1000.0;
		ProfilerMgr::log(
			"        %9u  %12.1f  %10.1f    %5.1f%% - ",
			t->mNumCalls,
			usecs,
			usecs / t->mNumCalls,
			(100.0 * usecs) / myUsecsTotal);
		t->logName();
		ProfilerMgr::log("\n");
	}
	ProfilerMgr::log("}}" "}2\n");
}
                            /*### @-Profiler::CallTree::dumpElapsed }}}2 ******/
                            /*### @+Profiler::CallTree::listTrees {{{2 ********/
uint32_t Profiler::CallTree::listTrees(
	Profiler::CallTree** list,
	uint32_t idx)
{
	for ( CallTree* t = mTrees; t; t = t->mNext ) {
		list[idx++] = t;
		idx = t->listTrees(list, idx);
	}

	return idx;
}
                            /*### @-Profiler::CallTree::listTrees }}}2 ********/
                            /*### @+Profiler::CallTree::logName {{{2 **********/
void Profiler::CallTree::logName() const
{
	if ( mFunction ) {
		ProfilerMgr::log("%s", mFunction->getName());
		if ( mFunction->getNumTrees() > 1 )
			ProfilerMgr::log("<%u>", mFunctionCaller);
	}
	else
		ProfilerMgr::log("[TOP]");
}
                            /*### @-Profiler::CallTree::logName }}}2 **********/
                            /*### @+Profiler::CallTree::sortElapsed {{{2 ******/
void Profiler::CallTree::sortElapsed()
{
	CallTree* newTrees = NULL;
	CallTree* t = mTrees;
	CallTree* nt;
	CallTree* ot;
	while ( t ) {
		t->sortElapsed();
		ot = t;
		t = t->mNext;
		if ( !newTrees || ot->mTotalElapsed > newTrees->mTotalElapsed ) {
			ot->mNext = newTrees;
			newTrees = ot;
		}
		else {
			for ( nt = newTrees; nt->mNext && nt->mNext->mTotalElapsed > ot->mTotalElapsed; nt = nt->mNext ) ;
			ot->mNext = nt->mNext;
			nt->mNext = ot;
		}
	}

	mTrees = newTrees;
}
                            /*### @-Profiler::CallTree::sortElapsed }}}2 ******/
                            /*### @+Profiler::CallTree::sumTrees {{{2 *********/
void Profiler::CallTree::sumTrees()
{
	for ( const CallTree* t = mTrees; t; t = t->mNext )
		addTotalTime(t->getTotalElapsed());
}
                            /*### @-Profiler::CallTree::sumTrees }}}2 *********/
                            /*### @+Profiler::CallTree::newTree {{{2 **********/
Profiler::CallTree* Profiler::CallTree::newTree(
	Function* func)
{
	CallTree* t = mProfiler->newTree();
	t->mFunction = func;
	t->mFunctionCaller = t->mFunction->addTree();
	t->mNext = mTrees;
	mTrees = t;
	return t;
}
                            /*### @-Profiler::CallTree::newTree }}}2 **********/
                            /*### @+Profiler::CallTree::operator+= {{{2 *******/
Profiler::CallTree& Profiler::CallTree::operator+=(
	const Profiler::CallTree& rhs)
{
	if ( &rhs == this )
		return *this;

	//
	// Add in the values for the top of the tree
	//
	mNumCalls += rhs.mNumCalls;
	mSelfElapsed += rhs.mSelfElapsed;
	mTotalElapsed += rhs.mTotalElapsed;

	//
	// Now do all the children.
	//
	for ( const CallTree* t = rhs.mTrees; t; t = t->mNext ) {
		// Get the right function pointer so that we can find matches.
		Function* f = mProfiler->getFunc(t->mFunction->getName());

		// Find (or allocate) a subtree for this function
		CallTree* T = getTree(f);

		// Now recursively call ourselves to populate this child.
		*T += *t;
	}

	return *this;
}
                            /*### @-Profiler::CallTree::operator+= }}}2 *******/
                            /*### @+Profiler::Profiler.init {{{2 **************/
Profiler::Profiler()
:
	mCurrentCall(NULL),
	mFuncAllocNum(0),
	mFuncCurrentHeap(mFuncStaticHeap),
	mFuncCurrentNum(0),
	mTreeAllocNum(0),
	mTreeCurrentHeap(mTreeStaticHeap),
	mTreeCurrentNum(0),
	mTreeTop(this)
{
}
                            /*### @-Profiler::Profiler.init }}}2 **************/
                            /*### @+Profiler::~Profiler {{{2 ******************/
Profiler::~Profiler()
{
}
                            /*### @-Profiler::~Profiler }}}2 ******************/
                            /*### @+Profiler::compareFuncsElapsed {{{2 ********/
int Profiler::compareFuncsElapsed(
	const void* lvp,
	const void* rvp)
{
	const Function* const* lp = reinterpret_cast<const Function* const*>(lvp);
	const Function* const* rp = reinterpret_cast<const Function* const*>(rvp);
	if ( (*lp)->getElapsed() > (*rp)->getElapsed() )
		return -1;
	else if ( (*lp)->getElapsed() < (*rp)->getElapsed() )
		return 1;
	else
		return 0;
}
                            /*### @-Profiler::compareFuncsElapsed }}}2 ********/
                            /*### @+Profiler::compareTreesElapsed {{{2 ********/
int Profiler::compareTreesElapsed(
	const void* lvp,
	const void* rvp)
{
	const CallTree* const* lp = reinterpret_cast<const CallTree* const*>(lvp);
	const CallTree* const* rp = reinterpret_cast<const CallTree* const*>(rvp);
	if ( (*lp)->getTotalElapsed() > (*rp)->getTotalElapsed() )
		return -1;
	else if ( (*lp)->getTotalElapsed() < (*rp)->getTotalElapsed() )
		return 1;
	else
		return 0;
}
                            /*### @-Profiler::compareTreesElapsed }}}2 ********/
                            /*### @+Profiler::dump {{{2 ***********************/
void Profiler::dump()
{
	//
	// We want to be able to sort trees and functions, so we'll need to create
	// temporary arrays of pointers so we don't disturb the real thing.
	//
	uint32_t numFuncs = mFuncCurrentNum + (mFuncAllocNum * PROF_FUNC_HEAPSIZE);
	Function** funcs = new Function*[numFuncs];
	uint32_t nf = 0;
	for ( uint32_t i = 0; i < PROF_FUNC_HASHSIZE && nf < numFuncs; ++i ) {
		for ( Function* f = mFuncTable[i]; f; f = f->getNext() )
			funcs[nf++] = f;
	}

	uint32_t numTrees = mTreeCurrentNum + (mTreeAllocNum * PROF_TREE_HEAPSIZE);
	CallTree** trees = new CallTree*[numTrees];
	mTreeTop.listTrees(trees, 0);

	double usecsElapsed = Time::NSEC.fromElapsed(mTreeTop.getTotalElapsed()) / 1000.0;

	//
	// Sort and report functions by elapsed time
	//
	qsort(funcs, numFuncs, sizeof(Function*), compareFuncsElapsed);
	ProfilerMgr::log("Functions (elapsed) {{" "{\n");
	ProfilerMgr::log(
			"%6s  %9s  %12s  %10s  %s\n"
			"%6s  %9s  %12s  %10s  %s\n",
			"perc", "#calls", "us(total)", "us(avg)", "name",
			"----", "------", "--------", "------", "----");
	for ( uint32_t i = 0; i < numFuncs; ++i )
		funcs[i]->dumpElapsed(usecsElapsed);
	ProfilerMgr::log("}}" "}\n");

	//
	// Sort and report trees by elapsed time.  We both sort subtrees of each tree as well
	// as the whole list of trees.
	//
	mTreeTop.sortElapsed();
	qsort(trees, numTrees, sizeof(CallTree*), compareTreesElapsed);
	ProfilerMgr::log("Trees (elapsed) {{" "{\n");
	ProfilerMgr::log(
			"%6s  %9s  %12s  %10s  %s\n"
			"%6s  %9s  %12s  %10s  %s\n",
			"perc", "#calls", "us(total)", "us(avg)", "name",
			"----", "------", "--------", "------", "----");
	mTreeTop.dumpElapsed(usecsElapsed);
	for ( uint32_t i = 0; i < numTrees; ++i )
		trees[i]->dumpElapsed(usecsElapsed);
	ProfilerMgr::log("}}" "}\n");
}
                            /*### @-Profiler::dump }}}2 ***********************/
                            /*### @+Profiler::finish {{{2 *********************/
void Profiler::finish()
{
	mTreeTop.addCall();
	mTreeTop.sumTrees();
}
                            /*### @-Profiler::finish }}}2 *********************/
                            /*### @+Profiler::getFunc {{{2 ********************/
Profiler::Function* Profiler::getFunc(
	const char* name)
{
	if ( name[0] == '.' )
		return NULL;

	uint32_t id = hash(name);
	Function* f = mFuncTable[id];
	Function* nf;
	if ( f ) {
		if ( (nf = f->findFunc(name)) )
			return nf;
	}

	if ( mFuncCurrentNum == PROF_FUNC_HEAPSIZE ) {
		if ( mFuncAllocNum == PROF_MAX_FUNCHEAPS )
			abort();
		mFuncCurrentHeap = mFuncAllocHeaps[mFuncAllocNum++] = new Function[PROF_FUNC_HEAPSIZE];
		mFuncCurrentNum = 0;
	}

	nf = &mFuncCurrentHeap[mFuncCurrentNum++];
	nf->setName(name);
	if ( f )
		f->storeFunc(nf);
	else
		mFuncTable[id] = nf;

	return nf;
}
                            /*### @-Profiler::getFunc }}}2 ********************/
                            /*### @+Profiler::newTree {{{2 ********************/
Profiler::CallTree* Profiler::newTree()
{
	if ( mTreeCurrentNum == PROF_TREE_HEAPSIZE ) {
		if ( mTreeAllocNum == PROF_MAX_TREEHEAPS )
			abort();
		mTreeCurrentHeap = mTreeAllocHeaps[mTreeAllocNum++] = new CallTree[PROF_TREE_HEAPSIZE];
		mTreeCurrentNum = 0;
	}

	CallTree* t = &mTreeCurrentHeap[mTreeCurrentNum++];
	t->setProfiler(this);
	return t;
}
                            /*### @-Profiler::newTree }}}2 ********************/
                            /*### @+Profiler::operator+= {{{2 *****************/
Profiler& Profiler::operator+=(
	const Profiler& rhs)
{
	if ( &rhs == this )
		return *this;

	// current heap
	for ( uint32_t i = 0; i < rhs.mFuncCurrentNum; ++i ) {
		const Function* f = &rhs.mFuncCurrentHeap[i];
		Function* F = getFunc(f->getName());
		*F += *f;
	}

	// if we have allocated heaps...
	if ( rhs.mFuncAllocNum ) {
		// do our static heap
		for ( uint32_t i = 0; i < PROF_FUNC_HEAPSIZE; ++i ) {
			const Function* f = &rhs.mFuncStaticHeap[i];
			Function* F = getFunc(f->getName());
			*F += *f;
		}

		// do all but last allocated heap (which was, by definition, the current one).
		for ( uint32_t i = 0; i < rhs.mFuncAllocNum - 1; ++i ) {
			for ( uint32_t j = 0; j < PROF_FUNC_HEAPSIZE; ++j ) {
				const Function* f = &((rhs.mFuncAllocHeaps[i])[j]);
				Function* F = getFunc(f->getName());
				*F += *f;
			}
		}
	}

	// merge the top tree
	mTreeTop += rhs.mTreeTop;
	return *this;
}
                            /*### @-Profiler::operator+= }}}2 *****************/
                            /*### @+Profiler::threadCleanup {{{2 **************/
void Profiler::threadCleanup()
{
	Profiler* p = ProfilerMgr::getProfiler();
	while ( p->mCurrentCall )
		p->mCurrentCall->~ProfiledCall();

	ProfilerMgr::mergeProfiler(p);
}
                            /*### @-Profiler::threadCleanup }}}2 **************/
ProfilerMgr* ProfilerMgr::sMgr = NULL;
                            /*### @+ProfilerMgr::error {{{2 *******************/
void ProfilerMgr::error(
	const char* fmt,
	...)
{
	if ( sMgr && sMgr->mLogFp ) {
		va_list ap;
		va_start(ap, fmt);
		fprintf(sMgr->mLogFp, "*** ");
		vfprintf(sMgr->mLogFp, fmt, ap);
		va_end(ap);
		fflush(sMgr->mLogFp);
	}

	stop();
	exit(1);
}
                            /*### @-ProfilerMgr::error }}}2 *******************/
                            /*### @+ProfilerMgr::merge {{{2 *******************/
void ProfilerMgr::merge(
	Profiler* p)
{
	if ( p == mMainProfiler )
		return;

	p->finish();
	Synchronize( mMutex ) {
		uint32_t pos = 0;
		for ( pos = 0; pos < mNumProfilers && mProfilers[pos] != p; ++pos );
		assert( pos < mNumProfilers );

		*mMainProfiler += *p;
		delete p;
		if ( pos != mNumProfilers - 1 )
			mProfilers[pos] = mProfilers[mNumProfilers - 1];
		mNumProfilers -= 1;

		mProfilerTSD.set(NULL);
	}
}
                            /*### @-ProfilerMgr::merge }}}2 *******************/
                            /*### @+ProfilerMgr::log {{{2 *********************/
void ProfilerMgr::log(
	const char* fmt,
	...)
{
	if ( sMgr && sMgr->mLogFp ) {
		va_list ap;
		va_start(ap, fmt);
		vfprintf(sMgr->mLogFp, fmt, ap);
		va_end(ap);
		fflush(sMgr->mLogFp);
	}
}
                            /*### @-ProfilerMgr::log }}}2 *********************/
                            /*### @+ProfilerMgr::ProfilerMgr {{{2 *************/
ProfilerMgr::ProfilerMgr(
	const char* arg0)
:
	mLogFp(NULL),
	mMainProfiler(NULL),
	mNumProfilers(0),
	mStopped(false)
{
	// init our output log
	char fname[256];
	const char* bname;
	LocalDate date(Time::now());

	/* create our output log file */
	if ( (bname = strrchr(arg0, '/')) && bname[1] )
		++bname;
	else
		bname = arg0;

	snprintf(fname, sizeof(fname), "/var/tmp/%s-cxprof.%u", bname, getpid());

	if ( (mLogFp = fopen(fname, "w")) ) {
		fprintf(mLogFp,
				"#\n"
				"# vim:set ts=4 sw=4 noet ft=cxprof foldenable foldmethod=marker:\n"
				"#\n"
				"# Profiling of \"%s\" started %4d-%02d-%02d %02d:%02d:%02d\n"
				"#\n",
				arg0, date.year(), date.mon(), date.mday(), date.hour(), date.min(), date.sec());
		fflush(mLogFp);
	}

	mMainProfiler = mProfilers[mNumProfilers++] = new Profiler();
#if 0
	mProfilerTSD.set(mMainProfiler);
#endif

	MonoTime::init();
}
                            /*### @-ProfilerMgr::ProfilerMgr }}}2 *************/
                            /*### @+ProfilerMgr::~ProfilerMgr {{{2 ************/
ProfilerMgr::~ProfilerMgr()
{
}
                            /*### @-ProfilerMgr::~ProfilerMgr }}}2 ************/
                            /*### @+ProfilerMgr::dump {{{2 ********************/
void ProfilerMgr::dump()
{
	Profiler::threadCleanup();

	// make sure only one gets in here
	Synchronize( mMutex ) {
		if ( mStopped )
			return;
		else
			mStopped = true;
	}

	// check if we have anything else to do
	if ( !mLogFp )
		return;

	sMgr->mMainProfiler->dump();

	// and we're out...
	if ( mLogFp ) {
		fclose(mLogFp);
		mLogFp = NULL;
	}
}
                            /*### @-ProfilerMgr::dump }}}2 ********************/
                            /*### @+ProfilerMgr::newProfiler {{{2 *************/
Profiler* ProfilerMgr::newProfiler()
{
	Profiler* p = new Profiler();

	Synchronize( mMutex ) {
		if ( mNumProfilers == PROF_MAX_THREADS )
			abort();
		mProfilers[mNumProfilers++] = p;
	}

	Thread* t = Thread::self();
	if ( t )
		t->addCleanup(Profiler::threadCleanup);

	mProfilerTSD.set(p);
	return p;
}
                            /*### @-ProfilerMgr::newProfiler }}}2 *************/
                            /*### @+ProfiledCall::ProfiledCall.empty {{{2 *****/
ProfiledCall::ProfiledCall(
	const char* name)
{
	MonoTime now(MonoTime::now());

	if ( !(mProfiler = ProfilerMgr::getProfiler()) )
		return;

	mFunction = mProfiler->getFunc(name);
	if ( (mParent = mProfiler->getCall()) )
		mParent->mTimer.stop(now);

	mProfiler->setCall(this);

	if ( mFunction ) {
		if ( mParent )
			mTree = mParent->mTree->getTree(mFunction);
		else
			mTree = mProfiler->getTree(mFunction);
	}

	mTimer.start();
}
                            /*### @-ProfiledCall::ProfiledCall.empty }}}2 *****/
                            /*### @+ProfiledCall::~ProfiledCall {{{2 **********/
ProfiledCall::~ProfiledCall()
{
	MonoTime now(MonoTime::now());

	if ( !mProfiler )
		return;

	mTimer.stop(now);
	Time::Elapsed e(mTimer.elapsed());

	mTotalElapsed += e;

	if ( mFunction ) {
		mFunction->addCall();
		mFunction->addTime(e);

		mTree->addCall();
		mTree->addSelfTime(e);
		mTree->addTotalTime(mTotalElapsed);
	}

	if ( mParent && mFunction )
		mParent->mTotalElapsed += mTotalElapsed;

	mProfiler->setCall(mParent);
	if ( mParent )
		mParent->mTimer.start();
}
                            /*### @-ProfiledCall::~ProfiledCall }}}2 **********/


/* ==================== editors ====================== */

