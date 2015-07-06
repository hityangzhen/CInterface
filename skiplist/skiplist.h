#ifndef SIMPLE_SKIPLIST_H
#define SIMPLE_SKIPLIST_H
/**
 * skiplist:
 * version 2.0
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


#define MAX_LEVEL 32 // maximum number of level

typedef enum {
	false = 0, 
	true = 1
} bool;

typedef struct skiplist_node {
	double score;
	void *obj;
	struct skiplist_node *backward; //backword pointer
	int levels;
	struct skiplist_node_level {
		struct skiplist_node *forward; //forward pointer
		unsigned int span; //span from the previous node
	} level[1];
} skiplist_node;

typedef void (*free_t)(void *);
typedef int (*cmp_t)(void *,void *);

typedef struct skiplist {
	int level;
	skiplist_node *header,*tail;
	int length;

	free_t free; //obj free handler
	cmp_t cmp; //obj cmp handler
} skiplist;

/**
 * skip list initialized 
 */
skiplist *skiplist_create(free_t f,cmp_t c);

/**
 * free the skiplist
 */
void skiplist_destroy(skiplist *list);

/**
 * insert a skiplist node
 */
bool skiplist_insert(skiplist *list,double score,void *obj);

/**
 * delete skiplist nodes
 * -if obj is not null, delete corresponding node
 * -otherwise, delete the nodes sharing the same score
 * -return the number of deleted nodes
 */
int skiplist_delete(skiplist *list,double score,void *obj);

/**
 * find skiplist nodes
 * -if obj is not null, return corresponding node
 * -otherwise, return all nodes sharing the same score
 */
void skiplist_find(skiplist *list,double score,void *obj,int *n,void ***result);

/**
 * print the skiplist
 */
void skiplist_print(skiplist *list);

#endif
