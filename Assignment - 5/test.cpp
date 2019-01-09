#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <string.h>

using namespace std;

unordered_set< int > non_working;
unordered_map< int,int > mapping;	//working map
queue< int > Q;						//working queue

int main(int argc,char **argv){
	if(argc!=5) {
		cout << "Usage <pages><no_of_instr><working_set_size><probability(%)from_working>\n";
		return 1;
	}
	srand(time(0));
	int pages=64,instr=100,wrkset=10,probnext = 80,probtype = 50;	//default
	int read=1,write=0;
	pages = atoi(argv[1]);
	instr = atoi(argv[2]);
	wrkset = atoi(argv[3]);
	probnext = atoi(argv[4]);
	cout << "1 0\n";
	for(int i=1;i<pages;i++) non_working.insert(i);
	mapping[0] = 1;
	Q.push(0);
	for(int i=0;i<instr-1;i++) {
		if(rand()%100 < probtype) {		//read instruction
			cout << 0;
			read++;
		} else {
			cout << 1;					//write instruction
			write++;
		}
		cout << " ";
		if(rand()%100 < probnext || non_working.size()==0) {		//from the working set
			auto it = mapping.begin();
			advance(it,rand()%(mapping.size()));
			int e = it->first;
			mapping[e]++;
			Q.push(e);
			cout << e;
		} else {						//not from the working set
			auto it = non_working.begin();
			advance(it,rand()%(non_working.size()));
			int e = *it;
			non_working.erase(e);
			mapping[e] = 1;
			Q.push(e);
			cout << e;
		}
		cout << "\n";
		if(i>wrkset) {
			int x = Q.front();
			if(mapping[x]==1) {
				mapping.erase(x);
				non_working.insert(x);
			} else {
				mapping[x]--;
			}
			Q.pop();
		}
		if(i%1000) {		//update the probability of read or write every 500 instr's
			probtype = (1+write)*100;
			probtype /= (2+write+read);
			if(probtype < 20) probtype = 30;	//if probability of reading is too low
			if(probtype > 99) probtype = 70;	//if probability of reading is too high
			write = 0;
			read = 0; 
		}
	}
	return 0;
}