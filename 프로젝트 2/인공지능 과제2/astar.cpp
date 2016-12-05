
#include <stdio.h>
#include <utility>
#include <algorithm>
#include <queue>
#include <unordered_map>
#include <vector>
#include <time.h>
#include <iostream>

#define print_debug_message false

using namespace std;
#define INF 2e9

#define PUZZLE_SIZE 4
#define SEARCHING_DEPTH 30
#define DIRECTION_SIZE 4
#define WEIGHT 1 

typedef __int64 ULL;
typedef ULL KEY;
typedef int STATE[PUZZLE_SIZE][PUZZLE_SIZE];
typedef int DIST;
STATE START_STATE, END_STATE;

//name must be chage
unordered_map < KEY, DIST > dist_map;
unordered_map < KEY, KEY > path;

int ans = 0;

clock_t begin_time;
clock_t stop_time;

#define NODE_MOVE_VECTOR_SIZE PUZZLE_SIZE*PUZZLE_SIZE
vector<int> NodeMoveVector[NODE_MOVE_VECTOR_SIZE];

vector<ULL> multipliers;

struct NODE {
	KEY key;
	int f;
	int h;
	int zero_idx;
	NODE() {};
	NODE(KEY key) : key(key) { zero_idx = -1; };
	NODE(KEY key, int f, int zero_idx) : key(key), f(f), zero_idx(zero_idx) {};
	NODE(KEY key, int f, int zero_idx, int h) : key(key), f(f), zero_idx(zero_idx), h(h) {};
	NODE(const STATE & state) {
		f = 0;
		key = 0;
		h = 0;
		int mul = PUZZLE_SIZE * PUZZLE_SIZE;
		int idx = PUZZLE_SIZE * PUZZLE_SIZE - 1;
		for (int i = 0; i < PUZZLE_SIZE; i++, idx--) {
			for (int j = 0; j < PUZZLE_SIZE; j++, idx--) {
				key = key*mul + state[i][j];
				if (state[i][j] == 0) zero_idx = idx;
			}
		}

	}

	bool operator() (const NODE &a, const NODE &b) {
		if (a.f != b.f) return a.f > b.f;
		return a.key < b.key;
	};
};

void start_time();
void end_time();

void initMul();
void initNodeMoveVector();

KEY getMovedKey(const KEY &key, const int &old_idx, const int &new_idx);

//this fuction can be optimize by memoization
int computeHuristicValue(const KEY & key, const KEY & endkey);
int Astar();
void getInput();

void print_NODE(const NODE & node);
void printState(const STATE & state);
void printMovement(const KEY & oldKey, const KEY & newKey);
void printResult();

int main() {

	//init 
	dist_map.clear();
	path.clear();
	initMul();
	initNodeMoveVector();
	
	//get input
	getInput();

	//printf start, end node
	NODE start_node(START_STATE);
	NODE end_node(END_STATE);
	printf("start NODE\n");
	print_NODE(start_node);
	printf("end NODE\n");
	print_NODE(end_node);

	//compute astar
	start_time();
	ans = Astar();
	end_time();

	//show result( ans,time, path,...
	printResult();

	return 0;
}


//init function
void initNodeMoveVector() {
#define UP_END		PUZZLE_SIZE - 1
#define DOWN_END	0
#define LEFT_END	PUZZLE_SIZE - 1 
#define RIGHT_END	0
	for (int i = 0; i < NODE_MOVE_VECTOR_SIZE; i++) {
		//up
		if (i / PUZZLE_SIZE != UP_END)
			NodeMoveVector[i].push_back(i + 4);
		//down
		if (i / PUZZLE_SIZE != DOWN_END)
			NodeMoveVector[i].push_back(i - 4);
		//left
		if (i%PUZZLE_SIZE != LEFT_END)
			NodeMoveVector[i].push_back(i + 1);
		//right
		if (i%PUZZLE_SIZE != RIGHT_END)
			NodeMoveVector[i].push_back(i - 1);
	}

#undef UP_END
#undef DOWN_END
#undef LEFT_END
#undef RIGHT_END
}

void initMul() {
	ULL val = 1;
	ULL MUL = PUZZLE_SIZE*PUZZLE_SIZE;
	for (int i = 0; i < PUZZLE_SIZE*PUZZLE_SIZE; i++) {
		multipliers.push_back(val);
		val *= MUL;
	}
}
//end init function

// astar algo 
KEY getMovedKey(const KEY &key, const int &old_idx, const int &new_idx) {
	KEY ret = key;
	int MUL = PUZZLE_SIZE*PUZZLE_SIZE;
	int val = (ret / multipliers[new_idx]) % MUL;

	ret +=  val*multipliers[old_idx] - val*multipliers[new_idx];

	return ret;
}

int computeHuristicValue(const KEY & key, const KEY & endkey) {
	int hval = 0;
	KEY curval = key;
	KEY endval = endkey;
	ULL mul = PUZZLE_SIZE * PUZZLE_SIZE;
	for (int i = 0; i < PUZZLE_SIZE*PUZZLE_SIZE; i++) {
		if ( ((int)(curval%mul) + mul)%mul != ((int)(endval%mul) + mul) % mul)
			hval++;
		curval /= mul;
		endval /= mul;
	}
	return hval;
}

