#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdbool.h>
#include "oslabs.h"

#define BLOCK_SIZE 1024
struct MEMORY_BLOCK NULLBLOCK = {0, 0, 0, 0};

// struct MEMORY_BLOCK {
//         int start_address;
//         int end_address;
//         int segment_size;
//         int process_id; //0 indicates a free block
// };

struct MEMORY_BLOCK best_fit_allocate(int request_size, struct MEMORY_BLOCK memory_map[], int *map_cnt, int process_id) {
    struct MEMORY_BLOCK candidate = {0, 0, 0, 0}; // candidate block
    int min_difference = -1;
    int i;
    int index = -1;

    // loop through memory_map to find the best fit block 
    for (i = 0; i < *map_cnt; i++) {
        // printf("start: %d, end: %d, size: %d, pid: %d\n\n", memory_map[i].start_address, memory_map[i].end_address, memory_map[i].segment_size, memory_map[i].process_id);
        if (memory_map[i].process_id == 0 && memory_map[i].segment_size >= request_size) {
            int difference = memory_map[i].segment_size - request_size;
            if (min_difference == -1 || difference < min_difference) {
                candidate = memory_map[i];
                min_difference = difference;
                index = i;
            }
        }
    }

    // printf("candidate: start: %d, end: %d, size: %d, pid: %d\n\n", candidate.start_address, candidate.end_address, candidate.segment_size, candidate.process_id);

    if (min_difference == -1) {
        // no free block found
        return NULLBLOCK;
    } else {
        // update candidate block
        candidate.process_id = process_id;
        int seg_size = candidate.segment_size;
        int last_address = 0;
        int left_size = seg_size - request_size;

        // split block
        memory_map[index].start_address = candidate.start_address;
        memory_map[index].end_address = candidate.start_address + request_size - 1;
        memory_map[index].segment_size = request_size;
        memory_map[index].process_id = process_id;
        candidate = memory_map[index];
        last_address = memory_map[index].end_address;

        // add new block to memory_map
        if (left_size > 0) {
            memory_map[*map_cnt].start_address = last_address + 1;
            memory_map[*map_cnt].end_address = last_address + left_size;
            memory_map[*map_cnt].segment_size = left_size;
            memory_map[*map_cnt].process_id = 0;
            (*map_cnt)++;
        }

        return candidate;
    }
}


struct MEMORY_BLOCK first_fit_allocate(int request_size, struct MEMORY_BLOCK memory_map[MAPMAX],int *map_cnt, int process_id) {
    int i;
    struct MEMORY_BLOCK first_block = NULLBLOCK;
    int left_size = 0;
    int start_address = 0;
    int end_address = 0;

    // Loop through the memory map
    for (i = 0; i < *map_cnt; i++) {
        if (memory_map[i].process_id == 0 && memory_map[i].segment_size >= request_size) {
            first_block = memory_map[i];
            start_address = first_block.start_address;
            end_address = start_address + request_size - 1;
            break;
        }
    }

    // If no suitable block was found, return the NULLBLOCK
    if (first_block.start_address == 0 && first_block.end_address == 0) {
        return NULLBLOCK;
    }

    if (first_block.segment_size == request_size) {     // percectly fit the memory block
        first_block.process_id = process_id;
        return first_block;
    } else {                                            // split the memory block
        left_size = first_block.segment_size - request_size;
        first_block.start_address = start_address;
        first_block.end_address = start_address + request_size - 1;
        first_block.segment_size = request_size;
        first_block.process_id = process_id;

        memory_map[*map_cnt].start_address = first_block.end_address + 1;
        memory_map[*map_cnt].end_address = first_block.end_address + left_size;
        memory_map[*map_cnt].segment_size = left_size;
        memory_map[*map_cnt].process_id = 0;
        (*map_cnt)++;

        return first_block;
    }
}


struct MEMORY_BLOCK worst_fit_allocate(int request_size, struct MEMORY_BLOCK memory_map[MAPMAX], int *map_cnt, int process_id) {
    int i, max_i = -1;
    int max_size = -1;

