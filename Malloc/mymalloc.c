#include "mymalloc.h"
#include "sorted-list.h"
/*Defines */
#define MEMENTRYSIZE sizeof(MemEntry)
#define max_size 5000



/*Static Variables*/
static char ALLMEM [5000];
char * head = ALLMEM;
static int numOfMallocs = 0;
static size_t memAllocated;
static size_t freeMemEntries; 
static MemEntry * tail;



void * mymalloc (size_t size,char * file, int line) {

	//checks is user asks for too much space
	if(size > (max_size+MEMENTRYSIZE)|| size <= 0){
		printf("Insufficient space available. Asked for in %s Line %d\n",file,line);
		return 0;
	}
	printf("Current space: %d\n",memAllocated);
	printf("Space taken by MemEntry: %d\n",MEMENTRYSIZE);
	printf("User asking for: %d\n",size);


	printf("Space taken after storage: %d\n",(size + memAllocated + MEMENTRYSIZE));
		printList((MemEntry *) head);
	
	//first time malloc called
	if(numOfMallocs ==0) {
		MemEntry * construct =(MemEntry *)head;
		construct->free =1;
		construct->next =0;
		construct->prev =0;
		construct->size = size;
		tail = construct;
		numOfMallocs++;
		memAllocated = MEMENTRYSIZE + size;
	} else {
		//printf("Current space: %d\n",memAllocated);
		//printf("Space taken after storage: %d\n",(size + memAllocated + MEMENTRYSIZE));
		if( (size + memAllocated + MEMENTRYSIZE) > max_size){
			printf("Insufficient space available. Asked for in %s Line %d\n",file,line);
			return 0;
		}
		if(freeMemEntries>0){
			//there are free memEntries.. look and see if any fit the bill
			void * lookReturn = lookForFreeMem(size);
			if(lookReturn != 0){
				MemEntry * skipAhead= (MemEntry *)lookReturn;
				lookReturn = (char *)lookReturn+(skipAhead->size);
				return (void *) lookReturn;
			} 
		}
		else {
			//this and first memEntry are only time a new MemEntry is made
		     char * endOfData = head+memAllocated;
		     MemEntry * construct = (MemEntry *) endOfData;
		     construct->free =1;
		     construct->size = size;
		     construct->next=0;
		     tail->next = construct;
		     construct->prev = tail;
		     tail = construct;
		     memAllocated = (memAllocated +MEMENTRYSIZE + size);
		     numOfMallocs++;
			}
	}
	printFromTail(tail);
	void * returnPtr = head+(memAllocated-size);
	return returnPtr;
}


/*FREE
 *First checks if valid pointer
 *By decrementing pointer by one memEntry struct, we should be able to determine if the pointer is viable
 * if free != 1 then something is wrong... because the memEntry struct should equal 0. So print error
 *then checks if in list
 *if in list, but freed, then skipped over
 *returns 0 and prints an error message if something fails
 */

void myfree(void * pointerToFree, char * file, int line) {
	printf("TRYING TO FREE\n");
	printf("LIST:\n");
	printFromTail(tail);
	if(pointerToFree == 0){
		printf("Attempted to free a null pointer\n");
		return;
	}
	if ( pointerToFree < (void *) head || pointerToFree > ((void *)head+max_size))
	{

		printf("Attempted to free pointer not allocated by malloc\n");
		return;
	}
	MemEntry * construct = (MemEntry *) pointerToFree -1;
	if(construct->free != 1){
		printf("Invalid pointer\n");
		return;
	}else {
		printList((MemEntry *) head);
		construct->free =0;
		defragment(construct);
	}

}

/*look for free mem tries to find an already free chunk of memory to place the data into
 *RETURNS THE MemEntry STRUCT
 *THE POINTER NEEDS INCREMENTED TO START OF DATA
 *If no memEntrys found that are the same size as the size requested && memAllocated at capacity
 *look for any memEntrys that are > then size looking for. Return that.
 */
void * lookForFreeMem(size_t  size) {
	MemEntry * memEntry = (MemEntry *)head;
	while(memEntry != 0){
		if((memEntry->free=0) && (memEntry->size == size)) {
			memEntry->free=1;
			return (void *) memEntry;
		}
	}
	if(memAllocated == max_size){
     memEntry = (MemEntry *)head;
     while(memEntry != 0){
	  if((memEntry->free=0) && (memEntry->size > size)) {
			memEntry->free=1;
			return (void *) memEntry;
		}
	 }
	}

	return 0;
}

/*defragment handles fragmentation
 *4 cases
 *1)node->next ==0 then freeing last node, can be deleted
 *2)next node is freed
 *3)prev node is freed
 *4) prev and next node are already freed
 * NEED TO DECREASE MEMALLOCATED IF CONSTRUCT->NEXT ==0 BECAUSE END NODE
 */
void defragment(MemEntry * construct){

	if(construct->next != 0 && construct->next->free ==0 && construct->prev != 0 && construct->prev->free == 0) {
		printf("First if\n");
		construct->size += (construct->next->size+MEMENTRYSIZE);
		construct->next = construct->next->next;
		if(construct->next != 0){ //if == 0 then construct is new tail node and  can be deleted
			construct->next->prev = construct;

		} else {
			memAllocated = memAllocated - (construct->size +MEMENTRYSIZE);
			if(construct->prev != 0)
				construct->prev->next = 0;
		}
		construct->size += (construct->prev->size + MEMENTRYSIZE);
		construct->prev=construct->prev->prev;
		if(construct->prev != 0) //construct is new head
			construct->prev->next = construct;
	}
	else if (construct->next != 0 && construct->next->free == 0){
		printf("Second  if\n");

		construct->size += (construct->next->size + MEMENTRYSIZE);
		construct->next = construct->next->next;
		if(construct->next != 0){
			construct->next->prev = construct;
		} else {
			memAllocated = memAllocated - (construct->size +MEMENTRYSIZE);
			if(construct->prev != 0)
				construct->prev->next = 0;
		}

	} else if(construct->prev != 0 && construct->prev->free ==0){
		printf("Third if\n");

		construct->size += (construct->prev->size +MEMENTRYSIZE);
		construct->prev = construct->prev->prev;
		if(construct->prev != 0){
			construct->prev->next = construct;
		}
		if(construct->next == 0){ // then construct is tail node and can be deleted
			memAllocated = memAllocated - (construct->size +MEMENTRYSIZE);
			if(construct->prev != 0)
				construct->prev->next = 0;
		}
	} 
}

void printList (MemEntry * construct)
{
	int i = 0;
	while(construct!=0){
		printf("ENTRY %d SIZE: %d  Free:%d\n", i, construct->size, construct->free );
		construct = construct->next;
		i++;
	}
	printf("END PRINT LIST\n");
}

void printFromTail ()
{
	printf("MemAllocated %d\n",memAllocated);
	if(tail == 0)
		return;
	MemEntry * construct = tail;
	int i = 0;
	while(construct!=0){
		printf("ENTRY %d SIZE: %d  Free:%d\n", i, construct->size, construct->free );
		construct = construct->prev;
		i++;
	}
	printf("END PRINT LIST\n");
}