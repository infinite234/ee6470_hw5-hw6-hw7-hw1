#include "System.h"
System::System( sc_module_name n, string input_bmp, string output_bmp ): sc_module( n ), 
	tb("tb"), sobel_filter("sobel_filter"), clk("clk", CLOCK_PERIOD, SC_NS), rst("rst"), _output_bmp(output_bmp)
{
	tb.i_clk(clk);
	tb.o_rst(rst);
	sobel_filter.i_clk(clk);
	sobel_filter.i_rst(rst);
	tb.o_r(r);
 tb.o_g(g);
 tb.o_b(b);
	tb.i_resultr(resultr);
 tb.i_resultg(resultg);
 tb.i_resultb(resultb);
	sobel_filter.i_r(r);
 sobel_filter.i_g(g);
 sobel_filter.i_b(b);
	sobel_filter.o_resultr(resultr);
 sobel_filter.o_resultg(resultg);
 sobel_filter.o_resultb(resultb);

  tb.read_bmp(input_bmp);
}

System::~System() {
  tb.write_bmp(_output_bmp);
}
