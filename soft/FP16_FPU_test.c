

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "FP16_FPU_test.h"
#include "FP16_MAC_algorithim.h"
#include "FP16_div_algorithim.h"
#include "FP16_ex_algorithim.h"
#include "FP16_lnx_algorithim.h"

int main()
{
    FILE *p = fopen("test.txt","w");
    short opcode;
    unsigned short data1,data2,data3;
    opcode = 5;

    switch (opcode)
    {
        case 0 :                //add
        {
            data1 = 0x0000;
            data2 = 0x0000;
            while(1)
            {
                while(1)
                {
                    fprintf(p,"%04x%04x%04x\n",data1,data2,FP16_FPU(data1,data2,0x0000,opcode));

                    if(data2 == 0x00ff)
                        data2 = 0x7b00;
                    else if(data2 == 0x7bff)
                        data2 = 0x8001;
                    else if(data2 == 0x80ff)
                        data2 = 0xfb00;
                    else if(data2 == 0xfbff)
                    {
                        data2 = 0x0000;
                        break;
                    }
                    else
                        data2 += 1; 
                }
                if(data1 == 0x00ff)
                    data1 = 0x7b00;
                else if(data1 == 0x7bff)
                    data1 = 0x8001;
                else if(data1 == 0x80ff)
                    data1 = 0xfb00;
                else if(data1 == 0xfbff)
                    break;
                else
                    data1 += 1; 
            }
        }break;
        case 1 :                //mcl
        {
            data1 = 0x0000;
            data2 = 0x0000;
            while(1)
            {
                while(1)
                {
                    fprintf(p,"%04x%04x%04x\n",data1,data2,FP16_FPU(data1,data2,0x0000,opcode));

                    if(data2 == 0x00ff)
                        data2 = 0x7b00;
                    else if(data2 == 0x7bff)
                        data2 = 0x8001;
                    else if(data2 == 0x80ff)
                        data2 = 0xfb00;
                    else if(data2 == 0xfbff)
                    {
                        data2 = 0x0000;
                        break;
                    }
                    else
                        data2 += 1; 
                }
                if(data1 == 0x00ff)
                    data1 = 0x7b00;
                else if(data1 == 0x7bff)
                    data1 = 0x8001;
                else if(data1 == 0x80ff)
                    data1 = 0xfb00;
                else if(data1 == 0xfbff)
                    break;
                else
                    data1 += 1; 
            }
        }break;
        case 2 :                //div       //0001-00ff,7b00-7bff
        {
            data1 = 0x0000;
            data2 = 0x0001;
            while(1)
            {
                while(1)
                {
                    fprintf(p,"%04x%04x%04x\n",data1,data2,FP16_FPU(data1,data2,0x0000,opcode));

                    if(data2 == 0x00ff)
                        data2 = 0x7b00;
                    else if(data2 == 0x7bff)
                        data2 = 0x8001;
                    else if(data2 == 0x80ff)
                        data2 = 0xfb00;
                    else if(data2 == 0xfbff)
                    {
                        data2 = 0x0001;
                        break;
                    }
                    else
                        data2 += 1; 
                }
                if(data1 == 0x00ff)
                    data1 = 0x7b00;
                else if(data1 == 0x7bff)
                    data1 = 0x8001;
                else if(data1 == 0x80ff)
                    data1 = 0xfb00;
                else if(data1 == 0xfbff)
                    break;
                else
                    data1 += 1; 
            }
        }break;
        case 3 :                //MAC
        {
            data1 = 0x0000;
            data2 = 0x0000;
            data3 = 0x0000;
            while(1)
            {
                while(1)
                {
                    while(1)
                    {
                        fprintf(p,"%04x%04x%04x%04x\n",data1,data2,data3,FP16_FPU(data1,data2,data3,opcode));
                        if(data3 == 0x000f)
                            data3 = 0x7bf0;
                        else if(data3 == 0x7bff)
                            data3 = 0x8001;
                        else if(data3 == 0x800f)
                            data3 = 0xfbf0;
                        else if(data3 == 0xfbff)
                        {
                            data3 = 0x0000;
                            break;
                        }
                        else
                            data3 += 1;
                    }
                    if(data2 == 0x000f)
                        data2 = 0x7bf0;
                    else if(data2 == 0x7bff)
                        data2 = 0x8001;
                    else if(data2 == 0x800f)
                        data2 = 0xfbf0;
                    else if(data2 == 0xfbff)
                    {
                        data2 = 0x0000;
                        break;
                    }
                    else
                        data2 += 1;
                }
                if(data1 == 0x000f)
                    data1 = 0x7bf0;
                else if(data1 == 0x7bff)
                    data1 = 0x8001;
                else if(data1 == 0x800f)
                    data1 = 0xfbf0;
                else if(data1 == 0xfbff)
                    break;
                else
                    data1 += 1;
            }
        }break;
        case 4 :                //ex
        {
            data1 = 0x0400;
            // data2 = 0x0001;
            while(1)
            {
                fprintf(p,"%04x%04x\n",data1,FP16_FPU(data1,data2,0x0000,opcode));

                if(data1 == 0x498c)
                    data1 = 0x8400;
                else if(data1 == 0xc8da)
                    break;
                else
                    data1 += 1; 
            }
        }break;
        case 5 :                //lnx
        {
            data1 = 0x0400;
            while(1)
            {
                fprintf(p,"%04x%04x\n",data1,FP16_FPU(data1,data2,0x0000,opcode));

                if(data1 == 0x7bff)
                    break;
                else
                    data1 += 1; 
            }
        }break;
    }

    fclose(p);
    return 0;
}

unsigned short FP16_FPU(unsigned short data1,unsigned short data2,unsigned short data3,short opcode)
{
    switch(opcode)
    {
        case 0 : return FP16_MAC(data1,0x3c00,data2);   break;
        case 1 : return FP16_MAC(data1,data2,0x0000);   break;
        case 2 : return FP16_div(data1,data2);          break;
        case 3 : return FP16_MAC(data1,data2,data3);    break;
        case 4 : return FP16_ex(data1);                 break;
        case 5 : return FP16_lnx(data1);                break;
        default : {printf("invalid opcode\n");return 0;}break;
    }
}