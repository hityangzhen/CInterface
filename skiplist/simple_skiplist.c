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
typedef enum {false = 0, true = 1} bool;

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
 * create a node 
 */
skiplist_node *create_skiplist_node(int level,int key,int value)
{
	skiplist_node *node=(skiplist_node *)malloc(sizeof(skiplist_node)+
		level*sizeof(skiplist_node *));
	if(node==NULL)
		return NULL;
	node->key=key;
	node->value=value;
	return node;
}

/**
 * skip list initialized 
 */
skiplist *create_skiplist()
{
	skiplist *list=(skiplist *)malloc(sizeof(skiplist));
	if(list==NULL)
		return NULL;
	// initial level is 0
	list->level=0;
	list->length=0;
	// create a header skiplist node
	list->header=create_skiplist_node(MAX_LEVEL-1,0,0); 
	memset(list->header->forward,0,MAX_LEVEL);
	return list;
}

int random_level()
{
	int k=1;
	while(rand()%2)
		k++;
	k=(k<MAX_LEVEL)?k:MAX_LEVEL;
	return k;
}

/**
 * step 1:find the position 
 * step 2:product a random level
 * step 3:from the higher level to low level insert 
 */
bool insert(skiplist *list,int key,int value)
{
	DEBUG_ASSERT(list);
	// keep previous node in each level
	skiplist_node *update[MAX_LEVEL];
	skiplist_node *p,*q=NULL;
	p=list->header;
	int k=list->level,i;

	for(i=k-1;i>=0;i--) {
		// loop over the skiplist
		while((q=p->forward[i]) && q->key<key)
			p=q;
		// suitable position in each level
		update[i]=p;
	}
	// below the lowest level is the key
	// disallow the same key
	if(q && q->key==key)
		return false;

	// get the new skiplist node's level
	k=random_level();

	// update the skiplist's level which is greater than prior
	if(k>list->level) {
		for(i=list->level;i<k;i++)
			update[i]=list->header;
		list->level=k;
	}
	q=create_skiplist_node(k,key,value);
	if(q==NULL) return false;

	for(i=0;i<k;i++) {
		q->forward[i]=update[i]->forward[i];
		update[i]->forward[i]=q;
	}
	list->length+=1;
	return true;
}

/**
 * delete skiplist node 
 */
bool delete(skiplist *list,int key)
{
	DEBUG_ASSERT(list);
	skiplist_node *update[MAX_LEVEL];
	skiplist_node *p,*q=NULL;
	p=list->header;
	int k=list->level,i;

	for(i=k-1;i>=0;i--) {
		while((q=p->forward[i]) && (q->key<key))
			p=q;
		update[i]=p;
	}

	if(q && q->key==key) {
		for(i=0;i<list->level;i++) {
			if(update[i]->forward[i]==q)
				update[i]->forward[i]=q->forward[i];
		}

		free(q);
		// whether update the highest level node
		for(i=list->level-1;i>=0;i--) {
			if(list->header->forward[i]==NULL)
			list->level--;	
		}
		list->length-=1;
		return true;
	}else
		return false;
}

/**
 * find the key's value
 */
int search(skiplist *list,int key)
{
	DEBUG_ASSERT(list);
	skiplist_node *p,*q=NULL;
	p=list->header;
	int k=list->level,i;

	for(i=k-1;i>=0;i--) {
		while((q=p->forward[i]) && q->key<=key) {
			if(q->key==key) {
				return q->value;
			}
			p=q;
		}
	}
	return INT_MIN;
}

/**
 * print the skiplist
 *  not formated
 */
void print(skiplist *list)
{
	DEBUG_ASSERT(list);
	skiplist_node *p,*q=NULL;
	int k=list->level,i;
	for(i=k-1;i>=0;i--) {
		p=list->header;
		while((q=p->forward[i])) {
			printf("%d -> ",q->value);
			p=q;
		}
		printf("\n");
	}
	printf("\n");
}

void free_skiplist(skiplist *list)
{
	DEBUG_ASSERT(list);
	skiplist_node *p=list->header,*q=NULL;
	while(p) {
		q=p->forward[0];
		free(p);
		p=q;
	}
	free(list);
}

/**
 * test
 */
int main()
{
	skiplist *list=create_skiplist();
	int i;
	for(i=0;i<=8;i++)
		insert(list,i,i*2);
	print(list);
	delete(list,1);
	print(list);
	delete(list,3);
	insert(list,i,i*2);
	print(list);
	free_skiplist(list);
	return 0;
}




