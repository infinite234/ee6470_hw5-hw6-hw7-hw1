# Homework-6: In this homework we will implement Cross-compile Gaussian Blur to RISC-V VP platform.

### _Shivani Singh (309591030)_

## Introduction
For making a gaussian blur filter in RISC-V.
Use RISCV-VP model with TLM 2.0 interface to build the hardware module for the gaussian blur.
Write a software application to control the hardware modules through bus.<br/>
In this homework, three types of implementation is done:
-Porting the Gaussian blur module to the "basic-acc" platform\\
-Part two is implementing the gaussian filter in systemc\\
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
```sh
for (v = -yBound; v != yBound + adjustY; ++v) {
          for (u = -xBound; u != xBound + adjustX; ++u) {
            if (x + u >= 0 && x + u < width && y + v >= 0 && y + v < height) {
              R = *(image_s + byte_per_pixel * (width * (y + v) + (x + u)) + 2);
              G = *(image_s + byte_per_pixel * (width * (y + v) + (x + u)) + 1);
              B = *(image_s + byte_per_pixel * (width * (y + v) + (x + u)) + 0);
               
              Red += R * ker[u + xBound][v + yBound];
              Green += G *ker[u + xBound][v + yBound];
              Blue += B* ker[u + xBound][v + yBound];

              total+= ker[u + xBound][v + yBound];
            }}}
```
In this code we are finding the RGB (double) value frrom the source image . since the output will only be blurred, so no need to convert into grayscale. After finding RGB value we will convolve it with kernel values to get the new RGB values. 

```sh
// Convert into rgb
        *(image_t + byte_per_pixel * (width * y + x) + 2) = int(Red/total);
        *(image_t + byte_per_pixel * (width * y + x) + 1) = int(Green/total);
        *(image_t + byte_per_pixel * (width * y + x) + 0) = int(Blue/total);
```
Since the value of the kernel value is in integer and the factor for 3*3 matrix is 16 we will edit the target image RGB value for the same positions as of source image to (new RGB value/factor). Thus, successful implementation of applying gaussian blur filter is done.


this is used to reasssign all the previous values that will be used in next iteration merged with the last new column and compute the result.
### - SystemC Implementation
In systemc, firstly three processes are made using sc_module i.e. input(read bmp), calculation(apply_gauss) and output(write bmp). So, the tesbench sends the value of r,g,b of target bitmap image to the gauss_filter where gauss_filter reads the r,g,b value and then compute the r,g,b and result value for output image and sends to the tesbench.cpp when the image is written.
```sh
"Testbench.cpp"                      |                "Gauss_filter.cpp"
o_r.write(R);                        |                 double R =i_r.read();
o_g.write(G);                        |                 double G =i_r.read();
o_b.write(B);                        |                 double B = i_b.read();
```

Initializing initiator and target socket where  simple_initiator_socket of TLM-2.0 can be found in Initiator.h and simple_target_socket of TLM-2.0 can be found in SobelFilter.h.
```sh
 #include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"
```
Function read_from_socket() will read dataLen data from addr to rdata[]. Function write_to_socket() will write dataLen data from rdata[] to addr.
      
 ```sh
 initiator.read_from_socket(gauss_FILTER_RESULT_ADDR, mask, data.uc, 4);
 total[0] = data.sint;
initiator.write_to_socket(gauss_FILTER_R_ADDR, mask, data.uc, 4);
```


The transport function blocking_transport() is registered to the target socket of SobelFilter::t_skt. It is called by Testbench to load RGB data into the SobelFilter and read the results back. Note that in this implementation, we assume the data length is always 4 bytes.Since there are 3 results to be given back we take 3 different adresses to give away the results.
```sh
 void gaussFilter::blocking_transport(tlm::tlm_generic_payload &payload,
                                     sc_core::sc_time &delay)
case tlm::TLM_READ_COMMAND:
    switch (addr) {
    case gauss_FILTER_RESULT_ADDR:
          buffer.uint= o_result1.read();
          break;
    case gauss_FILTER_RESULT2_ADDR:
          buffer.uint= o_result2.read();
          break;
    case gauss_FILTER_RESULT3_ADDR:
          buffer.uint= o_result3.read();
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


## Experimental results
The input given to the code is a bitmap file and the output we get is a blur image bitmap file.
lena.bmp is the original image whereas len_gauss.bmp is the  new blurr image. (software.(.cpp))<br/>
lena_std_short.bmp.<br/>

![source image](https://raw.githubusercontent.com/infinite234/ee6470/main/hw1/gauss_fiter_cpp/lena_std_short.bmp)<br/>
lena_gauss.bmp<br/>
![target image](https://raw.githubusercontent.com/infinite234/ee6470/main/hw1/gauss_fiter_cpp/lena_gauss2.bmp)<br/><br/>

## Simulation Results
The gaussian part implementation using FIFO channels completes in 1245187 ns.
Simulation for gaussian_fifo.<br/>

![source image](https://github.com/infinite234/ee6470_hw3/blob/main/Screenshot%20from%202021-03-30%2015-20-47.png)<br/>


## Conclusion
Thus, successful implementation of gaussian blur filter in TLM using TLM initiator socket and target socket has been done resulting to successful blurring lena.bmp and lena_gauss2.bmp image  as shown in results.

# ee6470_hw3
