#ifndef SYSTEM_H_
#define SYSTEM_H_
#include <systemc>
using namespace sc_core;

#include "Testbench.h"
#ifndef NATIVE_SYSTEMC
#include "SobelFilter_wrap.h"
#else
#include "SobelFilter.h"
#endif

class System: public sc_module
{
public:
	SC_HAS_PROCESS( System );
	System( sc_module_name n, std::string input_bmp, std::string output_bmp );
	~System();
private:
  Testbench tb;
#ifndef NATIVE_SYSTEMC
	SobelFilter_wrapper sobel_filter;
#else
	SobelFilter sobel_filter;
#endif
	sc_clock clk;
	sc_signal<bool> rst;
#ifndef NATIVE_SYSTEMC
	cynw_p2p< sc_dt::sc_uint<24> > r;
 cynw_p2p< sc_dt::sc_uint<24> > g;
 cynw_p2p< sc_dt::sc_uint<24> > b;
	cynw_p2p< sc_dt::sc_uint<24> > resultr;
 cynw_p2p< sc_dt::sc_uint<24> > resultg;
 cynw_p2p< sc_dt::sc_uint<24> > resultb;
#else
	sc_fifo< sc_dt::sc_uint<24> > r;
 sc_fifo< sc_dt::sc_uint<24> > g;
 sc_fifo< sc_dt::sc_uint<24> > b;
	sc_fifo< sc_dt::sc_uint<24> > resultr;
 sc_fifo< sc_dt::sc_uint<24> > resultg;
 sc_fifo< sc_dt::sc_uint<24> > resultb;
#endif

	std::string _output_bmp;
};
#endif
