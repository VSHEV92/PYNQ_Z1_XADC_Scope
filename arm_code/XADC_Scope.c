#include "lwip/netif.h"
#include "lwip/init.h"
#include "lwip/udp.h"

#include "netif/xadapter.h"

#include "xscugic.h"
#include "xscope_trigger.h"
#include "xaxidma.h"
#include "xparameters.h"

#include "xil_printf.h"

#include "platform.h"


struct netif device_netif;
XScuGic InterruptController;
XScope_trigger ScopeTrig;
XAxiDma PL_DMA;

#define UDP_Packet_Size 1024
static struct udp_pcb *pcb;
static struct pbuf *pbuf_to_be_sent = NULL;

u8 *Ping_Buff = (u8 *)0x10000000;
u8 *Pong_Buff = (u8 *)0x11000000;

u8 Ping_Pong_Flag = 0;
u8 DMA_Done_Flag = 0;

// обработчик перерываний от PL DMA
void DMA_RX_Handler(void *Callback){
	u32 IrqStatus;
	XAxiDma *AxiDmaInst = (XAxiDma *)Callback;

	IrqStatus = XAxiDma_IntrGetIrq(AxiDmaInst, XAXIDMA_DEVICE_TO_DMA);
	XAxiDma_IntrAckIrq(AxiDmaInst, IrqStatus, XAXIDMA_DEVICE_TO_DMA);

	DMA_Done_Flag = 1;
}


// callback функция при приеме пакета
void recv_callback(void *arg, struct udp_pcb *tpcb, struct pbuf *p, struct ip_addr *addr, u16_t port)
{
	u8* Data;
	u16 trig_lev;
	u8 trig_mode;
	u8 decimation;
	u8 trig_start;

	Data = (u8 *)(p->payload);
	Xil_DCacheInvalidateRange((UINTPTR)Data, 5);
	if (Data[0] == 255){
		trig_lev = (((u16)Data[1]) << 8) + Data[2];
		trig_mode = Data[3];
		decimation = Data[4];
		XScope_trigger_Set_trig_level_V(&ScopeTrig, trig_lev);
		XScope_trigger_Set_trig_mode_V(&ScopeTrig, trig_mode);
		XScope_trigger_Set_downsamp_V(&ScopeTrig, decimation);
	} else if (Data[0] == 254){
		trig_lev = (((u16)Data[1]) << 8) + Data[2];
		XScope_trigger_Set_trig_level_V(&ScopeTrig, trig_lev);
	} else if (Data[0] == 253){
		trig_start = Data[1];
		XScope_trigger_Set_once_start(&ScopeTrig, trig_start);
	}

	pbuf_free(p);
}

int main(){
	u8 *Send_Buff;

	// инициализация контроллера прерываний и включаем прерывания
	init_intr(&InterruptController);

	// инициализация DMA контроллера
	init_PL_DMA(&PL_DMA);

	// инициализация триггера осцилографа
	init_scopetrig(&ScopeTrig);

	// инициализируем сетевой интерфейс
	init_netif(&device_netif);

	// подключаем обработчик перерываний от PL DMA
	XScuGic_Connect(&InterruptController, XPAR_FABRIC_AXI_DMA_0_S2MM_INTROUT_INTR,	(XInterruptHandler)DMA_RX_Handler, &PL_DMA);
	XScuGic_Enable(&InterruptController, XPAR_FABRIC_AXI_DMA_0_S2MM_INTROUT_INTR);
	XAxiDma_IntrEnable(&PL_DMA, XAXIDMA_IRQ_IOC_MASK, XAXIDMA_DEVICE_TO_DMA);

	// настраиваем TCP соединение
	pcb = udp_new();
	udp_bind(pcb, IP_ADDR_ANY, 5001);
	udp_recv(pcb, recv_callback, NULL);

	ip_addr_t ipaddr_host;
	IP4_ADDR(&ipaddr_host, 192, 168, 1, 11);
	udp_connect(pcb, &ipaddr_host, 5001);

	// запускаем PL DMA
	XAxiDma_SimpleTransfer(&PL_DMA, (UINTPTR)Ping_Buff, UDP_Packet_Size, XAXIDMA_DEVICE_TO_DMA);

    // включаем триггер осцилографа
	XScope_trigger_EnableAutoRestart(&ScopeTrig);
	XScope_trigger_Start(&ScopeTrig);

	//u32 flag;

	while (1){
		if (DMA_Done_Flag == 1){
			DMA_Done_Flag = 0;
			if (Ping_Pong_Flag == 0){
				Xil_DCacheInvalidateRange((UINTPTR)Ping_Buff, UDP_Packet_Size);
			    // передаем полученные данные через UDP
				pbuf_to_be_sent = pbuf_alloc(PBUF_TRANSPORT, UDP_Packet_Size, PBUF_POOL);
				memcpy(pbuf_to_be_sent->payload, Ping_Buff, UDP_Packet_Size);
				udp_send(pcb, pbuf_to_be_sent);
				pbuf_free(pbuf_to_be_sent);
			    // начаинаем новую транзакцию DMA
			    XAxiDma_SimpleTransfer(&PL_DMA, (UINTPTR)Pong_Buff, UDP_Packet_Size, XAXIDMA_DEVICE_TO_DMA);
			    Ping_Pong_Flag = 1;
			} else {
				Xil_DCacheInvalidateRange((UINTPTR)Pong_Buff, UDP_Packet_Size);
				pbuf_to_be_sent = pbuf_alloc(PBUF_TRANSPORT, UDP_Packet_Size, PBUF_POOL);
				memcpy(pbuf_to_be_sent->payload, Pong_Buff, UDP_Packet_Size);
				udp_send(pcb, pbuf_to_be_sent);
				pbuf_free(pbuf_to_be_sent);
			    XAxiDma_SimpleTransfer(&PL_DMA, (UINTPTR)Ping_Buff, UDP_Packet_Size, XAXIDMA_DEVICE_TO_DMA);
			    Ping_Pong_Flag = 0;
			}
		}
		xemacif_input(&device_netif);
	}
	return 0;
}






