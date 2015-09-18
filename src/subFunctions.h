__inline void seek(struct tArgs*, unsigned long, struct seekRecord*);
__inline void initializeTypeAndUpdateMode(struct tArgs*, struct stateRecord*);
__inline void updateMode(struct tArgs*, struct stateRecord*);
__inline void inject(struct tArgs*, struct stateRecord*);
__inline void findSmallest(struct tArgs*, struct node*, struct seekRecord*);
__inline void findAndMarkSuccessor(struct tArgs*, struct stateRecord*);
__inline void removeSuccessor(struct tArgs*, struct stateRecord*);
__inline bool cleanup(struct tArgs*, struct stateRecord*);
__inline bool markChildEdge(struct tArgs*, struct stateRecord*, bool);
__inline void helpTargetNode(struct tArgs*, struct edge*, int);
__inline void helpSuccessorNode(struct tArgs*, struct edge*, int);

void populateEdge(struct edge* e, struct node* parent, struct node* child, int which)
{
	e->parent = parent;
	e->child = child;
	e->which = which;
}

void copyEdge(struct edge* d, struct edge* s)
{
	d->parent = s->parent;
	d->child = s->child;
	d->which = s->which;
}

void copySeekRecord(struct seekRecord* d, struct seekRecord* s)
{
	copyEdge(&d->lastEdge,&s->lastEdge);
	copyEdge(&d->pLastEdge,&s->pLastEdge);
	copyEdge(&d->injectionEdge,&s->injectionEdge);
}

__inline void seek(struct tArgs* t,unsigned long key, struct seekRecord* s)
{
	struct anchorRecord* pAnchorRecord;
	struct anchorRecord* anchorRecord;
	
	struct edge pLastEdge;
	struct edge lastEdge;
	
	struct node* curr;
	struct node* next;
	struct node* temp;
	int which;
	
	bool n;
	bool d;
	bool p;
	unsigned long cKey;
	unsigned long aKey;
	
	pAnchorRecord = &t->pAnchorRecord;
	anchorRecord = &t->anchorRecord;
	
	pAnchorRecord->node = S;
	pAnchorRecord->key = INF_S;
	
	while(true)
	{
		//initialize all variables used in traversal
		populateEdge(&pLastEdge,R,S,RIGHT);
		populateEdge(&lastEdge,S,T,RIGHT);
		curr = T;
		anchorRecord->node = S;
		anchorRecord->key = INF_S;
		while(true)
		{
			t->seekLength++;
			//read the key stored in the current node
			cKey = getKey(curr->markAndKey);
			//find the next edge to follow
			which = key<cKey ? LEFT:RIGHT;
			temp = curr->child[which];
			n=isNull(temp);	d=isDFlagSet(temp);	p=isPFlagSet(temp);	next=getAddress(temp);
			//check for completion of the traversal
			if(key == cKey || n)
			{
				//either key found or no next edge to follow. Stop the traversal
				s->pLastEdge = pLastEdge;
				s->lastEdge = lastEdge;
				populateEdge(&s->injectionEdge,curr,next,which);
				if(key == cKey)
				{
					//key matches. So return
					return;					
				}
				else
				{
					break;
				}
			}
			if(which == RIGHT)
			{
				//the next edge that will be traversed is a right edge. Keep track of the current node and its key
				anchorRecord->node = curr;
				anchorRecord->key = cKey;
			}
			//traverse the next edge
			pLastEdge = lastEdge;
			populateEdge(&lastEdge,curr,next,which);
			curr = next;			
		}
		//key was not found. check if can stop
		aKey = getKey(anchorRecord->node->markAndKey);
		if(anchorRecord->key == aKey)
		{
			temp = anchorRecord->node->child[RIGHT];
			d=isDFlagSet(temp);	p=isPFlagSet(temp);
			if(!d && !p)
			{
				//the anchor node is part of the tree. Return the results of the current traversal
				return;
			}
			if(pAnchorRecord->node == anchorRecord->node && pAnchorRecord->key == anchorRecord->key)
			{
				//return the results of previous traversal
				copySeekRecord(s,&t->pSeekRecord);
				return;
			}
		}
		//store the results of the current traversal and restart
		copySeekRecord(&t->pSeekRecord,s);
		pAnchorRecord->node = anchorRecord->node;
		pAnchorRecord->key = anchorRecord->key;
		t->seekRetries++;
	}
}

__inline void initializeTypeAndUpdateMode(struct tArgs* t,struct stateRecord* state)
{
	struct node* node;
	//retrieve the address from the state record
	node = state->targetEdge.child;
	if(isNull(node->child[LEFT]) || isNull(node->child[RIGHT]))
	{
		//one of the child pointers is null
		if(isKeyMarked(node->markAndKey))
		{
			state->type = COMPLEX;
		}
		else
		{
			state->type = SIMPLE;
		}
	}
	else
	{
		//both the child pointers are non-null
		state->type= COMPLEX;
	}
	updateMode(t,state);
}

