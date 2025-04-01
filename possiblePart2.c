// This file will NOT match correct.txt, but it is incorrect in a meaningful
// way. The physical addresses will be different because, since we have more
// pages coming in than memory available, a FIFO algorithm will replace pages.
// Thus, physical addresses will not match once pages are removed. Instead of
// the physical address being capped at 65k, it will be capped at 32k and loop
// "circularly".

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_ADDRESSES 20
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

// Used by TLB and pageTable, stored in binary
typedef struct {
  int page;
  int frame;
  long long timeStamp;
} pageInfo;

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
pageInfo TLB[TLB_SIZE];
physicalMemoryBlock physicalMemory[PHYSICAL_MEMORY_SIZE];

// conversion and loading methods ---------------------------------------------

long long int decToBinary(long int decimal) {
  long long int binary = 0;
  long long int place = 1;

  while (decimal > 0) {
    long long int remainder = decimal % 2;
    binary += remainder * place;
    place *= 10;
    decimal /= 2;
  }
  return binary;
}

long int binaryToDec(long int binary) {
  long int decimal = 0;
  long int base = 1;
  long int remainder;

  while (binary > 0) {
    remainder = binary % 10;
    decimal += remainder * base;
    binary /= 10;
    base *= 2;
  }

  return decimal;
}

