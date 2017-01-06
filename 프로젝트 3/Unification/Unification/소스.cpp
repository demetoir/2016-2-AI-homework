#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>

using namespace std;

#define isPrintDebugMsg false

typedef struct anode * nodePtr;
typedef struct anode {
	char *pStr; // string
	nodePtr down; // pointer to the linked list for a list element
	nodePtr right;  // pointer to the next node
} nodeType;

typedef struct substi_node *substi_nodeptr;
typedef struct substi_node {
	nodePtr  pReplace_term;
	nodePtr  pVar;
	substi_nodeptr next;
} substi_node;

#define MAX_STRING_SIZE 1234



FILE pInputFile;
#define INPUT_FILE_NAME "input.txt"
FILE * openFile();
char * getLine(FILE *fp);

nodePtr Head1 = NULL;
nodePtr Head2 = NULL;
int unify(nodePtr exp1, nodePtr exp2, substi_nodeptr &result_substi);
nodePtr Get_structure_of_expression(char rawString[MAX_STRING_SIZE]);
int Apply_substitution_element(nodePtr Head, char VarString[], nodePtr nodeTerm);

void addSubsti(substi_nodeptr &root, substi_nodeptr newTail);
substi_nodeptr makeSubsti(nodePtr exp1, nodePtr exp2);

void freeNodeType(nodePtr node);
void freeSubsti_node(substi_nodeptr node);

nodePtr deepCopyExpList(nodePtr list);
nodePtr deepCopyNodeType(nodePtr node);

bool isAtom(char * pStr);
bool isVar(char * pStr);
bool isList(char * pStr);
bool isDown(char * pStr);
bool isVarInList(nodePtr var, nodePtr List);
bool isSameLength(nodePtr exp1, nodePtr exp2);

void printTab(int tabNumber) {
	for (int i = 0; i < tabNumber; i++) {
		printf("	");
	}
	return;
}
void printStructOfExp(nodePtr exp, int tabNumber) {
	if (exp == NULL) {
		printf("node is NULL\n");
		return;
	}

	printTab(tabNumber);
	printf("pStr :%s\n", exp->pStr);

	if (exp->down != NULL) {
		printTab(tabNumber);
		printStructOfExp(exp->down, tabNumber + 1);
	}
	if (exp->right != NULL) {
		printTab(tabNumber - 1);
		printStructOfExp(exp->right, tabNumber);
	}


	return;
}
void printExpList(nodePtr expList) {
	if (expList == NULL) return;

	if (isDown(expList->pStr)) {
		printExpList(expList->down);
		if (expList->right != NULL)
			printExpList(expList->right);
	}
	else {
		printf(" %s", expList->pStr);
		if (expList->right != NULL)
			printExpList(expList->right);
	}
}
void printResult_substi(substi_nodeptr result) {
	substi_nodeptr cur = result;
	if (cur == NULL) return;

	while (1) {
		if (isList(cur->pReplace_term->pStr)) {
			printExpList(cur->pReplace_term);
			printf("/%s", cur->pVar->pStr);
		}
		else {
			printf("%s/%s", cur->pReplace_term->pStr, cur->pVar->pStr);
		}
		cur = cur->next;

		if (cur == NULL) break;

		printf(", ");
	}
	printf("\n");
	return;
}

#define MAX_TOKEN_SIZE 100
struct sTokenIterator {
	char *pString;
	char *pEnd;
	const int SPLITTER_SIZE = 4;
	const char splitter[4] = { '(', ')', '\n', '\0' };

	bool ischarInSplitter(char c) {
		bool ret = false;
		for (int i = 0; i < SPLITTER_SIZE; i++) {
			if (c == splitter[i])
				ret = true;
		}
		return ret;
	}

