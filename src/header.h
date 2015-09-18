#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>
#include<stdbool.h>
#include<math.h>
#include<time.h>
#include<stdint.h>
#include<gsl/gsl_rng.h>
#include<gsl/gsl_randist.h>
#include<assert.h>

#define TBB
#ifdef TBB
#include<tbb/atomic.h>
#else
#include <atomic>
#endif


#define K 2
#define LEFT 0
#define RIGHT 1

#define MAX_KEY 0x7FFFFFFF
#define INF_R 0x0
#define INF_S 0x1
#define INF_T 0x7FFFFFFE
#define KEY_MASK 0x80000000
#define ADDRESS_MASK 15	

#define NULL_BIT 8
#define INJECT_BIT 4
#define DELETE_BIT 2
#define PROMOTE_BIT 1

typedef enum {INJECTION, DISCOVERY, CLEANUP} Mode;
typedef enum {SIMPLE, COMPLEX} Type;
typedef enum {DELETE_FLAG, PROMOTE_FLAG} Flag;


struct node
{
	#ifdef TBB
		tbb::atomic<unsigned long> markAndKey;			//format <markFlag,address>
		tbb::atomic<node*> child[K];								//format <address,NullBit,InjectFlag,DeleteFlag,PromoteFlag>
		tbb::atomic<unsigned long> readyToReplace;
	#else
		std::atomic<unsigned long> markAndKey;			//format <markFlag,address>
		std::atomic<node*> child[K];								//format <address,NullBit,InjectFlag,DeleteFlag,PromoteFlag>
		std::atomic<unsigned long> readyToReplace;
	#endif
};

struct edge
{
	struct node* parent;
	struct node* child;
	int which;
};

struct seekRecord
{
	struct edge lastEdge;
	struct edge pLastEdge;
	struct edge injectionEdge;
};

struct anchorRecord
{
	struct node* node;
	unsigned long key;
};

struct stateRecord
{
	int depth;
	struct edge targetEdge;
	struct edge pTargetEdge;
	unsigned long targetKey;
	unsigned long currentKey;
	Mode mode;
	Type type;
	struct seekRecord successorRecord;
};

struct tArgs
{
	int tId;
	unsigned long lseed;
	unsigned long readCount;
	unsigned long successfulReads;
	unsigned long unsuccessfulReads;
	unsigned long readRetries;
	unsigned long insertCount;
	unsigned long successfulInserts;
	unsigned long unsuccessfulInserts;
	unsigned long insertRetries;
	unsigned long deleteCount;
	unsigned long successfulDeletes;
	unsigned long unsuccessfulDeletes;
	unsigned long deleteRetries;
	struct node* newNode;
	bool isNewNodeAvailable;
	struct seekRecord targetRecord;
	struct seekRecord pSeekRecord;
	struct stateRecord myState;	
	struct anchorRecord anchorRecord;
	struct anchorRecord pAnchorRecord;
	unsigned long seekRetries;
	unsigned long seekLength;
};

void createHeadNodes();
__inline bool search(struct tArgs*, unsigned long);
__inline bool insert(struct tArgs*, unsigned long);
__inline bool remove(struct tArgs*, unsigned long);
unsigned long size();
void printKeys();
bool isValidTree();