#include <string.h>
#include "stdio.h"
#include "math.h"
#include "lena_std_short.h"

// Gaussian Filter ACC
static char* const GaussFILTER_START_ADDR = reinterpret_cast<char* const>(0x73000000);
static char* const GaussFILTER_READ_ADDR  = reinterpret_cast<char* const>(0x73000004);

// Gaussian Filter ACC 1
static char* const GaussFILTER_START_ADDRA = reinterpret_cast<char* const>(0x74000000);
static char* const GaussFILTER_READ_ADDRA  = reinterpret_cast<char* const>(0x74000004);

// DMA 
static volatile uint32_t * const DMA_SRC_ADDR  = (uint32_t * const)0x40000000;
static volatile uint32_t * const DMA_DST_ADDR  = (uint32_t * const)0x40000004;
static volatile uint32_t * const DMA_LEN_ADDR  = (uint32_t * const)0x40000008;
static volatile uint32_t * const DMA_OP_ADDR   = (uint32_t * const)0x4000000C;
static volatile uint32_t * const DMA_STAT_ADDR = (uint32_t * const)0x40000010;
static const uint32_t DMA_OP_MEMCPY = 1;

bool _is_using_dma = true;

unsigned int ReadfromByteArray(unsigned char* array, unsigned int offset) {
	unsigned int output = (array[offset] << 0) | (array[offset + 1] << 8) | (array[offset + 2] << 16) | (array[offset + 3] << 24);
	return output;
}

void write_data_to_ACC(char* ADDR, unsigned char* buffer, int len){
  if(_is_using_dma){  
    // Using DMA 
    *DMA_SRC_ADDR = (uint32_t)(buffer);
    *DMA_DST_ADDR = (uint32_t)(ADDR);
    *DMA_LEN_ADDR = len;
    *DMA_OP_ADDR  = DMA_OP_MEMCPY;
  }else{
    // Directly Send
    memcpy(ADDR, buffer, sizeof(unsigned char)*len);
  }
}
void read_data_from_ACC(char* ADDR, unsigned char* buffer, int len){
  if(_is_using_dma){
    // Using DMA 
    *DMA_SRC_ADDR = (uint32_t)(ADDR);
    *DMA_DST_ADDR = (uint32_t)(buffer);
    *DMA_LEN_ADDR = len;
    *DMA_OP_ADDR  = DMA_OP_MEMCPY;
  }else{
    // Directly Read
    memcpy(buffer, ADDR, sizeof(unsigned char)*len);
  }
}

int sem_init (uint32_t *__sem, uint32_t count) __THROW
{
  *__sem=count;
  return 0;
}

int sem_wait (uint32_t *__sem) __THROW
{
  uint32_t value, success; //RV32A
  __asm__ __volatile__("\
L%=:\n\t\
     lr.w %[value],(%[__sem])            # load reserved\n\t\
     beqz %[value],L%=                   # if zero, try again\n\t\
     addi %[value],%[value],-1           # value --\n\t\
     sc.w %[success],%[value],(%[__sem]) # store conditionally\n\t\
     bnez %[success], L%=                # if the store failed, try again\n\t\
"
    : [value] "=r"(value), [success]"=r"(success)
    : [__sem] "r"(__sem)
    : "memory");
  return 0;
}

int sem_post (uint32_t *__sem) __THROW
{
  uint32_t value, success; //RV32A
  __asm__ __volatile__("\
L%=:\n\t\
     lr.w %[value],(%[__sem])            # load reserved\n\t\
     addi %[value],%[value], 1           # value ++\n\t\
     sc.w %[success],%[value],(%[__sem]) # store conditionally\n\t\
     bnez %[success], L%=                # if the store failed, try again\n\t\
"
    : [value] "=r"(value), [success]"=r"(success)
    : [__sem] "r"(__sem)
    : "memory");
  return 0;
}

int barrier(uint32_t *__sem, uint32_t *__lock, uint32_t *counter, uint32_t thread_count) {
	sem_wait(__lock);
	if (*counter == thread_count - 1) { //all finished
		*counter = 0;
		sem_post(__lock);
		for (int j = 0; j < thread_count - 1; ++j) sem_post(__sem);
	} else {
		(*counter)++;
		sem_post(__lock);
		sem_wait(__sem);
	}
	return 0;
}

