#include"LockFree.h"

int NUM_OF_THREADS;
int findPercent;
int insertPercent;
int deletePercent;
unsigned long keyRange;
double MOPS;
volatile bool start=false;
volatile bool stop=false;
volatile bool steadyState=false;
struct timespec runTime,transientTime;

static inline unsigned long getRandom(gsl_rng* r)
{
	return(gsl_rng_uniform_int(r,keyRange) + 2);
}

void *operateOnTree(void* tArgs)
{
  int chooseOperation;
  unsigned long lseed;
	unsigned long key;
  struct tArgs* tData = (struct tArgs*) tArgs;
  const gsl_rng_type* T;
  gsl_rng* r;
  gsl_rng_env_setup();
  T = gsl_rng_default;
  r = gsl_rng_alloc(T);
	lseed = tData->lseed;
  gsl_rng_set(r,lseed);

	tData->newNode=NULL;
  tData->isNewNodeAvailable=false;
	tData->readCount=0;
  tData->successfulReads=0;
  tData->unsuccessfulReads=0;
  tData->readRetries=0;
  tData->insertCount=0;
  tData->successfulInserts=0;
  tData->unsuccessfulInserts=0;
  tData->insertRetries=0;
  tData->deleteCount=0;
  tData->successfulDeletes=0;
  tData->unsuccessfulDeletes=0;
  tData->deleteRetries=0;
	tData->seekRetries=0;
	tData->seekLength=0;

  while(!start)
  {
  }
	
	while(!steadyState)
	{
	  chooseOperation = gsl_rng_uniform(r)*100;
		key = gsl_rng_uniform_int(r,keyRange) + 2;
    if(chooseOperation < findPercent)
    {
      search(tData, key);
    }
    else if (chooseOperation < insertPercent)
    {
      insert(tData, key);
    }
    else
    {
      remove(tData, key);
    }
	}
	
  tData->readCount=0;
  tData->successfulReads=0;
  tData->unsuccessfulReads=0;
  tData->readRetries=0;
  tData->insertCount=0;
  tData->successfulInserts=0;
  tData->unsuccessfulInserts=0;
  tData->insertRetries=0;
  tData->deleteCount=0;
  tData->successfulDeletes=0;
  tData->unsuccessfulDeletes=0;
  tData->deleteRetries=0;
	tData->seekRetries=0;
	tData->seekLength=0;
	
	while(!stop)
  {
    chooseOperation = gsl_rng_uniform(r)*100;
		key = gsl_rng_uniform_int(r,keyRange) + 2;

    if(chooseOperation < findPercent)
    {
			tData->readCount++;
      search(tData, key);
    }
    else if (chooseOperation < insertPercent)
    {
			tData->insertCount++;
      insert(tData, key);
    }
    else
    {
			tData->deleteCount++;
      remove(tData, key);
    }
  }
  return NULL;
}
struct tArgs** tArgs;

int main(int argc, char *argv[])
{
	unsigned long lseed;
	//get run configuration from command line
  NUM_OF_THREADS = atoi(argv[1]);
  findPercent = atoi(argv[2]);
  insertPercent= findPercent + atoi(argv[3]);
  deletePercent = insertPercent + atoi(argv[4]);

	runTime.tv_sec = atoi(argv[5]);
	runTime.tv_nsec =0;
	transientTime.tv_sec=0;
	transientTime.tv_nsec=2000000;

  keyRange = (unsigned long) atol(argv[6])-1;
	lseed = (unsigned long) atol(argv[7]);

  tArgs = (struct tArgs**) malloc(NUM_OF_THREADS * sizeof(struct tArgs*)); 

  const gsl_rng_type* T;
  gsl_rng* r;
  gsl_rng_env_setup();
  T = gsl_rng_default;
  r = gsl_rng_alloc(T);
  gsl_rng_set(r,lseed);
	
  createHeadNodes(); //Initialize the tree. Must be called before doing any operations on the tree
  
  struct tArgs* initialInsertArgs = (struct tArgs*) malloc(sizeof(struct tArgs));
  initialInsertArgs->successfulInserts=0;
	initialInsertArgs->newNode=NULL;
  initialInsertArgs->isNewNodeAvailable=false;
	
  while(initialInsertArgs->successfulInserts < keyRange/2) //populate the tree with 50% of keys
  {
    insert(initialInsertArgs,gsl_rng_uniform_int(r,keyRange) + 2);
  }
  pthread_t threadArray[NUM_OF_THREADS];
  for(int i=0;i<NUM_OF_THREADS;i++)
  {
    tArgs[i] = (struct tArgs*) malloc(sizeof(struct tArgs));
    tArgs[i]->tId = i;
    tArgs[i]->lseed = gsl_rng_get(r);
  }

	for(int i=0;i<NUM_OF_THREADS;i++)
	{
		pthread_create(&threadArray[i], NULL, operateOnTree, (void*) tArgs[i] );
	}
	
	start=true; 										//start operations
	nanosleep(&transientTime,NULL); //warmup
	steadyState=true;
	nanosleep(&runTime,NULL);
	stop=true;											//stop operations
	
	for(int i=0;i<NUM_OF_THREADS;i++)
	{
		pthread_join(threadArray[i], NULL);
	}	

  unsigned long totalReadCount=0;
  unsigned long totalSuccessfulReads=0;
  unsigned long totalUnsuccessfulReads=0;
  unsigned long totalReadRetries=0;
  unsigned long totalInsertCount=0;
  unsigned long totalSuccessfulInserts=0;
  unsigned long totalUnsuccessfulInserts=0;
  unsigned long totalInsertRetries=0;
  unsigned long totalDeleteCount=0;
  unsigned long totalSuccessfulDeletes=0;
  unsigned long totalUnsuccessfulDeletes=0;
  unsigned long totalDeleteRetries=0;
	unsigned long totalSeekRetries=0;
	unsigned long totalSeekLength=0;
 
  for(int i=0;i<NUM_OF_THREADS;i++)
  {
    totalReadCount += tArgs[i]->readCount;
    totalSuccessfulReads += tArgs[i]->successfulReads;
    totalUnsuccessfulReads += tArgs[i]->unsuccessfulReads;
    totalReadRetries += tArgs[i]->readRetries;

    totalInsertCount += tArgs[i]->insertCount;
    totalSuccessfulInserts += tArgs[i]->successfulInserts;
    totalUnsuccessfulInserts += tArgs[i]->unsuccessfulInserts;
    totalInsertRetries += tArgs[i]->insertRetries;
    totalDeleteCount += tArgs[i]->deleteCount;
    totalSuccessfulDeletes += tArgs[i]->successfulDeletes;
    totalUnsuccessfulDeletes += tArgs[i]->unsuccessfulDeletes;
    totalDeleteRetries += tArgs[i]->deleteRetries;
		totalSeekRetries += tArgs[i]->seekRetries;
		totalSeekLength += tArgs[i]->seekLength;
  }
	unsigned long totalOperations = totalReadCount + totalInsertCount + totalDeleteCount;
	MOPS = totalOperations/(runTime.tv_sec*1000000.0);
	printf("k%d;%d-%d-%d;%d;%ld;%ld;%ld;%ld;%ld;%ld;%ld;%ld;%.2f;%.2f\n",atoi(argv[6]),findPercent,(insertPercent-findPercent),(deletePercent-insertPercent),NUM_OF_THREADS,size(),totalReadCount,totalInsertCount,totalDeleteCount,totalReadRetries,totalSeekRetries,totalInsertRetries,totalDeleteRetries,totalSeekLength*1.0/totalOperations,MOPS);
	assert(isValidTree());
	pthread_exit(NULL);
}
