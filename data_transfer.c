
#include "xparameters.h"
#include <xil_io.h>
#include "platform.h"
#include "ps7_init.h"
#include "xscugic.h"
#include <stdio.h>
#include <time.h>


#define OFFSET 0x30
#define BUFFER_SIZE 32000000
XScuGic InterruptController;
static XScuGic_Config *GicConfig;
int16_t data[BUFFER_SIZE];



void InterruptHandler(){

	xil_printf("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%INTERRUPT CALLED%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% \n\r");

	for(int i=BUFFER_SIZE-10000; i<BUFFER_SIZE-1; i++){
		//xil_printf("counter value= %d \r\n", data[i]);
		xil_printf("%d ", data[i]);
	}

	xil_printf("\r\n********************************************************************. \r\n");

	xil_printf("Reading from DMA complete. \r\n");

}


int InitializeDma(){

	u32 temp;
// Read PG012 DMA documentation to find these offsets.
	temp=Xil_In32(XPAR_AXI_DMA_0_BASEADDR+OFFSET);
	temp=temp | 0x1001;
	Xil_Out32(XPAR_AXI_DMA_0_BASEADDR+OFFSET, temp);
	temp=Xil_In32(XPAR_AXI_DMA_0_BASEADDR+OFFSET);
	xil_printf("Value in DMACR register: %08x \n\r", temp);

	return 0;
}

int SetUpInterruptSystem(XScuGic *XScuGicInstancePtr){

	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, (Xil_ExceptionHandler) XScuGic_InterruptHandler, XScuGicInstancePtr);
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}

int InitializeInterruptSystem( deviceID ){

	int Status;

	GicConfig =  XScuGic_LookupConfig( deviceID );
	if ( NULL==GicConfig){
		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize( &InterruptController, GicConfig, GicConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS){
		return XST_FAILURE;
	}

	Status = SetUpInterruptSystem(&InterruptController);
	if (Status != XST_SUCCESS){
		return XST_FAILURE;
	}
// Give name of your interrupt handler function in this function
	Status = XScuGic_Connect( &InterruptController, XPAR_FABRIC_AXI_DMA_0_S2MM_INTROUT_INTR, (Xil_ExceptionHandler) InterruptHandler, NULL);
	if (Status != XST_SUCCESS){
		return XST_FAILURE;
	}

	XScuGic_Enable ( &InterruptController, XPAR_FABRIC_AXI_DMA_0_S2MM_INTROUT_INTR );

	return XST_SUCCESS;

}


void StartDMATransfer(unsigned int src_addr, unsigned int length){

	u32 temp;
	temp= Xil_In32(XPAR_AXI_DMA_0_BASEADDR + 0x48);
	xil_printf("Address value before: %08x \r\n", temp);
	Xil_Out32(XPAR_AXI_DMA_0_BASEADDR + 0x48 , src_addr);
	temp=Xil_In32(XPAR_AXI_DMA_0_BASEADDR + 0x48);
	xil_printf("Address value after: %08x \r\n", temp);

	temp= Xil_In32(XPAR_AXI_DMA_0_BASEADDR + 0x58);
	xil_printf("Length value before: %d \r\n", temp);

//DMA will start after writing to lenght register. Make sure the size of length variable is in accordance with value set in DMA IP.
	Xil_Out32(XPAR_AXI_DMA_0_BASEADDR + 0x58, length);
	temp= Xil_In32(XPAR_AXI_DMA_0_BASEADDR + 0x58);
	xil_printf("value of length: %d \r\n", length);
	xil_printf("Length value after: %d \r\n", temp);
}

int main(){

	init_platform();


//  Cache  needs to be flushed for proper transfer of data
	Xil_DCacheFlush();


	//initializing dma
	xil_printf("Initializing DMA.....\n\r");
	InitializeDma();
	xil_printf("DMA initialized.....\n\r");
	int status = 0;
	//initializing interrupts system
	xil_printf("Initializing Interrupts.....\n\r");
	status = InitializeInterruptSystem(XPAR_PS7_SCUGIC_0_DEVICE_ID);
	if (status==0){
		xil_printf("Interrupts Initialized.....\n\r");
	}

	//starting DMA transfer
	getchar();
	xil_printf("Address of array: %08x\r\n", data);
	xil_printf("Transfered bytes: %d \r\n", 2*BUFFER_SIZE );
	StartDMATransfer( data, 2*BUFFER_SIZE);
	xil_printf("At the end\r\n.");

	while(1)
	cleanup_platform();

	return 0;
}
