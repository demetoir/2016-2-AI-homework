
#include <stdio.h>
#include <time.h>
#include <iostream>
#include <stdlib.h>

#define print_debug_message true

using namespace std;
#define INF 2e9

int ans = 0;

clock_t begin_time;
clock_t stop_time;


typedef int TileBlk[4][4]; // 주의:  타일이 없는 셀에는 0 을 넣어 놓는다.
typedef struct anode *Ty_nodeptr;
typedef struct anode {
	TileBlk  tile;
	double fhat, ghat, hhat;
	Ty_nodeptr pred;
}nodetype;   // 부모에 대한 포인터임.


//Open, Closed 의  연결리스트의 노드에 대한 구조체 정의:
typedef struct alinkedlistnode *Ty_linkednodePtr;
typedef struct alinkedlistnode {
	Ty_nodeptr nodeptr;
	Ty_linkednodePtr next;
} Ty_linkednode;

//-Open, Closed :  노드에 대한 포인터들을 가지는 연결리스트이다.
Ty_linkednodePtr Open = NULL;
Ty_linkednodePtr Closed = NULL;

//시작 노드, 종료 노드
anode start_node,end_node;

bool isSameNode(Ty_nodeptr nodeA, Ty_nodeptr nodeB);
void computeHuristicValue(Ty_nodeptr pNode);
Ty_nodeptr popNodeWithMinimumFhatFromOpen();
void pushNodeToLinkedList(Ty_linkednodePtr *root, Ty_nodeptr pNode);
Ty_nodeptr popNodeFromLinkedList(Ty_linkednodePtr *root, const Ty_nodeptr node);
Ty_nodeptr moveNode(const Ty_nodeptr pNode, int dir);
void Astar();
void getInput();

void print_NODE(const Ty_nodeptr node);

void printMovement(const Ty_nodeptr a, const Ty_nodeptr b);
void printResult();


int main() {
	//get input
	getInput();
	
	//printf start, end node
	printf("start NODE\n");
	print_NODE(&start_node);
	printf("end NODE\n");
	print_NODE(&end_node);
	//Open = (Ty_linkednodePtr) malloc(sizeof(Ty_linkednode));
	//Closed = (Ty_linkednodePtr)malloc(sizeof(Ty_linkednode));
	//compute astar
	Astar();

	//show result( ans,time, path,...
	printResult();


	return 0;
}

bool isSameNode(Ty_nodeptr nodeA, Ty_nodeptr nodeB) {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			if (nodeA->tile[i][j] != nodeB->tile[i][j]) 
				return false;			
		}
	}
	return true;
}

void computeHuristicValue(Ty_nodeptr pNode){
	int h = 0;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			if (pNode->tile[i][j] != end_node.tile[i][j])
				h++;
		}
	}
	pNode->hhat = h;
}

Ty_nodeptr popNodeWithMinimumFhatFromOpen() {
	if (Open == NULL) return NULL;

	Ty_linkednodePtr ptr = Open;
	Ty_linkednodePtr *parent = &Open;
	Ty_linkednodePtr *min_ptr_parent;

	int f = 2e9;

	while (ptr != NULL) {
		if (f > ptr->nodeptr->fhat ) {
			min_ptr_parent = parent;
			f = ptr->nodeptr->fhat;
		}
		parent = &((*parent)->next);
		ptr = ptr->next;
	}

	Ty_linkednodePtr ret = (*min_ptr_parent);
	(*min_ptr_parent) = ret->next;
	
	//need delete???
	Ty_nodeptr retpNode = ret->nodeptr;
	free(ret);

	return retpNode;
}

void pushNodeToLinkedList(Ty_linkednodePtr *root, Ty_nodeptr pNode) {
	Ty_linkednodePtr next = (Ty_linkednode*)malloc(sizeof(Ty_linkednode));
	if (*root == NULL) {
		*root = next;
		next->next = NULL;
		next->nodeptr = pNode;
	}
	else {
		next->next = (*root)->next;
		(*root)->next = next;
		next->nodeptr = pNode;
	}
}

Ty_nodeptr popNodeFromLinkedList(Ty_linkednodePtr *root , const Ty_nodeptr node) {

	Ty_linkednodePtr *parent = root;
	Ty_linkednodePtr pCurNode = *root;
	Ty_nodeptr ret = NULL;
	while (pCurNode != NULL) {
		if (isSameNode(pCurNode->nodeptr, node)) {
			ret = pCurNode->nodeptr;
			*parent = pCurNode->next;

			//????
			free(pCurNode);

			break;
		}
		parent = &pCurNode;
		pCurNode = pCurNode->next;
	}
	return ret;
}

