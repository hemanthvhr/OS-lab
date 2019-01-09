#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <queue>
#include <string.h>
#include <vector>
#include <time.h>
#include <unordered_map>


#define PAGES 64
#define ESIZE 32

//set compilation flag -std=c++11
/*
Page entry structure <frame_no><valid_bit><ref_bit><mod_bit>
					|----29---|----1-----|----1---|----1----|
*/

/*
*Page Replcament Mode 
1 - FIFO 
2 - Random
3 - LRU
	we use a combination of hashmap and doubly linked list to implement LRU
4 - NRU
5 - Second chance FIFO
*/

using namespace std;

typedef struct node {
	int value;	//the page number
	struct node *front;
	struct node *back;
} node;

int mode;				//page replacement strategy
void *PT;				//The Page Table
unsigned int *x;

queue<int> page_load;	//containing the information about pages loaded into the memory
vector<int> page_list;	//containing the list of pages in the memory
unordered_map< int, node * > mapping;
node *head = nullptr,*tail = nullptr,*X;
vector< vector<int> > Class(4);	//Four classes of pages in NRU based on ref bit and modify bit

/*Usage Info*/
int page_in = 0,page_out = 0,map = 0,unmap = 0,mem_access = 0;
int page_faults = 0,PC = 1;

void swaper(vector<int>::iterator x,vector<int>::iterator y) {
	int z = *x;
	*x = *y;
	*y = z;
}

void moveto(int page,int initial,int final) {	//for moving the page from class-initial to class-final
	if(initial==final) return;
	int i,x = Class[initial].size();
	for(i=0;i<x;i++) {
		if(Class[initial][i]==page) break;
	}
	Class[initial][i] = Class[initial].back();
	Class[initial].back() = page;
	Class[initial].pop_back();
	Class[final].push_back(page);
}

void usedit(int page) {	//using the page after loading into the memory in LRU policy
	if(head!=tail) {		//put that node on the top of list
		node *temp = mapping[page];
		if(temp==head) return;	//the node corresponding to this page is already on the top
		if(tail->front==head && temp==tail) {	//i.e they have only two nodes and temp == tail
			tail = head;
			head = temp;
			tail->front = head;
			tail->back = nullptr;
			head->front = nullptr;
			head->back = tail;
			return;
		}
		(temp->front)->back = temp->back;
		if(temp->back!=nullptr) {
			(temp->back)->front = temp->front;
		} else {		//if it is the tail ptr then update it
			tail = temp->front;
		}
		temp->front = nullptr;
		temp->back = head;
		head->front = temp;
		head = temp;
	}
}

void reset_ref() {	//resetting the ref bit
	for(int i=0;i<PAGES;i++) {
		unsigned int *entry = x + i;
		*entry = *entry & 0xfffd; 	//resetting the ref bit
	}
	if(mode==4) {	//if using NRU then shift from class 3 & 4 lists to 1 & 2 respectively
		int x = Class[2].size();
		for(int i=0;i<x;i++) Class[0].push_back(Class[2][i]);
		x = Class[3].size();
		for(int i=0;i<x;i++) Class[1].push_back(Class[3][i]);
		if(Class[2].size()) Class[2].clear();
		if(Class[3].size()) Class[3].clear();
	}
}

int get_victim() {
	int c;
	switch(mode) {
		case 1: {		//FIFO Replacement policy
			if(page_load.empty()) return -1;	//queue is empty
			c = page_load.front();
			page_load.pop();
			return c;
		}
		case 2:	{		//Random Replacement Policy
			int n = page_list.size();
			if(n==0) return -1;
			int i = rand()%n;
			c = page_list[i];
			swaper(page_list.begin()+i,page_list.end()-1);	//then erase that element from the vector
			page_list.pop_back();
			return c;
		}
		case 3:			//Least Recently used replacement policy
			node *temp;
			if(head==nullptr) return -1;
			if(head==tail) {
				head->front = nullptr;
				head->back = nullptr;
				c = head->value;
				head = nullptr;
				tail = nullptr;
			} else {	//Give the page no on the bottom of linked list
				temp = tail->front;
				tail->front = nullptr;
				tail->back = nullptr;
				c = tail->value;
				temp->back = nullptr;
				tail = temp;
			}
			return c;
		case 4:	{		//Not Recently Used replacement policy
				int i;
				for(i=0;i<4;i++) if(!Class[i].empty()) break;
				if(i==4) return -1;	//when all of the lists are empty
				int j = rand()%(Class[i].size());
				c = Class[i][j];
				swaper(Class[i].begin()+j,Class[i].end()-1);	//then erase that element from the vector
				Class[i].pop_back();
			}
			return c;
		case 5: {		//Second chance Replacement policy
			if(page_load.empty()) return -1;	//queue is empty
			while(true) {
				c = page_load.front();
				page_load.pop();
				unsigned int *entry = x + c;
				if(*entry%2 == 1) {	//then give it a second chance
					*entry = *entry ^ 1;	//setting the ref bit to 0
					page_load.push(c); 
				} else {
					break;
				}
			}
			return c;
		}
		default:
			return -2;			//Mode error
	}
}

