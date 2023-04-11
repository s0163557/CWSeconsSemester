#include <iostream>
#include <vector>
#include <mutex>
#include <chrono>

using namespace std;

int CapacityOfGraph = 1000, Radius = 100;
mutex m;

void PrintGraph(vector<vector<int>> Graph)
{
	for (int i = 0; i < Graph.size(); i++)
	{
		cout << i << ":";
		for (int j = 0; j < Graph[i].size(); j++)
			cout << Graph[i][j] << " ";
		cout << endl;
	}
}

//Создадим большой массив геометрических графов
void CreateGraphs(vector<vector<vector<int>>>* MassiveOfGraphs, int Size, int Start, int End)
{
	srand(time(0));
	int i, j, k;
	for (j = Start; j < End; j++)
	{
		//Каждый элемент графа является силой ребра в рассматриваемой столбце - вершине.
		vector<vector<int>> Graph;
		vector<pair<int, int>> GeomGraph;
		Graph.resize(Size);
		for (i = 0; i < Size; i++)
			Graph[i].resize(Size);
		for (i = 0; i < Size; i++)
		{
			double x, y;
			x = rand() % CapacityOfGraph;
			y = rand() % CapacityOfGraph;
			GeomGraph.push_back(make_pair(x, y));
			for (k = 0; k < i; k++)
			{
				double lx, rx, uy, dy;
				lx = x + Radius;
				rx = x - Radius;
				uy = y + Radius;
				dy = y - Radius;
				if (rx < GeomGraph[k].first && GeomGraph[k].first < lx && dy < GeomGraph[k].second && GeomGraph[k].second < uy)
				{
					Graph[i][k] = 1;
					Graph[k][i] = 1;
				}
			}
		}

		m.lock();
		MassiveOfGraphs->push_back(Graph);
		m.unlock();
	}
}

vector<vector<vector<int>>> MassiveCreateGraphs(int Size, int NumberOfGraphs)
{
	vector<vector<vector<int>>> MassiveOfGraphs;
	thread T1(CreateGraphs, &MassiveOfGraphs, Size, 0, NumberOfGraphs / 4);
	thread T2(CreateGraphs, &MassiveOfGraphs, Size, NumberOfGraphs / 4, NumberOfGraphs / 2);
	thread T3(CreateGraphs, &MassiveOfGraphs, Size, NumberOfGraphs / 2, 3 * NumberOfGraphs / 4);
	thread T4(CreateGraphs, &MassiveOfGraphs, Size, 3 * NumberOfGraphs / 4, NumberOfGraphs);
	T1.join(); T2.join(); T3.join(); T4.join();
	return MassiveOfGraphs;
}

void AlgShtorVagner(vector<vector<int>> Graph, vector<int>* MincutVector)
{
	srand(time(0));
	int i, j, k, mincut = INT_MAX;
	vector <bool> A, Exist;
	vector<int> WeightsWithA;
	Exist.resize(Graph.size(), 1);
	int CurrentVertex, PrevVertex;
	for (i = 0; i < Graph.size() - 1; i++)
	{
		A.resize(Graph.size(), 0);
		int FirstVertex = 0;
		while (Exist[FirstVertex] != 1)
			FirstVertex++;
		A[FirstVertex] = 1; //выбрали случайную вершину
		WeightsWithA = Graph[FirstVertex];//Сразу же добавили всех соседей А
		for (j = 0; j < Graph.size() - i - 2; j++)
		{
			CurrentVertex = -1;
			//Выберем наиболее связанную вершину:
			for (k = 0; k < WeightsWithA.size(); k++)
				if (!A[k] && Exist[k])
				{
					if (CurrentVertex == -1)
					{
						CurrentVertex = k;
						break;
					}

					if (WeightsWithA[k] > WeightsWithA[CurrentVertex])
						CurrentVertex = k;
				}
			//Обработаем веса:
			A[CurrentVertex] = 1;
			for (k = 0; k < WeightsWithA.size(); k++)
				WeightsWithA[k] += Graph[k][CurrentVertex];
			PrevVertex = CurrentVertex;
		}
		//Находим последнюю оставшуюся вершину

		CurrentVertex = -1;
		for (k = 0; k < WeightsWithA.size(); k++)
			if (!A[k] && Exist[k])
			{
				if (CurrentVertex == -1)
				{
					CurrentVertex = k;
					break;
				}

				if (WeightsWithA[k] > WeightsWithA[CurrentVertex])
					CurrentVertex = k;
			}

		//Обновим минимальный разрез
		if (WeightsWithA[CurrentVertex] < mincut)
			mincut = WeightsWithA[CurrentVertex];


		//Обработаем соединение вершин:
		for (k = 0; k < Graph.size(); k++)
			Graph[PrevVertex][k] = Graph[k][PrevVertex] += Graph[CurrentVertex][k];
		//Покажем, что вершины больше не существует:
		Exist[CurrentVertex] = 0;

		A.clear();
		WeightsWithA.clear();
	}
	m.lock();
	MincutVector->push_back(mincut);
	m.unlock();
}

