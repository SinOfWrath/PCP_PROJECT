#include <atomic>
#include <cstdlib>
#include "AtomicMarkableReference.cpp"

template <typename T>
class PrioritySkipList {
	/* node class */
	template <typename U>
	class Node {
	public:
		U key;
		int priority;
		atomic<bool> marked;
		AtomicMarkableReference< Node<U> > *next;
		int top_level;

		/* either +inf or -inf */
		Node(int sentinel_priority) {
			priority = sentinel_priority;
			next = new AtomicMarkableReference< Node<U> >[MAX_LEVEL+1];
			for (int i = 0; i < MAX_LEVEL+1; i++) {
				next[i] = AtomicMarkableReference< Node<U> >(0, false);
			}
			top_level = MAX_LEVEL;
		}

		Node (int _key, int _priority) {
			key = _key;
			priority = _priority;
			next = new AtomicMarkableReference< Node<U> >[MAX_LEVEL+1];
			for (int i = 0; i < MAX_LEVEL+1; i++) { next[i] = AtomicMarkableReference< Node<U> >(0, false); }
			top_level = MAX_LEVEL;
		}	
	};
	/* end of node class */

	static const int MAX_LEVEL = 4;
	Node<T> *head, *tail;

	bool find(Node<T> *search_node, Node<T> *preds[], Node<T> *succs[]) {
		int bottom_level = 0;
		int search_priority = search_node->priority;

		bool *marked, snip;
		*marked = false;
		Node<T> *pred = 0, *curr = 0, *succ = 0;
		retry: 	
		while (true) {
			pred = head;
			for (int level = MAX_LEVEL; level >=bottom_level; level--) {
				curr = pred->next[level];
				while (true) {
					succ = curr->next[level].get(marked);
					while (*marked) {
						snip = pred->next[level].compareAndSet(curr, succ, false, false);	
						if (!snip) goto retry;
						curr = pred->next[level];
						succ = curr->next[level];
					}
					if (curr->priority <= search_priority) {
						if (curr == search_node) break;
						pred = curr; 
						curr = succ;
					} else {
						break;
					}
				}
				preds[level] = pred;
				succs[level] = curr;
			}
			return (curr == search_node);
		}
	}

	bool add(Node<T> *newNode) {
		int top_level = rand() % MAX_LEVEL;
		int bottom_level = 0;
		Node<T> *preds[MAX_LEVEL+1];
		Node<T> *succs[MAX_LEVEL+1];
		while (true) {
			bool found = find(newNode, preds, succs);
			for (int level = bottom_level; level <= top_level; level++) {
				Node<T> *succ = succs[level];
				newNode->next[level].set(succ, false);
			}
			Node<T> *pred = preds[bottom_level];
			Node<T> *succ = succs[bottom_level];
			if (!pred->next[bottom_level].compareAndSet(succ, newNode, false, false)) {
				continue;
			}
			for (int level = bottom_level+1; level <= top_level; level++) {
				while (true) {
					pred = preds[level];
					succ = succs[level];
					if (pred->next[level].compareAndSet(succ, newNode, false, false)) {
						break;
					}
					find(newNode->priority, preds, succs);
				}
			}
			return true;
		}
	}

	bool remove(Node<T>* remNode) {
		int bottom_level = 0;
		Node<T>* preds[MAX_LEVEL+1];
		Node<T>* succs[MAX_LEVEL+1];
		Node<T>* succ;
		while (true) {
			bool found = find(remNode, preds, succs);
			if (!found) return false;
			Node<T>* node_to_remove = remNode;
			for (int level = node_to_remove->top_level; level >= bottom_level+1; level--) {
				bool *marked; 
				*marked = false;
				succ = node_to_remove->next[level].get(marked);
				while (!*marked) {
					node_to_remove->next[level].compareAndSet(succ, succ, false, true);
					succ = node_to_remove->next[level].get(marked);
				}
			}
			bool *marked;
			*marked = false;
			succ = node_to_remove->next[bottom_level].get(marked);
			while (true) {
				bool iMarkedIt = node_to_remove->next[bottom_level].compareAndSet(succ, succ,
						false, true);
				succ = succs[bottom_level].next[bottom_level].get(marked);
				if (iMarkedIt) {
					find(remNode, preds, succs);
					return true;
				}
				else if (*marked) {
					return false;
				}
			}
		}
	}

	Node<T> findAndMarkMin() {
		Node<T> *curr = 0, *succ = 0;
		curr = head->next[0].get(new bool);
		while (curr != tail) {
			if (!curr->marked) {
				if (curr->marked.compare_exchange_weak(false, true)) {
					return curr;
				} else {
					curr = curr->next[0];
				}
			}
		}
		return 0;
	}
};
