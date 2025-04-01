// This file will NOT match correct.txt, but it is incorrect in a meaningful
// way. The physical addresses will be different because, since we have more
// pages coming in than memory available, a FIFO algorithm will replace pages.
// Thus, physical addresses will not match once pages are removed. Instead of
// the physical address being capped at 65k, it will be capped at 32k and loop
// "circularly".

#include <stdio.h>
#include <stdlib.h>

#define MAX_ADDRESSES 1000
#define TLB_SIZE 16
#define PAGE_TABLE_SIZE 128
#define PHYSICAL_MEMORY_SIZE 128
#define PAGE_SIZE 256

int virtualAddress;
int physicalAddress;
int memValue;
int physicalMemoryIndex = 0;
int TLBIndex = 0;
int pageTableIndex = 0;
int TLBHits = 0;
int pageFaults = 0;

typedef struct {
  int value;
  int page;
  int offset;
  int physicalAddress;
} address;

// Used by pageTable, stored in binary
typedef struct {
  int page;
  int frame;
} pageInfo;

// Used by the TLB
typedef struct {
    int page;
    int frame;
    int idle;
  } tlbInfo;

// utilized by physical memory to store the page number and the page's actual
// binary data. the frames are the actual index of this object in the
// physicalMemory array. for example, if page 5 is stored at index 15, it is
// frame 15.
typedef struct {
  signed char *pageData;
} physicalMemoryBlock;

// instantiate arrays
address addresses[MAX_ADDRESSES];
pageInfo pageTable[PAGE_TABLE_SIZE];
pageInfo TLB[TLB_SIZE]; // using FIFO
tlbInfo realTLB[TLB_SIZE]; //When using LRU
physicalMemoryBlock physicalMemory[PHYSICAL_MEMORY_SIZE];

// conversion and loading methods ---------------------------------------------

// Converts base 10 numbers to base 2, but omits leading 0's
long long decToBinary(long long decimal) {
    long long binary = 0;
    long long place = 1;
  
    while (decimal > 0) {
      binary += (decimal % 2) * place;
      place *= 10;
      decimal /= 2;
    }
    return binary;
  }
  
  // Converts base 2 numbers to base 2
  long long binaryToDec(long long binary) {
    long long decimal = 0;
    long long base = 1;
  
    while (binary > 0) {
      decimal += (binary % 10) * base;
      binary /= 10;
      base *= 2;
    }
    return decimal;
  }

int loadAddresses() {
  FILE *file;
  long long value;
  long long binValue;
  long long divisor = 100000000;
  int pageNumber;
  int pageOffset;
  int index = 0;

  file = fopen("addresses.txt", "r");
  if (file == NULL) {
    perror("Error opening addresses file");
    return 1;
  }

  while (fscanf(file, "%lld", &value) == 1 && index < MAX_ADDRESSES) {
    // convert integer to binary in order to retrieve page num and offset
    binValue = decToBinary(value);
    // split binary into first 8 and last 8 digits (exclude useless zeros)
    pageNumber = binValue / divisor;
    long long leftMultipler = pageNumber * divisor;
    pageOffset = binValue - leftMultipler;
    // return to decimal so we can iterate through pages with pageNumber and the
    // offset
    addresses[index].page = binaryToDec(pageNumber);
    addresses[index].offset = binaryToDec(pageOffset);
    addresses[index].value = value;
    index++;
  }

  fclose(file);
  return 0;
}

// reads entire page data and returns it (to be assigned to a physical memory's
// page's pageData)
signed char *readBinPage(int page) {
  int index = page * PAGE_SIZE;

  FILE *file = fopen("BACKING_STORE.bin", "rb");
  if (!file) {
    perror("Error opening file");
    return NULL;
  }

  // make 256 char sized buffer for the page data
  signed char *buffer = (signed char *)malloc(PAGE_SIZE);
  if (!buffer) {
    perror("Memory allocation failed");
    fclose(file);
    return NULL;
  }

  fseek(file, index, SEEK_SET);
  fread(buffer, sizeof(signed char), PAGE_SIZE, file);
  fclose(file);

  return buffer;
}

// ------------------------------------------------------------------

// if page is found, return index in the TLB itself, else return -1
int searchTLB(int page) {
  for (int i = 0; i < TLB_SIZE; i++) {
    if (realTLB[i].page == page) {
      return realTLB[i].frame;
    }
  }
  return -1;
}

// use FIFO to add to the TLB
void addToTLBFIFO(int page, int frame) {
  // if TLB is full,
  if (TLB[TLB_SIZE - 1].page != -1) {
    for (int i = 0; i < TLB_SIZE; i++) {
      TLB[i] = TLB[i + 1];
    }
    // Set the last element equal to the new page and frame
    TLB[TLB_SIZE - 1].page = page;
    TLB[TLB_SIZE - 1].frame = frame;
  } else {
    // TLB is not full. Add at TLB Index and then increment TLB Index.
    realTLB[TLBIndex].page = page;
    TLB[TLBIndex].frame = frame;
    TLBIndex++;
  }
}

