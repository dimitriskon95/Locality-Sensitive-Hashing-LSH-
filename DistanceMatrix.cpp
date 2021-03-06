#include <fstream>
#include <sstream>
#include <string.h>
#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <cstdlib>  //rand, RAND_MAX
#include <ctime>
#include <iomanip>
#include <time.h>
#include "DistanceMatrix.h"
#include "Hashtable.h"
#include "List.h"
#include "Node.h"

using namespace std;

DistanceMatrix::DistanceMatrix(int k, int l, int n_items, int** array)
{
	cout << "A Distance Matrix class was created." << endl;
	nItems = n_items;
	L = l;
	table = array;
	hashtable = new HashTable<int, int>[l];
	for (int i=0; i <= l - 1; i++)
	{
		hashtable[i].HashTable_Init(i, k, n_items, pow(2, k), 'd', nItems);   //no 5th argument, so w = 0 by default
	}
}

DistanceMatrix::~DistanceMatrix()
{
	for (int i = 0; i <= nItems - 1; i++)
		delete[] table[i];
	delete[] table;
	delete[] hashtable;
	cout << "Delete DistanceMatrix class" << endl;
}

int DistanceMatrix::get_nitems(){
	return nItems;
}

int DistanceMatrix::get_L(){
	return L;
}

void DistanceMatrix::set_R(double varR)
{
	R = varR;
}

void DistanceMatrix::set_c(double varc)
{
	c = varc;
}

void DistanceMatrix::DistanceMatrix_Reader(int Hash_index, int Number){
	int line_count = 0;
	int *dist;
	dist = new int[get_nitems()];
	
	Node<int>* node;
	
	for(int i=0; i <= get_nitems() - 1; i++)
		dist[i] = table[Number-1][i];
	
	node = new Node<int>(dist, get_nitems(), Number);
	
	if (Hash_index == 0)		//Do it one time
	{
		set_Median(node);		//W(median) is changed for all Hashtables
	}
	hashtable[Hash_index].Insert_Node(node, table);
}


void DistanceMatrix::DistanceMatrix_LSH(char *line, ofstream& output, int Number, char option){  //Dont care to create one node for all hashtables 
	double time_LSH, time_ALL;
	int line_count = 0;
	char *str = new char[3];
	int in = 0;
	int tn = 0;
	int dist = 0;
	int NumberItems = 1000;
	Node<int>* node;
	
	List<int>* qlist;
	List<int>* final_list_LSH;
	List<int>* final_list_ALL;
	
	int *distance = new int[get_nitems()];
	while(line[line_count] != '\0')
	{
		if (line[line_count] != '\t')
		{
			str[in] = line[line_count];
			in++;
			line_count++;
		}
		if (line[line_count] == '\t')
		{
			str[in] = '\0';
			distance[tn] = atof(str);
			line_count++;
			tn++;
			in = 0;
		}
	}
	str[in] = '\0';
	distance[tn] = atof(str); //last element	int *dist;
	
	node = new Node<int>(distance, get_nitems(), Number);
	
	set_Median(node);					//W(median) is changed for all Hashtables
	
	qlist = new List<int>;
	final_list_LSH = new List<int>;
	final_list_ALL = new List<int>;
	
	output << "Query: Item";
	output << node->get_Number();
	output << '\n';
	
	if (option == 'y')				//Chose to run LSH for 3L neighbor
		NumberItems = 3*get_L();
	
	//Search closer neighbor and if R > 0 find those with dist > R*c and the closest neighbor
	cout << "item_idS" << Number << '\t';
	const clock_t begin_time_LSH  = clock();						//time	
	for(int i=0; i <= L -1; i++)
	{
		hashtable[i].HashTable_LSH(node, qlist, table);
		final_list_LSH->Insert_List(qlist, NumberItems);
	}

	dist = final_list_LSH->DistanceMatrix_Distance(node, R, c, output, 0);			//0 for LSH
	output << "distanceLSH: ";
	output << dist;
	output << '\n';
		
	time_LSH = (double)((clock() - begin_time_LSH ) /  (double)CLOCKS_PER_SEC);
	
	//Search closer neighbor for ALL
	const clock_t begin_time_ALL = clock();							//time
	hashtable[0].HashTable_Search_All(node, final_list_ALL);

	dist = final_list_ALL->DistanceMatrix_Distance(node, R, c, output, 1);		//1 for ALL
	output << "distanceTrue: ";
	output << dist;
	output << '\n';
	time_ALL = (double)(( clock() - begin_time_ALL ) /  (double)CLOCKS_PER_SEC);			//time
	
	output << "tLSH: ";
	output << time_LSH;
	output << '\n';
	
	output << "tTrue: ";
	output << time_ALL;
	output << '\n';
	output << '\n';
	
	delete [] str;
	
	qlist->set_start(NULL);
	delete qlist;
	delete final_list_LSH;
	delete final_list_ALL;
}



void DistanceMatrix::printList(){
	for (int i=0; i <= L - 1; i++)
	{
		cout << "Hashtable " << i << endl;
		hashtable[i].printList();
	}
}

void DistanceMatrix::set_Median(Node<int> *node)
{
	int k = hashtable[0].get_k();
	int median;
	double *T1 = new double[k*L];
	double **array = new double*[L];
	for(int i=0; i <= L - 1; i++)
		array[i] = new double[k];
	
	for(int i=0; i <= L - 1; i++)
	{
		hashtable[i].t1_Estimation(node, table, array);
	}
	
	int t =0;
	for(int i=0; i <= L - 1; i++)
	{
		for(int j=0; j <= k - 1; j++)
		{
			T1[t] = array[i][j];
			t++;
			//cout << T1[t-1] << "  ";
		}
	}
	//cout << endl;
	
	int length = k*L - 1;
	int end = k*L - 1;
	double temp;

	for(int count = k*L ; count > 0; count--)
	{	
		for (int index =0; index < end; index++)
		{
			if (T1[index] > T1[index+1])
			{
				temp = T1[index+1];
				T1[index+1] = T1[index];
				T1[index] = temp;
			}
		}
		end--;
	}
	
	median = T1[(k*L-1)/2];
		
	for(int i =0; i <= L -1; i++)
		hashtable[i].set_Median(median);
	
	for(int i=0; i <= L - 1; i++)
		delete [] array[i];
	delete [] array;
	delete [] T1;
		
}



