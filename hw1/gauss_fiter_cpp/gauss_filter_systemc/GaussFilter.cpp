#include <cmath>

#include "GaussFilter.h"

GaussFilter::GaussFilter(sc_module_name n) : sc_module(n) {
  SC_THREAD(do_filter);
  sensitive << i_clk.pos();
  dont_initialize();
  reset_signal_is(i_rst, false);
}

// sobel mask
/*const int mask[MASK_X][MASK_Y] = {{{-1, -2, -1}, {0, 0, 0}, {1, 2, 1}},

                                          {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}}};*/
const int ker[ker_x][ker_y] = {{1,2,1},{2,4,2},{1,2,1}};

void GaussFilter::do_filter() {
  { wait(); }
  while (true) {
      Red =0; Blue =0; Green = 0;
      wait();
    float total = 0;
    for (unsigned int v = 0; v < ker_y; ++v) {
      for (unsigned int u = 0; u < ker_x; ++u) {
        
        double R = i_r.read();
        double G = i_r.read();
        double B =  i_b.read();
        wait();
          Red += R * ker[u][v];
          Green+=G*  ker[u][v];
          Blue += B* ker[u][v];
           
          wait();
          
           total+= ker[u][v];
           wait();
      }
    }
    
    int result = (int)(total);
    o_result.write(result);
  }
}
