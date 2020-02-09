#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include "adcheader.h"

uint16_t adc; //adc data that is recevied

int CHANNEL_SELECT_PATH(int file_desc,int ch)
{
	int value;
	value= ioctl(file_desc, IOCTL_CHANNEL_SELECTION, ch);
	if(value<0) {
		printf("channel selection failed\n");
		exit(-1);
	}
	return 0;
}

int ALIGNMENT_SELECTION(int file_desc,int alg)
{
	int value;
	value= ioctl(file_desc, IOCTL_ALIGNMENT, alg);
	if(value<0) {
		printf("Aligment arrangement failed\n");
		exit(-1);
	}
	return 0;
}


/// IOCTL CALLS


int main() {
	
	int file_desc, value, ch, alg;
	file_desc= open(FILE_NAME, 0);
	if(file_desc<0) {
		printf("device file not opened\n");
		exit(-1);
	}
	
	printf("Please select the channel= \n");
	scanf("%d", &ch);
	printf("Choose the alignment(0 for right and 1 for left)=\n");
	scanf("%d", &alg);

	if(ch<0 || ch>7 || (alg!=0 && alg!=1)) {
	printf("Wrong Entry of DATA\n");
	exit(-1);
	}
	
	CHANNEL_SELECT_PATH(file_desc,ch);
	ALIGNMENT_SELECTION(file_desc,alg);
	
	if(read(file_desc, &adc, sizeof(adc))) {
		if(alg==1) {
		adc=adc/64;
		printf("Alignment selected is LEFT=%u\n",adc);
	}
	else {
		printf("Alignment selected is RIGHT=%u\n",adc);
	}
       }

	close(file_desc);
	return 0;
}
			