//Total number of cores
//static const int PROCESSORS = 2;
#define PROCESSORS 2
//the barrier synchronization objects
uint32_t barrier_counter=0; 
uint32_t barrier_lock; 
uint32_t barrier_sem; 
//the mutex object to control global summation
uint32_t lock;  
//print synchronication semaphore (print in core order)
uint32_t print_sem[PROCESSORS]; 
float pi_over_4 = 0;
unsigned char result_saver[260][260][4];

int main(unsigned hart_id) {

	/////////////////////////////
	// thread and barrier init //
	/////////////////////////////
	if (hart_id == 0) {
		// create a barrier object with a count of PROCESSORS
		sem_init(&barrier_lock, 1);
		sem_init(&barrier_sem, 0); //lock all cores initially
		for(int i=0; i< PROCESSORS; ++i){
			sem_init(&print_sem[i], 0); //lock printing initially
		}
		// Create mutex lock
		sem_init(&lock, 1);
	}

	/////////////////////////////////////////
	// accumulate local sum to shared data //
	/////////////////////////////////////////
  unsigned char* source_array= lena_std_short_bmp;

  unsigned int input_rgb_raw_data_offset = ReadfromByteArray(source_array, 10);
  unsigned int width = ReadfromByteArray(source_array, 18);
  unsigned int length = ReadfromByteArray(source_array, 22);
  unsigned int bytes_per_pixel = ReadfromByteArray(source_array, 28) / 8;
  unsigned char* source_bitmap = &source_array[input_rgb_raw_data_offset];
  sem_wait(&lock);
  printf ("hart_id = %d\n",hart_id);
  printf("======================================\n");
  printf("\t  Reading from array\n");
  printf("======================================\n");
  printf(" input_rgb_raw_data_offset\t= %d\n", input_rgb_raw_data_offset);
  printf(" width\t\t\t\t= %d\n", width);
  printf(" length\t\t\t\t= %d\n", length);
  printf(" bytes_per_pixel\t\t= %d\n",bytes_per_pixel);
  printf("======================================\n");
  sem_post(&lock);

  int start_width = width / PROCESSORS * hart_id, end_width = width / PROCESSORS * hart_id + width / PROCESSORS;
  unsigned char  buffer[4] = {0};
  for(int i = 0; i < width; i++){
    for(int j = 0; j < length; j++){
      for(int v = -1; v <= 1; v ++){
        for(int u = -1; u <= 1; u++){
          if((v + i) >= 0  &&  (v + i ) < width && (u + j) >= 0 && (u + j) < length ){
            buffer[0] = *(source_bitmap + bytes_per_pixel * ((j + u) * width + (i + v)) + 2);
            buffer[1] = *(source_bitmap + bytes_per_pixel * ((j + u) * width + (i + v)) + 1);
            buffer[2] = *(source_bitmap + bytes_per_pixel * ((j + u) * width + (i + v)) + 0);
            buffer[3] = 0;
          }else{
            buffer[0] = 0;
            buffer[1] = 0;
            buffer[2] = 0;
            buffer[3] = 0;
          }
          sem_wait(&lock);

          if (hart_id == 0) write_data_to_ACC(GaussFILTER_START_ADDR, buffer, 4);
          else write_data_to_ACC(GaussFILTER_START_ADDRA, buffer, 4);
          sem_post(&lock);

        }
      }

      sem_wait(&lock);
      if (hart_id == 0) read_data_from_ACC(GaussFILTER_READ_ADDR, buffer, 4);
      else read_data_from_ACC(GaussFILTER_READ_1_ADDRA, buffer, 4);
      sem_post(&lock);

    }
  }

	////////////////////////////
	// barrier to synchronize //
	////////////////////////////
	//Wait for all threads to finish
	barrier(&barrier_sem, &barrier_lock, &barrier_counter, PROCESSORS);

	if (hart_id == 0) {  // Core 0 print first and then others
    sem_wait(&lock);
      }
    }
    sem_post(&lock);
	} 
      }
      sem_post(&lock);
		}
	}

	return 0;
}