	sTokenIterator(char * _pString) {
		int stringIndex = 0;
		pString = (char*)malloc(MAX_TOKEN_SIZE * sizeof(char*));
		for (int i = 0; i < MAX_TOKEN_SIZE; i++) {
			pString[i] = 0;
		}
		pEnd = pString;

		int strSize = strlen(_pString);
		for (int i = 0; i < strSize; i++) {
			if (ischarInSplitter(_pString[i])) {
				pString[stringIndex++] = ' ';
				pString[stringIndex++] = _pString[i];
				pString[stringIndex++] = ' ';
			}
			else {
				pString[stringIndex++] = _pString[i];
			}
		}
	}

	~sTokenIterator() {
		free(pString);
	}

	char* getNext() {
		int iTokenIndex = 0;
		char *pToken = NULL;
		pToken = (char*)malloc(MAX_TOKEN_SIZE * sizeof(char*));
		for (int i = 0; i < MAX_TOKEN_SIZE; i++)
			pToken[i] = 0;

		//skip space or tab 
		while (1) {
			if (*pEnd != ' ' && *pEnd != '	')
				break;
			pEnd++;
		}

		if (*pEnd == '\0') return NULL;

		while (1) {
			//reach end
			if (*pEnd == '\0' || *pEnd == '\n') {
				free(pToken);
				pToken = NULL;
				break;
			}
			if (*pEnd == ' ' || *pEnd == '	') {
				break;
			}
			pToken[iTokenIndex++] = *pEnd;
			pEnd = pEnd++;
		}

		return pToken;
	}
};

int main() {
	FILE *fp = openFile();
	if (fp == NULL)
		return -1;

	while (1) {
		char * rawString1 = getLine(fp);
		char * rawString2 = getLine(fp);
		char * blocker = NULL;

		if (isPrintDebugMsg) {
			printf("<<%s>>\n", rawString1);
			printf("<<%s>>\n", rawString2);

			printf("token list1\n");
			sTokenIterator splitter1(rawString1);

			while (1) {
				char * pStr = splitter1.getNext();
				if (pStr == NULL) break;
				printf("<<%s>>\n", pStr);
			}
			printf("\n");

			printf("token list2\n");
			sTokenIterator splitter2(rawString2);
			while (1) {
				char * pStr = splitter2.getNext();
				if (pStr == NULL) break;
				printf("<<%s>>\n", pStr);
			}
			printf("\n");
		}

		nodePtr exp1 = Get_structure_of_expression(rawString1);
		nodePtr exp2 = Get_structure_of_expression(rawString2);
		Head1 = exp1;
		Head2 = exp2;

		if (isPrintDebugMsg) {
			printf("<<%s>>\n", rawString1);
			printf("############ start exp1 :\n");
			printStructOfExp(exp1, 0);
			printf("############ end exp1 :\n");

			printf("<<%s>>\n", rawString2);
			printf("############ start exp2 :\n");
			printStructOfExp(exp2, 0);
			printf("############ end exp2 :\n");

		}

		substi_nodeptr  res_substi = NULL;
		int res = unify(exp1, exp2, res_substi);
		//printf("#####################################\n");
		if (res) {
			printf("Unify result : ");
			printResult_substi(res_substi);
		}
		else {
			printf("Unify result : Unification fail\n");
		}
		//printf("free exp1\n");
		freeNodeType(exp1);
		//printf("free exp2\n");
		freeNodeType(exp2);
		freeSubsti_node(res_substi);
		blocker = getLine(fp);
		if (strcmp(blocker, "%\n")) {
			break;
		}
	}

	printf("program ends\n");
	_fcloseall();
	getchar();
	return 0;
}


FILE * openFile() {
	FILE *fp = fopen(INPUT_FILE_NAME, "r");

	if (fp == NULL) {
		printf("input.txt not found\n");
	}
	return fp;
}

char * getLine(FILE *fp) {
	char *pLine;
	pLine = (char*)malloc(sizeof(char) * MAX_STRING_SIZE);

	for (int i = 0; i < MAX_STRING_SIZE; i++) {
		pLine[i] = 0;
	}
	bool isOK = true;

	int i = 0;
	char c = 0;
	while (feof(fp) == 0) {
		if (i >= MAX_STRING_SIZE) {
			isOK = false;
			break;
		}
		c = fgetc(fp);
		pLine[i++] = c;
		if (c == '\n') break;
	}

	if (isOK) {
		return pLine;
	}
	else {
		free(pLine);
		return NULL;
	}
}


