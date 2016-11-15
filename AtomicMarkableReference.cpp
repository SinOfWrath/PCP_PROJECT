#include <atomic>
using namespace std;

template <typename T>
class AtomicMarkableReference {

	/* class: */
	template <typename U>
	class ReferenceBooleanPair {
		public:
		U *reference;
		bool mark;
		ReferenceBooleanPair(U* r, bool b) {
			reference = r; mark = b;
		}
	};

	atomic< ReferenceBooleanPair<T>* > atomic_ref;

public:
	AtomicMarkableReference(T *init_ref, bool init_mark) {
		atomic_ref.store(new ReferenceBooleanPair<T>(init_ref, init_mark));	
	}

	T* getReference() {
		return atomic_ref.load()->reference;
	}

	bool isMarked() {
		return atomic_ref.load()->mark;
	}

	T* get(bool *mark_holder) {
		ReferenceBooleanPair<T> *p = atomic_ref.load();
		mark_holder = p->mark;
		return p->reference;
	}

	bool weakCompareAndSet( T* expRef, T* newRef, 
			bool expMark, bool newMark ) {
		ReferenceBooleanPair<T> *curr = atomic_ref.load();
		return expRef == curr->reference && 
			expMark == curr->mark &&
			((newRef == curr->reference && newMark == curr->mark) || 
			 atomic_ref.compare_exchange_weak(curr, new AtomicMarkableReference<T>(newRef, newMark)));
	}

	bool compareAndSet( T* expRef, T* newRef, 
			bool expMark, bool newMark ) {
		ReferenceBooleanPair<T> *curr = atomic_ref.load();
		return expRef == curr->reference && 
			expMark == curr->mark &&
			((newRef == curr->reference && newMark == curr->mark) || 
			 atomic_ref.compare_exchange_strong(curr, new AtomicMarkableReference<T>(newRef, newMark)));
	}

	void set(T* newRef, bool newMark) {
		ReferenceBooleanPair<T> *curr = atomic_ref.load();
		if (newRef != curr->reference || newMark != curr->mark) {
			atomic_ref.store(new ReferenceBooleanPair<T>(newRef, newMark));
		}
	}

	bool attemptMark(T* expRef, bool newMark) {
		ReferenceBooleanPair<T>* curr = atomic_ref.load();
		return expRef == curr->reference && 
			(newMark == curr->mark ||
			 atomic_ref.compare_exchange_strong(curr, new ReferenceBooleanPair<T>(expRef, newMark)));
	}
};
