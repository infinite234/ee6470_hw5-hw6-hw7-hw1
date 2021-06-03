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
	i_rgb.clk_rst(i_clk, i_rst);
  o_result.clk_rst(i_clk, i_rst);
#endif
}

SobelFilter::~SobelFilter() {}

// sobel mask
const int mask[MASK_Y][MASK_X] = {{1,2,1},{2,4,2},{1,2,1}};

void SobelFilter::do_filter() {
	{
#ifndef NATIVE_SYSTEMC
		HLS_DEFINE_PROTOCOL("main_reset");
		i_rgb.reset();
		o_result.reset();
#endif
		wait();
	}
	while (true) {
			HLS_CONSTRAIN_LATENCY(0, 1, "lat00");
			Red = 0,Green = 0, Blue = 0;
      float total = 0;
		for (unsigned int v = 0; v<MASK_Y; ++v) {
			for (unsigned int u = 0; u<MASK_X; ++u) {
				sc_dt::sc_uint<24> rgb;
#ifndef NATIVE_SYSTEMC
				{
					HLS_DEFINE_PROTOCOL("input");
					rgb = i_rgb.get();
					wait();
				}
#else
				rgb = i_rgb.read();
#endif
				//unsigned char grey = (rgb.range(7,0) + rgb.range(15,8) + rgb.range(23, 16))/3;
					HLS_CONSTRAIN_LATENCY(0, 1, "lat01");
					Red += rgb.range(7,0) * mask[u][v];
          Green+= rgb.range(15,8) * mask[u][v];
          Blue+= rgb.range(23,16) * mask[u][v];
          
          total+= mask[u][v];
			}
		}
			HLS_CONSTRAIN_LATENCY(0, 1, "lat01");
			 int result1 = (int)(Red/total);
      int result2 = (int)(Green/total);
      int result3 = (int)(Blue/total);
      sc_dt::sc_uint<24> result;
					result.range(7, 0) = result1;
					result.range(15, 8) = result2;
					result.range(23, 16) = result3;
#ifndef NATIVE_SYSTEMC
		{
			HLS_DEFINE_PROTOCOL("output");
			o_result.put(result);
			wait();
		}
#else
		o_result.write(result);
#endif
	}
}