void add_page(int page,int classno) {
	switch(mode) {
		case 1:	//FIFO
		case 5:	//Second Chance
			page_load.push(page);
			break;
		case 2:	//Random 
			page_list.push_back(page);
			break;
		case 3:	//LRU
			node *temp;
			if(mapping.find(page)==mapping.end()){
				temp = new node;
				temp->value = page;
				temp->front = nullptr;
				temp->back = nullptr;
				if(head==nullptr) {
					head = temp;
					tail = head;
				} else {
					head->front = temp;
					temp->back = head;
					head = temp; 
				}
				mapping[page] = head;
			} else {
				temp = mapping[page];
				if(head==nullptr) {
					head = temp;
					tail = temp;
				}
				if(temp==head) break;
				head->front = temp;
				temp->back = head;
				temp->front = nullptr;
				head = temp; 
			}
			break;
		case 4:	//NRU
			Class[classno].push_back(page);
			break;
		default:
			return;		
	}
}

void pagetable_print(void *PT) {
	unsigned int *entry;
	cout << "Page no |Frame no | valid | ref | modify\n";
	for(int i=0;i<PAGES;i++) {
		entry = x + i;
		printf("%2d   :   ",i);
		printf("%d 	      %d       %d        %d\n",*entry/8,(*entry & 4)/4,(*entry & 2)/2,(*entry)%2);
	}
}

void print_usage() {	
	cout << "\n------------------ Usage Info -----------------------------\n";
	cout << "Total instructions = " << PC-1 << "\n";
	cout << "Page Faults = " << page_faults << "\n";
	cout << "Total Page transfer operations = " << page_in + page_out << "\n";
	cout << "Total Execution cycles = " << ((page_in+page_out)*3000 + (map+unmap)*250 + mem_access) << "\n";
}

void simulator(void *PT,int *count,int m) {
	char c;
	int page,modify;
	unsigned int frame;
	unsigned int *entry;
	cout << "Output Generated\n";
	while(true) {
		c = getchar();
		if(c=='\n' || c==EOF) break;
		getchar();
		scanf("%d",&page);
		getchar();
		if(c=='#') {
			while(getchar()!='\n');
			continue;
		} else {
			/*for debugging
			X = head;
			while(X!=nullptr) {
				cout << X->value << " , ";
				X = X->back;
			}
			cout << "\n";
			if(tail && tail->front==nullptr) cout << "OMG !\n";*/
			entry = x + page;
			modify = (*entry)%2;	//initial mod bit of that entry
			if(c=='1') modify = 1;
			if((*entry & 4)==0) {	//valid bit is not set ,i.e a page fault
				page_faults += 1;
				if(page_faults%10 == 9) reset_ref();	//reseting refbit every 10th page fault
				mem_access += 1;		//one failed memory access is counted
				if(*count == m) {	//table is full replace some page
					int prev = get_victim();
					unsigned int *temp = x + prev;
					frame = *temp/8;
					cout << "Line " << PC << " : UNMAP " << prev << " " << frame << "\n";
					unmap += 1;
					if((*temp % 2)==1) {
						cout << "Line " << PC << " : OUT   " << prev << " " << frame << "\n";
						page_out += 1;
					}
					*temp = *temp & (unsigned int)0;
					add_page(page,2+modify);
				} else {	
					add_page(page,2+modify);
				 	frame = (*count)++;
				}
				//map the page to a frame
				cout << "Line " << PC << " : IN    " << page << " " << frame << "\n";
				page_in += 1;
				cout << "Line " << PC << " : MAP   " << page << " " << frame << "\n";
				map += 1;
				*entry = *entry | 4;	//setting the valid bit
				*entry = frame*8 + (*entry)%8;
			} else {
				if(mode==3) usedit(page);	//when using the NRU or LRU policies
				else {
					int prev_class = *entry%4;
					*entry = 4*((*entry)/4) + 2+modify;
					if(mode==4) moveto(page,prev_class,2+modify);
				}
			}
			mem_access += 2;
			if(modify) {	//write operation
				*entry = *entry | 1;
			}
			*entry = *entry | 2;	//setting the ref bit
		}
		PC++;
	}
}

int main(int argc,char **argv) {
	srand(time(0));
	if(argc != 3) {
		cout << "Expected arguments <no_of_frames><page_replacement_mode>\n";
		return 0;
	}
	int m;		//no of frames available in memory
	m = atoi(argv[1]);
	mode = atoi(argv[2]);
	PT = (void *) calloc(PAGES,4);
	x = (unsigned int *)PT;
	int free_count = 0;		//no of frames in the memory that are currently occupied
	simulator(PT,&free_count,m);
	pagetable_print(PT);
	print_usage();
	return 0;
}