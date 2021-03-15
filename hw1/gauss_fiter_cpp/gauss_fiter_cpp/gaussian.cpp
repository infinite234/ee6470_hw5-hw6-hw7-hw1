/*
Filename    : sobel.cpp
Compiler    : Clang++ 8.0.0
Description : Demo the how to use sobel detector on gray level image
*/
#define sigma 1

# define bias 0
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>
#include <stdio.h>


using namespace std;
typedef vector<double> Array;
typedef vector<Array> mat;



const int MASK_X = 3;
const int MASK_Y = 3;
const int WHITE = 255;
const int BLACK = 0;

unsigned char *image_s = nullptr; // source image array
unsigned char *image_t = nullptr; // target image array
FILE *fp_s = nullptr;             // source file handler
FILE *fp_t = nullptr;             // target file handler

int width = 0;               // image width
 int height = 0;              // image height
unsigned int rgb_raw_data_offset = 0; // RGB raw data offset
unsigned char bit_per_pixel = 0;      // bit per pixel
unsigned short byte_per_pixel = 0;    // byte per pixel

// bitmap header
unsigned char header[54] = {
    0x42,          // identity : B
    0x4d,          // identity : M
    0,    0, 0, 0, // file size
    0,    0,       // reserved1
    0,    0,       // reserved2
    54,   0, 0, 0, // RGB data offset
    40,   0, 0, 0, // struct BITMAPINFOHEADER size
    0,    0, 0, 0, // bmp width
    0,    0, 0, 0, // bmp height
    1,    0,       // planes
    24,   0,       // bit per pixel
    0,    0, 0, 0, // compression
    0,    0, 0, 0, // data size
    0,    0, 0, 0, // h resolution
    0,    0, 0, 0, // v resolution
    0,    0, 0, 0, // used colors
    0,    0, 0, 0  // important colors
};



int read_bmp(const char *fname_s) {
  fp_s = fopen(fname_s, "rb");
  if (fp_s == nullptr) {
    std::cerr << "fopen fp_s error" << std::endl;
    return -1;
  }

  // move offset to 10 to find rgb raw data offset
  fseek(fp_s, 10, SEEK_SET);
  assert(fread(&rgb_raw_data_offset, sizeof(unsigned int), 1, fp_s));

  // move offset to 18 to get width & height;
  fseek(fp_s, 18, SEEK_SET);
  assert(fread(&width, sizeof(unsigned int), 1, fp_s));
  assert(fread(&height, sizeof(unsigned int), 1, fp_s));

  // get bit per pixel
  fseek(fp_s, 28, SEEK_SET);
  assert(fread(&bit_per_pixel, sizeof(unsigned short), 1, fp_s));
  byte_per_pixel = bit_per_pixel / 8;

  // move offset to rgb_raw_data_offset to get RGB raw data
  fseek(fp_s, rgb_raw_data_offset, SEEK_SET);

  size_t size = width * height * byte_per_pixel;
  
  image_s = reinterpret_cast<unsigned char *>(new void *[size]);
  if (image_s == nullptr) {
    std::cerr << "allocate image_s error" << std::endl;
    return -1;
  }

  image_t = reinterpret_cast<unsigned char *>(new void *[size]);
  if (image_t == nullptr) {
    std::cerr << "allocate image_t error" << std::endl;
    return -1;
  }

  assert(fread(image_s, sizeof(unsigned char),
               (size_t)(long)width * height * byte_per_pixel, fp_s));
  fclose(fp_s);

  return 0;
}

// convert RGB to gray level int
inline int color_to_int(int r, int g, int b) { return (r + g + b) / 3; }

/*mat gaussian_ker(int x,int x1){
    mat ker(x, Array(x1));
    double sum=0.0;*/
    
    /*for(int i =0; i< x;i++){
    	for (int j = 0;j<x1;j++){
            ker[i][j] = exp(-(i*i+j*j)/(2*sigma*sigma))/(2*M_PI*sigma*sigma);
            sum += ker[i][j];
        }
    }
        for (int i=0 ; i<x ; i++) {
        for (int j=0 ; j<x1 ; j++) {
            ker[i][j] /= sum;
            //printf("%f",ker[i][j]);
	    //printf("\t");
            
        }
    }*/
    
  
/*double ker[3][3] =
{
  0.077847, 0.123317, 0.077847,
  0.123317, 0.195346, 0.123317,
  0.077847, 0.123317, 0.077847,
};

return ker; 
}*/

const int ker[3][3] =
{
  {1, 2, 1},
  {2,4,2},
  {1,2,1}
};
static int factor =1/16;

void apply_gauss(void) {
  unsigned int x, y, i, v, u, total; // for loop counter
  double R, G, B;      // color of R, G, B
  double Red,Green, Blue= 0.0;
  int adjustX, adjustY, xBound, yBound;


  for (y = 0; y != height; ++y) {
    for (x = 0; x != width; ++x) {
        adjustX = (MASK_X % 2) ? 1 : 0;
        adjustY = (MASK_Y % 2) ? 1 : 0;
        xBound = MASK_X / 2;
        yBound = MASK_Y / 2;

        Red =0.0,Green = 0.0,Blue = 0.0;
        total = 0;
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
            }
          }
        }

     
        // Convert into rgb
        *(image_t + byte_per_pixel * (width * y + x) + 2) = int(Red/total);
        *(image_t + byte_per_pixel * (width * y + x) + 1) = int(Green/total);
        *(image_t + byte_per_pixel * (width * y + x) + 0) = int(Blue/total);
      } 
    }
  }


int write_bmp(const char *fname_t) {
  unsigned int file_size = 0; // file size

  fp_t = fopen(fname_t, "wb");
  if (fp_t == nullptr) {
    std::cerr << "fopen fname_t error" << std::endl;
    return -1;
  }

  // file size
  file_size = width * height * byte_per_pixel + rgb_raw_data_offset;
  header[2] = (unsigned char)(file_size & 0x000000ff);
  header[3] = (file_size >> 8) & 0x000000ff;
  header[4] = (file_size >> 16) & 0x000000ff;
  header[5] = (file_size >> 24) & 0x000000ff;

  // width
  header[18] = width & 0x000000ff;
  header[19] = (width >> 8) & 0x000000ff;
  header[20] = (width >> 16) & 0x000000ff;
  header[21] = (width >> 24) & 0x000000ff;

  // height
  header[22] = height & 0x000000ff;
  header[23] = (height >> 8) & 0x000000ff;
  header[24] = (height >> 16) & 0x000000ff;
  header[25] = (height >> 24) & 0x000000ff;

  // bit per pixel
  header[28] = bit_per_pixel;

  // write header
  fwrite(header, sizeof(unsigned char), rgb_raw_data_offset, fp_t);

  // write image
  fwrite(image_t, sizeof(unsigned char),
         (size_t)(long)width * height * byte_per_pixel, fp_t);

  fclose(fp_t);

  return 0;
}

int main(void) {
  assert(read_bmp("lena.bmp") == 0); // 24 bit gray level image
  //mat ker = gaussian_ker(3,3);
  apply_gauss();
  assert(write_bmp("lena_gauss.bmp") == 0);
  delete (image_s);
  delete (image_t);

  return 0;
}
