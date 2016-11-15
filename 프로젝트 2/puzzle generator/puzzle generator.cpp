
#include <stdio.h>
#include <utility>
#include <algorithm>
#include <queue>
#include <unordered_map>
#include <vector>
#include <time.h>
#include <random>
#include <iostream>
using namespace std;

#define print_debug_message		false
#define dir_size				4


int PUZZLE_SIZE;
int MIMINUM_DIST;
typedef vector< vector<int> > STATE;
const int dx[4] = { 1,0,-1,0 };
const int dy[4] = { 0,-1,0,1 };
typedef __int64 ULL;
typedef ULL KEY;


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


NODE startNode, endNode;
void input();
void output();
void puzzle_generator();
NODE movePuzzle(const NODE &state, int dir);
NODE makeStartNode();
bool isMoveAble(NODE node, int dir);


int main() {
	input();
	puzzle_generator();
	output();
	return 0;
}



void input(){
	printf("puzzle size ? : ");
	scanf("%d", &puzzle_size);
	printf("minimum dist ?:");
	scanf("%d", &miminum_dist);
}

void output(){
	printf("----------------------------------------------\n\n\n");

	for (int i = 0; i < puzzle_size; i++) {
		for (int j = 0; j < puzzle_size; j++) {
			printf("%d ", startNode.state[i][j]);
		}
		printf("\n");
	}
	printf("\n");

	for (int i = 0; i < puzzle_size; i++) {
		for (int j = 0; j < puzzle_size; j++) {
			printf("%d ", endNode.state[i][j]);
		}
		printf("\n");
	}
	printf("\n");
}

NODE movePuzzle(const NODE &node, int dir) {
	NODE ret;
	ret.state = node.state;

	int x = node.x;
	int y = node.y;
	int a = x + dx[dir];
	int b = y + dy[dir];

	ret.state[y][x] = node.state[b][a];
	ret.state[b][a] = node.state[y][x];

	return ret;
}

//need fix
NODE makeStartNode() {
	NODE ret(puzzle_size);

	// make start state

	// set seed
	int psize = puzzle_size * puzzle_size;
	vector<int> v;
	for (int i = 0; i < psize; i++)
		v.push_back(i);

	for (int i = 0; i < puzzle_size; i++) {
		for (int j = 0; j < puzzle_size; j++) {
			int r = rand() / v.size();
			ret.state[i][j] = v[r];

			// is it work??
			v.erase(v.begin() + r);
		}
	}

	return ret;
}

bool isMoveAble(NODE node, int dir) {
	int x = node.x + dx[dir];
	int y = node.y + dy[dir];
	if (x < 0 || x >= puzzle_size || y < 0 || x >= puzzle_size) return false;
	return true;
}

void puzzle_generator( ){
	unordered_map<NODE, int> dist_map;
	vector<NODE> res;
	queue<NODE> q;
	NODE start = makeStartNode();
	dist_map[start] = 0;

	while (!q.empty()) {
		NODE cur = q.front();
		q.pop();
		if (dist_map[cur] > miminum_dist) continue;
		if (dist_map[cur] == miminum_dist) 
			res.push_back(cur);
		

		for (int i = 0; i < dir_size ; i++) {
			if (isMoveAble(cur, i) == false) continue;
			NODE next = movePuzzle(cur, i);
			if (dist_map.find(next) == dist_map.end() || dist_map[next] > dist_map[cur] + 1) {
				dist_map[next] = dist_map[cur] + 1;
				q.push(next);
			}
		}
	}

	int r = rand() / res.size();
	endNode = res[r];
}
