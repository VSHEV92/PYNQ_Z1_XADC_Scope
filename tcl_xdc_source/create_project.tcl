# если проект уже существует, удаляем его
if {[file exist ../PYNQ_Z1_XADC_Scope_Prj]} {
    file delete -force ../PYNQ_Z1_XADC_Scope_Prj
}

# создаем проект и указываем board file
create_project PYNQ_Z1_XADC_Scope_Prj ../PYNQ_Z1_XADC_Scope_Prj -part xc7z020clg400-1
set_property board_part www.digilentinc.com:pynq-z1:part0:1.0 [current_project]

# настраиваем ip cache каталого
config_ip_cache -import_from_project -use_cache_location ../ip_cache
update_ip_catalog

# добавляем репозиторий с HLS IP ядром
set_property  ip_repo_paths  ../hls_code [current_project]
update_ip_catalog

# добавляем xdc file с LOC constraints
add_files -fileset constrs_1 -norecurse LOC.xdc

# создаем block design, добавляем zynq и делаем block automation
create_bd_design "zynq_bd"
create_bd_cell -type ip -vlnv xilinx.com:ip:processing_system7:5.5 processing_system7_0
apply_bd_automation -rule xilinx.com:bd_rule:processing_system7 -config {make_external "FIXED_IO, DDR" apply_board_preset "1" Master "Disable" Slave "Disable" }  [get_bd_cells processing_system7_0]

# удаляем всю переферию, которая добавлена по умолчанию
set_property -dict [list CONFIG.PCW_USE_M_AXI_GP0 {0} CONFIG.PCW_EN_CLK0_PORT {0} CONFIG.PCW_EN_RST0_PORT {0} CONFIG.PCW_QSPI_PERIPHERAL_ENABLE {0} CONFIG.PCW_QSPI_GRP_SINGLE_SS_ENABLE {1} CONFIG.PCW_ENET0_PERIPHERAL_ENABLE {0} CONFIG.PCW_SD0_PERIPHERAL_ENABLE {0} CONFIG.PCW_UART0_PERIPHERAL_ENABLE {0} CONFIG.PCW_USB0_PERIPHERAL_ENABLE {0} CONFIG.PCW_GPIO_MIO_GPIO_ENABLE {0}] [get_bd_cells processing_system7_0]

# добавляем UART_0 в Zynq и устанавливаем baud rate в 9600
set_property -dict [list CONFIG.PCW_UART0_BAUD_RATE {9600} CONFIG.PCW_UART0_PERIPHERAL_ENABLE {1} CONFIG.PCW_UART0_UART0_IO {MIO 14 .. 15}] [get_bd_cells processing_system7_0]

# добавляем в Zynq Ethernet_0
set_property -dict [list CONFIG.PCW_ENET0_PERIPHERAL_ENABLE {1} CONFIG.PCW_ENET0_ENET0_IO {MIO 16 .. 27} CONFIG.PCW_ENET0_GRP_MDIO_ENABLE {1} CONFIG.PCW_ENET0_GRP_MDIO_IO {MIO 52 .. 53}] [get_bd_cells processing_system7_0]

# добавляем в Zynq порт для тактирования PL
set_property -dict [list CONFIG.PCW_EN_CLK0_PORT {1}] [get_bd_cells processing_system7_0]
set_property -dict [list CONFIG.PCW_EN_RST0_PORT {1}] [get_bd_cells processing_system7_0]

# добавляем в Zynq порт прерываний от PL
set_property -dict [list CONFIG.PCW_USE_FABRIC_INTERRUPT {1} CONFIG.PCW_IRQ_F2P_INTR {1}] [get_bd_cells processing_system7_0]

# добавляем в Zynq M_GP порт для управления ядрами в PL 
set_property -dict [list CONFIG.PCW_USE_M_AXI_GP0 {1}] [get_bd_cells processing_system7_0]

# добавляем в Zynq HP порт для получения данных из PL от DMA контроллера 
set_property -dict [list CONFIG.PCW_USE_S_AXI_HP0 {1}] [get_bd_cells processing_system7_0]

# добавляем и настраиваем XADC
create_bd_cell -type ip -vlnv xilinx.com:ip:xadc_wiz:3.3 xadc_wiz_0
set_property -dict [list CONFIG.ENABLE_AXI4STREAM {true} CONFIG.OT_ALARM {false} CONFIG.USER_TEMP_ALARM {false} CONFIG.VCCINT_ALARM {false} CONFIG.VCCAUX_ALARM {false} CONFIG.ENABLE_VCCPINT_ALARM {false} CONFIG.ENABLE_VCCPAUX_ALARM {false} CONFIG.ENABLE_VCCDDRO_ALARM {false} CONFIG.SINGLE_CHANNEL_SELECTION {VP_VN} CONFIG.BIPOLAR_OPERATION {true} CONFIG.FIFO_DEPTH {512}] [get_bd_cells xadc_wiz_0]
make_bd_intf_pins_external  [get_bd_intf_pins xadc_wiz_0/Vp_Vn]

