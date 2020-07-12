// режимы работы триггера
#define AUTO 0
#define TRIGGER 1
#define ONCE 2
#define ONCE_TRIG 3

void init_intr(XScuGic* IntrInstPtr);
void init_scopetrig(XScope_trigger *TrigInstPtr);
void init_PL_DMA(XAxiDma *PL_DMAInstPtr);
void init_netif(struct netif *NetifPtr);
