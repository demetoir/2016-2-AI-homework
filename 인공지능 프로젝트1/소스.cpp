#include <stdio.h>
#include <time.h>
#include <math.h>
#include <vector>
#include <string.h>
using namespace std;
#define INF 2e9 

#define NLAYER_NUMBER 3 // NN의 층수
#define MLAYER_SIZE 65 // NN의 각 층의 최대 뉴런 수
#define m0 5 // 0번층의 뉴런수
#define m1 5 // 1번층의 뉴런수
#define m2 3 // 2번층의 뉴런수
#define N 65 // 더미 입력을 포함한 입력갯수
#define TRAIN_ERROR_THRESHOLD 0.001 // 학습하는데 에러의 threshold
const double c = 0.5; // 학습률

//NN의 각 뉴런들이 가지는 값들
int M[NLAYER_NUMBER] = { m0,m1,m2 }; // NN의 각층 의 뉴런 수
double s[NLAYER_NUMBER][MLAYER_SIZE]; // NN의 모든 뉴런의 s값
double f[NLAYER_NUMBER][MLAYER_SIZE]; // NN의 모든 뉴런의 f(출력)값
double delta[NLAYER_NUMBER][MLAYER_SIZE]; //NN의 모든 뉴런의 delta값
double W[NLAYER_NUMBER][MLAYER_SIZE][MLAYER_SIZE]; //NN의 모든 뉴런의 가중치 벡터

//train과 test에 사용되어지는 파일이름 
#define TRAINDATA_FILENAME "traindata.txt" 
#define TESTDATA_FILENAME "testdata.txt"
//train과 test의 예제들의 갯수
#define N_tr_examples  600 
#define N_te_examples  90 
//train과 test에 사용되어지는 input,output 
int TrainDataInput[N_tr_examples][N];
int TrainDataOutput[N_tr_examples][m2];
int TestDataInput[N_te_examples][N];
int TestDataOutput[N_te_examples][m2];

//NN의 입력값과 목표 출력값
int input[N];
int d[m2];

void readTrainData();//train data file을 읽어서 저장한다
void readTestData();//test data file을 읽어서 저장한다
void init_W();//가중치를 초기화한다
void forward_compute(); // 모든 뉴런의 f값을 forward 방향으로 계산한다
void backward_compute(); // 모든 뉴런의 delta값을 backward 방향으로 계산한다
void weight_update(); //모든 뉴런의 가중치벡터를 갱신한다
void train_one_epoch(); //NN을 1 epoch만큼 훈련시킨다
double get_average_error();//NN의 평균 에러율을 구한다
double test_NN(); //NN을 test data로 test해본다

void print_NN_info(); //NN의 정보를 출력한다

int main() {
	print_NN_info();

	readTrainData();
	readTestData();

	init_W();
	bool is_want_to_show_average_error = false;
	printf("show each epoch average error (y/n)");
	char inputc;
	scanf("%c",&inputc);

	if (inputc == 'y'|| inputc == 'Y') is_want_to_show_average_error = true;

	printf("train start\n");
	int numberOfTrain = 0;
	double avg_error;
	while (1) {
		train_one_epoch();
		avg_error = get_average_error();
			
		if (avg_error <= TRAIN_ERROR_THRESHOLD)break;

		if(is_want_to_show_average_error)
			printf("epoch %d  :  avg_error  %f\n", numberOfTrain, avg_error);

		numberOfTrain++;
	}
	printf("train end\n");
	printf("last epoch %d  :  avg_error  %f\n\n", numberOfTrain, avg_error);

	printf("test start\n");
	double avg_acc = test_NN();
	printf("test end\n");
	printf("avg_acc : %f\n\n", avg_acc);
	return 0;
}

void readTrainData() {
	FILE *fpTrainDataFile = fopen(TRAINDATA_FILENAME, "r");

	memset(TrainDataInput, 0, sizeof(TrainDataInput));
	memset(TrainDataOutput, 0, sizeof(TrainDataOutput));
	for (int iter = 0; iter < N_tr_examples; iter++) {
		int output = 0;
		char splitter;

 		fscanf(fpTrainDataFile, "%d %c ", &output,&splitter);
		TrainDataOutput[iter][output] = 1;
		for (int i = 0; i < N; i++) 
			fscanf(fpTrainDataFile, "%d ", &TrainDataInput[iter][i]);
	}

	fclose(fpTrainDataFile);
}

void readTestData() {
	FILE *fpTestDataFile = fopen(TESTDATA_FILENAME, "r");

	memset(TestDataInput, 0, sizeof(TestDataInput));
	memset(TestDataOutput, 0, sizeof(TestDataOutput));
	for (int iter = 0; iter < N_te_examples; iter++) {
		int output;
		char splitter;

		fscanf(fpTestDataFile, "%d %c ", &output, &splitter);
		TestDataOutput[iter][output] = 1;
		for (int i = 0; i < N; i++) 
			fscanf(fpTestDataFile, "%d ", &TestDataInput[iter][i]);		
	}

	fclose(fpTestDataFile);
}

void init_W() {
	srand(time(NULL));
	for (int i = 0; i < NLAYER_NUMBER; i++) {
		for (int j = 0; j < MLAYER_SIZE; j++) {
			for (int k = 0; k < MLAYER_SIZE; k++) {
				int sign = 1;
				if (rand() % 2 == 1) sign = -1;

				double val = ((double)rand()) / (double)RAND_MAX;
				W[i][j][k] = sign*val;
			}
		}
	}
}

