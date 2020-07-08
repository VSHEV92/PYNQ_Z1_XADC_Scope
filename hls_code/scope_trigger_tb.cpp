#include "scope_trigger.h"
#include <iostream>

void scope_trigger( hls::stream<in_data_AXI>&   in_data,     // данные от XADC
		            hls::stream<out_data_AXI>&  out_data,    // данные для DMA
		            trig_mode_t                 trig_mode,   // режим триггера
					trig_level_t                trig_level,  // уровень триггера
					blocks_num_t                blocks_num,  // количество выдаваемых в DMA блоков
					bool                        once_start   // старт однократной выдачи данных (срабатываем по фронту)
				);

int main(){
	// входной и выходной данные
	hls::stream<in_data_AXI>  in_data;
	hls::stream<out_data_AXI> out_data;
	in_data_AXI in_data_samp;
	out_data_AXI out_data_samp;

    // управляющие сигналы
	trig_mode_t    trig_mode;
	trig_level_t   trig_level;
	blocks_num_t   blocks_num;
	bool           once_start;

	bool stream_empty;
	int samp_value;

	// генерируем входные данные
	for (int i = 0; i < BLOCK_SIZE*300; i++){
		samp_value = (i % 130) - 65;
		in_data_samp.data = samp_value;
		in_data_samp.id = 0;
		in_data << in_data_samp;
	}

	// --------------------------------------------
	// ---------- тест режима AUTO ---------------
	trig_mode = AUTO;
	trig_level = 0;
	blocks_num = 1;
	once_start = false;
	for (int i = 0; i < BLOCK_SIZE*3; i++)
		scope_trigger(in_data, out_data, trig_mode, trig_level, blocks_num, once_start);

	std::cout << "----------------------------------------------------------" << std::endl;
	std::cout << "-------------------- Test AUTO ---------------------------" << std::endl;
	stream_empty = false;
	while(!stream_empty){
		out_data >> out_data_samp;
		std::cout << "Data = " << out_data_samp.data <<"\t Last = " << out_data_samp.last << std::endl;
		stream_empty = out_data.empty();
	}

	// --------------------------------------------
	// ---------- тест режима TRIGGER ------------
	trig_mode = TRIGGER;
	trig_level = 10;
	blocks_num = 2;
	once_start = false;
	for (int i = 0; i < BLOCK_SIZE*50; i++)
		scope_trigger(in_data, out_data, trig_mode, trig_level, blocks_num, once_start);

	std::cout << "----------------------------------------------------------" << std::endl;
	std::cout << "-------------------- Test TRIGGER ------------------------" << std::endl;
	stream_empty = false;
	while(!stream_empty){
		out_data >> out_data_samp;
		std::cout << "Data = " << out_data_samp.data <<"\t Last = " << out_data_samp.last << std::endl;
		stream_empty = out_data.empty();
	}


	// --------------------------------------------
	// ---------- тест режима ONCE ------------
	trig_mode = ONCE;
	trig_level = 10;
	blocks_num = 4;
	once_start = false;
	for (int i = 0; i < BLOCK_SIZE*16; i++){
		scope_trigger(in_data, out_data, trig_mode, trig_level, blocks_num, once_start);
		if (i == BLOCK_SIZE*3-5 || i == BLOCK_SIZE*10+8)
			once_start = true;
		else
			once_start = false;
	}

	std::cout << "----------------------------------------------------------" << std::endl;
	std::cout << "----------------------- Test ONCE ------------------------" << std::endl;
	stream_empty = false;
	while(!stream_empty){
		out_data >> out_data_samp;
		std::cout << "Data = " << out_data_samp.data <<"\t Last = " << out_data_samp.last << std::endl;
		stream_empty = out_data.empty();
	}

	// --------------------------------------------
	// ---------- тест режима ONCE_TRIG ------------
	trig_mode = ONCE_TRIG;
	trig_level = -17;
	blocks_num = 3;
	once_start = false;
	for (int i = 0; i < BLOCK_SIZE*176; i++){
		scope_trigger(in_data, out_data, trig_mode, trig_level, blocks_num, once_start);
		if (i == BLOCK_SIZE*3-5 || i == BLOCK_SIZE*100+13)
			once_start = true;
		else
			once_start = false;
	}

	std::cout << "----------------------------------------------------------" << std::endl;
	std::cout << "----------------------- Test ONCE_TRIG -------------------" << std::endl;
	stream_empty = false;
	while(!stream_empty){
		out_data >> out_data_samp;
		std::cout << "Data = " << out_data_samp.data <<"\t Last = " << out_data_samp.last << std::endl;
		stream_empty = out_data.empty();
	}





	return 0;
}
