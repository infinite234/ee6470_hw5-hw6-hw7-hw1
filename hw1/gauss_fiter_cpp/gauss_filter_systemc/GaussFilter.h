#ifndef Gauss_FILTER_H_
#define Gauss_FILTER_H_
#include <systemc>
using namespace sc_core;

#include "filter_def.h"

class GaussFilter : public sc_module {
public:
  sc_in_clk i_clk;
  sc_in<bool> i_rst;
  sc_fifo_in<unsigned char> i_r;
  sc_fifo_in<unsigned char> i_g;
  sc_fifo_in<unsigned char> i_b;
  sc_fifo_out<int> o_result;

  SC_HAS_PROCESS(GaussFilter);
  GaussFilter(sc_module_name n);
  ~GaussFilter() = default;

private:
  void do_filter();
  int Red;
  int Blue;
  int Green;
};
#endif