int Astar() {
	DIST dist = INF;
	NODE start_node(START_STATE);
	NODE end_node(END_STATE);
	dist_map[end_node.key] = INF;
	dist_map[start_node.key] = 0;
	path[start_node.key] = start_node.key;
	start_node.h = computeHuristicValue(start_node.key, end_node.key);
	start_node.f = start_node.h;
	if (print_debug_message) {
		printf("start node\n");
		print_NODE(start_node);
		printf("end node\n");
		print_NODE(end_node);
		printf("\n\n");
		printf("in pq \n");
	}

	priority_queue< NODE, vector<NODE>, NODE> pq;
	pq.push(start_node);

	while (!pq.empty()) {
		NODE cur = pq.top();

		if(print_debug_message) print_NODE(cur);

		while (!pq.empty() && pq.top().key == cur.key)
			pq.pop();

		if (dist_map[cur.key] > SEARCHING_DEPTH) continue;
		if (cur.key == end_node.key) {
			dist = dist_map[end_node.key];
			break;
		}

		for (int i = 0; i < NodeMoveVector[cur.zero_idx].size(); i++) {
			int new_idx = NodeMoveVector[cur.zero_idx][i];
			KEY newKey = getMovedKey(cur.key, cur.zero_idx, new_idx);

			if (dist_map.find(newKey) == dist_map.end() || dist_map[newKey] > dist_map[cur.key] + WEIGHT) {
				dist_map[newKey] = dist_map[cur.key] + WEIGHT;
				int h = computeHuristicValue( newKey, end_node.key);
				int f = dist_map[newKey] + h;
				NODE nextNode(newKey, f, new_idx, h);
				pq.push(nextNode);
				path[newKey] = cur.key;
			}
		}
	}

	return dist;
}
// end astar algo 

// input & print function
void getInput() {
	printf("시작노드를 입력하시오(1행, 2행, 3행, 4행 순으로 각 행마다 4 개의 수):\n");
	int size = PUZZLE_SIZE;
	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
			scanf("%d", &START_STATE[i][j]);
		}
	}

	printf("목표노드를 입력하시오:\n");
	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
			scanf("%d", &END_STATE[i][j]);
		}
	}

	printf("\n\n");
}

void print_NODE(const NODE & node) {
	int mul = PUZZLE_SIZE* PUZZLE_SIZE;
	ULL val = node.key;
	printf("key : %I64u  zidx : %d   f : %d  h : %d\n", 
		node.key, node.zero_idx, dist_map[node.key], node.h);
	printf("------------------------------\n");

	vector<int> v;
	for (int i = 0; i < PUZZLE_SIZE * PUZZLE_SIZE; i++) {
		v.push_back(((int)(val%mul) + mul)%mul);
		val = val / mul;
	}
	for (int i = v.size()-1, c = 1; i >= 0; i--, c++) {		
		printf("%3d ", v[i]);
		if (c == PUZZLE_SIZE) {
			c = 0;
			printf("\n");
		}
	}
	printf("------------------------------\n\n");
}

void printState(const STATE & state) {
	int size = PUZZLE_SIZE;
	printf("---------------\n");
	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
			printf("%d ", state[i][j]);
		}
		printf("\n");
	}
	printf("---------------\n");
}

void printMovement(const KEY & oldKey, const KEY & newKey) {

	int mul = PUZZLE_SIZE* PUZZLE_SIZE;
	ULL oldval = oldKey, newval = newKey;
	vector<int> oldv, newv;
	for (int i = 0; i < PUZZLE_SIZE * PUZZLE_SIZE; i++) {
		oldv.push_back(((int)(oldval%mul) + mul) % mul);
		oldval = oldval / mul;
		newv.push_back(((int)(newval%mul) + mul) % mul);
		newval = newval / mul;
	}

	printf("-----------------------------------------------------\n");
	printf("current state        ->     next state\n");

	int oldi = oldv.size()-1, newi = newv.size() - 1;
	for (int iline = 0 ; iline< PUZZLE_SIZE; iline++) {
		for (int i = 0; i < PUZZLE_SIZE; i++, oldi--) {
			printf("%3d ", oldv[oldi]);
		}
		printf("           ");
		for (int i = 0; i < PUZZLE_SIZE; i++,newi--) {
			printf("%3d ", newv[newi]);
		}
		printf("\n");

	}
	printf("-----------------------------------------------------\n\n");

	printf("\n");
}

void printResult() {
	printf("#######################\n");
	printf("end\n");
	printf("#######################\n");
	
	//ans
	printf("number of minimum move : %d\n", ans);

	//time
	printf("total time : %.4f\n", double(stop_time - begin_time)/CLOCKS_PER_SEC );

	//path
	NODE end_node(END_STATE);
	KEY key = end_node.key;
	printf("movement of puzzle\n");
	int count = 1;

	vector<pair<KEY, KEY>> shortPath;
	while (path[key] != key) {
		//printf("[%d] move\n", count++);
		shortPath.push_back( pair<KEY, KEY>(key, path[key]) );
		//printMovement(key, path[key]);
		//NODE node(key);
		//print_NODE(node);
		key = path[key];
	}
	reverse(shortPath.begin(), shortPath.end());

	for (int i = 0; i < shortPath.size(); i++) {
		printf("[%d] move\n", i+1);
		KEY a = shortPath[i].second, b = shortPath[i].first;
		printMovement(a, b);
	}

}
// end input & print function

// compute time
void start_time() {
	begin_time = clock();
}

void end_time() {
	stop_time = clock();
}
// end compute time