void addSubsti(substi_nodeptr &root, substi_nodeptr newTail) {
	substi_nodeptr cur = root;
	if (root == NULL) {
		root = newTail;
		return;
	}
	while (cur->next != NULL) {
		cur = cur->next;
	}
	cur->next = newTail;
}

nodePtr deepCopyExpList(nodePtr oldList) {
	nodePtr newNodeRoot = deepCopyNodeType(oldList);

	nodePtr newCur = newNodeRoot;
	nodePtr oldCur = oldList->right;
	
	while (oldCur != NULL) {		
		nodePtr newNode = deepCopyNodeType(oldCur);
		newCur->right = newNode;
		newCur = newCur->right;
		if (isDown(oldCur->pStr)) {
			nodePtr newListNode = deepCopyExpList(oldCur->down);
			newCur->down = newListNode;
		}
		oldCur = oldCur->right;
	}
		
	return newNodeRoot;
}
nodePtr deepCopyNodeType(nodePtr node) {
	nodePtr ret;
	ret = (nodePtr)malloc(sizeof(nodeType));
	ret->down = NULL;
	ret->right = NULL;

	char *pStr = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
	for (int i = 0; i < MAX_TOKEN_SIZE; i++) {
		pStr[i] = node->pStr[i];
	}
	ret->pStr = pStr;

	return ret;
}

substi_nodeptr makeSubsti(nodePtr replace_term, nodePtr var) {
	substi_nodeptr newSubsti = NULL;
	newSubsti = (substi_nodeptr)malloc(sizeof(substi_node));

	if (isList(replace_term->pStr))
		newSubsti->pReplace_term = deepCopyExpList(replace_term);
	else
		newSubsti->pReplace_term = deepCopyNodeType(replace_term);
	newSubsti->pVar = deepCopyNodeType(var);
	newSubsti->next = NULL;

	return newSubsti;
}


