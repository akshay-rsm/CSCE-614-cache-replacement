#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/mman.h>
#include <map>
#include <iostream>

using namespace std;

#include "replacement_state.h"
#define MAX_COUNTER_VALUE 7			//Saturating value of RRPV (means distant re-reference)
#define LONG_REFERENCE (MAX_COUNTER_VALUE - 3)	//Corresponds to intermediate re-reference interval

////////////////////////////////////////////////////////////////////////////////
// Cache replacement policy                                                   //
//  Author : Akshay RSM                                                       //
////////////////////////////////////////////////////////////////////////////////

/*
** This file implements the cache replacement state. Users can enhance the code
** below to develop their cache replacement ideas.
**
*/


////////////////////////////////////////////////////////////////////////////////
// The replacement state constructor:                                         //
// Inputs: number of sets, associativity, and replacement policy to use       //
// Outputs: None                                                              //
//                                                                            //
// DO NOT CHANGE THE CONSTRUCTOR PROTOTYPE                                    //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
CACHE_REPLACEMENT_STATE::CACHE_REPLACEMENT_STATE( UINT32 _sets, UINT32 _assoc, UINT32 _pol )
{

    numsets    = _sets;
    assoc      = _assoc;
    replPolicy = _pol;

    mytimer    = 0;

    InitReplacementState();
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// The function prints the statistics for the cache                           //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
ostream & CACHE_REPLACEMENT_STATE::PrintStats(ostream &out)
{

    out<<"=========================================================="<<endl;
    out<<"=========== Replacement Policy Statistics ================"<<endl;
    out<<"=========================================================="<<endl;

    // CONTESTANTS:  Insert your statistics printing here
    
    return out;

}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function initializes the replacement policy hardware by creating      //
// storage for the replacement state on a per-line/per-cache basis.           //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

void CACHE_REPLACEMENT_STATE::InitReplacementState()
{
    // Create the state for sets, then create the state for the ways

	repl  = new LINE_REPLACEMENT_STATE* [ numsets ];
    // ensure that we were able to create replacement state

    assert(repl);

    // Create the state for the sets
   for(UINT32 setIndex=0; setIndex<numsets; setIndex++) 
    {
        repl[ setIndex ]  = new LINE_REPLACEMENT_STATE[ assoc ];

        for(UINT32 way=0; way<assoc; way++) 
        {
            // initialize stack position (for true LRU)
            repl[ setIndex ][ way ].LRUstackposition = way;
        }
    }

    if (replPolicy != CRC_REPL_CONTESTANT) return;

// Contestants:  ADD INITIALIZATION FOR YOUR HARDWARE HERE

   for(UINT32 setIndex=0; setIndex<numsets; setIndex++) 
    {
        for(UINT32 way=0; way<assoc; way++) 
        {
           // initialize metadata
            repl[ setIndex ][ way ].counter = MAX_COUNTER_VALUE;
	    repl[setIndex][way].outcome = false;
       	    repl[setIndex][way].signature_m = 0;
	    repl[setIndex][way].is_prefetched = false; 
	}
    }
   for(UINT32 index=0;index<sizeof(shct);index++)
   {
   	shct[index] = 3;	//inital value of signature based counter	
   }
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function is called by the cache on every cache miss. The input        //
// argument is the set index. The return value is the physical way            //
// index for the line being replaced.                                         //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
INT32 CACHE_REPLACEMENT_STATE::GetVictimInSet( UINT32 tid, UINT32 setIndex, const LINE_STATE *vicSet, UINT32 assoc, Addr_t PC, Addr_t paddr, UINT32 accessType ) {
    // If no invalid lines, then replace based on replacement policy
    if( replPolicy == CRC_REPL_LRU ) 
    {
        return Get_LRU_Victim( setIndex );
    }
    else if( replPolicy == CRC_REPL_RANDOM )
    {
        return Get_Random_Victim( setIndex );
    }
    else if( replPolicy == CRC_REPL_CONTESTANT )
    {
        // Contestants:  ADD YOUR VICTIM SELECTION FUNCTION HERE
	return Get_My_Victim (setIndex);
    }

    // We should never here here

    assert(0);
    return -1;
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function is called by the cache after every cache hit/miss            //
// The arguments are: the set index, the physical way of the cache,           //
// the pointer to the physical line (should contestants need access           //
// to information of the line filled or hit upon), the thread id              //
// of the request, the PC of the request, the accesstype, and finall          //
// whether the line was a cachehit or not (cacheHit=true implies hit)         //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
void CACHE_REPLACEMENT_STATE::UpdateReplacementState( 
    UINT32 setIndex, INT32 updateWayID, const LINE_STATE *currLine, 
    UINT32 tid, Addr_t PC, UINT32 accessType, bool cacheHit, Addr_t address)
{
	//fprintf (stderr, "ain't I a stinker? %lld\n", get_cycle_count ());
	//fflush (stderr);
    // What replacement policy?
    if( replPolicy == CRC_REPL_LRU ) 
    {
        UpdateLRU( setIndex, updateWayID );
    }
    else if( replPolicy == CRC_REPL_RANDOM )
    {
        // Random replacement requires no replacement state update
    }
    else if( replPolicy == CRC_REPL_CONTESTANT )
    {
        // Contestants:  ADD YOUR UPDATE REPLACEMENT STATE FUNCTION HERE
        // Feel free to use any of the input parameters to make
        // updates to your replacement policy
	UpdateMyPolicy(setIndex, updateWayID, accessType, cacheHit, PC, address);
    }
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//////// HELPER FUNCTIONS FOR REPLACEMENT UPDATE AND VICTIM SELECTION //////////
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function finds the LRU victim in the cache set by returning the       //
// cache block at the bottom of the LRU stack. Top of LRU stack is '0'        //
// while bottom of LRU stack is 'assoc-1'                                     //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
INT32 CACHE_REPLACEMENT_STATE::Get_LRU_Victim( UINT32 setIndex )
{
	// Get pointer to replacement state of current set

	LINE_REPLACEMENT_STATE *replSet = repl[ setIndex ];
	INT32   lruWay   = 0;

	// Search for victim whose stack position is assoc-1

	for(UINT32 way=0; way<assoc; way++) {
		if (replSet[way].LRUstackposition == (assoc-1)) {
			lruWay = way;
			break;
		}
	}

	// return lru way

	return lruWay;
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function finds a random victim in the cache set                       //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
INT32 CACHE_REPLACEMENT_STATE::Get_Random_Victim( UINT32 setIndex )
{
    INT32 way = (rand() % assoc);
    
    return way;
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function implements the LRU update routine for the traditional        //
// LRU replacement policy. The arguments to the function are the physical     //
// way and set index.                                                         //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

void CACHE_REPLACEMENT_STATE::UpdateLRU( UINT32 setIndex, INT32 updateWayID )
{
	// Determine current LRU stack position
	UINT32 currLRUstackposition = repl[ setIndex ][ updateWayID ].LRUstackposition;

	// Update the stack position of all lines before the current line
	// Update implies incremeting their stack positions by one

	for(UINT32 way=0; way<assoc; way++) {
		if( repl[setIndex][way].LRUstackposition < currLRUstackposition ) {
			repl[setIndex][way].LRUstackposition++;
		}
	}

	// Set the LRU stack position of new line to be zero
	repl[ setIndex ][ updateWayID ].LRUstackposition = 0;
}

INT32 CACHE_REPLACEMENT_STATE::Get_My_Victim( UINT32 setIndex ) 
{
	//SRRIP implementation
	LINE_REPLACEMENT_STATE* set = repl[setIndex];
	bool done = false;	//indicates if victim has been chosen
	UINT32 chosen_line;	//the victim's way number
	while(!done)
	{
		for(UINT32 way=0;way<assoc;way++)
		{
			if(set[way].counter == MAX_COUNTER_VALUE && done == false)	//Finds the first line with the max counter value
			{
				done = true;
				chosen_line = way;
			}		
		}

		if(!done)		//increment all rrpv values
		{
			for(UINT32 way=0;way<assoc;way++)
			{
				if(set[way].counter < MAX_COUNTER_VALUE)
				{
					set[way].counter++;
				}
			}
	
		}
	}	
	if(done)
	{
		return chosen_line;
	}
	else
	{
		assert(false);	//shouldn't come here
	}
}

void CACHE_REPLACEMENT_STATE::UpdateMyPolicy(UINT32 setIndex, INT32 updateWayID, UINT32 accessType, bool cacheHit, Addr_t PC, Addr_t address) 
{
	bool is_prefetch=false;		//check for prefetch accesses
	if(accessType == ACCESS_PREFETCH)
	{
		is_prefetch = true;
	}	
	LINE_REPLACEMENT_STATE* set = repl[setIndex];	
	UINT32 signature = (UINT32)( ((PC) + is_prefetch ) % (sizeof(shct)));		//hash into the shct, the counter also learns if access is a prefetch
	if(accessType == ACCESS_WRITEBACK)			//optimization #2
	{
		set[updateWayID].counter = MAX_COUNTER_VALUE;
		return;
	}
	if(cacheHit)
	{
		if(accessType == ACCESS_PREFETCH && set[updateWayID].is_prefetched == false)	//optimization #4, sets is_prefetched bit 
		{
			set[updateWayID].is_prefetched = true;
			set[updateWayID].counter = MAX_COUNTER_VALUE;
			return;
		}
		set[updateWayID].is_prefetched == false;
		set[updateWayID].counter = 0;		//directly insert hit lines with rrpv=0, optimization #1 
		set[updateWayID].outcome = true;
		if(shct[set[updateWayID].signature_m] < 3 )
		{
			shct[set[updateWayID].signature_m]++;
		}
	}
	else						//SHiP algortihm
	{
		if (set[updateWayID].outcome != true)
		{
			if(shct[set[updateWayID].signature_m] > 0)
			{
				shct[set[updateWayID].signature_m]--;
			}
		}
		set[updateWayID].outcome = false;
		set[updateWayID].signature_m = signature;
		if (shct[signature] == 0)
		{
			set[updateWayID].counter = MAX_COUNTER_VALUE;
		}
		else
		{
			set[updateWayID].counter = LONG_REFERENCE;
		} 	
	}

}

CACHE_REPLACEMENT_STATE::~CACHE_REPLACEMENT_STATE (void) {
}
