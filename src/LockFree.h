#include"header.h"
#include"helper.h"
#include"subFunctions.h"

__inline bool search(struct tArgs* t, unsigned long key)
{
	struct node* node;
	unsigned long nKey;
	
	seek(t,key,&t->targetRecord);
	node = t->targetRecord.lastEdge.child;
	nKey = getKey(node->markAndKey);
	if(nKey == key)
	{
		t->successfulReads++;
		return(true);
	}
	else
	{
		t->unsuccessfulReads++;
		return(false);
	}	
}

bool insert(struct tArgs* t, unsigned long key)
{
	struct node* node;
	struct node* newNode;
	struct node* address;
	unsigned long nKey;
	bool result;
	int which;
	struct node* temp;
	
	while(true)
	{
		seek(t,key,&t->targetRecord);
		node = t->targetRecord.lastEdge.child;
		nKey = getKey(node->markAndKey);
		if(nKey == key)
		{
			t->unsuccessfulInserts++;
			return(false);
		}
		
		//create a new node and initialize its fields
		if(!t->isNewNodeAvailable)
		{
			t->newNode = newLeafNode(key);
			t->isNewNodeAvailable = true;
		}
		newNode = t->newNode;
		newNode->markAndKey = key;
		which = t->targetRecord.injectionEdge.which;
		address = t->targetRecord.injectionEdge.child;
		result = CAS(node,which,setNull(address),newNode);
		if(result)
		{
			t->isNewNodeAvailable = false;
			t->successfulInserts++;
			return(true);
		}
		t->insertRetries++;
		
		temp = node->child[which];
		if(isDFlagSet(temp))
		{
			helpTargetNode(t,&t->targetRecord.lastEdge,1);
		}
		else if(isPFlagSet(temp))
		{
			helpSuccessorNode(t,&t->targetRecord.lastEdge,1);
		}
	}
}

bool remove(struct tArgs* t, unsigned long key)
{
	struct stateRecord* myState;
	struct seekRecord* targetRecord;
	struct edge targetEdge;
	struct edge pTargetEdge;
	unsigned long nKey;
	bool result;
	
	//initialize the state record
	myState = &t->myState;
	myState->depth = 0;
	myState->targetKey = key;
	myState->currentKey = key;
	myState->mode = INJECTION;
	targetRecord = &t->targetRecord;
	
	while(true)
	{
		seek(t,myState->currentKey,targetRecord);
		targetEdge = targetRecord->lastEdge;
		pTargetEdge = targetRecord->pLastEdge;
		nKey = getKey(targetEdge.child->markAndKey);
		if(myState->currentKey != nKey)
		{
			//the key does not exist in the tree
			if(myState->mode == INJECTION)
			{
				t->unsuccessfulDeletes++;
				return(false);
			}
			else
			{
				t->successfulDeletes++;
				return(true);
			}
		}
		//perform appropriate action depending on the mode
		if(myState->mode == INJECTION)
		{
			//store a reference to the target node
			myState->targetEdge = targetEdge;
			myState->pTargetEdge = pTargetEdge;
			//attempt to inject the operation at the node
			inject(t,myState);
		}
		//mode would have changed if the operation was injected successfully
		if(myState->mode != INJECTION)
		{
			//if the target node found by the seek function is different from the one stored in state record, then return
			if(myState->targetEdge.child != targetEdge.child)
			{
				t->successfulDeletes++;
				return(true);
			}
			//update the target edge using the most recent seek
			myState->targetEdge = targetEdge;
		}
		if(myState->mode == DISCOVERY)
		{
			findAndMarkSuccessor(t,myState);
		}
		if(myState->mode == DISCOVERY)
		{
			removeSuccessor(t,myState);
		}
		if(myState->mode == CLEANUP)
		{
			result = cleanup(t,myState);
			if(result)
			{
				t->successfulDeletes++;
				return(true);
			}
			else
			{
				nKey = getKey(targetEdge.child->markAndKey);
				myState->currentKey = nKey;
			}
		}
	}
}