//
// Data Prefetching Championship Simulator 2
// Seth Pugsley, seth.h.pugsley@intel.com
//

/*

  This file implements a version of AMP prefetcher

 */
// #include <bits/stdc++.h>
#include <stdio.h>
#include <stdlib.h>
#include "../inc/prefetcher.h"



typedef struct page {
  unsigned long long int addr;    // the page address
  unsigned long long int issued_addr; // the page address who prefetched this page
  int last;   // set to 1 if it is the last page in a set of prefetched pages
  int p;      // degree of prefetch
  int g;      // trigger distance
} page;

page cache[1000];  // strucure which stores the prefetced pages
int curr_entries = 0;   // a counter

int min(int a, int b){
  if(a < b)
    return a;
  return b;
}

int max(int a, int b){
  if(a > b)
    return a;
  return b;
}


int curr_offset = 1;  // start from degree of prefetch = 1
float trigger = 0.2;  // the variable which is used to calculate the trigger distance


void l2_prefetcher_initialize(int cpu_num)
{
  for(int i = 0; i < 1000; i++)
    cache[i].addr = -1;
  // you can inspect these knob values from your code to see which configuration you're runnig in
  //printf("Knobs visible from prefetcher: %d %d %d\n", knob_scramble_loads, knob_small_llc, knob_low_bandwidth);
}

void l2_prefetcher_operate(int cpu_num, unsigned long long int addr, unsigned long long int ip, int cache_hit)
{
    

  // uncomment this line to see all the information available to make prefetch decisions
  // printf("(0x%llx 0x%llx %d %d %d) ", addr, ip, cache_hit, get_l2_read_queue_occupancy(0), get_l2_mshr_occupancy(0));
  if(cache_hit == 1){
    for(int i =0; i < 1000; i++){
      if(cache[i].addr == addr>>6){
        // printf("%d\n",  );
        if(cache[i].addr >= cache[i].issued_addr + cache[i].p - cache[i].g){ //if the page that is hit is within the trigger distance of the prefetched set or not
          addr = cache[i].issued_addr + cache[i].p;
          l2_prefetch_line(0, addr<<6, (addr + curr_offset)<<6, FILL_LLC);       
          for(int j = 1; j <= curr_offset; j++){
            int k;
            for(k =0; k < 1000; k++){
              if(cache[k].addr == -1){    // a cache entry is issued for each  prefetched page
                cache[k].addr = addr + j;
                if(j == curr_offset)
                  cache[k].last = 1;
                else
                  cache[k].last = 0;
                cache[k].p = curr_offset;
                cache[k].issued_addr = addr;
                cache[k].g = trigger*curr_offset;
              }
              break;
            }
            if(k == 1000){        // if all the entries were previously allocated, then find the one which was updated 
                cache[curr_entries%1000].addr = addr + j;
                if(j == curr_offset)
                  cache[curr_entries%1000].last = 1;
                else
                  cache[curr_entries%1000].last = 0;
                cache[curr_entries%1000].p = curr_offset;
                cache[curr_entries%1000].issued_addr = addr;
                cache[curr_entries%1000].g = trigger*curr_offset;
                curr_entries++;
            }  
          }
        }
        if(cache[i].last == 1){
                
          l2_prefetch_line(0, (cache[i].addr + 1)<<6, (cache[i].addr + curr_offset)<<6, FILL_L2);        
          if(curr_offset < 60)
            curr_offset *= 2;
          if(trigger < 1)
            trigger += 0.1;
        }
        break;
      }
    }
  }
  else{
    l2_prefetch_line(0, addr, ((addr>>6) + curr_offset)<<6, FILL_LLC);       
    for(int j = 1; j <= curr_offset; j++){
      int i;
      for(i =0; i < 1000; i++){
        if(cache[i].addr == -1){
          cache[i].addr = (addr>>6) + j;
          if(j == curr_offset)
            cache[i].last = 1;
          else
            cache[i].last = 0;
          cache[i].p = curr_offset;
          cache[i].issued_addr = addr>>6;
          cache[i].g = trigger*curr_offset;
        }
        break;
      }
      if(i == 1000){
          cache[curr_entries%1000].addr = (addr>>6) + j;
          if(j == curr_offset)
            cache[curr_entries%1000].last = 1;
          else
            cache[curr_entries%1000].last = 0;
          cache[curr_entries%1000].p = curr_offset;
          cache[curr_entries%1000].issued_addr = addr>>6;
          cache[curr_entries%1000].g = trigger*curr_offset;
          curr_entries++;
      }  
    }
  }
  
}

void l2_cache_fill(int cpu_num, unsigned long long int addr, int set, int way, int prefetch, unsigned long long int evicted_addr)
{
  // uncomment this line to see the information available to you when there is a cache fill event
  // printf("0x%llx %d %d %d 0x%llx\n", addr, set, way, prefetch, evicted_addr);
  
  for(int i = 0; i < 1000; i++){
    if(cache[i].addr == evicted_addr>>6){
      if(cache[i].last == 1){
        if(curr_offset > 1)
          curr_offset -= 1;
        if(trigger >= 0.1)
          trigger -= 0.1;
      }
    }
  }

  if(prefetch == 1){
    int i;
    for(i =0; i < 1000; i++){
      if(cache[i].addr == addr>>6){
          if(cache[i].last == 1){
            cache[i].p = max(cache[i].p, cache[i].g + 1);
          }
        break;    
      }
    }
  }
 
}

void l2_prefetcher_heartbeat_stats(int cpu_num)
{
  //////printf("Prefetcher heartbeat stats\n");
}

void l2_prefetcher_warmup_stats(int cpu_num)
{
  //////printf("Prefetcher warmup complete stats\n\n");
}

void l2_prefetcher_final_stats(int cpu_num)
{
  //////printf("Prefetcher final stats\n");
}
