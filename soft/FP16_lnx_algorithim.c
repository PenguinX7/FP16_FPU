#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "FP16_lnx_algorithim.h"
#include "FP16_MAC_algorithim.h"

unsigned short table_reflect(unsigned short exp);
void linear_approximate(short seg,unsigned short* coefficient,unsigned short* bias);
unsigned short FP16_lnx(unsigned short datai);


// int main()
// {
//     unsigned short a = 0x3802; 
//     unsigned short b = FP16_lnx(a);
    
//     return 0;
// }

unsigned short FP16_lnx(unsigned short datai)
{
    unsigned short exp,e;
    unsigned short rm,m,s;
    unsigned short ln2 = 0x398c;
    unsigned short cache1,cache2,cache3;
    unsigned short datao;
    short seg;
    unsigned short coefficient = 0;
    unsigned short bias= 0;

    if(((datai >> 15) == 1) || ((datai & 0x7c00)== 0x7c00))      //datai[15] = 0 or datai[14:10] == 00000
        return 0xffff;
    else
    {
        exp = (datai >> 10) & 0x1f;
        //rm = (datai & 0x03f8) >> 3;     //rm = datai[9:3]
        rm = datai & 0x03ff | 0x3c00 ;      //rm = {6'b001111,datai[9:0]}
    }

    if(rm < 0x3c0a)
        seg = 0;
    else if(rm < 0x3c14)
        seg = 1;
    else if(rm < 0x3c1f)
        seg = 2;
    else if(rm < 0x3c33)
        seg = 3;
    else if(rm < 0x3c48)
        seg = 4;
    else if(rm < 0x3c66)
        seg = 5;
    else if(rm < 0x3c85)
        seg = 6;
    else if(rm < 0x3ca4)
        seg = 7;
    else if(rm < 0x3ccd)
        seg = 8;
    else if(rm < 0x3d00)
        seg = 9;
    else if(rm < 0x3d33)
        seg = 10;
    else if(rm < 0x3d66)
        seg = 11;
    else if(rm < 0x3d9a)
        seg = 12;
    else if(rm < 0x3dd7)
        seg = 13;
    else if(rm < 0x3e14)
        seg = 14;
    else if(rm < 0x3e52)
        seg = 15;
    else if(rm < 0x3e9a)
        seg = 16;
    else if(rm < 0x3ecd)
        seg = 17;
    else if(rm < 0x3f00)
        seg = 18;
    else if(rm < 0x3f33)
        seg = 19;
    else if(rm < 0x3f66)
        seg = 20;
    else if(rm < 0x3f9a)
        seg = 21;
    else if(rm < 0x3fb8)
        seg = 22;
    else if(rm < 0x3fcd)
        seg = 23;
    else if(rm < 0x3fe1)
        seg = 24;
    else if(rm < 0x3fec)
        seg = 25;
    else if(rm < 0x3ff6)
        seg = 26;
    else 
        seg = 27;

    linear_approximate(seg, &coefficient, &bias);

    //替换原来的cache3 = rmtable(rm);
    cache3 = FP16_MAC(rm,coefficient,bias);

    cache1 = table_reflect(exp);
    datao = FP16_MAC(cache1,ln2,cache3);


    s = datao >> 15;
    e = (datao >> 10) & 0x1f;
    m = datao & 0x3ff | 0x400;

    if((datao>=0xa8c0) && (datao <= 0xaaa0))
        m = m - 0x16;
    else if((datao>=0xa420) && (datao <= 0xa5e0))
        m = m - 0x13;
    else if((datao>=0x9c00) && (datao <= 0x9d80))
        m = m - 0x7e;
    else if((datao <= 0x9b00) && (datao >= 0x9800))
        m = m - 0xff;
    else if((datao == 0x9600) || (datao == 0x9400))
        m = m - 0x200;

    if((m >> 11) == 1)
    {
        e += 1;
        m = m >> 1;
    }
    else if((m >> 10) == 1)
    {}
    else if((m >> 9) == 1)
    {
        e -= 1;
        m = m << 1; 
    }

    datao = (s << 15) | (e << 10) | (m & 0x3ff);
    

    return datao;

}