void forward_compute() {
	for (int i = 0; i < NLAYER_NUMBER; i++)
		for (int j = 0; j < MLAYER_SIZE; j++)
			s[i][j] = 0.0;

	//0층 s계산
	for (int i = 0; i < N; i++) {		
		for (int k = 0; k < N; k++) {
			s[0][i] += input[k] * W[0][i][k];
		}
	}
	//0층 f계산
	for (int i = 0; i < N; i++) 
		f[0][i] = 1/(1+ exp(-s[0][i]));
	
	//1층이후
	for (int j = 1; j < NLAYER_NUMBER; j++) {		
		for (int i = 0; i < M[j]; i++) {
			//j층 i번쟤 뉴런의 s계산
			for (int k = 0; k < M[j - 1]; k++) 
				s[j][i] += f[j-1][k] * W[j][i][k];			
			s[j][i] += W[j][i][M[j - 1]];

			//j층 i번쟤 뉴런의 f계산
			f[j][i] = 1/(1 + exp( -s[j][i] ));
		}
	}
}

void backward_compute() {
	//마지막층 delta계산
	int lastlayer = NLAYER_NUMBER - 1;
	for (int i = 0; i < M[lastlayer]; i++) {
		delta[lastlayer][i] = (d[i] - f[lastlayer][i]) * f[lastlayer][i] * (1 - f[lastlayer][i]);
	}

	//중간층 delta 계산
	for (int j = lastlayer - 1; j >= 0; j--) {
		for (int i = 0; i < M[j]; i++) {
			double tsum = 0;
			for (int k = 0; k < M[j + 1]; k++) 
				tsum += delta[j + 1][k] * W[j + 1][k][i];
			delta[j][i] = f[j][i] * (1 - f[j][i])*tsum;
		}
	}
}

void weight_update() {
	//첫번쨰층
	for (int i = 0; i < M[0]; i++) {
		for (int k = 0; k < N; k++) {
			W[0][i][k] += c * delta[0][i] * input[k];
		}
	}

	//나머지층
	for (int j = 1; j < NLAYER_NUMBER; j++) {
		for (int i = 0; i < M[j]; i++) {
			for (int k = 0; k < M[j - 1]; k++) {
				W[j][i][k] += c * delta[j][i] * f[j-1][k];
			}
			W[j][i][M[j - 1]] += c * delta[j][i];
		}
	}
}

void train_one_epoch() {
	double errorSum = 0;

	for (int iter = 0; iter < N_tr_examples; iter++) {
		for (int i = 0; i < N; i++)
			input[i] = TrainDataInput[iter][i];
		for (int i = 0; i < m2; i++)
			d[i] = TrainDataOutput[iter][i];	

		forward_compute();
		backward_compute();
		weight_update();
	}
}

double get_average_error() {
	double errorSum = 0;

	for (int iter = 0; iter < N_tr_examples; iter++) {
		for (int i = 0; i < N; i++)	input[i] = TrainDataInput[iter][i];
		for (int i = 0; i < m2; i++) d[i] = TrainDataOutput[iter][i];

		forward_compute();

		for (int i = 0; i < M[NLAYER_NUMBER - 1]; i++) {
			errorSum += (d[i] - f[NLAYER_NUMBER - 1][i])*(d[i] - f[NLAYER_NUMBER - 1][i]);
		}
	}

	double avg_error = errorSum / (N_tr_examples * M[NLAYER_NUMBER - 1]);
	return  avg_error;
}

double test_NN() {
	double acc = 0;
	for (int iter = 0; iter < N_te_examples; iter++) {
		for (int i = 0; i < N; i++)
			input[i] = TestDataInput[iter][i];
		for (int i = 0; i < M[NLAYER_NUMBER - 1]; i++)
			d[i] = TestDataOutput[iter][i];

		forward_compute();

		int output[ m2 ]; //need hack
		memset(output, 0, sizeof(output));
		int maxindex = -1;
		double maxval = -INF;
		for (int i = 0; i < M[NLAYER_NUMBER - 1]; i++) {
			if (maxval < f[NLAYER_NUMBER - 1][i]) {
				maxindex = i;
				maxval = f[NLAYER_NUMBER - 1][i];
			}
		}
		output[maxindex] = 1;

 		bool iscorrect = true;
		for (int i = 0; i < M[NLAYER_NUMBER - 1]; i++) 
			if (output[i] != d[i]) 
				iscorrect = false;

		if (iscorrect) acc++;
	}

	double total_acc = acc / N_te_examples;
	return total_acc;
}

void print_NN_info() {
	printf("NN info\n");
	printf("	NLAYER_NUMBER : %d\n", NLAYER_NUMBER);
	printf("	MLAYER_SIZE : %d\n", MLAYER_SIZE);
	printf("	m0 : %d\n", m0);
	printf("	m1 : %d\n", m1);
	printf("	m2 : %d\n", m2);
	printf("	N  : %d\n", N);
	printf("	TRAIN_ERROR_THRESHOLD : %f \n", TRAIN_ERROR_THRESHOLD);
	printf("	c : %f\n", c);
	printf("	\n");
}