int unify(nodePtr exp1, nodePtr exp2, substi_nodeptr &result_substi) {
	bool isOK = true;
	result_substi = NULL;
	substi_nodeptr stp = NULL;
	//1. exp1 과 exp2 각각이 어떤 종류인지를 판단한다.
	//	가능성은 다음과 같다 :
	//	변수 atom : exp1 포인터가 가리키는 노드가 좌측괄호를 갖지 않음.그리고 저장된 스트링이 u ~ z 사이의 글자 한 개.
	//	상수 atom : exp1 포인터가 가리키는 노드가 좌측괄호를 갖지 않음.그리고 변수가 아닌 형태의 스트링을 가짐
	//	list : exp1 포인터가 가리키는 노드가 스트링 “(“ 를 가진 경우임.
	//2. exp1 의 종류에 따라 다음 중 한가지를 처리한다 :

	if (isAtom(exp1->pStr)) {
		if (isAtom(exp2->pStr)) {
			if (strcmp(exp1->pStr, exp2->pStr) == 0)
				return true;
			else
				return false;
		}
		else if (isVar(exp2->pStr)) {
			addSubsti(result_substi, makeSubsti(exp1, exp2));
			// Head2 의 구조 중 exp2 변수 노드마다 exp1 노드의 내용으로 변경한다.
			Apply_substitution_element(Head2, exp2->pStr, exp1);
			return true; // 치환요소는 한개 만을 넘긴다.
		}
		else if (isList(exp2->pStr)) {
			return false;
		}
	}
	else if (isVar(exp1->pStr)) {
		if (isVar(exp2->pStr)) {
			return 1;
		}
		else if (isAtom(exp2->pStr)) {
			addSubsti(result_substi, makeSubsti(exp2, exp1));
			Apply_substitution_element(Head1, exp1->pStr, exp2);
			return 1;
		}
		else if (isList(exp2->pStr)) {
			if (isVarInList(exp1, exp2)) {
				return false;
			}
			else {
				//*substi 에(exp2 / exp1) 치환요소를 매단다.
				addSubsti(result_substi, makeSubsti(exp2, exp1));
				Apply_substitution_element(Head1, exp1->pStr, exp2);
				return true; // 치환요소는 한개 만을 넘긴다.
			}
		}
	}
	else if (isList(exp1->pStr)) {
		if (isAtom(exp2->pStr)) {
			return 0;
		}
		else if (isVar(exp2->pStr)) {
			if (isVarInList(exp2, exp1))
				return false;
			else {
				//*substi 에(exp1 / exp2) 치환요소를 매단다.
				addSubsti(result_substi, makeSubsti(exp1, exp2));

				
				Apply_substitution_element(Head2, exp2->pStr, exp1);
				// Head2 의 구조내에 exp2의 변수를 가지는 노드를 모두 exp1 을 가지는 노드로 변경한다.
				return true;
			}
		}

		else if (isList(exp2->pStr)) {
			if (isSameLength(exp1, exp2) == false) {
				return false;
			}

			// skip '('
			nodePtr p1 = exp1->right;
			nodePtr p2 = exp2->right; // 각자 다음 노드로 이동.

			while (1) {
				nodePtr pDown1 = p1;
				nodePtr pDown2 = p2;

				if (isDown(p1->pStr)) {
					pDown1 = p1->down;
				}
				if (isDown(p2->pStr)) {
					pDown2 = p2->down;
				}

				substi_nodeptr pTemp_substi = NULL;
				int Res = unify(pDown1, pDown2, pTemp_substi); // recursice call

				if (Res == false)
					return false;

				//Attach substitution elements in the list of temp_substi to the list of *substi;
				addSubsti(result_substi, pTemp_substi);

				p1 = p1->right;
				p2 = p2->right;

				if (strcmp(p1->pStr, ")") == 0 && strcmp(p1->pStr, ")") == 0) {
					return true;
				}
				else if (strcmp(p1->pStr, ")") == 0 || strcmp(p1->pStr, ")") == 0) {
					return false;
				}
			}
		}
	}

	return  isOK;
}

nodePtr Get_structure_of_expression(char rawString[MAX_STRING_SIZE]) {
	nodePtr stack[100];
	int  top = -1; // 스택 초기화. We use a stack to handle lists embedding sub-lists.
	nodePtr exp = NULL;
	nodePtr curr = NULL, tp, tp2;
	char* pToken = NULL;

	// set sTokenIterator
	sTokenIterator itor(rawString);
	pToken = itor.getNext();
	if (pToken == NULL)
		return NULL;

	//read token into Tok;
	if (strcmp(pToken, "(") != 0)
		return NULL;  // 첫 토큰은 반드시 좌측괄호이어야 한다.

	exp = (nodePtr)malloc(sizeof(nodeType));
	curr = exp;
	curr->pStr = pToken;
	curr->down = NULL;
	curr->right = NULL;


	top = 0;
	stack[0] = NULL; // return position is null for left parentheis corresponding to the first left paranthesis.

	while (1) {
		pToken = itor.getNext();
		if (pToken == NULL)
			return  NULL; //  Some token is required  to come.  But no more. So  error has occurred.


		tp = (nodePtr)malloc(sizeof(nodeType));  // 새 노드 할당한다
		tp->down = NULL;
		tp->right = NULL;

		if (strcmp(pToken, ")") == 0) {
			tp->pStr = pToken;
			tp->down = tp->right = NULL;
			curr->right = tp;
			curr = stack[top--];

			if (curr == NULL)
				break;
			//exit the loop; 
			//  the final right  ) corresponding to the first ( was inputed. So nothing more to do.
		}

		else if (strcmp(pToken, "(") == 0) {
			char * _pStr = (char *)malloc(1 * sizeof(char*));
			_pStr[0] = '\0';
			tp->pStr = _pStr;
			curr->right = tp;
			stack[++top] = tp;  // 돌아올 곳 저장

			tp2 = (nodePtr)malloc(sizeof(nodeType));
			tp2->pStr = pToken;
			tp2->down = NULL;
			tp2->right = NULL;
			tp->down = tp2;  
			curr = tp2;  // 하위 레벨로 내려감
		}
		else if (strcmp(pToken, ")") != 0 && strcmp(pToken, "(") != 0) {
			tp->pStr = pToken;
			tp->down = NULL;
			curr->right = tp;
			curr = tp;
		}
	}

	pToken = itor.getNext();
	if (pToken == NULL)
		return  exp;  // Success since final right ) is read and no tokens are left.
	else
		return  NULL;   // Some tokens are remaining after the final right ).


	itor.~sTokenIterator();
	return exp;
}

