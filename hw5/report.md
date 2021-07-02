# Homework-5: In this homework, we will synthesize the kernel function of the Gaussian blur and annotate the timing back to the platform from previous homework and make a comparison of the simulated cycles between splited and non-splited input features.ALSO Showing the resource usage of the synthesized circuits. 
### _Shivani Singh (309591030)_

## Introduction
For making a gaussian blur filter in TLM.
The system architecture is similar as gaussian_fifo as done in hw2 and a top-level module System is instantiated to contain Testbench and Gaussian filter.
Compared to hw2, the three R, G, and B channels are combined as a channel for non - splitted case and for splitted R,G,B values are used as a different registers.
Instead of sc_fifo, the data channels are defined as the synthesizable streaming interface (cynw_p2p<>) of Stratus HLS.
The directory stratus contains the Stratus HLS project file (project.tcl) and Makefile to run Stratus HLS.
In this homework, two types of implementation is done:
-all modules will be connected through point-to-point TLM.\\
-Part two is implementing the gaussian filter in systemc


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
System.h includes the generated GaussFilter_wrap.h instead of GaussFilter.h. And the class GaussFilter is replaced by GaussFilter_wrapper.In systemc, firstly three processes are made using sc_module i.e. input(read bmp), calculation(apply_gauss) and output(write bmp).
```sh
{
	tb.i_clk(clk);
	tb.o_rst(rst);
	sobel_filter.i_clk(clk);
	sobel_filter.i_rst(rst);
	tb.o_rgb(rgb);
	tb.i_result(result);
	sobel_filter.i_rgb(rgb);
	sobel_filter.o_result(result);

  tb.read_bmp(input_bmp);
}

System::~System() {
  tb.write_bmp(_output_bmp);
}

```

In System.h, sc_fifo is replaced with cynw_p2p.
For non splitted,
```sh
	cynw_p2p< sc_dt::sc_uint<24> > rgb;
	cynw_p2p< sc_dt::sc_uint<24> > result;

```
For splitted,
```sh
	cynw_p2p< sc_dt::sc_uint<8> >::in i_r;
  cynw_p2p< sc_dt::sc_uint<8> >::in i_g;
  cynw_p2p< sc_dt::sc_uint<8> >::in i_g;
	cynw_p2p< sc_dt::sc_uint<8> >::out o_result1;
  cynw_p2p< sc_dt::sc_uint<8> >::out o_result2;
  cynw_p2p< sc_dt::sc_uint<8> >::out o_result3;


```

This implementation is done in gauss Filter.cpp where the results are taken in non splitted format and the R G B values are also taken in non splitted format.
 In SobelFilter.h, sc_fifo_in and sc_fifo_out are replaced with cynw_p2p<>::in and cynw_p2p<>::out respectively. 
 getting input,
 ```sh
  sc_dt::sc_uint<24> rgb;
					Red += rgb.range(7,0) * mask[u][v];
          Green+= rgb.range(15,8) * mask[u][v];
          Blue+= rgb.range(23,16) * mask[u][v];


```
putting output,
```sh
sc_dt::sc_uint<24> result;
					result.range(7, 0) = result1;
					result.range(15, 8) = result2;
					result.range(23, 16) = result3;


```

In Testbench.h, sc_fifo_out and sc_fifo_in are replaced with cynw_p2p<>::base_out and cynw_p2p<>::base_in respectively.
```sh
	cynw_p2p< sc_dt::sc_uint<24> >::base_out o_rgb;
	cynw_p2p< sc_dt::sc_uint<24> >::base_in i_result;

```



In Testbench.cpp and SobelFilter.cpp, the read and write access methods for sc_fifo are replaced with the get and put methods for cynw_p2p<>.
 So, the tesbench sends the value of r,g,b of target bitmap image to the gauss_filter where gauss_filter reads the r,g,b value and then compute the r,g,b and result value for output image and sends to the tesbench.cpp when the image is written.
```sh
"Testbench.cpp"                      |                "Gauss_filter.cpp"
o_rgb.put(rgb);                    |                 rgb = i_rgb.get();
total = i_result.get();            |                 o_result.put(result);                
```
for splitted,
```sh
"Testbench.cpp"                      |                "Gauss_filter.cpp"
o_r.put(r);                          |               r = i_r.get();
o_g.put(g);                          |               g = i_g.get();
o_b.put(b);                          |               b = i_b.get();
total1 = i_result1.get();            |                 o_result1.put(result1);   
total2 = i_result2.get();            |                 o_result2.put(result2);  
total3 = i_result3.get();            |                 o_result3.put(result3);  
```

## Algorithm and Design
The following design is for the FIFO implementation of the gaussian filter in STRATUS HLS.

non splitted            |  non splitted
:-------------------------:|:-------------------------:
![](https://github.com/infinite234/ee6470_hw5-hw6-hw7-hw1/blob/main/hw5/siv%20(1).png)  |  ![](https://github.com/infinite234/ee6470_hw5-hw6-hw7-hw1/blob/main/hw5/siv.png)


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
FOR SPLITTED,
![source image](https://github.com/infinite234/ee6470_hw5-hw6-hw7-hw1/blob/main/hw5/SPLITTED.PNG)<br/>

FOR NON SPLITTED,<br/>
![source image](https://github.com/infinite234/ee6470_hw5-hw6-hw7-hw1/blob/main/hw5/SHIV_STRATUS.PNG)<br/>


BASIC Splitted,<br/>
![source image](https://github.com/infinite234/ee6470_hw5-hw6-hw7-hw1/blob/main/hw5/Capture.PNG)<br/>

DPA Splitted,<br/>
![source image](https://github.com/infinite234/ee6470_hw5-hw6-hw7-hw1/blob/main/hw5/Capture1.PNG)<br/>
![source image](https://github.com/infinite234/ee6470_hw5-hw6-hw7-hw1/blob/main/hw5/Capture2.PNG)<br/>
Area metrics
![source image](https://github.com/infinite234/ee6470_hw5-hw6-hw7-hw1/blob/main/hw5/Capture3.PNG)<br/>

## Conclusion
Thus, successful implementation of gaussian blur filter in TLM using TLM initiator socket and target socket has been done resulting to successful blurring lena.bmp and lena_gauss2.bmp image  as shown in results.
