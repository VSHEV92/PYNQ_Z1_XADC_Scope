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

u8 *Ping_Buff = (u8 *)0x10000000;
u8 *Pong_Buff = (u8 *)0x11000000;

// обработчик перерываний от PL DMA
void DMA_RX_Handler(void *Callback){
	u32 IrqStatus;
	XAxiDma *AxiDmaInst = (XAxiDma *)Callback;

	IrqStatus = XAxiDma_IntrGetIrq(AxiDmaInst, XAXIDMA_DEVICE_TO_DMA);
	XAxiDma_IntrAckIrq(AxiDmaInst, IrqStatus, XAXIDMA_DEVICE_TO_DMA);

	xil_printf("DMA RX Transfer Done!\n");
}


// callback функция при приеме пакета
void recv_callback(void *arg, struct udp_pcb *tpcb, struct pbuf *p, struct ip_addr *addr, u16_t port)
{
//	u8* Data;
//	Data = (u8 *)(p->payload);
//
//    for(int i=0; i<4; i++)
//    	RX_Frame[i] = Data[i];
//
//    Update_Flag = 1;
	// освобождаем буфер пакета
	xil_printf("Recv UDP\n");
	pbuf_free(p);
}

int main(){
	// ------------------------------------------------------------------------------------------------------

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
	struct udp_pcb *pcb;
	pcb = udp_new();
	udp_bind(pcb, IP_ADDR_ANY, 5001);
	udp_recv(pcb, recv_callback, NULL);

	// запускаем PL DMA
	 XAxiDma_SimpleTransfer(&PL_DMA, (UINTPTR)Ping_Buff, 1024, XAXIDMA_DEVICE_TO_DMA);

    // включаем триггер осцилографа
	XScope_trigger_EnableAutoRestart(&ScopeTrig);
	XScope_trigger_Start(&ScopeTrig);

	u32 flag;

	while (1){

		flag = XAxiDma_Busy(&PL_DMA, XAXIDMA_DEVICE_TO_DMA);
	    xil_printf("DMA Busy %u\n", flag);
	    flag = XScope_trigger_IsIdle(&ScopeTrig);
	    xil_printf("Trigger Idle %u\n", flag);

		xemacif_input(&device_netif);
	}
	return 0;
}