void ConnectifityFunction(vector<vector<int>> Graph, vector<vector<int>>* MincutVector)
{
	int i, j, k;
	vector<int> MinCut;
	AlgShtorVagner(Graph, &MinCut); //Нужна для нахождения минимального разреза графа без удалённых вершин
	//Пройдёмся по каждой вершине и удалим её, а затем запустим алгоритм. Если возвращенное значение окажется меньше - нашли одну из связных вершин
	for (i = 0; i < Graph.size(); i++)
	{
		vector<vector<int>> DamagedGraph = Graph;
		//Удалим вершину номер i
		DamagedGraph.erase(DamagedGraph.begin() + i);
		for (j = 0; j < DamagedGraph.size(); j++)
			DamagedGraph[j].erase(DamagedGraph[j].begin() + i);

		//Посчитаем связность
		AlgShtorVagner(DamagedGraph, &MinCut);


	}

	//Поскольку они отсутствуют, сразу избавимся от них:
	MinCut[1] = MinCut[0];
	MinCut[MinCut.size() - 1] = MinCut[0];

	for (i = 0; i < MinCut.size(); i++)
		if (i != 0) MinCut[i] = MinCut[0] - MinCut[i];

	//Отоброжает количество рёбер которое нужно удалить, в номере ячкейке, соответствующей количеству удалённых вершин.
	vector<int> CorrectmincutData;
	CorrectmincutData.resize(MinCut.size());
	CorrectmincutData[0] = MinCut[0];
	i = 1;
	while (MinCut[0] > 0)
	{
		int max = -INT_MAX, NumOfVertex;
		for (j = 1; j < MinCut.size(); j++)
			if (MinCut[j] > max)
			{
				max = MinCut[j];
				NumOfVertex = j;
			}
		MinCut[0] -= max;
		MinCut[NumOfVertex] = 0;
		CorrectmincutData[i] = MinCut[0];
		i++;
	}
	MincutVector->push_back(CorrectmincutData);
}

void ParallelHelper(vector<vector<vector<int>>> MassiveOfGraphs, int Start, int End, vector<vector<int>>* MincutVector)
{
	int i;
	for (i = Start; i < End; i++)
	{
		ConnectifityFunction(MassiveOfGraphs[i], MincutVector);
		cout << i<<endl;
	}
}

void MassiveSWAlgorithm(vector<vector<vector<int>>> MassiveOfGraphs)
{
	vector<vector<int>> MincutOfGraphsVector;
	thread T1(ParallelHelper, MassiveOfGraphs, 0, MassiveOfGraphs.size() / 3, &MincutOfGraphsVector);
	thread T2(ParallelHelper, MassiveOfGraphs, MassiveOfGraphs.size() / 3, 2 * MassiveOfGraphs.size() / 3, &MincutOfGraphsVector);
	thread T3(ParallelHelper, MassiveOfGraphs, 2 * MassiveOfGraphs.size() / 3, MassiveOfGraphs.size(), &MincutOfGraphsVector);
	T1.join(); T2.join(); T3.join();

	cout << "Ready!" << endl;
	double summ = 0;
	for (int i = 0; i < MassiveOfGraphs.size(); i++)
	{
			for (int j = 0; j < MincutOfGraphsVector.size(); j++)
			summ += MincutOfGraphsVector[j][i];
		cout << "Для количества удалённых вершин " << i << " Среднее количество удаляемых рёбер:" << summ / MassiveOfGraphs.size() << endl;
		summ = 0;
	}

	cout << summ / MassiveOfGraphs.size();
}

int main()
{
	setlocale(LC_ALL, "rus");
	int Size, i, j, NumberOfGraphs, k;
	cout << "Введите количество графов:";
	cin >> NumberOfGraphs;
	cout << "Введите количество вершин в графе:";
	cin >> Size;
	cout << endl;
	vector<vector<vector<int>>> NumericGraphs = MassiveCreateGraphs(Size, NumberOfGraphs);
	cout << "Graphs Created!" << endl;
	vector<vector<int>> Mincut;
	/*
	auto Begin = chrono::steady_clock::now();
	cout << AlgShtorVagner(NumericGraphs[0], );
	auto End = chrono::steady_clock::now();
	auto Difference = chrono::duration_cast<std::chrono::milliseconds>(End -
		Begin);
	cout << endl << "Время работы алгоритма: " << Difference.count() << " ms\n" << endl;
	*/

	MassiveSWAlgorithm(NumericGraphs);

	//Допиши массовую версию, и, впринципе, можно закруглятсья и писать текст.

	//ConnectifityFunction(NumericGraphs[0], &Mincut);


	return 0;
}