void freeNodeType(nodePtr node) {
	if (node == NULL) return;

	nodePtr down = node->down;
	nodePtr right = node->right;

	free(node->pStr);
	free(node);
	node = NULL;

	freeNodeType(down);
	freeNodeType(right);
}
void freeSubsti_node(substi_nodeptr node) {
	if (node == NULL) return;

	freeSubsti_node(node->next);
	free(node);	
	node = NULL;
}

bool isAtom(char * pStr) {
	if (isVar(pStr) || isList(pStr)) return false;
	return true;
}
bool isVar(char * pStr) {
	int strSize = strlen(pStr);
	if (strSize == 1 && (pStr[0] >= 'u' && pStr[0] <= 'z'))
		return true;
	return false;
}
bool isList(char * pStr) {
	if (pStr[0] == '(') return true;
	return false;
}
bool isDown(char *pStr) {
	if (pStr[0] == '\0') return true;
	return false;
}
bool isVarInList(nodePtr var, nodePtr List) {
	nodePtr cur = List;
	while (cur->right != NULL) {
		if (strcmp(cur->pStr, var->pStr) == 0)
			return true;
		cur = cur->right;
	}

	return false;
}
bool isSameLength(nodePtr exp1, nodePtr exp2) {
	nodePtr cur = NULL;
	int a = 0;
	int b = 0;

	cur = exp1;
	while (cur->right != NULL) {
		a++;
		cur = cur->right;
	}

	cur = exp2;
	while (cur->right != NULL) {
		b++;
		cur = cur->right;
	}

	if (a == b)
		return true;
	else
		return false;
}

int Apply_substitution_element(nodePtr Head, char VarString[], nodePtr nodeTerm) {
	int ret = 1;
	// Head : 메인함수에서 준비된 expression 구조의 첫 노드에 대한 포인터
	// Var : 변수 노드에 대한 포인터. 이 노드는 Head 구조의 한 노드이다.
	// nodeTerm : Head 구조 내에서 nodeVar 의 변수를 모두 nodeTerm 으로 변경하여 주어야 한다.
	//
	// Return value: it returns 1 if successful; 
	//Otherwise 0 is returned. (질문: 0 이 반환되는 경우가 있는가?)

	char *pStr = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
	for (int i = 0; i < MAX_TOKEN_SIZE; i++) {
		pStr[i] = VarString[i];
	}

	while (1) {
		if (isVar(Head->pStr)) {
			if (strcmp(Head->pStr, pStr) == 0) { // 치환이 필요한 노드를 발견함.
				if (isAtom(nodeTerm->pStr)) {
					// 변수를 atom 으로 변경함
					for(int i =0; i<MAX_TOKEN_SIZE; i++)
						Head->pStr[i] = nodeTerm->pStr[i]; 
				}
				else if (strcmp(nodeTerm->pStr, "(") == 0) {
					Head->pStr[0] ='\0';
					Head->down = deepCopyExpList(nodeTerm); // 변수를 리스트로 변경함
				}
			}
		}
		else if (isDown(Head->pStr)) {
			ret = Apply_substitution_element(Head->down, VarString, nodeTerm);
			if (ret == 0)
				break;
		}

		Head = Head->right; // 우측 노드로 이동한다.
		if (strcmp(Head->pStr, ")") == 0) { // 이 리스트의 원소를 모두 처리하였음.
			break;
		}
	}
	//free(pNewStr);
	free(pStr);
	return ret;
}




