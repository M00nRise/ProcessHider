#include "linkedList.h"
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <time.h>
#include <vector>

using namespace std;

vector<processData> mainVec;
processData* findNodeinList(int x);

void addData(int newData)
{
	processData p;
	p.data= newData;
	p.isTouched = true;
	p.isOnline = false;
	p.time_Created = clock();
	mainVec.push_back(p);
}

void setOnline(int x)
{
	processData* node=findNodeinList(x);
	if (node != NULL)
		node->isOnline = true;
}

bool getOnline(int x)
{
	processData* node = findNodeinList(x);
	if (node != NULL)
		return node->isOnline;
	return false;
}
processData* findNodeinList(int x)
{
	for (unsigned int i = 0; i < mainVec.size(); i++)
	{
		if (mainVec[i].data == x)
			return &mainVec[i];
	}
	return NULL;
}

bool isInPidList(int x)
{
	for (unsigned int i = 0; i < mainVec.size(); i++)
	{
		if (mainVec[i].data == x)
		{
			mainVec[i].isTouched = true;
			return true;
		}
	}
	return false;

}

double timeFromCreation(int x)
{
	for (unsigned int i = 0; i < mainVec.size();i++)
	{
		if (mainVec[i].data==x)
		{
			time_t curr_clk = clock();
			double res = (curr_clk - mainVec[i].time_Created) / (double)CLOCKS_PER_SEC;
			return res;
		}
	}
	return -1; //doesn't matter, as long as it's bigger than a few seconds
}
void deleteEntry(int x)
{
	for (unsigned int i = 0; i < mainVec.size(); i++)
	{
		if (mainVec[i].data == x)
		{
			mainVec.erase(mainVec.begin() + i);
			i = 0;
			continue;
		}
	}
}

void updateList()
{
	vector<int> deleteLocs;
	for (unsigned int i = 0; i < mainVec.size(); i++)
	{
		if (!mainVec[i].isTouched)
		{
			deleteLocs.push_back(i);
		}
		else
			mainVec[i].isTouched = false;
	}
	reverse(mainVec.begin(), mainVec.end()); //first delete further elements, so indexes won't change
	for (unsigned int i = 0; i < deleteLocs.size(); i++)
	{
		mainVec.erase(mainVec.begin() + deleteLocs[i]);
	}
}