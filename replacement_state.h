#ifndef REPL_STATE_H
#define REPL_STATE_H
#define DANSHIP 
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This file is distributed as part of the Cache Replacement Championship     //
// workshop held in conjunction with ISCA'2010.                               //
//                                                                            //
//                                                                            //
// Everyone is granted permission to copy, modify, and/or re-distribute       //
// this software.                                                             //
//                                                                            //
// Please contact Aamer Jaleel <ajaleel@gmail.com> should you have any        //
// questions                                                                  //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include <cstdlib>
#include <cassert>
#include "utils.h"
#include "crc_cache_defs.h"
#include <iostream>

using namespace std;

// Replacement Policies Supported
typedef enum 
{
    CRC_REPL_LRU        = 0,
    CRC_REPL_RANDOM     = 1,
    CRC_REPL_CONTESTANT = 2
} ReplacemntPolicy;

// Replacement State Per Cache Line
typedef struct
{
    UINT32  LRUstackposition;
    UINT32  counter;		//Re-reference predictor value (RRPV)
    UINT32  signature_m;	//The signature field of the cache line
    bool    outcome;		//Tracks the outcome of the last reference to the cache line
    bool    is_prefetched;     //To track whether line has been brought in by a prefetcher
} LINE_REPLACEMENT_STATE;

struct sampler; // Jimenez's structures

// The implementation for the cache replacement policy
class CACHE_REPLACEMENT_STATE
{
public:
 
    LINE_REPLACEMENT_STATE   **repl;
  private:
    UINT32 numsets;
    UINT32 assoc;
    UINT32 replPolicy;
    COUNTER mytimer;  // tracks # of references to the cache
    unsigned char shct[14000]; 	  //Signature Hit Counter Table, of 14k entries


  public:
    ostream & PrintStats(ostream &out);

    // The constructor CAN NOT be changed
    CACHE_REPLACEMENT_STATE( UINT32 _sets, UINT32 _assoc, UINT32 _pol );

    INT32 GetVictimInSet( UINT32 tid, UINT32 setIndex, const LINE_STATE *vicSet, UINT32 assoc, Addr_t PC, Addr_t paddr, UINT32 accessType );

    void   UpdateReplacementState( UINT32 setIndex, INT32 updateWayID );

    void   SetReplacementPolicy( UINT32 _pol ) { replPolicy = _pol; } 
    void   IncrementTimer() { mytimer++; } 

    void   UpdateReplacementState( UINT32 setIndex, INT32 updateWayID, const LINE_STATE *currLine, 
                                   UINT32 tid, Addr_t PC, UINT32 accessType, bool cacheHit, Addr_t address);

    ~CACHE_REPLACEMENT_STATE(void);

  private:
    
    void   InitReplacementState();
    INT32  Get_Random_Victim( UINT32 setIndex );
    INT32  Get_LRU_Victim( UINT32 setIndex );
    INT32  Get_My_Victim( UINT32 setIndex );
    void   UpdateLRU( UINT32 setIndex, INT32 updateWayID );
    void   UpdateMyPolicy( UINT32 setIndex, INT32 updateWayID, UINT32 accessType, bool cacheHit, Addr_t PC, Addr_t address );
};

#endif
