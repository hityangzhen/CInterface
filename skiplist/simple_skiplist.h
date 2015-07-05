#ifndef SIMPLE_SKIPLIST_H
#define SIMPLE_SKIPLIST_H
/**
 * simple_skiplist: only available for integer
 * version 1.0
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <assert.h>

#ifdef DEBUG
	#define DEBUG_ASSERT(condition) do{} while(0)
#else
	#define DEBUG_ASSERT(condition) assert(condition)
#endif


#define MAX_LEVEL 10 // maximum number of level

typedef enum {
	false = 0, 
	true = 1
} bool;

typedef struct skiplist_node {
	int key;
	int value;
	struct skiplist_node *forward[1]; //store forward pointers
} skiplist_node;

typedef struct skiplist {
	int level;
	skiplist_node *header;
	int length;
} skiplist;

/**
 * skip list initialized 
 */
skiplist *create_skiplist();

/**
 * free the skiplist
 */
void free_skiplist(skiplist *list);

/**
 * create a node 
 */
skiplist_node *create_skiplist_node(int level,int key,int value);

/**
 * insert a skiplist node
 */
bool insert(skiplist *list,int key,int value);

/**
 * delete skiplist node 
 */
bool delete(skiplist *list,int key);

/**
 * find the key's value
 */
int search(skiplist *list,int key);

/**
 * print the skiplist
 */
void print(skiplist *list);

#endif



