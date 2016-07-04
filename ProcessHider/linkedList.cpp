#include "linkedList.h"
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <time.h>


PNODE listHead = NULL;
int itemNums = 0;
void addData(int newData)
{
	PNODE pNewNode =(PNODE) malloc(sizeof(NODE));
	pNewNode->data = newData;
	pNewNode->next = listHead;
	pNewNode->isTouched = true;
	pNewNode->isOnline = false;
	pNewNode->time_Created = clock();
	listHead = pNewNode;
	itemNums++;
}

void setOnline(int x)
{
	PNODE node=findNodeinList(x);
	if (node != NULL)
		node->isOnline = true;
}

bool getOnline(int x)
{
	PNODE node = findNodeinList(x);
	if (node != NULL)
		return node->isOnline;
	return false;
}
PNODE findNodeinList(int x)
{
	PNODE lookup = listHead;
	while (lookup != NULL)
	{
		if (lookup->data == x)
		{
			return lookup;
		}
		else
		{
			lookup = lookup->next;
		}
	}
	return NULL;
}

bool isInPidList(int x)
{
	PNODE lookup = listHead;
	while (lookup!=NULL)
	{
		if (lookup->data == x)
		{
			lookup->isTouched = true;
			return true;
		}
		else
		{
			lookup = lookup->next;
		}
	}
	return false;

}

double timeFromCreation(int x)
{
	PNODE lookup = listHead;
	while (lookup != NULL)
	{
		if (lookup->data == x)
		{
			time_t curr_clk = clock();
			double res = (curr_clk - lookup->time_Created) / (double)CLOCKS_PER_SEC;
			return res;
		}
		else
		{
			lookup = lookup->next;
		}
	}
	return 9999;
}
void deleteEntry(int x)
{
	PNODE prev=NULL,lookup = listHead;
	if (listHead == NULL) //nolist
		return;
	
	if (listHead->data == x)
	{
		lookup = listHead;
		listHead = lookup->next;
		itemNums--;
		free(lookup);
	}
	if (listHead == NULL)
	{
		return;
	}
	prev = listHead;
	lookup = listHead->next;
	while (lookup != NULL)
	{
		PNODE nextNode = lookup->next;
		if (lookup->data == x)
		{
			prev->next = lookup->next;
			itemNums--;
			free(lookup);
		}
		else
		{
			prev = lookup;
		}
		lookup = nextNode;
	}
}

void updateList()
{
	PNODE next, curr = listHead;
	while (curr != NULL)
	{
		next = curr->next;
		if (!curr->isTouched)
			deleteEntry(curr->data);
		else
			curr->isTouched = false;
		curr = next;
	}
}