#include "replacement_state.h"
#include <iostream>
using namespace std;
#define NUM 10000

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
// This function initializes the replacement policy hardware by creating      //
// storage for the replacement state on a per-line/per-cache basis.           //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
void CACHE_REPLACEMENT_STATE::InitReplacementState()
{
    // Create the state for sets, then create the state for the ways
    repl  = new LINE_REPLACEMENT_STATE* [ numsets ];
    counter=0;
    duel1counter=5;
    duel2counter=5;

    virt = new Addr_t [ numsets ];
    pointer = new INT32 [ numsets ];
    bypass = new bool[numsets];
    bypass_avail = new bool[numsets];
    bypass_rate = 0.03125;

    // ensure that we were able to create replacement state
    assert(repl);

    // Create the state for the sets
    for(UINT32 setIndex=0; setIndex<numsets; setIndex++) 
    {
        repl[ setIndex ]  = new LINE_REPLACEMENT_STATE[ assoc ];
        bypass_avail[setIndex]=0;
        for(UINT32 way=0; way<assoc; way++) 
        {
            // initialize stack position (for true LRU)
            repl[ setIndex ][ way ].LRUstackposition = way;
            repl[ setIndex ][ way ].reference=0;
            repl[ setIndex ][ way ].age=0;
        }
    }

    // Contestants:  ADD INITIALIZATION FOR YOUR HARDWARE HERE

}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function is called by the cache on every cache miss. The input        //
// arguments are the thread id, set index, pointers to ways in current set    //
// and the associativity.  We are also providing the PC, physical address,    //
// and accesstype should you wish to use them at victim selection time.       //
// The return value is the physical way index for the line being replaced.    //
// Return -1 if you wish to bypass LLC.                                       //
//                                                                            //
// vicSet is the current set. You can access the contents of the set by       //
// indexing using the wayID which ranges from 0 to assoc-1 e.g. vicSet[0]     //
// is the first way and vicSet[4] is the 4th physical way of the cache.       //
// Elements of LINE_STATE are defined in crc_cache_defs.h                     //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
INT32 CACHE_REPLACEMENT_STATE::GetVictimInSet( UINT32 tid, UINT32 setIndex, const LINE_STATE *vicSet, UINT32 assoc,
                                               Addr_t PC, Addr_t paddr, UINT32 accessType )
{
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
        return Get_MY_Victim( setIndex ,PC);
        // Contestants:  ADD YOUR VICTIM SELECTION FUNCTION HERE
    }

    // We should never get here
    assert(replPolicy);

    return -1; // Returning -1 bypasses the LLC
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
    UINT32 tid, Addr_t PC, UINT32 accessType, bool cacheHit )
{
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
        UpdateMY( setIndex, updateWayID,cacheHit,PC );
        // Contestants:  ADD YOUR UPDATE REPLACEMENT STATE FUNCTION HERE
        // Feel free to use any of the input parameters to make
        // updates to your replacement policy
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
    for(UINT32 way=0; way<assoc; way++) 
    {
        if( replSet[way].LRUstackposition == (assoc-1) ) 
        {
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


INT32 CACHE_REPLACEMENT_STATE::Get_BIP_Victim( UINT32 setIndex )
{
    // Get pointer to replacement state of current set
    LINE_REPLACEMENT_STATE *replSet = repl[ setIndex ];

    INT32   lruWay   = 0;

    // Search for victim whose stack position is assoc-1
    for(UINT32 way=0; way<assoc; way++) 
    {
        if( replSet[way].LRUstackposition == (assoc-1) ) 
        {           
            int segma=NUM*0.9;
            if (rand()%NUM<segma)  
                replSet[way].reference=1;
            else
                replSet[way].reference=0;
            segma=NUM*0.1;
            if (rand()%NUM<segma) 
            {
                UINT32 currLRUstackposition = replSet[way].LRUstackposition;
                for(UINT32 i=0; i<assoc; i++) 
                {
                     if( repl[setIndex][i].LRUstackposition < currLRUstackposition ) 
                    {
                        repl[setIndex][i].LRUstackposition++;
                    }
                }
                replSet[way].LRUstackposition=0;
            }
            lruWay = way;
            break;
        }
    }

    // return lru way
    return lruWay;
}


INT32 CACHE_REPLACEMENT_STATE::Get_SLRU_Victim( UINT32 setIndex )
{
    // Get pointer to replacement state of current set
    LINE_REPLACEMENT_STATE *replSet = repl[ setIndex ];

    INT32   lruWay   = 0;

    //Search for victim whose stack position is assoc-1
    bool flag=0;
    for(UINT32 way=0; way<assoc; way++) 
    {
        if(replSet[way].reference==0)
        {
            lruWay = way; 
            break;
        }
        else 
        if( replSet[way].LRUstackposition == (assoc-1) ) 
        {
            if(flag==0)
            {   
                flag=1;
                lruWay = way; 
            } 
        }
    }

    int segma=NUM*0.9;
    if (rand()%NUM<segma)  
        replSet[lruWay].reference=1;
    else
        replSet[lruWay].reference=0;
    // return lru way
    UINT32 currLRUstackposition = replSet[lruWay].LRUstackposition;
    for(UINT32 i=0; i<assoc; i++) 
    {
         if( repl[setIndex][i].LRUstackposition < currLRUstackposition ) 
        {
            repl[setIndex][i].LRUstackposition++;
        }
    }
    replSet[lruWay].LRUstackposition=0;
    return lruWay;
}

INT32 CACHE_REPLACEMENT_STATE::Get_MY_Victim( UINT32 setIndex,  Addr_t PC  )
{
     //      cout<<"counter "<<counter<<endl;
    float temp;
    temp=(float)duel1counter/duel2counter;
    INT32 way;
    if(setIndex<32) {counter+=1/temp; way = Get_BIP_Victim(setIndex);}
    else if(setIndex<64) {counter-=temp; way = Get_SLRU_Victim(setIndex);}

    else if(counter<=0) way = Get_BIP_Victim(setIndex);
    else way = Get_SLRU_Victim(setIndex);
    if(bypass_avail[setIndex]==0)
    {   
        virt[setIndex]=PC;
        pointer[setIndex]=way;
        bypass_avail[setIndex]=1;
    }
    else
    {
        int segma=NUM*0.03125;
        if (rand()%NUM<segma)  
        {
            virt[setIndex]=PC;
            pointer[setIndex]=way;            
        }
    }
    int segma=NUM*bypass_rate;
    // cout<<"bypass "<<bypass_rate<<endl;
    if (rand()%NUM<segma)  
    {
        bypass[setIndex]=1;
        return -1;
    }
    else
    {
        bypass[setIndex]=0;
        return way;
    }
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
    for(UINT32 way=0; way<assoc; way++) 
    {
        if( repl[setIndex][way].LRUstackposition < currLRUstackposition ) 
        {
            repl[setIndex][way].LRUstackposition++;
        }
    }

    // Set the LRU stack position of new line to be zero
    repl[ setIndex ][ updateWayID ].LRUstackposition = 0;
}

void CACHE_REPLACEMENT_STATE::UpdateMY( UINT32 setIndex, INT32 updateWayID,bool cacheHit,  Addr_t PC  )
{
    // Determine current LRU stack position
    if(setIndex<32) duel1counter++;
    if(setIndex<64 && setIndex>=32) duel2counter++;
    UINT32 currLRUstackposition = repl[ setIndex ][ updateWayID ].LRUstackposition;
    if(cacheHit) repl[setIndex][updateWayID].reference=1;
    // Update the stack position of all lines before the current line
    // Update implies incremeting their stack positions by one
    for(UINT32 way=0; way<assoc; way++) 
    {
        if(repl[setIndex][way].reference==1) repl[setIndex][way].age++;
        if(repl[setIndex][way].age>=1024) 
        {
            repl[setIndex][way].age=0;         
            repl[setIndex][way].reference=0;
        }
        if(cacheHit)
        {
            if( repl[setIndex][way].LRUstackposition < currLRUstackposition ) 
            {
                repl[setIndex][way].LRUstackposition++;
            }
        }
        
    }



    // Set the LRU stack position of new line to be zero
    if(cacheHit) 
    {
        repl[ setIndex ][ updateWayID ].LRUstackposition = 0;
        if(bypass_avail[setIndex]==1)
        {
            if(bypass[setIndex]==1)
            {
                if(PC==virt[setIndex]) { bypass_rate/=2; }
                if(updateWayID==pointer[setIndex]) { bypass_rate*=2; }
            }
            else
            {
                if(PC==virt[setIndex]) { bypass_rate*=2; }
                if(updateWayID==pointer[setIndex]) { bypass_rate/=2; }
            }
            if(bypass_rate>1) bypass_rate=1;
            bypass_avail[setIndex]=0;
        }
    }
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