# выполняем connection automation для XADC
apply_bd_automation -rule xilinx.com:bd_rule:axi4 -config { Clk_master {Auto} Clk_slave {Auto} Clk_xbar {Auto} Master {/processing_system7_0/M_AXI_GP0} Slave {/xadc_wiz_0/s_axi_lite} ddr_seg {Auto} intc_ip {New AXI Interconnect} master_apm {0}}  [get_bd_intf_pins xadc_wiz_0/s_axi_lite]
apply_bd_automation -rule xilinx.com:bd_rule:clkrst -config { Clk {/processing_system7_0/FCLK_CLK0 (100 MHz)} Freq {100} Ref_Clk0 {} Ref_Clk1 {} Ref_Clk2 {}}  [get_bd_pins xadc_wiz_0/s_axis_aclk]

# добавляем Scope trigger и выполняем connection automation
create_bd_cell -type ip -vlnv xilinx.com:hls:scope_trigger:1.0 scope_trigger_0
apply_bd_automation -rule xilinx.com:bd_rule:axi4 -config { Clk_master {/processing_system7_0/FCLK_CLK0 (100 MHz)} Clk_slave {Auto} Clk_xbar {/processing_system7_0/FCLK_CLK0 (100 MHz)} Master {/processing_system7_0/M_AXI_GP0} Slave {/scope_trigger_0/s_axi_ctrl} ddr_seg {Auto} intc_ip {/ps7_0_axi_periph} master_apm {0}}  [get_bd_intf_pins scope_trigger_0/s_axi_ctrl]

# добавляем и настраиваем DMA контроллер
create_bd_cell -type ip -vlnv xilinx.com:ip:axi_dma:7.1 axi_dma_0
set_property -dict [list CONFIG.c_s_axis_s2mm_tdata_width.VALUE_SRC USER] [get_bd_cells axi_dma_0]
set_property -dict [list CONFIG.c_include_sg {0} CONFIG.c_sg_include_stscntrl_strm {0} CONFIG.c_include_mm2s {0} CONFIG.c_m_axis_mm2s_tdata_width {32} CONFIG.c_include_s2mm {1} CONFIG.c_s_axis_s2mm_tdata_width {16}] [get_bd_cells axi_dma_0]

# выполняем connection automation для DMA контроллера
apply_bd_automation -rule xilinx.com:bd_rule:axi4 -config { Clk_master {/processing_system7_0/FCLK_CLK0 (100 MHz)} Clk_slave {Auto} Clk_xbar {/processing_system7_0/FCLK_CLK0 (100 MHz)} Master {/processing_system7_0/M_AXI_GP0} Slave {/axi_dma_0/S_AXI_LITE} ddr_seg {Auto} intc_ip {/ps7_0_axi_periph} master_apm {0}}  [get_bd_intf_pins axi_dma_0/S_AXI_LITE]
apply_bd_automation -rule xilinx.com:bd_rule:axi4 -config { Clk_master {Auto} Clk_slave {Auto} Clk_xbar {Auto} Master {/axi_dma_0/M_AXI_S2MM} Slave {/processing_system7_0/S_AXI_HP0} ddr_seg {Auto} intc_ip {New AXI Interconnect} master_apm {0}}  [get_bd_intf_pins processing_system7_0/S_AXI_HP0]

# подключаем порт прерываний от DMA к Zynq
connect_bd_net [get_bd_pins processing_system7_0/IRQ_F2P] [get_bd_pins axi_dma_0/s2mm_introut]

# подключаем axis интерфейсы
connect_bd_intf_net [get_bd_intf_pins xadc_wiz_0/M_AXIS] [get_bd_intf_pins scope_trigger_0/in_data]
connect_bd_intf_net [get_bd_intf_pins scope_trigger_0/out_data] [get_bd_intf_pins axi_dma_0/S_AXIS_S2MM]

# проверяем, сохраняем и закрываем block design
validate_bd_design
regenerate_bd_layout
save_bd_design
close_bd_design [get_bd_designs zynq_bd]

# создаем hdl_wrapper
make_wrapper -files [get_files ../PYNQ_Z1_XADC_Scope_Prj/PYNQ_Z1_XADC_Scope_Prj.srcs/sources_1/bd/zynq_bd/zynq_bd.bd] -top
add_files -norecurse ../PYNQ_Z1_XADC_Scope_Prj/PYNQ_Z1_XADC_Scope_Prj.srcs/sources_1/bd/zynq_bd/hdl/zynq_bd_wrapper.v
