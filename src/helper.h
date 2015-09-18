#include"bitManipulations.h"

struct node* R;
struct node* S;
struct node* T;
unsigned long numOfNodes;


static inline bool CAS(struct node* parent, int which, struct node* oldChild, struct node* newChild)
{
	#ifdef TBB
	return parent->child[which].compare_and_swap(newChild,oldChild) == oldChild;
	#else
	parent->child[which].compare_exchange_strong(oldChild,newChild,std::memory_order_seq_cst);
	#endif
}

static inline struct node* newLeafNode(unsigned long key)
{
  struct node* node = (struct node*) malloc(sizeof(struct node));
  node->markAndKey = key;
  node->child[LEFT] = setNull(NULL); 
  node->child[RIGHT] = setNull(NULL);
	node->readyToReplace=false;
  return(node);
}

void createHeadNodes()
{
  R = newLeafNode(INF_R);
  R->child[RIGHT] = newLeafNode(INF_S);
  S = R->child[RIGHT];
	S->child[RIGHT] = newLeafNode(INF_T);
	T = S->child[RIGHT];
}

void nodeCount(struct node* node)
{
  if(isNull(node))
  {
    return;
  }
  numOfNodes++;
	
	nodeCount(node->child[LEFT]);
  nodeCount(node->child[RIGHT]);
}

unsigned long size()
{
  numOfNodes=0;
  nodeCount(T->child[LEFT]);
  return numOfNodes;
}

void printKeysInOrder(struct node* node)
{
  if(isNull(node))
  {
    return;
  }
  printKeysInOrder(getAddress(node)->child[LEFT]);
	printf("%10x\t%10lu\t%10x\t%10x\t%10d\n",(struct node*) node,getKey(getAddress(node)->markAndKey),(struct node*) getAddress(node)->child[LEFT],(struct node*) getAddress(node)->child[RIGHT],(int) getAddress(node)->readyToReplace);
  printKeysInOrder(getAddress(node)->child[RIGHT]);
}

void printKeys()
{
  printKeysInOrder(T);
  printf("\n");
}

bool isValidBST(struct node* node, unsigned long min, unsigned long max)
{
  if(isNull(node))
  {
    return true;
  }
	struct node* address = getAddress(node);
	unsigned long nKey = getKey(address->markAndKey);
  if(nKey > min && nKey < max && isValidBST(address->child[LEFT],min,nKey) && isValidBST(address->child[RIGHT],nKey,max))
  {
    return true;
  }
  return false;
}

bool isValidTree()
{
  return(isValidBST(T->child[LEFT],0,MAX_KEY));
}