void linear_approximate(short seg,unsigned short* coefficient,unsigned short* bias)
{
    unsigned short a,b;
    switch(seg)
    {
        case 0 : {a = 0x3bf6;b = 0xbbf6;}break;
        case 1 : {a = 0x3be1;b = 0xbbe1;}break;
        case 2 : {a = 0x3bcd;b = 0xbbcc;}break;
        case 3 : {a = 0x3baf;b = 0xbbae;}break;
        case 4 : {a = 0x3b8a;b = 0xbb87;}break;
        case 5 : {a = 0x3b5d;b = 0xbb56;}break;
        case 6 : {a = 0x3b2a;b = 0xbb1f;}break;
        case 7 : {a = 0x3afa;b = 0xbae8;}break;
        case 8 : {a = 0x3ac5;b = 0xbaaa;}break;
        case 9 : {a = 0x3a85;b = 0xba5d;}break;
        case 10 : {a = 0x3a46;b = 0xba0f;}break;
        case 11 : {a = 0x3a0a;b = 0xb9c0;}break;
        case 12 : {a = 0x39d2;b = 0xb974;}break;
        case 13 : {a = 0x3995;b = 0xb920;}break;
        case 14 : {a = 0x395f;b = 0xb8d0;}break;
        case 15 : {a = 0x3929;b = 0xb87f;}break;
        case 16 : {a = 0x38f4;b = 0xb82b;}break;
        case 17 : {a = 0x38c7;b = 0xb7c0;}break;
        case 18 : {a = 0x38a3;b = 0xb747;}break;
        case 19 : {a = 0x3882;b = 0xb6d2;}break;
        case 20 : {a = 0x3862;b = 0xb660;}break;
        case 21 : {a = 0x3843;b = 0xb5ec;}break;
        case 22 : {a = 0x382d;b = 0xb59b;}break;
        case 23 : {a = 0x3820;b = 0xb566;}break;
        case 24 : {a = 0x3815;b = 0xb53c;}break;
        case 25 : {a = 0x380d;b = 0xb51c;}break;
        case 26 : {a = 0x3808;b = 0xb508;}break;
        case 27 : {a = 0x3801;b = 0xb4ed;}break;
        default : {a = 0xffff;b = 0xffff;}break;
    }
    *coefficient = a;
    *bias = b;
}


unsigned short table_reflect(unsigned short exp)
{
    switch(exp)
    {
        case 1 : return 0xcb00; break;  //-14
        case 2 : return 0xca80; break;  //-13
        case 3 : return 0xca00; break;  //-12
        case 4 : return 0xc980; break;  //-11
        case 5 : return 0xc900;   break;    //-10
        case 6 : return 0xc880;   break;    //-9
        case 7 : return 0xc800;   break;    //-8
        case 8 : return 0xc700;   break;    //-7
        case 9 : return 0xc600;   break;    //-6
        case 10 : return 0xc500;   break;   //-5
        case 11 : return 0xc400;   break;   //-4
        case 12 : return 0xc200;   break;   //-3
        case 13 : return 0xc000;   break;   //-2
        case 14 : return 0xbc00;   break;   //-1
        case 15 : return 0x0000;   break;   //0
        case 16 : return 0x3c00;  break;    //1
        case 17 : return 0x4000;  break;    //2
        case 18 : return 0x4200;  break;    //3
        case 19 : return 0x4400;  break;    //4
        case 20 : return 0x4500;  break;    //5
        case 21 : return 0x4600;  break;    //6
        case 22 : return 0x4700;  break;    //7
        case 23 : return 0x4800;  break;    //8
        case 24 : return 0x4880;  break;    //9
        case 25 : return 0x4900;  break;    //10
        case 26 : return 0x4980;  break;    //11
        case 27 : return 0x4a00;  break;    //12
        case 28 : return 0x4a80;  break;    //13
        case 29 : return 0x4b00;  break;    //14
        case 30 : return 0x4b80;  break;    //15
    }
}