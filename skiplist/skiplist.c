#include "skiplist.h"

#define ARRAY_SIZE 4
#define PAGE_SIZE 1024
typedef struct s_array {
	int size;
	int match;
	void **objs;
} s_array;

static s_array *s_array_create()
{
	s_array *array=(s_array *)malloc(sizeof(*array));
	array->size=ARRAY_SIZE;
	array->match=0;
	array->objs=(void **)malloc(sizeof(void *) * array->size);
	return array;
}

static bool s_array_add(s_array *array,void *obj,bool flag)
{
	DEBUG_ASSERT(array);
	if(!flag) {
		array->match++;
		return true;
	}
	array->objs[array->match++]=obj;
	// need expand
	if(array->match>=array->size) {
		void **new_objs=(void **)malloc(sizeof(void *)*array->match*2);
		if(new_objs==NULL)
			return false;
		array->size=array->match*2;
		int i;
		for(i=0;i<array->match;i++)
			new_objs[i]=array->objs[i];
		free(array->objs);
		array->objs=new_objs;
	}
	return true;
}

/**
 * create a node 
 */
static skiplist_node *create_skiplist_node(int level,double score,void *obj)
{
	DEBUG_ASSERT(obj);
	skiplist_node *node=(skiplist_node *)malloc(sizeof(skiplist_node)+
		level*sizeof(struct skiplist_node_level));
	if(node==NULL)
		return NULL;
	node->score=score;
	node->obj=obj;
	node->levels=level;
	return node;
}

skiplist *skiplist_create(free_t f,cmp_t c)
{
	skiplist *list=(skiplist *)malloc(sizeof(skiplist));
	if(list==NULL)
		return NULL;
	// initial level is 0
	list->level=0;
	list->length=0;
	list->free=f;
	list->cmp=c;
	// create a header skiplist node
	list->header=create_skiplist_node(MAX_LEVEL-1,0,0); 
	list->tail=NULL;
	memset(list->header->level,0,MAX_LEVEL*sizeof(struct skiplist_node_level));
	return list;
}

static int random_level()
{
	int k=1;
	while(rand()%2)
		k++;
	k=(k<MAX_LEVEL)?k:MAX_LEVEL;
	return k;
}

/**
 * nodes sharing the same score
 */
static skiplist_node **skiplist_find_all(skiplist *list,double score,int *n)
{
	DEBUG_ASSERT(list);
	skiplist_node *p=NULL;
	skiplist_node **result;
	p=list->header;
	int k=list->level,i;

	s_array *array=s_array_create();
	for(i=k-1;i>=0;i--) {
		while(p->level[i].forward && p->level[i].forward->score<=score) {
			if(score==p->level[i].forward->score)
				s_array_add(array,p->level[i].forward,true);
			p=p->level[i].forward;
		}
	}
	*n=array->match;
	if(array->match==0)
		return NULL;
	result=(skiplist_node **)array->objs;
	if(array) {
		if(array->match==0)
			free(array->objs);
		free(array);
	}
	return result;
}

void skiplist_find(skiplist *list,double score,void *obj,int *n,void ***result)
{
	DEBUG_ASSERT(list);
	int match;
	skiplist_node **res=skiplist_find_all(list,score,&match);
	if(match==0) {
		*n=0;
		return ;
	}
	// filter nodes sharing the same score
	s_array *array=s_array_create();
	skiplist_node *tmp;
	int i;
	for(i=0;i<match;i++) {
		tmp=res[i];

		if(obj) {
			if(list->cmp) {
				if(list->cmp(tmp->obj,obj)==0)
					s_array_add(array,tmp->obj,result!=NULL);
				else
					if(tmp->obj==obj) s_array_add(array,tmp->obj,result!=NULL);
			}
		}
		else
			s_array_add(array,tmp->obj,result!=NULL);
	}

	*n=array->match;
	if(result)
		*result=array->objs;
	if(array) {
		if(array->match==0)
			free(array->objs);
		free(array);
	}
}

static void skiplist_get_update(skiplist *list,skiplist_node *node,
	skiplist_node ***update)
{
	DEBUG_ASSERT(list && node);
	skiplist_node **new_update=(skiplist_node **)malloc(sizeof(skiplist_node *) *
		(MAX_LEVEL));
	if(new_update==NULL) {
		*update=NULL;
		return ;
	}
		
	skiplist_node *tmp=list->header;
	int i;
	for(i=0;i<MAX_LEVEL;i++)
		new_update[i]=tmp;
	// record the insert position
	for(i=list->level-1;i>=0;i--) {
		while(tmp->level[i].forward && tmp->level[i].forward->score<=node->score)
			tmp=tmp->level[i].forward;
		new_update[i]=tmp;
	}
	*update=new_update;
}

static void skiplist_get_update2(skiplist *list,skiplist_node *node,
	skiplist_node ***update)
{
	DEBUG_ASSERT(list && node);
	skiplist_node **new_update=(skiplist_node **)malloc(sizeof(skiplist_node *) *
		(MAX_LEVEL));
	if(new_update==NULL) {
		*update=NULL;
		return ;
	}
	skiplist_node *tmp=list->header;
	int i;
	for(i=0;i<MAX_LEVEL;i++)
		new_update[i]=tmp;

	for(i=list->level-1;i>=0;i--) {
		while(tmp->level[i].forward && tmp->level[i].forward->score<=node->score) {
			if(tmp->level[i].forward->score==node->score) {
				if(list->cmp) {
					if(list->cmp(node->obj,tmp->level[i].forward->obj)==0)
						break;
				}
				else if(node->obj==tmp->level[i].forward->obj)
					break;
			}
			tmp=tmp->level[i].forward;
		}
		new_update[i]=tmp;
	}

	*update=new_update;
}