__inline void updateMode(struct tArgs* t,struct stateRecord* state)
{
	struct node* node;
	//retrieve the address from the state record
	node = state->targetEdge.child;
	
	//update the operation mode
	if(state->type == SIMPLE)
	{
		//simple delete
		state->mode = CLEANUP;
	}
	else
	{
		//complex delete
		if(node->readyToReplace)
		{
			assert(isKeyMarked(node->markAndKey));
			state->mode = CLEANUP;
		}
		else
		{
			state->mode = DISCOVERY;
		}
	}
	return;
}

__inline void inject(struct tArgs* t,struct stateRecord* state)
{
	struct node* parent;
	struct node* node;
	struct edge targetEdge;
	int which;
	bool result;
	bool i;
	bool d;
	bool p;
	struct node* temp;
	
	targetEdge = state->targetEdge;
	parent = targetEdge.parent;
	node = targetEdge.child;
	which = targetEdge.which;
	
	result = CAS(parent,which,node,setIFlag(node));
	if(!result)
	{
		//unable to set the intention flag on the edge. help if needed
		temp = parent->child[which];
		i=isIFlagSet(temp);	d=isDFlagSet(temp);	p=isPFlagSet(temp);
		
		if(i)
		{
			helpTargetNode(t,&targetEdge,1);
		}
		else if(d)
		{
			helpTargetNode(t,&state->pTargetEdge,1);
		}
		else if(p)
		{
			helpSuccessorNode(t,&state->pTargetEdge,1);
		}
		return;
	}
	//mark the left edge for deletion
	result = markChildEdge(t,state,LEFT);
	if(!result)
	{
		return;
	}
	//mark the right edge for deletion
	result = markChildEdge(t,state,RIGHT);
	
	//initialize the type and mode of the operation
	initializeTypeAndUpdateMode(t,state);
	return;
}

__inline bool markChildEdge(struct tArgs* t, struct stateRecord* state, bool which)
{
	struct node* node;
	struct edge edge;
	Flag flag;
	struct node* address;
	struct node* temp;
	bool n;
	bool i;
	bool d;
	bool p;
	struct edge helpeeEdge;
	struct node* oldValue;
	struct node* newValue;
	bool result;
	
	if(state->mode == INJECTION)
	{	
		edge = state->targetEdge;
		flag = DELETE_FLAG;
	}
	else
	{
		edge = state->successorRecord.lastEdge;
		flag = PROMOTE_FLAG;
	}
	node = edge.child;
	while(true)
	{
		temp = node->child[which];
		n=isNull(temp);	i=isIFlagSet(temp);	d=isDFlagSet(temp);	p=isPFlagSet(temp);	address=getAddress(temp);
		if(i)
		{
			populateEdge(&helpeeEdge,node,address,which);
			helpTargetNode(t,&helpeeEdge,state->depth+1);
			continue;
		}
		else if(d)
		{
			if(flag == PROMOTE_FLAG)
			{
				helpTargetNode(t,&edge,state->depth+1);
				return false;
			}
			else
			{
				return true;
			}
		}
		else if(p)
		{
			if(flag == DELETE_FLAG)
			{
				helpSuccessorNode(t,&edge,state->depth+1);
				return false;
			}
			else
			{
				return true;
			}
		}
		if(n)
		{
			oldValue = setNull(address);
		}
		else
		{
			oldValue = address;
		}
		if(flag == DELETE_FLAG)
		{
			newValue = setDFlag(oldValue);
		}
		else
		{
			newValue = setPFlag(oldValue);
		}
		result = CAS(node,which,oldValue,newValue);
		if(!result)
		{
			continue;
		}
		else
		{
			break;
		}
	}
	return true;
}

__inline void findSmallest(struct tArgs* t,struct node* node, struct seekRecord* s)
{
	struct node* curr;
	struct node* left;
	struct node* right;
	struct node* temp;
	bool n;
	struct edge lastEdge;
	struct edge pLastEdge;
	
	//find the node with the smallest key in the subtree rooted at the right child
	//initialize the variables used in the traversal
	right = getAddress(node->child[RIGHT]);
	populateEdge(&lastEdge,node,right,RIGHT);
	populateEdge(&pLastEdge,node,right,RIGHT);
	while(true)
	{
		curr = lastEdge.child;
		temp = curr->child[LEFT];
		n=isNull(temp);	left=getAddress(temp);
		if(n)
		{
			break;
		}
		//traverse the next edge
		pLastEdge = lastEdge;
		populateEdge(&lastEdge,curr,left,LEFT);
	}
	//initialize seek record and return
	s->lastEdge = lastEdge;
	s->pLastEdge = pLastEdge;
	return;
}

