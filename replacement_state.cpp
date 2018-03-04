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

sampler_cache sampler;
signed char pred_table1[PRED_TABLE_SIZE]={0};
signed char pred_table2[PRED_TABLE_SIZE]={0};
signed char pred_table3[PRED_TABLE_SIZE]={0};
signed char pred_table4[PRED_TABLE_SIZE]={0};
signed char pred_table5[PRED_TABLE_SIZE]={0};
signed char pred_table6[PRED_TABLE_SIZE]={0}; 

Addr_t PC_History[16][3];

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

// log base 2

int log2 (int n) {
	int i, m = n, c = -1;
	for (i=0; m; i++) {
		m /= 2;
		c++;
	}
	assert (n == 1<<c);
	return c;
}

/* Sampler Cache Initialization */

void init_sampler_cache (sampler_cache *c, int nsets, int assoc, int blocksize, int replacement_policy) {
	int i, j,k,l;
	c->sets = new sampler_set[nsets];
	c->replacement_policy = replacement_policy;
	// c->repl = new CACHE_REPLACEMENT_STATE (nsets, assoc, replacement_policy);
	// c->set_shift = set_shift;
	c->nsets = nsets;
	c->assoc = assoc;
	c->blocksize = blocksize;
	c->offset_bits = log2 (blocksize);
	c->index_bits = log2 (nsets);
	c->tagshiftbits = c->offset_bits + c->index_bits;
	c->index_mask = nsets - 1;
	c->misses = 0;
	c->accesses = 0;
	c->hits = 0;
	c->evictions = 0;
	// memset (c->counts, 0, sizeof (c->counts));
	for (i=0; i<nsets; i++) {
		for (j=0; j<assoc; j++) {
			sampler_block *b = &c->sets[i].blocks[j];
			b->tag = 0;
			b->valid = 0;
			b->Yout = 0;
			for(k=0;k<6;k++)
			{
				b->hash[k]=0;
			}
		}
		c->sets[i].valid = 0;
	}
	for(l=0;l<16;l++)
	{
		memset (PC_History[l], 0, sizeof(PC_History[0]));
	}
}