bool skiplist_insert(skiplist *list,double score,void *obj)
{
	DEBUG_ASSERT(list && obj);
	int match;
	skiplist_find(list,score,obj,&match,NULL);
	if(match>0) return false;

	skiplist_node **update;
	int i;
	int level=random_level();

	skiplist_node *node=create_skiplist_node(level,score,obj);
	if(node==NULL)
		return false;
	skiplist_get_update(list,node,&update);
	if(update==NULL)
		return false;

	for(i=level-1;i>=0;i--) {
		node->level[i].forward=update[i]->level[i].forward;
		update[i]->level[i].forward=node;
	}
	// update the backward pointer
	node->backward=(update[0]==list->header?NULL:update[0]);
	// update the backward pointer of the forward node
	if(node->level[0].forward)
		node->level[0].forward->backward=node;
	else
		list->tail=node;

	// update the maximux level of the skiplist
	if(list->level<level) list->level=level;
	list->length+=1;
	// free the temporary memory
	free(update);
	return true;
}

static void skiplist_delete_node(skiplist *list,skiplist_node *node)
{
	DEBUG_ASSERT(list && node);
	skiplist_node *p=node->backward;
	skiplist_node *q=node->level[0].forward;

	skiplist_node **update;
	skiplist_get_update2(list,node,&update);
	int i;
	// update the previous node's forward pointers
	for(i=node->levels-1;i>=0;i--)
		update[i]->level[i].forward=node->level[i].forward;
	// update the forward node's backward pointer
	if(q)
		q->backward=p;
	else
		list->tail=p;
	// free the temporary memory
	free(update);

	// update the maximum level of the skiplist
	if(node->levels>=list->level) {
		skiplist_node *tmp=list->header;
		for(i=list->level-1;i>=0;i--)
			if(tmp->level[i].forward==NULL)
				list->level--;
	}

	list->length-=1;
	if(list->free) list->free(node->obj);
	//free the skiplist node
	free(node);
}

int skiplist_delete(skiplist *list,double score,void *obj)
{
	DEBUG_ASSERT(list);
	int match;
	int length=list->length;
	// find all nodes sharing the same score
	skiplist_node **nodes=skiplist_find_all(list,score,&match);
	int i;
	for(i=0;i<match;i++) {
		if(obj) {
			if(list->cmp) {
				if(list->cmp(nodes[i]->obj,obj)==0)
					skiplist_delete_node(list,nodes[i]);
			}
			else
				if(nodes[i]->obj==obj)
					skiplist_delete_node(list,nodes[i]);	
		}
		else
			skiplist_delete_node(list,nodes[i]);
	}
	// free the temporary matched nodes
	free(nodes);
	return length-list->length;
}

void skiplist_print(skiplist *list)
{
	DEBUG_ASSERT(list);
	skiplist_node *tmp=list->header;
	printf("skiplist: [lenght:%d],[max_level:%d]\n",list->length,list->level);
	int i=1;
	while(tmp->level[0].forward) {
		printf("node%d,[score:%f],[level:%d]\n",i++,tmp->level[0].forward->score,
			tmp->level[0].forward->levels);
		tmp=tmp->level[0].forward;
	}
}

void skiplist_destroy(skiplist *list)
{
	DEBUG_ASSERT(list);
	skiplist_node *start=list->header->level[0].forward,*next;
	while(start) {
		next=start->level[0].forward;
		if(list->free) list->free(start->obj);
		free(start);
		start=next;
	}
	//
	free(list->header);
	free(list);
}

#ifdef DEBUG
/**
 * test
 */

typedef struct obj {
	int id;
	char *name;
} obj;

int cmp(void *obj1,void *obj2)
{
	obj *o1=(obj *)obj1;
	obj *o2=(obj *)obj2;
	return (o1->id==o1->id && o1->name==o2->name)?0:-1;
}

int main()
{
	skiplist *list=skiplist_create(NULL,NULL);
	int a[8]={0,2,4,6,8,10,12,14};
	int i;
	for(i=0;i<8;i++)
		skiplist_insert(list,i,&a[i]);
	skiplist_print(list);
	skiplist_delete(list,1,NULL);
	skiplist_print(list);
	skiplist_delete(list,3,NULL);
	skiplist_print(list);
	skiplist_destroy(list);

	obj o1={1,"o1"};
	obj o2={2,"o2"};
	obj o3={3,"o3"};
	list=skiplist_create(NULL,cmp);
	skiplist_insert(list,o1.id,&o1);
	skiplist_insert(list,o2.id,&o2);
	skiplist_insert(list,o3.id,&o3);
	skiplist_print(list);
	skiplist_delete(list,2,&o2);
	obj o4={3,"o4"};
	skiplist_delete(list,3,&o4);
	skiplist_print(list);
	skiplist_destroy(list);	
	return 0;
}
#endif




