// use LRU to add to the TLB
void addToTLBLRU(int page, int frame) {
    // if TLB is full
    if (realTLB[TLB_SIZE - 1].page != -1) {
      int highIdle = 0;
      for (int i = 0; i < TLB_SIZE; i++) {
        realTLB[i].idle++;
        // Get TLB index for highest idle page
        if (realTLB[i].idle > realTLB[highIdle].idle) {
          highIdle = i;
        }
      }
      realTLB[highIdle].page = page;
      realTLB[highIdle].frame = frame;
    } else {
      //TLB is not full. Add at TLB Index and then increment the TLB index.
      realTLB[TLBIndex].page = page;
      realTLB[TLBIndex].frame = frame;
      TLBIndex++;
    }
}

// takes the actual page data and puts it into physical memory at the specified
// frame
void addToPhysicalMemory(int page, int frame) {
  physicalMemory[frame].pageData = readBinPage(page);
  // Increment physical memory index so we can put the next page in the
  // successive spot
  physicalMemoryIndex++;

  // if memory is full (if the index is at max), go back to beginning,
  // simulating FIFO (first index will be overwritten)
  if (physicalMemoryIndex == PHYSICAL_MEMORY_SIZE) {
    physicalMemoryIndex = 0;
  }
}

// Use FIFO to add to the page table
void addToPageTable(int page, int frame) {
  // If pageTable is full,
  if (pageTableIndex == PAGE_TABLE_SIZE) {
    for (int i = 0; i < PAGE_TABLE_SIZE; i++) {
      pageTable[i] = pageTable[i + 1];
    }
    // Set the last element equal to the new page and frame
    pageTable[PAGE_TABLE_SIZE - 1].page = page;
    pageTable[PAGE_TABLE_SIZE - 1].frame = frame;
  } else {
    // page table is not full. Add at page table index and then increment page
    // table index
    pageTable[pageTableIndex].page = page;
    pageTable[pageTableIndex].frame = frame;
    pageTableIndex++;
  }
}

// if page is found, return index in the table itself, else return -1
int searchPageTable(int page) {
  for (int i = 0; i < PAGE_TABLE_SIZE; i++) {
    if (pageTable[i].page == page) {
      return pageTable[i].frame;
    }
  }
  return -1;
}

int main() {
  // create output.txt file
  FILE *output = fopen("output.txt", "w");

  if (loadAddresses() != 0) {
    return -1;
  }

  // Instantiate pageTable array and TLB array
  for (int i = 0; i < PAGE_TABLE_SIZE; i++) {
    pageTable[i].frame = -1;
    pageTable[i].page = -1;
  }
  for (int i = 0; i < TLB_SIZE; i++) {
    realTLB[i].frame = -1;
    realTLB[i].page = -1;
  }

  int frame;        // stores current frame of page
  signed char data; // stores current data of page

  // Main loop, iterates through list of addresses
  for (int i = 0; i < MAX_ADDRESSES; i++) {
    // search TLB for current page.
    frame = searchTLB(addresses[i].page);
    if (frame == -1) {
      // TLB miss, search page table for current page
      frame = searchPageTable(addresses[i].page);
      if (frame == -1) {
        pageFaults++;
        // Page fault. Update the frame to
        // the next free memory position, and load page into memory. (calling
        // the add to physical memory function increments physicalMemoryIndex,
        // so we must save the frame before doing anything.) Then, load into the
        // page table with the frame it was loaded into in physical memory.
        frame = physicalMemoryIndex;
        addToPhysicalMemory(addresses[i].page, physicalMemoryIndex);
        addresses[i].physicalAddress = (frame * PAGE_SIZE) + addresses[i].offset;

        // Add the page to the page table at frame, storing the page
        // and frame info, and grab the data at the memory spot.
        addToPageTable(addresses[i].page, frame);
        data = physicalMemory[frame].pageData[addresses[i].offset];
      } else {
        // Page was found in page table. Get the data and set the physical
        // address.
        data = physicalMemory[frame].pageData[addresses[i].offset];
        addresses[i].physicalAddress = frame * PAGE_SIZE + addresses[i].offset;
      }
      // Add the page to the TLB. This will run regardless of what happened,
      // following the true nature of the TLB.
      addToTLBLRU(addresses[i].page, frame);
    } else {
      // page was found in TLB. save the data and physical address.
      if (i == 206) {
        printf("Here");
    }
      TLBHits++;
      data = physicalMemory[frame].pageData[addresses[i].offset];
      addresses[i].physicalAddress = frame * PAGE_SIZE + addresses[i].offset;
    }
    // write the data to output.txt
    fprintf(output, "Virtual address: %d Physical address: %d Value: %d\n", addresses[i].value, addresses[i].physicalAddress, data);
  }
  // calculate TLB hit rate and page fault rate
  float tlbHitRate = (float)TLBHits / MAX_ADDRESSES;
  float pageFaultRate = (float)pageFaults / MAX_ADDRESSES;
  fprintf(output, "TLB hit rate: %d/%d, or %.2f%%\n", TLBHits, MAX_ADDRESSES, tlbHitRate * 100);
  fprintf(output, "Page fault rate: %d/%d, or %.2f%%\n", pageFaults, MAX_ADDRESSES, pageFaultRate * 100);

  fclose(output);

  for (int i = 0; i < PHYSICAL_MEMORY_SIZE; i++) {
    free(physicalMemory[i].pageData);
  }

  return 0;
}