void sampler_cache_access (UINT32 setIndex, Addr_t tag, unsigned long long int pc, unsigned int *hash)
{
	sampler.accesses++;
	int assoc = sampler.assoc;
	int set_valid = sampler.sets[setIndex].valid;
	sampler_block *v;
	v = &sampler.sets[setIndex].blocks[0];
	int i;
	signed int tmp_out;
	for(i=0;i<assoc;i++)
	{
		if(v[i].tag == tag)
		{
			sampler.hits++;
			if(v[i].lru_stack_position < (assoc-1))
			{
				// bool mispredict = v[i].Yout >= Treplace; 
				if((v[i].Yout > (-THETA)))
				{
					pred_table1[v[i].hash[0]] = (pred_table1[v[i].hash[0]]>(-32)) ? pred_table1[v[i].hash[0]]-1 : -32;
					pred_table2[v[i].hash[1]] = (pred_table2[v[i].hash[1]]>(-32)) ? pred_table2[v[i].hash[1]]-1 : -32;
					pred_table3[v[i].hash[2]] = (pred_table3[v[i].hash[2]]>(-32)) ? pred_table3[v[i].hash[2]]-1 : -32;
					pred_table4[v[i].hash[3]] = (pred_table4[v[i].hash[3]]>(-32)) ? pred_table4[v[i].hash[3]]-1 : -32;
					pred_table5[v[i].hash[4]] = (pred_table5[v[i].hash[4]]>(-32)) ? pred_table5[v[i].hash[4]]-1 : -32;
					pred_table6[v[i].hash[5]] = (pred_table6[v[i].hash[5]]>(-32)) ? pred_table6[v[i].hash[5]]-1 : -32;
				}
			}
			

			//Implement LRU update logic
			Update_sampler_LRU(setIndex,i);
			v[i].tag = tag;
			v[i].valid = 1;
			v[i].hash[0]=hash[0];
			v[i].hash[1]=hash[1];
			v[i].hash[2]=hash[2];
			v[i].hash[3]=hash[3];
			v[i].hash[4]=hash[4];
			v[i].hash[5]=hash[5];
			tmp_out = (int)pred_table1[hash[0]]+(int)pred_table2[hash[1]]+(int)pred_table3[hash[2]]+(int)pred_table4[hash[3]]+(int)pred_table5[hash[4]]+(int)pred_table6[hash[5]];
			if(tmp_out > 255)
			{
				v[i].Yout = 255;
			}
			else if(tmp_out < (-256))
			{
				v[i].Yout = -256;
			}
			else
			{
				v[i].Yout = tmp_out;
			}
			return;
		}
	}
	sampler.misses++;

	if (!set_valid) {
		for (i=0; i<assoc; i++) {
			if (v[i].valid == 0) break;
		}
		if (i == assoc) {
			sampler.sets[setIndex].valid = 1; // mark this set as having only valid blocks so we don't search it again
			set_valid = 1;
		}
	}

	if(set_valid)
	{
		sampler.evictions++;
		// for(i=0;i<assoc;i++)
		// {
		// 	if(v[i].Yout>Treplace) break;
		// }
		// if(i==assoc)
		// {
			i = Get_sampler_LRU_victim(setIndex);
		// }
		
		// Find the LRU victim
		if(v[i].lru_stack_position == (assoc-1))
		{
			//Before evicting the block train the predictor
			bool mispredict = v[i].Yout < Treplace; 
			if((v[i].Yout < THETA) || (mispredict))
			{
				pred_table1[v[i].hash[0]] = (pred_table1[v[i].hash[0]]<32) ? pred_table1[v[i].hash[0]]+1 : 32;
				pred_table2[v[i].hash[1]] = (pred_table2[v[i].hash[1]]<32) ? pred_table2[v[i].hash[1]]+1 : 32;
				pred_table3[v[i].hash[2]] = (pred_table3[v[i].hash[2]]<32) ? pred_table3[v[i].hash[2]]+1 : 32;
				pred_table4[v[i].hash[3]] = (pred_table4[v[i].hash[3]]<32) ? pred_table4[v[i].hash[3]]+1 : 32;
				pred_table5[v[i].hash[4]] = (pred_table5[v[i].hash[4]]<32) ? pred_table5[v[i].hash[4]]+1 : 32;
				pred_table6[v[i].hash[5]] = (pred_table6[v[i].hash[5]]<32) ? pred_table6[v[i].hash[5]]+1 : 32;
			}
		}
	}

	//Store the features,tag,prediction_outcome(Yout)
	Update_sampler_LRU(setIndex,i);
	v[i].tag = tag;
	v[i].valid = 1;
	v[i].hash[0]=hash[0];
	v[i].hash[1]=hash[1];
	v[i].hash[2]=hash[2];
	v[i].hash[3]=hash[3];
	v[i].hash[4]=hash[4];
	v[i].hash[5]=hash[5];

	tmp_out = (int)pred_table1[hash[0]]+(int)pred_table2[hash[1]]+(int)pred_table3[hash[2]]+(int)pred_table4[hash[3]]+(int)pred_table5[hash[4]]+(int)pred_table6[hash[5]];
	if(tmp_out > 255)
	{
		v[i].Yout = 255;
	}
	else if(tmp_out < (-256))
	{
		v[i].Yout = -256;
	}
	else
	{
		v[i].Yout = tmp_out;
	}
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
    init_sampler_cache(&sampler, SAMPLER_SETS, SAMPLER_ASSOC, 64, CRC_REPL_LRU);

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
        return Get_LRU_Victim( setIndex);
    }
    else if( replPolicy == CRC_REPL_RANDOM )
    {
        return Get_Random_Victim( setIndex );
    }
    else if( replPolicy == CRC_REPL_CONTESTANT )
    {
    	if(accessType ==5)
    	{
    		// Contestants:  ADD YOUR VICTIM SELECTION FUNCTION HERE
			// return Get_My_Victim (tid,setIndex, PC, paddr);
			PC = 0x12345678;
    	}
    	return Get_My_Victim (tid,setIndex, PC, paddr);
    	// else
    	// {
    	// 	return -1;
    	// }
        
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
    UINT32 tid, Addr_t PC, UINT32 accessType, bool cacheHit )
{
	//fprintf (stderr, "ain't I a stinker? %lld\n", get_cycle_count ());
	//fflush (stderr);
    // What replacement policy?
    if( replPolicy == CRC_REPL_LRU ) 
    {
        UpdateLRU( setIndex, updateWayID);
    }
    else if( replPolicy == CRC_REPL_RANDOM )
    {
        // Random replacement requires no replacement state update
    }
    else if( replPolicy == CRC_REPL_CONTESTANT )
    {
    	if(accessType ==5)
    	{
    		// Contestants:  ADD YOUR VICTIM SELECTION FUNCTION HERE
			// return Get_My_Victim (tid,setIndex, PC, paddr);
			PC = 0x12345678;
    	}
    	// if(accessType !=5)
    	// {
    		// Contestants:  ADD YOUR UPDATE REPLACEMENT STATE FUNCTION HERE
	        // Feel free to use any of the input parameters to make
	        // updates to your replacement policy
	        UpdateMyPolicy(tid, setIndex, updateWayID, currLine, PC, accessType, cacheHit);
    	// }
        
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
	INT32   lruWay   = 0;
		// Get pointer to replacement state of current set

	LINE_REPLACEMENT_STATE *replSet = repl[ setIndex ];

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

INT32 Get_sampler_LRU_victim(UINT32 setIndex)
{
	INT32   lruWay   = 0;
	sampler_block *v = &sampler.sets[setIndex].blocks[0];

	// Search for victim whose stack position is assoc-1

	for(UINT32 way=0; way<SAMPLER_ASSOC; way++) {
		if (v[way].lru_stack_position == (SAMPLER_ASSOC-1)) {
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

void CACHE_REPLACEMENT_STATE::UpdateLRU( UINT32 setIndex, INT32 updateWayID)
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

void Update_sampler_LRU(UINT32 setIndex, INT32 updateWayID)
{
	sampler_block *v = &sampler.sets[setIndex].blocks[0];
		// Determine current LRU stack position
	UINT32 currLRUstackposition = v[updateWayID].lru_stack_position;

	// Update the stack position of all lines before the current line
	// Update implies incremeting their stack positions by one

	for(UINT32 way=0; way<SAMPLER_ASSOC; way++) {
		if( v[way].lru_stack_position < currLRUstackposition ) {
			v[way].lru_stack_position++;
		}
	}

	// Set the LRU stack position of new line to be zero
	v[updateWayID].lru_stack_position = 0;
}

// /* Hash function for producing a rondomized constant value for an iteration */
// unsigned char hash_func(unsigned long long int i){
// 	unsigned char arr[8];
// 	int j;
// 	int q = 33149;
// 	unsigned char result=0;
// 	for(j=0;j<8;j++)
// 	{
// 		arr[j] = (unsigned char)((i>>(8*j))&0xFF);
// 		result += (unsigned char)(arr[j]*q);
// 	}

// 	return result;
// }

INT32 CACHE_REPLACEMENT_STATE::Get_My_Victim(UINT32 tid, UINT32 setIndex, Addr_t PC, Addr_t paddr) {
	// return first way always
	
	unsigned int hash[6];
	//Addr_t feature[6];
	UINT32 way = 0;
	Addr_t tag = paddr>>(12+6);

	// feature[0]= (PC>>2)%256;
	// feature[1]= (PC_History[0]>>1)%256;
	// feature[2]= (PC_History[1]>>2)%256;
	// feature[3]= (PC_History[2]>>3)%256;
	// feature[4]= (tag>>4)%256;
	// feature[5]= (tag>>7)%256;

	// hash[0] = (unsigned char)((PC^feature[0])&0xFF);
	// hash[1] = (unsigned char)((PC^feature[1])&0xFF);
	// hash[2] = (unsigned char)((PC^feature[2])&0xFF);
	// hash[3] = (unsigned char)((PC^feature[3])&0xFF);
	// hash[4] = (unsigned char)((PC^feature[4])&0xFF);
	// hash[5] = (unsigned char)((PC^feature[5])&0xFF);

	hash[0] = ((PC^(PC>>2))%PRED_TABLE_SIZE);
	hash[1] = ((PC^(PC_History[tid][0]>>1))%PRED_TABLE_SIZE);
	hash[2] = ((PC^(PC_History[tid][1]>>2))%PRED_TABLE_SIZE);
	hash[3] = ((PC^(PC_History[tid][2]>>3))%PRED_TABLE_SIZE);
	hash[4] = ((PC^(tag>>4))%PRED_TABLE_SIZE);
	hash[5] = ((PC^(tag))%PRED_TABLE_SIZE);

	bool bypass = ((int)pred_table1[hash[0]]+(int)pred_table2[hash[1]]+(int)pred_table3[hash[2]]+(int)pred_table4[hash[3]]+(int)pred_table5[hash[4]]+(int)pred_table6[hash[5]])>=Tbypass;
	if(bypass==true)
	{
		return -1;
	}
	else
	{
		for(way=0; way<assoc; way++)
		{
			if(repl[setIndex][way].pred_dead == true)
				break;
		}
		if(way!=assoc)
		{
			return way;
		}
		else
		{
			// if((setIndex%64) == 0)
			// {
			// 	for(way=0; way<assoc; way++)
			// 	{
			// 		if(sampler.sets[setIndex/64].blocks[way].Yout>Treplace) break;
			// 	}
			// 	if(way!=assoc)
			// 	{
			// 		return way;
			// 	}
			// }

			return Get_LRU_Victim(setIndex);
		}
	}
	return 0;
}

/*Update PC History */
void UpdatePCHistory(UINT32 tid, Addr_t PC)
{
	PC_History[tid][2]=PC_History[tid][1];
	PC_History[tid][1]=PC_History[tid][0];
	PC_History[tid][0]=PC;
}

void CACHE_REPLACEMENT_STATE::UpdateMyPolicy(UINT32 tid, UINT32 setIndex, INT32 updateWayID, const LINE_STATE *currLine, Addr_t PC,UINT32 accessType, bool cacheHit) {
	// do nothing
	unsigned int hash[6];
	// int y_out;
	//Addr_t feature[6];
	
	if(!((accessType == 5) && (cacheHit)))
	{
		UpdateLRU(setIndex,updateWayID);

		Addr_t tag = currLine->tag;
		// feature[0]= (PC>>2)%256;
		// feature[1]= (PC_History[0]>>1)%256;
		// feature[2]= (PC_History[1]>>2)%256;
		// feature[3]= (PC_History[2]>>3)%256;
		// feature[4]= (tag>>4)%256;
		// feature[5]= (tag>>7)%256;

		// hash[0] = (unsigned char)((PC^feature[0])&0xFF);
		// hash[1] = (unsigned char)((PC^feature[1])&0xFF);
		// hash[2] = (unsigned char)((PC^feature[2])&0xFF);
		// hash[3] = (unsigned char)((PC^feature[3])&0xFF);
		// hash[4] = (unsigned char)((PC^feature[4])&0xFF);
		// hash[5] = (unsigned char)((PC^feature[5])&0xFF);

		// if(((PC^feature[0])&0xFF) == 0)
		// {
		// 	hash[0]=hash[0];
		// 	//Do nothing
		// }

		hash[0] = ((PC^(PC>>2))%PRED_TABLE_SIZE);
		hash[1] = ((PC^(PC_History[tid][0]>>1))%PRED_TABLE_SIZE);
		hash[2] = ((PC^(PC_History[tid][1]>>2))%PRED_TABLE_SIZE);
		hash[3] = ((PC^(PC_History[tid][2]>>3))%PRED_TABLE_SIZE);
		hash[4] = ((PC^(tag>>4))%PRED_TABLE_SIZE);
		hash[5] = ((PC^(tag))%PRED_TABLE_SIZE);

		repl[setIndex][updateWayID].pred_dead = ((int)pred_table1[hash[0]]+(int)pred_table2[hash[1]]+(int)pred_table3[hash[2]]+(int)pred_table4[hash[3]]+(int)pred_table5[hash[4]]+(int)pred_table6[hash[5]])>=Treplace;
		// if((repl[setIndex][updateWayID].pred_dead == false) && (cacheHit))
		// {
		// 	UpdateLRU(setIndex,updateWayID);
		// }

		if((setIndex%16)==0)
		{
			sampler_cache_access((setIndex/16),tag,PC,hash);
		}
	}

	//Implement PC History Update logic
	UpdatePCHistory(tid, PC);

}

CACHE_REPLACEMENT_STATE::~CACHE_REPLACEMENT_STATE (void) {
}