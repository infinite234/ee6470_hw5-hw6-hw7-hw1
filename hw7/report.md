# Homework-7: In this homework we will implement Data Partitioning of the Gaussian Blur Processing in RISC-V VP platform.

### _Shivani Singh (309591030)_

## Introduction
For implementing Data Partitioning of the Gaussian Blur Processing in RISC-V.
Use RISCV-VP model with TLM 2.0 interface to build the hardware module for the gaussian blur.In this homework, we added two Gaussian Blur modules to the 2-core riscv-vp platform ("tiny32-mc"). The image is partitioned into equal parts and a multi-core program is written to issue the processing to the two modules. 
Write a software application to control the hardware modules through bus.<br/>
In this homework, three types of implementation is done:
-Porting the Gaussian blur module to the "tiny32-mc" platform\\
-Part two is implementing the partitioned gaussian filter in systemc\\
-Porting your test bench as a RISC-V software.


## Implementation details 

### - Gaussian Kernel Generation
The approximation to 3x3 Gaussian kernel used in designing gaussian filter is:\\
```sh
#define filterWidth 3
#define filterHeight 3

double filter[filterHeight][filterWidth] ={{1, 2, 1},{2, 4, 2},{1, 2, 1}};
double factor = 1.0 / 16.0;
double bias = 0.0;
```
After designing the kernel we need to implement it to bitmap image for blurring. The center value of this filter will be the largest. 


### - Applying Gaussian kernel to bitmap image
This code represents the application of the gaussian blur on bitmap image. Lets say the height and width of the image is equal to h and w. Thus to apply filter we need to slide the kernel through the whole image. In this example, to find index of 0th row and oth column after applying blur filter we need to take only some part of kernel thus we will take the kernel indices as (-1,0,1) and will check if the summation of image indices and kernel indices in the range of width and height, the remaining part of kernel not in the bound will be discarded.
This is the part done in main.cpp file in sw.
```sh
for(int i = 0; i < width; i++){
    for(int j = 0; j < length; j++){
      for(int v = -1; v <= 1; v ++){
        for(int u = -1; u <= 1; u++){
          if((v + i) >= 0  &&  (v + i ) < width && (u + j) >= 0 && (u + j) < length ){
            buffer[0] = *(source_bitmap + bytes_per_pixel * ((j + u) * width + (i + v)) + 2);
            buffer[1] = *(source_bitmap + bytes_per_pixel * ((j + u) * width + (i + v)) + 1);
            buffer[2] = *(source_bitmap + bytes_per_pixel * ((j + u) * width + (i + v)) + 0);
            buffer[3] = 0;
          }
 ```
 This is the part done in GaussFilter.cpp file in vp. In this code we are finding the RGB (double) value frrom the source image . since the output will only be blurred, so no need to convert into grayscale. After finding RGB value we will convolve it with kernel values to get the new RGB values. 
```sh
for (unsigned int v = 0; v < MASK_Y; ++v) {
        for (unsigned int u = 0; u < MASK_X; ++u) {
          //unsigned char grey = (i_r.read() + i_g.read() + i_b.read()) / 3;
          wait(CLOCK_PERIOD, SC_NS);

            Red += i_r.read()* G_mask[u][v];
          Green += i_g.read()* G_mask[u][v];
          Blue += i_b.read()* G_mask[u][v];
```
### - SystemC Implementation
The following part shows the dma usage for reading the data from the virtual platform(vp) part and writing it into the software part i.e. the data is written to the tlm bus and then read from there using dma method.
```sh
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
```

This is done in main.cpp file of sw for reading and writing the data i.e. calling the tlm command.
```sh
// reading and writing the data
  sem_wait(&lock);

          if (hart_id == 0) write_data_to_ACC(GaussFILTER_START_ADDR, buffer, 4);
          else write_data_to_ACC(GaussFILTER_START_ADDRA, buffer, 4);
          sem_post(&lock);
          
           sem_wait(&lock);
      if (hart_id == 0) read_data_from_ACC(GaussFILTER_READ_ADDR, buffer, 4);
      else read_data_from_ACC(GaussFILTER_READ_1_ADDRA, buffer, 4);
      sem_post(&lock);
```
Since the value of the kernel value is in integer and the factor for 3*3 matrix is 16 we will edit the target image RGB value for the same positions as of source image to (new RGB value/factor). Thus, successful implementation of applying gaussian blur filter is done.


this is used to reasssign all the previous values that will be used in next iteration merged with the last new column and compute the result.

Initializing initiator and target socket where  simple_initiator_socket of TLM-2.0 can be found in main.cpp and simple_target_socket of TLM-2.0 can be found in GaussFilter.h.
```sh
 #include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"
```
Function tsock.register_b_transport() is used to assign the data as a part of blocking transport. It just mentions the connection between target and initiator socket.
      
 ```sh
tsock("t_skt"),  
  {
    tsock.register_b_transport(this, &GaussFilter::blocking_transport);
  }
```


The transport function blocking_transport() is registered to the target socket of SobelFilter::t_skt. It is called by Testbench to load RGB data into the SobelFilter and read the results back. Note that in this implementation, we assume the data length is always 4 bytes.Since there are 3 results to be given back we take 3 different adresses to give away the results.
```sh
 void gaussFilter::blocking_transport(tlm::tlm_generic_payload &payload,
                                     sc_core::sc_time &delay)
 case tlm::TLM_READ_COMMAND:
 switch (addr) {
          case Gauss_FILTER_RESULT_ADDR:
            buffer.uc[0] = (char)(o_result1.read());
            buffer.uc[1] = (char)(o_result2.read());
            buffer.uc[2] = (char)(o_result3.read());
            buffer.uc[3] = 0;
            break;
          default:
```
In main.cpp, We bind the initiator socket of Testbench to the target sockets of SobelFilter.

```sh
Testbench tb("tb");
  gaussFilter gauss_filter("gauss_filter");
  tb.initiator.i_skt(gauss_filter.t_skt);

```
## Algorithm and Design
The following design is for the FIFO implementation of the gaussian filter in TLM.

![source image](https://github.com/infinite234/ee6470_hw3/blob/main/tlm.png)<br/>



## Simulation Results
The gaussian part implementation using FIFO channels completes in 1245187 ns.
Simulation for gaussian_fifo.<br/>
Solarized dark             |  Solarized Ocean
:-------------------------:|:-------------------------:
![](https://github.com/infinite234/ee6470_hw5-hw6-hw7-hw1/blob/main/hw7/Screenshot%20from%202021-06-29%2011-22-01.png)  |  ![](https://github.com/infinite234/ee6470_hw5-hw6-hw7-hw1/blob/main/hw7/Screenshot%20from%202021-06-29%2011-22-04.png)
![source image](https://github.com/infinite234/ee6470_hw5-hw6-hw7-hw1/blob/main/Screenshot%20from%202021-06-29%2011-21-56.png)<br/>


## Conclusion
Thus, successful implementation of partitioned gaussian blur filter with 2 cores in TLM using TLM initiator socket and target socket in RISCV in both vp and sw has been done resulting to successful blurring lena.bmp and lena_gauss2.bmp image  as shown in results.
