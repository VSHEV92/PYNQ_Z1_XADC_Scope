#include "scope_trigger.h"

// блок реализует функцию триггера осцилографа и выдает в данные DMA с заданным коэффициентом прореживания
// триггер срабатывает при пересечении уровня сверху вниз

void scope_trigger( hls::stream<in_data_AXI>&   in_data,     // данные от XADC
		            hls::stream<out_data_AXI>&  out_data,    // данные для DMA
		            trig_mode_t                 trig_mode,   // режим триггера
					trig_level_t                trig_level,  // уровень триггера
					downsamp_t                  downsamp,    // коэффицент прореживания
					bool                        once_start   // старт однократной выдачи данных (срабатываем по фронту)
				){

#pragma HLS INTERFACE s_axilite port=return bundle=ctrl
#pragma HLS INTERFACE s_axilite port=trig_mode bundle=ctrl
#pragma HLS INTERFACE s_axilite port=trig_level bundle=ctrl
#pragma HLS INTERFACE s_axilite port=downsamp bundle=ctrl
#pragma HLS INTERFACE s_axilite port=once_start bundle=ctrl

#pragma HLS INTERFACE axis register both port=in_data
#pragma HLS INTERFACE axis register both port=out_data

static trig_mode_t trig_mode_internal = AUTO;

// счетчики числа отсчетов
static unsigned int samp_count = 0;
static unsigned int downsamp_count = 0;

// флаг пересечения уровня триггера
static bool trig_flag = false;

// сигналы для однократной выдачи
static bool once_start_new = false;
bool once_start_old;
static bool once_start_flag = false;

// входные и выходные данные
static in_data_AXI in_data_samp_new = {0, 0};
in_data_AXI in_data_samp_old;
out_data_AXI out_data_samp;

// ----------------------------------------------------------------------------------------------
once_start_old = once_start_new;
once_start_new = once_start;

in_data_samp_old = in_data_samp_new;
in_data >> in_data_samp_new;

switch (trig_mode_internal)
{
    // режим постоянной выдачи без триггра
    case AUTO:
    	downsamp_count++;
		if (downsamp_count >= downsamp){
			downsamp_count = 0;
            samp_count++;
	        out_data_samp.data = in_data_samp_new.data;
	        if (samp_count == BLOCK_SIZE){
		        samp_count = 0;
		        out_data_samp.last = 1;
		        trig_mode_internal = trig_mode; // обновляем режим триггра после выдачи блока
	        } else
	    	    out_data_samp.last = 0;

	        out_data << out_data_samp;
		}
        break;

    // режим выдачи при пересечении уровня триггера
    case TRIGGER:
	    if (trig_flag == false){
	 	    if (in_data_samp_old.data < trig_level && in_data_samp_new.data >= trig_level)
		        trig_flag = true;
	 	    else
	 	    	trig_mode_internal = trig_mode; // если триггер не сработал, можем обновить режим триггера
	    } else {
	    	downsamp_count++;
	    	if (downsamp_count >= downsamp){
	    		downsamp_count = 0;
		        samp_count++;
		        out_data_samp.data = in_data_samp_new.data;
		        if (samp_count == BLOCK_SIZE){ // после выдачи заданного числа отсчетов сбрасываем флаг и обновляем режим триггра
		            samp_count = 0;
		            out_data_samp.last = 1;
		            trig_flag = false;
		    	    trig_mode_internal = trig_mode;
		        } else
		   	        out_data_samp.last = 0;

		        out_data << out_data_samp;
	        }
	    }
        break;

    // режим однократной выдачи
    case ONCE:
    	if (once_start_flag == false){
    	    if (once_start_old == false && once_start_new == true)
    	    	once_start_flag = true;
    	    else
    	    	trig_mode_internal = trig_mode; // если старт еще не нажат, можем обновить режим триггера
    	} else {
    		downsamp_count++;
    	    if (downsamp_count >= downsamp){
    	    	downsamp_count = 0;
    	        samp_count++;
    		    out_data_samp.data = in_data_samp_new.data;
    		    if (samp_count == BLOCK_SIZE){ // после выдачи заданного числа отсчетов сбрасываем флаг и обновляем режим триггра
    		        samp_count = 0;
    		        out_data_samp.last = 1;
    		        once_start_flag = false;
    				trig_mode_internal = trig_mode;
    			} else
    			    out_data_samp.last = 0;

    		    out_data << out_data_samp;
    	    }
    	}
    	break;

    // режим однократной выдачи при пересечении уровня триггера
    default:
    	if (once_start_flag == false){
    	    if (once_start_old == false && once_start_new == true)
    	        once_start_flag = true;
    	    else
    	        trig_mode_internal = trig_mode; // если старт еще не нажат, можем обновить режим триггера
    	} else if (trig_flag == false){
    	    if (in_data_samp_old.data < trig_level && in_data_samp_new.data >= trig_level)
    		    trig_flag = true;
    		else
    			if (trig_mode_internal != trig_mode) // если произошло переключение режима, сбрасываем флаг
    				once_start_flag = false;
    		    trig_mode_internal = trig_mode;
    	} else {
    		downsamp_count++;
    		if (downsamp_count >= downsamp){
    		    downsamp_count = 0;
    	        samp_count++;
    	        out_data_samp.data = in_data_samp_new.data;
    	        if (samp_count == BLOCK_SIZE){  // после выдачи заданного числа отсчетов сбрасываем флаги и обновляем режим триггра
    	            samp_count = 0;
    	    	    out_data_samp.last = 1;
    	    		once_start_flag = false;
    	    		trig_flag = false;
    	    		trig_mode_internal = trig_mode;
    	    	} else
    	            out_data_samp.last = 0;

    	        out_data << out_data_samp;
    	    }
    	}
    }

}
