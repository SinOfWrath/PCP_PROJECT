#include<iostream>
#include<atomic>

using namespace std;
#define MIN_VAL -1000
#define MAX_VAL  1000

class Node {
	public:
		int _value;
		int _key;
		int _top_level;

		AtmoicMarkedReference<Node> *_next;

		//Sentinel
		Node(int key){
			_value = -1;//null
			_key = key
			_next = new AtmoicMarkedReference[MAX_LEVEL+1];
			for(int i=0;i<MAX_LEVEL+1;i++){
				_next[i] = AtmoicMarkedReference(null, false);
			}
			_top_level = MAX_LEVEL;
		}

		Node(int x, int height){
			_value = x;
			_key = hashcode(x);
			_next= new AtmoicMarkedReference[height+1];
			for(int i=0;i<height;i++){
				_next[i] = AtmoicMarkedReference(null,false);
			}
			_top_level = height;
		}
};

class LockFreeSkipList {
	public:
		static final int MAX_LEVEL = 100;
		Node head(MIN_VAL);
		Node tail(MAX_VAL);
		LockFreeSkipList(){
			for int
		}
};
