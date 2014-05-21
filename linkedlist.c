/*Linked List struct used for storing a character pointers*/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <errno.h>

struct node
{
	char *elem;
	struct node *next;
};

void printList(struct node *head)
{
	struct node *cur = head;
	while(cur!=NULL)
	{
		printf(":%s", cur->elem);
		//printf(":%p", cur->next);
		cur = cur->next;
	}
}

struct node *addNode(struct node *head, char *key)
{
	struct node *newnode = (struct node *)malloc(sizeof(struct node));
	assert(newnode!=NULL);
	newnode->elem = strdup(key);

	newnode->next = head;

	return newnode;
}

struct node *deleteNode(struct node *head, char *key)
{
	struct node *cur = head;

	while(cur->next!=NULL)				//check each node except head for matching key
	{
		if(strcmp((cur->next)->elem, key) == 0)
		{
			struct node *delnode = cur->next;
			cur->next = (cur->next)->next;
			free(delnode->elem);
			free(delnode);
		}
		else
		{
			cur = cur->next;
		}
	}
	cur = head;							//reset cur to head of list
	if(strcmp(cur->elem, key) == 0)		//if head matches key, delete head
	{
		struct node *delnode = cur;
		cur = cur->next;
		free(delnode->elem);
		free(delnode);
	}
	return cur;
}

void freeList(struct node *head)
{
	struct node *delnode = head;
	while(delnode!=NULL)
	{
		struct node *nextnode = delnode->next;
		free(delnode->elem);
		free(delnode);
		delnode = nextnode;
	}
}