int loadAddresses() {
  FILE *file;
  long long int value;
  long long int binValue;
  long long int divisor = 100000000;
  int pageNumber;
  int pageOffset;
  int index = 0;

  file = fopen("addresses.txt", "r");
  if (file == NULL) {
    perror("Error opening addresses file");
    return -1;
  }

  while (fscanf(file, "%lld", &value) == 1 && index < MAX_ADDRESSES) {
    // convert integer to binary in order to retrieve page num and offset
    binValue = decToBinary(value);
    // split binary into first 8 and last 8 digits (exclude useless zeros)
    pageNumber = binValue / divisor;
    long int leftMultipler = pageNumber * divisor;
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

// used for timestamps
long long get_nanoseconds() {
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);             // Get current time
  return (long long)ts.tv_sec * 1e9 + ts.tv_nsec; // Convert to nanoseconds
}

int findVictimInPageTable() {
  long long oldestTimeStamp = pageTable[0].timeStamp;
  int victim = 0;
  for (int i = 0; i < PAGE_TABLE_SIZE; i++) {
    if (pageTable[i].timeStamp <
        oldestTimeStamp) { // if the current page is older than current oldest,
      oldestTimeStamp =
          pageTable[i].timeStamp; // set the oldest to the current page
      victim = i;
    }
  }

  return victim;
}

int findVictimInTLB() {
  long long oldestTimeStamp = TLB[0].timeStamp;
  int victim = 0;
  for (int i = 0; i < TLB_SIZE; i++) {
    if (TLB[i].timeStamp < oldestTimeStamp) {
      oldestTimeStamp = TLB[i].timeStamp;
      victim = i;
    }
  }

  return victim;
}

// ------------------------------------------------------------------

// if page is found, return index in the TLB itself, else return -1
int searchTLB(int page) {
  for (int i = 0; i < TLB_SIZE; i++) {
    if (TLB[i].page == page) {
      TLB[i].timeStamp = get_nanoseconds(); // Update timestamp since we just used it
      return TLB[i].frame;
    }
  }
  return -1;
}

// use FIFO to add to the TLB (CHANGE TO LRU)
void addToTLB(int page, int frame) {
  // if TLB is full,
  if (TLBIndex == TLB_SIZE) {
    int victimIndex = findVictimInTLB();
    // Set the LRU element equal to the new page and frame
    TLB[victimIndex].page = page;
    TLB[victimIndex].frame = frame;
    TLB[victimIndex].timeStamp = get_nanoseconds();
  } else {
    // TLB is not full. Add at TLB Index and then increment TLB Index.
    TLB[TLBIndex].page = page;
    TLB[TLBIndex].frame = frame;
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
}

// LRU algorithm in memory
void addToPhysicalMemoryWithVictim(int victimFrame, int newPage) {
    physicalMemory[victimFrame].pageData = readBinPage(newPage);
  // No need to increment physical memory index since we are full
}

// now returns the frame of the removed victim so we can remove it in memory
// too.
int addToPageTable(int page, int frame) {
  // if we don't find a victim, return -1.
  int victimFrame = -1;
  // If pageTable is full, eliminate the LRU element
  if (pageTableIndex == PAGE_TABLE_SIZE) {
    int victimIndex = findVictimInPageTable();
    // Set the LRU element equal to the new page and frame
    victimFrame = pageTable[victimIndex].frame;
    pageTable[victimIndex].page = page;
    pageTable[victimIndex].frame = frame;
    pageTable[victimIndex].timeStamp = get_nanoseconds();
  } else {
    // page table is not full. Add at page table index and then increment page
    // table index
    pageTable[pageTableIndex].page = page;
    pageTable[pageTableIndex].frame = frame;
    pageTable[pageTableIndex].timeStamp = get_nanoseconds();
    pageTableIndex++;
  }

  return victimFrame;
}

// if page is found, return index in the table itself, else return -1
int searchPageTable(int page) {
  for (int i = 0; i < PAGE_TABLE_SIZE; i++) {
    if (pageTable[i].page == page) {
      pageTable[i].timeStamp =
          get_nanoseconds(); // Update timestamp since we just used it
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
    pageTable[i].timeStamp = -1;
  }
  for (int i = 0; i < TLB_SIZE; i++) {
    TLB[i].frame = -1;
    TLB[i].page = -1;
    TLB[i].timeStamp = -1;
  }

  int frame;        // stores current frame of page
  signed char data; // stores current data of page

  // Main loop, iterates through list of addresses
  for (int i = 0; i < MAX_ADDRESSES; i++) {
    // search TLB for current page.
    printf("Searching tlb: %d\n", i);
    frame = searchTLB(addresses[i].page);
    printf("tlb searched. frame: %d\n", frame);
    if (frame == -1) {
      // TLB miss, search page table for current page
      printf("Searching page table\n");
      frame = searchPageTable(addresses[i].page);
      printf("page table searched\n");

      if (frame == -1) {
        pageFaults++;
        // Page fault. Update the frame to
        // the next free memory position, and load page into memory. (calling
        // the add to physical memory function increments physicalMemoryIndex,
        // so we must save the frame before doing anything.) Then, load into the
        // page table with the frame it was loaded into in physical memory.
        if (physicalMemoryIndex != 128) {
            frame = physicalMemoryIndex;
        } else {
            frame = findVictimInPageTable();
        }
        // Add the page to the page table at frame, storing the page
        // and frame info, and grab the data at the memory spot.
        printf("adding to page table, frame: %d\n", frame);
        int victimFrame = addToPageTable(addresses[i].page, frame);
        printf("added to page table\n");
        
        if (physicalMemoryIndex == 128) {
            frame = victimFrame;
        }

        // memory is full, we need to remove LRU page
        printf("Victim frame: %d\n", victimFrame);
        if (victimFrame != -1) {
            printf("adding to memory when its full\n");
          addToPhysicalMemoryWithVictim(victimFrame, addresses[i].page); // make new method that takes victim frame as an index
          addresses[i].physicalAddress = (victimFrame * PAGE_SIZE) + addresses[i].offset;
          data = physicalMemory[victimFrame].pageData[addresses[i].offset];
          printf("added to full memory\n");
        } else {
          // memory is not full, continue with scheduled programming
          printf("adding to memory when its not full\n");
          addToPhysicalMemory(addresses[i].page, physicalMemoryIndex);
          addresses[i].physicalAddress =
              (frame * PAGE_SIZE) + addresses[i].offset;
          printf("Grabbing data at frame: %d\n", frame);
          data = physicalMemory[frame].pageData[addresses[i].offset];
          printf("added to not full memory\n");
        }

      } else {
        // Page was found in page table. Get the data and set the physical
        // address.
        printf("page was found in page table\n");
        data = physicalMemory[frame].pageData[addresses[i].offset];
        addresses[i].physicalAddress = frame * PAGE_SIZE + addresses[i].offset;
        printf("page was found in page table done\n");
      }
      // Add the page to the TLB. This will run regardless of what happened,
      // following the true nature of the TLB.
      printf("adding to tlb\n");
      addToTLB(addresses[i].page, frame);
      printf("tlb added to\n");
    } else {
      // page was found in TLB. save the data and physical address.
      printf("page found in tlb\n");
      TLBHits++;
      data = physicalMemory[frame].pageData[addresses[i].offset];
      addresses[i].physicalAddress = frame * PAGE_SIZE + addresses[i].offset;
      printf("page saved from tlb\n");
    }
    // write the data to output.txt
    fprintf(output, "Virtual address: %d Physical address: %d Value: %d\n",
            addresses[i].value, addresses[i].physicalAddress, data);
  }
  // calculate TLB hit rate and page fault rate
  float tlbHitRate = (float)TLBHits / MAX_ADDRESSES;
  float pageFaultRate = (float)pageFaults / MAX_ADDRESSES;
  fprintf(output, "TLB hit rate: %d/%d, or %.2f%%\n", TLBHits, MAX_ADDRESSES,
          tlbHitRate * 100);
  fprintf(output, "Page fault rate: %d/%d, or %.2f%%\n", pageFaults,
          MAX_ADDRESSES, pageFaultRate * 100);

  fclose(output);

  for (int i = 0; i < PHYSICAL_MEMORY_SIZE; i++) {
    free(physicalMemory[i].pageData);
  }

  return 0;
}
