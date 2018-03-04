#ifndef REPL_STATE_H
#define REPL_STATE_H
 
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

#define Tbypass     3
#define Treplace    124
#define THETA       68
#define SAMPLER_SETS 256
#define SAMPLER_ASSOC   16
#define PRED_TABLE_SIZE 512

void Update_PC_History(Addr_t PC);

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

    // CONTESTANTS: Add extra state per cache line here
    bool pred_dead;

} LINE_REPLACEMENT_STATE;

//struct sampler; // Jimenez's structures

/* Sampler cache */
struct sampler_block {
    unsigned int lru_stack_position;
    unsigned long long int tag;
    signed int Yout;
    unsigned char valid;
    unsigned int hash[6]; // pc that filled this block

    sampler_block (void) {
        valid = false;
        tag = 0;
        Yout = 0;
    }
};

struct sampler_set {
    sampler_block blocks[SAMPLER_ASSOC];
    unsigned char valid; // means entire set is valid

    sampler_set (void) {
        valid = false;
        for (int i=0; i<SAMPLER_ASSOC; i++) {
            blocks[i].lru_stack_position = i;
        }
    }
};

struct sampler_cache {
    int nsets, assoc, blocksize;
    int offset_bits, index_bits, replacement_policy, tagshiftbits;
    unsigned int index_mask;
    unsigned long long misses, accesses,hits,evictions;
    sampler_set *sets;
    // long long int counts[DAN_MAX];

    // CACHE_REPLACEMENT_STATE *repl;

    sampler_cache (void) {
        index_mask = 0;
    }
};

void init_sampler_cache (sampler_cache *c, int nsets, int assoc, int blocksize, int policy);
void sampler_cache_access (UINT32 setIndex,Addr_t tag, unsigned long long int pc, unsigned int *hash);
void UpdatePCHistory(Addr_t PC);
INT32 Get_sampler_LRU_victim(UINT32 setIndex);
void Update_sampler_LRU(UINT32 setIndex, INT32 updateWayID);

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

    // CONTESTANTS:  Add extra state for cache here

  public:
    ostream & PrintStats(ostream &out);

    // The constructor CAN NOT be changed
    CACHE_REPLACEMENT_STATE( UINT32 _sets, UINT32 _assoc, UINT32 _pol );

    INT32 GetVictimInSet( UINT32 tid, UINT32 setIndex, const LINE_STATE *vicSet, UINT32 assoc, Addr_t PC, Addr_t paddr, UINT32 accessType );

    void   UpdateReplacementState( UINT32 setIndex, INT32 updateWayID );

    void   SetReplacementPolicy( UINT32 _pol ) { replPolicy = _pol; } 
    void   IncrementTimer() { mytimer++; } 

    void   UpdateReplacementState( UINT32 setIndex, INT32 updateWayID, const LINE_STATE *currLine, 
                                   UINT32 tid, Addr_t PC, UINT32 accessType, bool cacheHit );

    ~CACHE_REPLACEMENT_STATE(void);

  private:
    
    void   InitReplacementState();
    INT32  Get_Random_Victim( UINT32 setIndex );

    INT32  Get_LRU_Victim( UINT32 setIndex);
    INT32  Get_My_Victim( UINT32 tid,UINT32 setIndex,  Addr_t PC, Addr_t paddr);
    void   UpdateLRU( UINT32 setIndex, INT32 updateWayID);
    void   UpdateMyPolicy(UINT32 tid, UINT32 setIndex, INT32 updateWayID, const LINE_STATE *currLine, Addr_t PC, UINT32 accessType, bool cacheHit);
};

#endif
