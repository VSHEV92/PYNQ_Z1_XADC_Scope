#ifndef scope_trigger_h
#define scope_trigger_h

#include "ap_int.h"
#include "hls_stream.h"

// режим работы триггера
typedef ap_uint<2> trig_mode_t;
#define AUTO 0
#define TRIGGER 1
#define ONCE 2
#define ONCE_TRIG 3

// разрядность входных данных и уровня триггера
typedef ap_int<16> trig_level_t;
typedef ap_int<16> in_data_t;

// размер DMA пакета и коэффициент прореживания
const unsigned int BLOCK_SIZE = 512;
typedef ap_uint<5> downsamp_t;

// структуры для входного и выходного AXIS
struct in_data_AXI{
	in_data_t   data;
    ap_uint<4>  id;  // номер канала XADC
};

struct out_data_AXI{
	in_data_t   data;
    ap_uint<1>  last;
};

#endif