__inline void findAndMarkSuccessor(struct tArgs* t, struct stateRecord* state)
{
	struct node* node;
	struct seekRecord* s;
	struct edge successorEdge;
	bool m;
	bool n;
	bool d;
	bool p;
	bool result;
	struct node* temp;
	struct node* left;
	
	//retrieve the addresses from the state record
	node = state->targetEdge.child;
	s = &state->successorRecord;
	while(true)
	{
		//read the mark flag of the key in the target node
		m=isKeyMarked(node->markAndKey);
		//find the node with the smallest key in the right subtree
		findSmallest(t,node,s);
		if(m)
		{
			//successor node has already been selected before the traversal
			break;
		}
		//retrieve the information from the seek record
		successorEdge = s->lastEdge;
		temp = successorEdge.child->child[LEFT];
		n=isNull(temp);	p=isPFlagSet(temp);	left=getAddress(temp);
		if(!n)
		{
			continue;
		}
		//read the mark flag of the key under deletion
		m=isKeyMarked(node->markAndKey);
		if(m)
		{
			//successor node has already been selected
			if(p)
			{
				break;
			}
			else
			{
				continue;
			}
		}
		//try to set the promote flag on the left edge
		result = CAS(successorEdge.child,LEFT,setNull(left),setPFlag(setNull(node)));
		if(result)
		{
			break;
		}
		//attempt to mark the edge failed; recover from the failure and retry if needed
		temp = successorEdge.child->child[LEFT];
		n = isNull(temp); d = isDFlagSet(temp); p = isPFlagSet(temp);
		if(p)
		{	
			break;
		}
		if(!n)
		{
			//the node found has since gained a left child
			continue;
		}
		if(d)
		{
			//the node found is undergoing deletion; need to help
			helpTargetNode(t,&s->lastEdge,state->depth+1);
		}
	}
	// update the operation mode
	updateMode(t,state);
	return;
}

__inline void removeSuccessor(struct tArgs* t, struct stateRecord* state)
{
	struct node* node;
	struct seekRecord* s;
	struct edge successorEdge;
	struct edge pLastEdge;
	struct node* temp;
	struct node* right;
	struct node* address;
	struct node* oldValue;
	struct node* newValue;
	bool n;
	bool d;
	bool p;
	bool i;
	bool dFlag;
	bool which;
	bool result;
	
	//retrieve addresses from the state record
	node = state->targetEdge.child;
	s = &state->successorRecord;
  findSmallest(t,node,s);
	//extract information about the successor node
	successorEdge = s->lastEdge;
	//ascertain that the seek record for the successor node contains valid information
	temp = successorEdge.child->child[LEFT];
	p=isPFlagSet(temp);	address=getAddress(temp);
	if(address!=node)
	{
		node->readyToReplace = true;
		updateMode(t,state);
		return;
	}
	if(!p)
	{
		node->readyToReplace = true;
		updateMode(t,state);
		return;
	}

	//mark the right edge for promotion if unmarked
	temp = successorEdge.child->child[RIGHT];
	p=isPFlagSet(temp);
	if(!p)
	{
		//set the promote flag on the right edge
		markChildEdge(t,state,RIGHT);
	}
	//promote the key
	node->markAndKey = setReplaceFlagInKey(successorEdge.child->markAndKey);
	while(true)
	{
		//check if the successor is the right child of the target node itself
		if(successorEdge.parent == node)
		{
			dFlag = true; which = RIGHT;
		}
		else
		{
			dFlag = false; which = LEFT;
		}
		i=isIFlagSet(successorEdge.parent->child[which]);
		temp = successorEdge.child->child[RIGHT];
		n=isNull(temp);	right=getAddress(temp);
		if(n)
		{
			//only set the null flag. do not change the address
			if(i)
			{
				if(dFlag)
				{
					oldValue = setIFlag(setDFlag(successorEdge.child));
					newValue = setNull(setDFlag(successorEdge.child));
				}
				else
				{
					oldValue = setIFlag(successorEdge.child);
					newValue = setNull(successorEdge.child);
				}
			}
			else
			{
				if(dFlag)
				{
					oldValue = setDFlag(successorEdge.child);
					newValue = setNull(setDFlag(successorEdge.child));
				}
				else
				{
					oldValue = successorEdge.child;
					newValue = setNull(successorEdge.child);
				}
			}
			result = CAS(successorEdge.parent,which,oldValue,newValue);
		}
		else
		{
			if(i)
			{
				if(dFlag)
				{
					oldValue = setIFlag(setDFlag(successorEdge.child));
					newValue = setDFlag(right);
				}
				else
				{
					oldValue = setIFlag(successorEdge.child);
					newValue = right;
				}
			}
			else
			{
				if(dFlag)
				{
					oldValue = setDFlag(successorEdge.child);
					newValue = setDFlag(right);
				}
				else
				{
					oldValue = successorEdge.child;
					newValue = right;
				}
			}
			result = CAS(successorEdge.parent,which,oldValue,newValue);
		}
		if(result)
		{
			break;
		}	
		if(dFlag)
		{
			break;
		}
		temp=successorEdge.parent->child[which];
		d=isDFlagSet(temp);
		pLastEdge = s->pLastEdge;
		if(d && pLastEdge.parent != NULL)
		{
			helpTargetNode(t,&pLastEdge,state->depth+1);
		}
		findSmallest(t,node,s);
		if(s->lastEdge.child != successorEdge.child)
		{
			//the successor node has already been removed
			break;
		}
		else
		{
			successorEdge = s->lastEdge;
		}
	}
	node->readyToReplace = true;
	updateMode(t,state);
	return;
}

