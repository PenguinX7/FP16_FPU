#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "FP16_div_algorithim.h"

// int main()
// {
//     int res = FP16_div(0x5543,0x3e82);
//     return 0;
// }

unsigned short FP16_div(unsigned short dividend,unsigned short divisor)
{
    bool overflow;
    unsigned short S_X,S_D,S_Q;
    short E_X,E_D,E_Q;
    unsigned long long M_X,M_D,M_Q; //[33:0]
    short n;
    unsigned short M_new;
    short E_new;
    bool round_flag;

    //step1:inputcheck
    if((divisor & 0x7fff) == 0x0000)
    {
        overflow = 1;
        return 0x7bff;
    }
    else
    {
        overflow = 0;
        S_X = dividend >> 15;
        S_D = divisor >> 15;
        E_X = (dividend >> 10) & 0x1f;
        E_D = (divisor >> 10) & 0x1f;
        // M_X = (unsigned long long)(dividend & 0x3ff) << 11;     //拉出11位小数
        // M_D = (unsigned long long)(divisor & 0x3ff) << 22;      //被除数是22位，所以除数左移22
        M_X = (unsigned long long)(dividend & 0x3ff);               //S,11'd0M_X[10:0],22'd0,45bits
        M_D = (unsigned long long)(divisor & 0x3ff);                
        if(E_X == 0)
        {
            if(M_X == 0)
            {}                  //0
            else    
                E_X = 1;        //denormal
        }
        else
            // M_X = M_X | 0x200000;   //normal
            M_X = M_X | 0x400;
        
        if(E_D == 0)
            E_D = 1;            //denormal
        else
            // M_D = M_D | 0x100000000;
            M_D = M_D | 0x400;
    }

    //step2:E,S cal
    E_Q = E_X - E_D + 15;
    S_Q = (S_X == S_D)? 0 : 1;
    M_X = M_X << 22;                        //拉出22位小数
    M_D = M_D << 33;                        //被除数是33位，所以除数左移33位
    n = 32;
    

    //step3:cal M
    while(1)
    {
        if((M_X & 0x100000000000) == 0x100000000000)  //M_X[44]     //0x80000000000000 pass 0x800000000000 pass 0x100000000000 pass
        {
            M_X = M_X << 1;
            M_X = M_X + M_D;
        }
        else
        {
            M_X = M_X << 1;
            M_X = M_X - M_D;
        }
        if((M_X & 0x100000000000) == 0x100000000000)
            M_X = M_X & 0xfffffffffffffffe;     //M_X[0]=0
        else
            M_X = M_X | 0x0000000000000001;     //M_X[0]=1

        if(n == 0)
            break;
        else
            n -= 1;
    }

    //step4:nor_pre
    if(((M_X & 0x1ffffffff) & 0x100000000)==0x100000000)                  //M_X[21:0]==1_xxxx_xxxx_xx.xx_xxxx_xxxx_xxxx_xxxx_xxxx
    {
        M_new = (M_X >> 22) & 0x7ff;          //M_new = M_X[32:22]
        round_flag = (M_X >> 21) & 0x01;    //round_flag = M_X[21]
        E_new = E_Q + 10;
    }
    else if(((M_X & 0x1ffffffff) & 0x080000000)==0x080000000)                  //M_X[21:0]==0_1xxx_xxxx_xx.xx_xxxx_xxxx_xxxx_xxxx_xxxx
    {
        M_new = (M_X >> 21) & 0x7ff;          //M_new = M_X[31:21]
        round_flag = (M_X >> 20) & 0x01;    //round_flag = M_X[20]
        E_new = E_Q + 9;
    }
    else if(((M_X & 0x1ffffffff) & 0x040000000)==0x040000000)                  //M_X[21:0]==0_01xx_xxxx_xx.xx_xxxx_xxxx_xxxx_xxxx_xxxx
    {
        M_new = (M_X >> 20) & 0x7ff;          //M_new = M_X[30:20]
        round_flag = (M_X >> 19) & 0x01;    //round_flag = M_X[19]
        E_new = E_Q + 8;
    }
    else if(((M_X & 0x1ffffffff) & 0x020000000)==0x020000000)                  //M_X[21:0]==0_001x_xxxx_xx.xx_xxxx_xxxx_xxxx_xxxx_xxxx
    {
        M_new = (M_X >> 19) & 0x7ff;          //M_new = M_X[29:19]
        round_flag = (M_X >> 18) & 0x01;    //round_flag = M_X[18]
        E_new = E_Q + 7;
    }
    else if(((M_X & 0x1ffffffff) & 0x010000000)==0x010000000)                  //M_X[21:0]==0_0001_xxxx_xx.xx_xxxx_xxxx_xxxx_xxxx_xxxx
    {
        M_new = (M_X >> 18) & 0x7ff;          //M_new = M_X[28:18]
        round_flag = (M_X >> 17) & 0x01;    //round_flag = M_X[17]
        E_new = E_Q + 6;
    }
    else if(((M_X & 0x1ffffffff) & 0x008000000)==0x008000000)                  //M_X[21:0]==0_0000_1xxx_xx.xx_xxxx_xxxx_xxxx_xxxx_xxxx
    {
        M_new = (M_X >> 17) & 0x7ff;          //M_new = M_X[27:17]
        round_flag = (M_X >> 16) & 0x01;    //round_flag = M_X[16]
        E_new = E_Q + 5;
    }
    else if(((M_X & 0x1ffffffff) & 0x004000000)==0x004000000)                  //M_X[21:0]==0_0000_01xx_xx.xx_xxxx_xxxx_xxxx_xxxx_xxxx
    {
        M_new = (M_X >> 16) & 0x7ff;          //M_new = M_X[26:16]
        round_flag = (M_X >> 15) & 0x01;    //round_flag = M_X[15]
        E_new = E_Q + 4;
    }
    else if(((M_X & 0x1ffffffff) & 0x002000000)==0x002000000)                  //M_X[21:0]==0_0000_001x_xx.xx_xxxx_xxxx_xxxx_xxxx_xxxx
    {
        M_new = (M_X >> 15) & 0x7ff;          //M_new = M_X[25:15]
        round_flag = (M_X >> 14) & 0x01;    //round_flag = M_X[14]
        E_new = E_Q + 3;
    }
    else if(((M_X & 0x1ffffffff) & 0x001000000)==0x001000000)                  //M_X[21:0]==0_0000_0001_xx.xx_xxxx_xxxx_xxxx_xxxx_xxxx
    {
        M_new = (M_X >> 14) & 0x7ff;          //M_new = M_X[24:14]
        round_flag = (M_X >> 13) & 0x01;    //round_flag = M_X[13]
        E_new = E_Q + 2;
    }
    else if(((M_X & 0x1ffffffff) & 0x000800000)==0x000800000)                  //M_X[21:0]==0_0000_0000_1x.xx_xxxx_xxxx_xxxx_xxxx_xxxx
    {
        M_new = (M_X >> 13) & 0x7ff;          //M_new = M_X[23:13]
        round_flag = (M_X >> 12) & 0x01;    //round_flag = M_X[12]
        E_new = E_Q + 1;
    }
    else if(((M_X & 0x1ffffffff) & 0x000400000)==0x000400000)                  //M_X[21:0]==0_0000_0000_01.xx_xxxx_xxxx_xxxx_xxxx_xxxx
    {
        M_new = (M_X >> 12) & 0x7ff;          //M_new = M_X[22:12]
        round_flag = (M_X >> 11) & 0x01;    //round_flag = M_X[11]
        E_new = E_Q;
    }
    else if(((M_X & 0x1ffffffff) & 0x000200000)==0x000200000)                  //M_X[21:0]==0_0000_0000_00.1x_xxxx_xxxx_xxxx_xxxx_xxxx
    {
        M_new = (M_X >> 11) & 0x7ff;          //M_new = M_X[21:11]
        round_flag = (M_X >> 10) & 0x01;    //round_flag = M_X[10]
        E_new = E_Q - 1;
    }
    else if(((M_X & 0x1ffffffff) & 0x000100000)==0x000100000)                  //M_X[21:0]==0_0000_0000_00.01_xxxx_xxxx_xxxx_xxxx_xxxx
    {
        M_new = (M_X >> 10) & 0x7ff;          //M_new = M_X[20:10]
        round_flag = (M_X >> 9) & 0x01;    //round_flag = M_X[9]
        E_new = E_Q - 2;
    }
    else if(((M_X & 0x1ffffffff) & 0x000080000)==0x000080000)                  //M_X[21:0]==0_0000_0000_00.00_1xxx_xxxx_xxxx_xxxx_xxxx
    {
        M_new = (M_X >> 9) & 0x7ff;          //M_new = M_X[19:9]
        round_flag = (M_X >> 8) & 0x01;    //round_flag = M_X[8]
        E_new = E_Q - 3;
    }
    else if(((M_X & 0x1ffffffff) & 0x000040000)==0x000040000)                  //M_X[21:0]==0_0000_0000_00.00_01xx_xxxx_xxxx_xxxx_xxxx
    {
        M_new = (M_X >> 8) & 0x7ff;          //M_new = M_X[18:8]
        round_flag = (M_X >> 7) & 0x01;    //round_flag = M_X[7]
        E_new = E_Q - 4;
    }
    else if(((M_X & 0x1ffffffff) & 0x000020000)==0x000020000)                  //M_X[21:0]==0_0000_0000_00.00_001x_xxxx_xxxx_xxxx_xxxx
    {
        M_new = (M_X >> 7) & 0x7ff;          //M_new = M_X[17:7]
        round_flag = (M_X >> 6) & 0x01;    //round_flag = M_X[6]
        E_new = E_Q - 5;
    }
    else if(((M_X & 0x1ffffffff) & 0x000010000)==0x000010000)                  //M_X[21:0]==0_0000_0000_00.00_0001_xxxx_xxxx_xxxx_xxxx
    {
        M_new = (M_X >> 6) & 0x7ff;          //M_new = M_X[16:6]
        round_flag = (M_X >> 5) & 0x01;    //round_flag = M_X[5]
        E_new = E_Q - 6;
    }
    else if(((M_X & 0x1ffffffff) & 0x000008000)==0x000008000)                  //M_X[21:0]==0_0000_0000_00.00_0000_1xxx_xxxx_xxxx_xxxx
    {
        M_new = (M_X >> 5) & 0x7ff;          //M_new = M_X[15:5]
        round_flag = (M_X >> 4) & 0x01;    //round_flag = M_X[4]
        E_new = E_Q - 7;
    }
    else if(((M_X & 0x1ffffffff) & 0x000004000)==0x000004000)                  //M_X[21:0]==0_0000_0000_00.00_0000_01xx_xxxx_xxxx_xxxx
    {
        M_new = (M_X >> 4) & 0x7ff;          //M_new = M_X[14:4]
        round_flag = (M_X >> 3) & 0x01;    //round_flag = M_X[3]
        E_new = E_Q - 8;
    }
    else if(((M_X & 0x1ffffffff) & 0x000002000)==0x000002000)                  //M_X[21:0]==0_0000_0000_00.00_0000_001x_xxxx_xxxx_xxxx
    {
        M_new = (M_X >> 3) & 0x7ff;          //M_new = M_X[13:3]
        round_flag = (M_X >> 2) & 0x01;    //round_flag = M_X[2]
        E_new = E_Q - 9;
    }
    else if(((M_X & 0x1ffffffff) & 0x000001000)==0x000001000)                  //M_X[21:0]==0_0000_0000_00.00_0000_0001_xxxx_xxxx_xxxx
    {
        M_new = (M_X >> 2) & 0x7ff;          //M_new = M_X[12:2]
        round_flag = (M_X >> 1) & 0x01;    //round_flag = M_X[1]
        E_new = E_Q - 10;
    }
    else if(((M_X & 0x1ffffffff) & 0x000000800)==0x000000800)                  //M_X[21:0]==0_0000_0000_00.00_0000_0000_1xxx_xxxx_xxxx
    {
        M_new = (M_X >> 1) & 0x7ff;          //M_new = M_X[11:1]
        round_flag = M_X & 0x01;    //round_flag = M_X[0]
        E_new = E_Q - 11;
    }
    else
    {
        M_new = 0;
        round_flag = 0;
        E_new = 0;
    }

    //step5:denormal operation
    if(E_new  <= 0)
    {
        switch(E_new)
        {
            case 0 : {round_flag = M_new & 0x1; M_new = M_new >> 1;} break;
            case -1 : {round_flag = (M_new>>1) & 0x1; M_new = M_new >> 2;} break;
            case -2 : {round_flag = (M_new>>2) & 0x1; M_new = M_new >> 3;} break;
            case -3 : {round_flag = (M_new>>3) & 0x1; M_new = M_new >> 4;} break;
            case -4 : {round_flag = (M_new>>4) & 0x1; M_new = M_new >> 5;} break;
            case -5 : {round_flag = (M_new>>5) & 0x1; M_new = M_new >> 6;} break;
            case -6 : {round_flag = (M_new>>6) & 0x1; M_new = M_new >> 7;} break;
            case -7 : {round_flag = (M_new>>7) & 0x1; M_new = M_new >> 8;} break;
            case -8 : {round_flag = (M_new>>8) & 0x1; M_new = M_new >> 9;} break;
            case -9 : {round_flag = (M_new>>9) & 0x1; M_new = M_new >> 10;} break;
            case -10 : {round_flag = (M_new>>10) & 0x1; M_new = M_new >> 11;} break;
            default : {round_flag = 0;  M_new = 0;} break;
        }
        E_new = 0;
    }

    //step6:round and carry
    M_new = M_new + round_flag;
    if(E_new > 0)   //normal
    {
        if((M_new & 0x800) == 0x800)    //M_new[11]
        {
            M_new = M_new >> 1;
            E_new += 1;
        }
    }
    else            //denormal to normal
    {
        if((M_new & 0x400) == 0x400)    //M_new[10]
            E_new = 1;
    }

    //step7:output
    if(E_new > 30)
        return (S_Q << 15) | 0x7bff;
    else if(M_new == 0)
        return 0x0000;
    else
        return (S_Q << 15) | ((E_new & 0x1f) << 10) | (M_new & 0x3ff);
}


//test int div
// int mydiv(long long x,long long d)
// {
//     short n = 23;
//     unsigned long long X = x << 11;//S,11'd0,{x[10:0],11'd0},34bits
//     unsigned long long D = d << n;//d[10:0],22'd0;,33bits
//     // long long X = x << 11;//S,11'd0,{x[10:0],11'd0},34bits
//     // long long D = d << n;//d[10:0],22'd0;,33bits

//     while(n > 0)
//     {
//         // if(X>=0)
//         // {
//         //     X = X << 1;
//         //     X = X - D;
//         // }
//         // else
//         // {
//         //     X = X << 1;
//         //     X = X + D;
//         // }
//         // if(X>= 0)
//         //     X = X | 0x0000000000000001;
//         // else
//         //     X = X & 0xfffffffffffffffe;
//         //--------------------------------------------------------------
//         if((X & 0x200000000) == 0x200000000)
//         {
//             X = X << 1;
//             X = X + D;
//         }
//         else
//         {
//             X = X << 1;
//             X = X - D;
//         }
//         if((X & 0x200000000) == 0x200000000)
//             X = X & 0xfffffffffffffffe;
//         else
//             X = X | 0x0000000000000001;

//         n -= 1;
//     }    
//     return X & 0x3fffff;
// }