    // Find the largest free block
    for (i = 0; i < *map_cnt; i++) {
        if (memory_map[i].process_id == 0 && memory_map[i].segment_size >= request_size) {
            if (memory_map[i].segment_size > max_size) {
                max_size = memory_map[i].segment_size;
                max_i = i;
            }
        }
    }

    // If a large enough block was found
    if (max_i != -1) {
        // If the block is exactly the same size as the requested size
        if (memory_map[max_i].segment_size == request_size) {
            memory_map[max_i].process_id = process_id;
            return memory_map[max_i];
        }
        // If the block is larger than the requested size
        else {
            struct MEMORY_BLOCK new_block;
            new_block.start_address = memory_map[max_i].start_address;
            new_block.end_address = memory_map[max_i].start_address + request_size - 1;
            new_block.segment_size = request_size;
            new_block.process_id = process_id;

            memory_map[max_i].start_address = new_block.end_address + 1;
            memory_map[max_i].segment_size -= request_size;

            return new_block;
        }
    }
    // If no large enough block was found
    else {
        struct MEMORY_BLOCK null_block = {0, 0, 0, 0};
        return null_block;
    }
}


struct MEMORY_BLOCK next_fit_allocate(int request_size, struct MEMORY_BLOCK memory_map[MAPMAX], int *map_cnt, int process_id, int last_address) {
    struct MEMORY_BLOCK result = NULLBLOCK;
    int start_searching = 0;
    int i;

    for (i = 0; i < *map_cnt; i++) {
        if (memory_map[i].process_id == 0 && memory_map[i].segment_size >= request_size) {
            if (memory_map[i].start_address >= last_address) {
                result = memory_map[i];
                break;
            } else if (!start_searching) {
                start_searching = 1;
                result = memory_map[i];
            }
        }
    }

    if (result.segment_size == 0) {
        return NULLBLOCK;
    }

    if (result.segment_size > request_size) {
        int new_start = result.start_address + request_size;
        int new_end = result.end_address;
        int new_size = result.segment_size - request_size;

        result.segment_size = request_size;
        result.end_address = result.start_address + request_size - 1;
        result.process_id = process_id;

        memory_map[*map_cnt].start_address = new_start;
        memory_map[*map_cnt].end_address = new_end;
        memory_map[*map_cnt].segment_size = new_size;
        memory_map[*map_cnt].process_id = 0;
        (*map_cnt)++;
    } else {
        result.process_id = process_id;
    }

    return result;
}


void release_memory(struct MEMORY_BLOCK freed_block, struct MEMORY_BLOCK memory_map[MAPMAX], int *map_cnt) {
    // Mark the block as free by updating its process id to -1
    freed_block.process_id = -1;

    // Merge the freed block with any adjacent free blocks if they exist
    int i;
    for (i = 0; i < *map_cnt; i++) {
        // Check if the freed block is adjacent to a free block
        if (memory_map[i].end_address + 1 == freed_block.start_address) {
            // Update the end address and size of the previous free block
            memory_map[i].end_address = freed_block.end_address;
            memory_map[i].segment_size = memory_map[i].end_address - memory_map[i].start_address + 1;   //11111

            // Remove the freed block from the memory map
            int j;
            for (j = i + 1; j < *map_cnt - 1; j++) {
                memory_map[j] = memory_map[j + 1];
            }
            *map_cnt -= 1;
            break;
        }
        else if (freed_block.end_address + 1 == memory_map[i].start_address) {
            // Update the start address and size of the next free block
            memory_map[i].start_address = freed_block.start_address;        //1111111
            memory_map[i].segment_size = memory_map[i].end_address - memory_map[i].start_address + 1;

            // Remove the freed block from the memory map
            int j;
            for (j = i + 1; j < *map_cnt - 1; j++) {
                memory_map[j] = memory_map[j + 1];
            }
            *map_cnt -= 1;
            break;
        }
    }

    // If the freed block was not merged with any adjacent blocks, add it to the memory map
    if (i == *map_cnt) {
        memory_map[*map_cnt] = freed_block;
        *map_cnt += 1;
    }
}

