#include <atomic>
#include <cstdlib>
#include <limits>
#include <random>
#include "AtomicMarkableReference.cpp"

#include <iostream>
using namespace std;

template <typename T>
class PrioritySkipList {
	public:
	/* node class */
	template <typename U>
	class Node {
	public:
		U key;
		int priority;
		// array of AMR each referring to a Node
		AtomicMarkableReference< Node<U> > *next;
		int top_level;

		Node (U in_key, int in_priority) {
			key = in_key;
			priority = in_priority;
			next = new AtomicMarkableReference< Node<U> >[MAX_LEVEL+1];
			for (int i = 0; i < MAX_LEVEL+1; i++) { 
				next[i].set(0, false);
				// next[i] = AtomicMarkableReference< Node<U> >(0, false); 
			}
			top_level = MAX_LEVEL;
		}	
	};
	/* end of node class */

	static const int MAX_LEVEL = 1;
	Node<T> *head, *tail;

	PrioritySkipList() {
		head = new Node<T>(0, numeric_limits<T>::min());
		tail = new Node<T>(0, numeric_limits<T>::max());
		for (int i = 0; i < MAX_LEVEL+1; i++) { 
			// head->next[i] = new AtomicMarkableReference< Node<T> > (tail, false);
			head->next[i].set(tail, false);
		}
	}

	bool find(Node<T> *search_node, Node<T> **preds, Node<T> **succs) {
		int bottom_level = 0;
		int search_priority = search_node->priority;
		bool *marked;
		marked = new bool;
		*marked = false;

		bool snip = false;
		Node<T> *pred = 0, *curr = 0, *succ = 0;
		retry: 	
		while (true) {
			pred = head;
			// cout << head->next[0]->getReference()->priority << endl;
			for (int level = MAX_LEVEL; level >=bottom_level; level--) {
				curr = pred->next[level].getReference();
				while (true) {
					succ = curr->next[level].get(marked);
					while (*marked) {
						snip = pred->next[level].compareAndSet(curr, succ, false, false);	
						if (!snip) goto retry;
						curr = pred->next[level].getReference();
						succ = curr->next[level].get(marked);
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

	bool add(T t, int p) {
		Node<T> *newNode = new Node<T>(t, p);
		return add(newNode);
	}

	bool add(Node<T> *newNode) {
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> dis(0, MAX_LEVEL);

		int top_level = dis(gen); 
		int bottom_level = 0;
		Node<T> **preds = new Node<T>*[MAX_LEVEL+1];
		Node<T> **succs = new Node<T>*[MAX_LEVEL+1];
		// Node<T> *succs[MAX_LEVEL+1];

		while (true) {
			bool found = find(newNode, preds, succs);
			for (int level = bottom_level; level <= top_level; level++) {
				Node<T> *succ = succs[level];
				newNode->next[level].set(succ, false);
				// cout << succ->priority << endl;
			}
			Node<T> *pred = preds[bottom_level];
			Node<T> *succ = succs[bottom_level];
			newNode->next[bottom_level].set(succ, false);
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
					find(newNode, preds, succs);
				}
			}
			return true;
		}
	}

	bool remove(Node<T>* node_to_remove) {
		int bottom_level = 0;
		Node<T> **preds = new Node<T>*[MAX_LEVEL+1];
		Node<T> **succs = new Node<T>*[MAX_LEVEL+1];
		Node<T>* succ;
		while (true) {
			bool found = find(node_to_remove, preds, succs);
			if (!found) return false;
			for (int level = node_to_remove->top_level; level >= bottom_level+1; level--) {
				bool *marked = new bool; 
				*marked = false;
				succ = node_to_remove->next[level].get(marked);
				while (*marked == false) {
					node_to_remove->next[level].attemptMark(succ, true);
					succ = node_to_remove->next[level].get(marked);
				}
			}
			bool *marked = new bool;
			*marked = false;
			succ = node_to_remove->next[bottom_level].get(marked);
			while (true) {
				bool iMarkedIt = node_to_remove->next[bottom_level].compareAndSet(succ, succ,
						false, true);
				succ = succs[bottom_level]->next[bottom_level].get(marked);
				if (iMarkedIt) {
					find(node_to_remove, preds, succs);
					return true;
				} else if (*marked == true) {
					return false;
				}
			}
		}
	}

	Node<T> findAndMarkMin() {
		Node<T> *curr = 0;
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

	void test() {
		PrioritySkipList<int> psl;
		Node<T> *n1 = new Node<T>(3, 33);
		Node<T> *n2 = new Node<T>(4, 44);
		Node<T> *n3 = new Node<T>(2, 22);
		Node<T> *n4 = new Node<T>(1, 11);
		psl.add(n1);
		psl.add(n2);
		psl.add(n3);
		psl.remove(n3);
		psl.add(n4);
	}
};