__inline bool cleanup(struct tArgs* t, struct stateRecord* state)
{
	struct node* parent;
	struct node* node;
	struct node* newNode;
	struct node* left;
	struct node* right;
	struct node* address;
	struct node* temp;
	bool pWhich;
	bool nWhich;
	bool result;
	bool n;
	
	//retrieve the addresses from the state record
	parent = state->targetEdge.parent;
	node = state->targetEdge.child;
	pWhich = state->targetEdge.which;
		
	if(state->type == COMPLEX)
	{
		//replace the node with a new copy in which all the fields are unmarked
		newNode = (struct node*) malloc(sizeof(struct node));
		newNode->markAndKey = getKey(node->markAndKey);
		newNode->readyToReplace = false;
		left = getAddress(node->child[LEFT]);
		newNode->child[LEFT] = left;
		temp=node->child[RIGHT];
		n=isNull(temp);	right=getAddress(temp);
		if(n)
		{
			newNode->child[RIGHT] = setNull(NULL);
		}
		else
		{
			newNode->child[RIGHT] = right;
		}
		
		//switch the edge at the parent
		result = CAS(parent,pWhich,setIFlag(node),newNode);
	}
	else
	{
		//remove the node. determine to which grand child will the edge at the parent be switched
		if(isNull(node->child[LEFT]))
		{
			nWhich = RIGHT;
		}
		else
		{
			nWhich = LEFT;
		}
		temp = node->child[nWhich];
		n = isNull(temp); address = getAddress(temp);
		if(n)
		{
			//set the null flag only; do not change the address
			result = CAS(parent,pWhich,setIFlag(node),setNull(node));
		}
		else
		{
			result = CAS(parent,pWhich,setIFlag(node),address);
		}
	}
	return result;	
}

__inline void helpTargetNode(struct tArgs* t, struct edge* helpeeEdge, int depth)
{
	struct stateRecord* state;
	bool result;
	// intention flag must be set on the edge
	// obtain new state record and initialize it
	state = (struct stateRecord*) malloc(sizeof(struct stateRecord));
	state->targetEdge = *helpeeEdge;
	state->depth = depth;
	state->mode = INJECTION;
	
	// mark the left and right edges if unmarked
	result = markChildEdge(t,state, LEFT);
	if(!result)
	{
		return;
	}
	markChildEdge(t,state,RIGHT);
	initializeTypeAndUpdateMode(t,state);
	if(state->mode == DISCOVERY)
	{
		findAndMarkSuccessor(t,state);
	}
	
	if(state->mode == DISCOVERY)
	{
		removeSuccessor(t,state);
	}
	if(state->mode == CLEANUP)
	{
		cleanup(t,state);
	}
	
	return;
}

__inline void helpSuccessorNode(struct tArgs* t, struct edge* helpeeEdge, int depth)
{
	struct node* parent;
	struct node* node;
	struct node* left;
	struct stateRecord* state;
	struct seekRecord* s;
	// retrieve the address of the successor node
	parent = helpeeEdge->parent;
	node = helpeeEdge->child;
	// promote flag must be set on the successor node's left edge
	// retrieve the address of the target node
	left=getAddress(node->child[LEFT]);
	// obtain new state record and initialize it
	state = (struct stateRecord*) malloc(sizeof(struct stateRecord));
	populateEdge(&state->targetEdge,NULL,left,LEFT);
	state->depth = depth;
	state->mode = DISCOVERY;
	s = &state->successorRecord;
	// initialize the seek record in the state record
	s->lastEdge = *helpeeEdge;
	populateEdge(&s->pLastEdge,NULL,parent,LEFT);
	// remove the successor node
	removeSuccessor(t,state);
	return;
}
