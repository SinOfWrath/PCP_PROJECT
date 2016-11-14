#include<iostream>
#include<atomic>

using namespace std;

template <class T> class PrioritySkipList {
	public:
		class Node;
		PrioritySkipList(){

		}
		bool add(Node node){

		}
		bool remove(Node node){

		}

		// 1.find the node to be removed.
		// 2.Logically Remove it by setting 'marked'
		//	 field to true.
		// 3. return pointer to 'marked' node.
		Node* findAndMarkMin(){
			Node *curr = NULL;
			curr = head.next[0].getRef();
			while(curr != tail){
				if(!curr.marked.get()){
					if(curr.marked.compareAndSet(false, true))
						return curr;
					else
						curr = curr.next[0].getRef();
				}
			}
			return NULL; // No unmarked Nodes were Found.
		}
};

template <class T> class AtmoicMarkedReference {
	public:
		AtmoicMarkedReference(){

		}
};

template <class T> class PrioritySkipList<T> ::  Node {
	public:
		T _item;
		int _score;
		atomic<bool> _marked;
		AtmoicMarkedReference<Node>* _next;

		//Sentinel Node Constructor
		Node(int myPriority){
			//code
		}

		//Ordinary Node Constructor
		Node(T x, int myPriority){
			// code
		}

};

int main(){
	return 0;
}
