#include <cmath>
#ifndef NATIVE_SYSTEMC
#include "stratus_hls.h"
#endif

#include "SobelFilter.h"

SobelFilter::SobelFilter( sc_module_name n ): sc_module( n )
{
#ifndef NATIVE_SYSTEMC
	//HLS_FLATTEN_ARRAY(val);
#endif
	SC_THREAD( do_filter );
	sensitive << i_clk.pos();
	dont_initialize();
	reset_signal_is(i_rst, false);
        
#ifndef NATIVE_SYSTEMC
	i_r.clk_rst(i_clk, i_rst);
 i_g.clk_rst(i_clk, i_rst);
 i_b.clk_rst(i_clk, i_rst);
  o_resultr.clk_rst(i_clk, i_rst);
  o_resultg.clk_rst(i_clk, i_rst);
  o_resultb.clk_rst(i_clk, i_rst);
  
#endif
}

SobelFilter::~SobelFilter() {}

// sobel mask
const int mask[MASK_Y][MASK_X] = {{1,2,1},{2,4,2},{1,2,1}};

void SobelFilter::do_filter() {
	{
#ifndef NATIVE_SYSTEMC
		HLS_DEFINE_PROTOCOL("main_reset");
		i_r.reset();
   i_g.reset();
   i_b.reset();
		o_resultr.reset();
   o_resultg.reset();
   o_resultb.reset();
#endif
		wait();
	}
	while (true) {
			HLS_CONSTRAIN_LATENCY(0, 1, "lat00");
			Red = 0,Green = 0, Blue = 0;
      float total = 0;
		for (unsigned int v = 0; v<MASK_Y; ++v) {
			for (unsigned int u = 0; u<MASK_X; ++u) {
				sc_dt::sc_uint<24> r;
        sc_dt::sc_uint<24> g;
        sc_dt::sc_uint<24> b;
#ifndef NATIVE_SYSTEMC
				{
					HLS_DEFINE_PROTOCOL("input");
					r = i_r.get();
              g = i_g.get();
              b = i_b.get();
					wait();
				}
#else
				rgb = i_rgb.read();
#endif
				//unsigned char grey = (rgb.range(7,0) + rgb.range(15,8) + rgb.range(23, 16))/3;
					HLS_CONSTRAIN_LATENCY(0, 2, "lat01");
					Red += r * mask[u][v];
          Green+= g * mask[u][v];
          Blue+= b * mask[u][v];
          
          total+= mask[u][v];
			}
		}
			HLS_CONSTRAIN_LATENCY(0, 8, "lat01");
			 int result1 = (int)(Red/total);
      int result2 = (int)(Green/total);
      int result3 = (int)(Blue/total);
      sc_dt::sc_uint<24> resultr;
      sc_dt::sc_uint<24> resultg;
      sc_dt::sc_uint<24> resultb;
					resultr = result1;
					resultg = result2;
					resultb = result3;
#ifndef NATIVE_SYSTEMC
		{
			HLS_DEFINE_PROTOCOL("output");
			o_resultr.put(resultr);
      o_resultg.put(resultg);
      o_resultb.put(resultb);
			wait();
		}
#else
		o_resultr.write(resultr);
   o_resultg.write(resultg);
   o_resultb.write(resultb);
#endif
	}
}