Ty_nodeptr moveNode(const Ty_nodeptr pNode, int dir) {
	//if can not move 
	int a, b,x,y;
	int dx[4] = { 0,1,0,-1 };
	int dy[4] = { 1,0,-1,0 };

	Ty_nodeptr ret = (Ty_nodeptr)malloc(sizeof(anode));
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			ret->tile[i][j] = pNode->tile[i][j];
			if (pNode->tile[i][j] == 0) {
				x = j;
				y = i;
				b = y + dy[dir];
				a = x + dx[dir];
			
			}
		}
	}

	if (a < 0 || a >= 4 || b < 0 || b >= 4) {
		return NULL;
		free(ret);
	}

	int temp = ret->tile[y][x];
	ret->tile[y][x] = ret->tile[b][a];
	ret->tile[b][a] = temp;

	return ret;
}

void Astar() {
	computeHuristicValue(&start_node);
	start_node.ghat = 0;
	start_node.fhat = start_node.ghat + start_node.hhat;

	pushNodeToLinkedList(&Open, &start_node);
		
	while (Open != NULL) {
		Ty_nodeptr pCurNode = popNodeWithMinimumFhatFromOpen();
		pushNodeToLinkedList(&Closed, pCurNode);

		if (isSameNode(pCurNode, &end_node)) break;

		for (int dirtion = 0; dirtion < 4; dirtion++) {
			//make nextnode 
			Ty_nodeptr pNextNode = moveNode(pCurNode, dirtion);
			if (pNextNode == NULL) continue;

			computeHuristicValue(pNextNode);
			pNextNode->ghat = pCurNode->ghat + 1;
			pNextNode->fhat = pNextNode->ghat + pNextNode->hhat;
			pNextNode->pred = pCurNode;

			Ty_nodeptr pNodeFromOpen = popNodeFromLinkedList(&Open, pNextNode);
			Ty_nodeptr pNodeFromClose = popNodeFromLinkedList(&Closed, pNextNode);
			Ty_nodeptr oldNode;

			if (pNodeFromOpen != NULL)	oldNode = pNodeFromOpen;
			else oldNode = pNodeFromClose;
			if (oldNode == NULL) {
				pushNodeToLinkedList(&Open, pNextNode);
			}
			else if (oldNode->fhat > pNextNode->fhat) {
				pushNodeToLinkedList(&Open, pNextNode);
			}
			else {
				pushNodeToLinkedList(&Closed, oldNode);
			}
		}
	}
	
}
// end astar algo 

// input & print function
void getInput() {
	
	printf("시작노드를 입력하시오(1행, 2행, 3행, 4행 순으로 각 행마다 4 개의 수):\n");
	int size = 4;
	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
			scanf("%d", &start_node.tile[i][j]);
		}
	}

	printf("목표노드를 입력하시오:\n");
	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
			scanf("%d", &end_node.tile[i][j]);
		}
	}
	
	printf("\n\n");
}

void print_NODE(const Ty_nodeptr node) {

	printf("fhat : %.0f , ghat: %.0f , hhat : %.0f\n", node->fhat, node->ghat, node->hhat);
	printf("------------------------------\n");
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			printf("%3d ", node->tile[i][j]);
		}
		printf("\n");
	}
	printf("------------------------------\n\n");
}

void printMovement(const Ty_nodeptr a, const Ty_nodeptr b) {
	printf("------------------------------------\n");
	printf("                   ->                \n");
	for (int i = 0; i < 4; i++) {
		
		for (int j = 0; j < 4; j++) {
			printf("%3d ", a->tile[i][j]);
		}
		printf("	");
		for (int j = 0; j < 4; j++) {
			printf("%3d ", b->tile[i][j]);
		}
		printf("\n");
	}
	printf("------------------------------------\n\n");
}

void printResult() {
	printf("#######################\n");
	printf("end\n");
	printf("#######################\n");

	Ty_nodeptr goal_node = popNodeFromLinkedList(&Closed, &end_node);
	ans = goal_node->fhat;
		
	//ans
	printf("number of minimum move : %d\n", ans);

	//time
	//printf("total time : %.4f\n", double(stop_time - begin_time) / CLOCKS_PER_SEC);

	//path
	int pathsize = 0;
	Ty_nodeptr path[100];

	Ty_nodeptr cur = goal_node;
	while (cur != NULL) {
		path[pathsize] = cur;
		pathsize++;

		cur = cur->pred;
	}


	for (int i = pathsize - 1, numerOfmove = 1; i > 0; i--, numerOfmove++) {
		printf("[%d] move\n",numerOfmove);
		Ty_nodeptr a = path[i];
		Ty_nodeptr b = path[i-1];
		printMovement(a, b);
	}
	
}
// end input & print function
