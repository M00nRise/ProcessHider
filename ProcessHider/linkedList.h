#ifndef LINKEDLIST_H
#define LIMKEDLIST_H


#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <Windows.h>

//#define DATA_TYPE int

typedef struct _procData
{
	int data;
	time_t time_Created;
	bool isTouched;
	bool isOnline;
} processData;
bool getOnline(int x);
void setOnline(int x);


void addData(int newData);
bool isInPidList(int x);
void deleteEntry(int x);
void updateList();
double timeFromCreation(int x);

#endif