#include <stdint.h>
#include <string.h>
#include "printf.h"
//#include "platform.h"


#define APP_START (0x00000000)
#define APP_LEN   (0x200)
#define APP_ENTRY (0x00000000)

#define SIZE_SIEB 2000

uint8_t ui8 = 0x4;
uint16_t ui16 = 0x5678;
uint32_t ui32 = 0x9ABCD;

int8_t i8 = 0x1234;
int16_t i16 = 0x5678;
int32_t i32 = 0x9ABCD;

int test_array[] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18};
void main(void)
{
    printf("Test Addition: 0x%X\n", ui16 + i8);	    
    printf("Test xor: 0x%X\n", ui16 ^ i32);	 
    printf("Test and: 0x%X\n", ui16 & i8);
    printf("Test or: 0x%X\n", ui16 | i32);
    printf("Test shift <<: 0x%X\n",ui32 << ui8);
    printf("Test shift >>: 0x%X\n", ui32 >> ui8);
    printf("Test <: 0x%X\n", ui16 < i8);
    printf("Test >: 0x%X\n", ui16 > i8);
    printf("Test == 0x%X\n", ui16 == i8);
    printf("Test != 0x%X\n", ui16 != i8);

  
    for(int i = 0; i < 18; i++) {
	printf("test_array[%d]: %d\n",i,test_array[i]);
    }
    printf("%\nEnde!\n ");
    while(1) ;
}
