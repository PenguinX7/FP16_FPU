#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "FP16_MAC_algorithim.h"
#include "FP16_ex_algorithim.h"

// int main()
// {
//     unsigned short a = 0x47ce;
//     a = FP16_ex(a);
//     return 0;
// }

unsigned short FP16_ex(unsigned short data_i)
{
    unsigned short sign;
    short exp;
    unsigned short rm_a,y_a,y_b,y_exp,y_cache;
    unsigned int rm;    //[23:0]rm
    bool overflow;
    short comp;
    unsigned short sign_res,exp_res,rm_res;
    float a;

    //step1:input_check
    if((((data_i & 0x7fff) > 0x498c) && ~(data_i >> 15)) || (((data_i & 0x7fff) > 0x48da) && (data_i >> 15)))   //if((data_i[15] && data_i[14:0] > 15'h431c) || (~data_i[15] && data_i[14:0] > 15'h48da))
        overflow = 1;
    else
    {
        overflow = 0;
        sign = data_i >> 15;
        exp = (data_i >> 10) & 0x1f;    //exp = data_i[14:10]
        rm = (data_i & 0x3ff) << 14;    //rm = {data_i[9:0],14'd0}
    }

    if(overflow)
    {
        return sign ? 0x0000 : 0xffff;
    }
    else
    {
        //step2 : shift
        switch(exp - 15)
        {
            case 3 : {rm_a = rm >> 21;rm = (rm << 3) & 0xffffff;}   break;  //rm_a = rm[23:21],rm = rm << 3
            case 2 : {rm_a = rm >> 22;rm = (rm << 2) & 0xffffff;}   break;
            case 1 : {rm_a = rm >> 23;rm = (rm << 1) & 0xffffff;}  break;
            case 0 : {rm_a = 0; rm = rm;}    break;
            case -1: {rm_a = 0; rm = rm >> 1;}  break;
            case -2: {rm_a = 0; rm = rm >> 2;}  break;
            case -3: {rm_a = 0; rm = rm >> 3;}  break;
            case -4: {rm_a = 0; rm = rm >> 4;}  break;
            case -5: {rm_a = 0; rm = rm >> 5;}  break;
            case -6: {rm_a = 0; rm = rm >> 6;}  break;
            case -7: {rm_a = 0; rm = rm >> 7;}  break;
            case -8: {rm_a = 0; rm = rm >> 8;}  break;
            case -9: {rm_a = 0; rm = rm >> 9;}  break;
            case -10: {rm_a = 0; rm = rm >> 10;}  break;
            case -11: {rm_a = 0; rm = rm >> 11;}  break;
            case -12: {rm_a = 0; rm = rm >> 12;}  break;
            case -13: {rm_a = 0; rm = rm >> 13;}  break;
            case -14: {rm_a = 0; rm = rm >> 14;}  break;
        }

        //step3:table
        comp = (rm >> 14) & 0x7;
        rm = rm >> 17;  //rm = rm[23:17]
        if(sign)            //负,s = 1
        {
            y_a = table1(rm_a + 41);
            y_exp = table1(exp + 18);
            y_b = table2(rm + 128);
        }
        else
        {
            y_a = table1(rm_a + 37);
            y_exp = table1(exp);
            y_b = table2(rm);
        }

        //step4:mcl
        // printf("y_a = \n");
        // a = FP162float(y_a);
        // printf("y_exp = \n");
        // a = FP162float(y_exp);
        // printf("y_b = \n");
        // a = FP162float(y_b);
        // y_cache = FP16_addandmcl(y_a,y_exp,1);
        y_cache = FP16_MAC(y_b,y_exp,0x0000);
        // printf("y_cache = y_a * y_exp = \n");
        // a = FP162float(y_cache);

        y_cache = FP16_MAC(y_cache,y_a,0x0000);
        // printf("y_cache = y_exp * y_a * y_b = \n");
        // a = FP162float(y_cache);

        //step5:comp
        ///*
        //comp = y_cache & 0x7;
        sign_res = y_cache >> 15;
        exp_res = (y_cache >> 10) & 0x1f;
        rm_res = y_cache & 0x3ff | 0x400;

        if((sign == 0) && (exp <= 0x11))  //正数补偿,注意要用输入的sign和exp
        {
            switch(comp)
            {
                case 0 : rm_res = rm_res;   break;
                case 1 : rm_res += 1;   break;
                case 2 : rm_res += 2;   break;
                case 3 : rm_res += 3;   break;
                case 4 : rm_res += 5;   break;
                case 5 : rm_res += 6;   break;
                case 6 : rm_res += 8;   break;
                case 7 : rm_res += 10;  break;
            }
        }
        else if((sign == 1) && (exp > 0x1))
        {
            switch(comp)
            {
                case 0 : rm_res = rm_res;   break;
                case 1 : rm_res -= 1;   break;
                case 2 : rm_res -= 2;   break;
                case 3 : rm_res -= 3;   break;
                case 4 : rm_res -= 5;   break;
                case 5 : rm_res -= 6;   break;
                case 6 : rm_res -= 8;   break;
                case 7 : rm_res -= 10;  break;
            }
        }

        if((rm_res >> 11) == 1)
        {
            exp_res += 1;
            rm_res = rm_res >> 1;
        }
        else if((rm_res >> 10) == 1)
        {}
        else if((rm_res >> 9) == 1)
        {
            exp_res -= 1;
            rm_res = rm_res << 1; 
        }

        y_cache = (sign_res << 15) | (exp_res << 10) | (rm_res & 0x3ff);
        //*/
        return y_cache;
    }
}

//for exp and a
unsigned short table1(short addra)
{
    switch(addra)
    {
        case 0 : return 0x3c00; break;
        case 1 : return 0x3c00; break;
        case 2 : return 0x3c00; break;
        case 3 : return 0x3c00; break;
        case 4 : return 0x3c01; break;
        case 5 : return 0x3c01; break;
        case 6 : return 0x3c02; break;
        case 7 : return 0x3c04; break;
        case 8 : return 0x3c08; break;
        case 9 : return 0x3c10; break;
        case 10: return 0x3c21; break;
        case 11: return 0x3c42; break;
        case 12: return 0x3c88; break;
        case 13: return 0x3d23; break;
        case 14: return 0x3e98; break;
        case 15: return 0x4170; break;
        case 16: return 0x4764; break;
        case 17: return 0x52d3; break;
        case 18: return 0x69d1; break;
        case 19: return 0x3c00; break;
        case 20: return 0x3c00; break;
        case 21: return 0x3c00; break;
        case 22: return 0x3bff; break;
        case 23: return 0x3bfe; break;
        case 24: return 0x3bfc; break;
        case 25: return 0x3bf8; break;
        case 26: return 0x3bf0; break;
        case 27: return 0x3be0; break;
        case 28: return 0x3bc1; break;
        case 29: return 0x3b84; break;
        case 30: return 0x3b0f; break;
        case 31: return 0x3a3b; break;
        case 32: return 0x38da; break;
        case 33: return 0x35e3; break;
        case 34: return 0x3055; break;
        case 35: return 0x24b0; break;
        case 36: return 0x0d7f; break;
        case 37: return 0x3c00; break;
        case 38: return 0x4170; break;
        case 39: return 0x4764; break;
        case 40: return 0x4d05; break;
        case 41: return 0x3c00; break;
        case 42: return 0x35e3; break;
        case 43: return 0x3055; break;
        case 44: return 0x2a5f; break;
        default  : return 0xffff;   break;
    }
}

//for b
unsigned short table2(unsigned short addra)
{
 switch(addra)
 {
  case 0 : return 0x3c00; break;
  case 1 : return 0x3c08; break;
  case 2 : return 0x3c10; break;
  case 3 : return 0x3c18; break;
  case 4 : return 0x3c21; break;
  case 5 : return 0x3c29; break;
  case 6 : return 0x3c31; break;
  case 7 : return 0x3c3a; break;
  case 8 : return 0x3c42; break;
  case 9 : return 0x3c4b; break;
  case 10 : return 0x3c53; break;
  case 11 : return 0x3c5c; break;
  case 12 : return 0x3c65; break;
  case 13 : return 0x3c6d; break;
  case 14 : return 0x3c76; break;
  case 15 : return 0x3c7f; break;
  case 16 : return 0x3c88; break;
  case 17 : return 0x3c91; break;
  case 18 : return 0x3c9b; break;
  case 19 : return 0x3ca4; break;
  case 20 : return 0x3cad; break;
  case 21 : return 0x3cb7; break;
  case 22 : return 0x3cc0; break;
  case 23 : return 0x3cca; break;
  case 24 : return 0x3cd3; break;
  case 25 : return 0x3cdd; break;
  case 26 : return 0x3ce7; break;
  case 27 : return 0x3cf0; break;
  case 28 : return 0x3cfa; break;
  case 29 : return 0x3d04; break;
  case 30 : return 0x3d0e; break;
  case 31 : return 0x3d19; break;
  case 32 : return 0x3d23; break;
  case 33 : return 0x3d2d; break;
  case 34 : return 0x3d38; break;
  case 35 : return 0x3d42; break;
  case 36 : return 0x3d4d; break;
  case 37 : return 0x3d57; break;
  case 38 : return 0x3d62; break;
  case 39 : return 0x3d6d; break;
  case 40 : return 0x3d78; break;
  case 41 : return 0x3d83; break;
  case 42 : return 0x3d8e; break;
  case 43 : return 0x3d99; break;
  case 44 : return 0x3da4; break;
  case 45 : return 0x3daf; break;
  case 46 : return 0x3dbb; break;
  case 47 : return 0x3dc6; break;
  case 48 : return 0x3dd2; break;
  case 49 : return 0x3dde; break;
  case 50 : return 0x3de9; break;
  case 51 : return 0x3df5; break;
  case 52 : return 0x3e01; break;
  case 53 : return 0x3e0d; break;
  case 54 : return 0x3e19; break;
  case 55 : return 0x3e26; break;
  case 56 : return 0x3e32; break;
  case 57 : return 0x3e3e; break;
  case 58 : return 0x3e4b; break;
  case 59 : return 0x3e58; break;
  case 60 : return 0x3e64; break;
  case 61 : return 0x3e71; break;
  case 62 : return 0x3e7e; break;
  case 63 : return 0x3e8b; break;
  case 64 : return 0x3e98; break;
  case 65 : return 0x3ea6; break;
  case 66 : return 0x3eb3; break;
  case 67 : return 0x3ec0; break;
  case 68 : return 0x3ece; break;
  case 69 : return 0x3edc; break;
  case 70 : return 0x3ee9; break;
  case 71 : return 0x3ef7; break;
  case 72 : return 0x3f05; break;
  case 73 : return 0x3f13; break;
  case 74 : return 0x3f21; break;
  case 75 : return 0x3f30; break;
  case 76 : return 0x3f3e; break;
  case 77 : return 0x3f4d; break;
  case 78 : return 0x3f5b; break;
  case 79 : return 0x3f6a; break;
  case 80 : return 0x3f79; break;
  case 81 : return 0x3f88; break;
  case 82 : return 0x3f97; break;
  case 83 : return 0x3fa6; break;
  case 84 : return 0x3fb6; break;
  case 85 : return 0x3fc5; break;
  case 86 : return 0x3fd5; break;
  case 87 : return 0x3fe5; break;
  case 88 : return 0x3ff4; break;
  case 89 : return 0x4002; break;
  case 90 : return 0x400a; break;
  case 91 : return 0x4012; break;
  case 92 : return 0x401b; break;
  case 93 : return 0x4023; break;
  case 94 : return 0x402b; break;
  case 95 : return 0x4033; break;
  case 96 : return 0x403c; break;
  case 97 : return 0x4044; break;
  case 98 : return 0x404d; break;
  case 99 : return 0x4056; break;
  case 100 : return 0x405e; break;
  case 101 : return 0x4067; break;
  case 102 : return 0x4070; break;
  case 103 : return 0x4079; break;
  case 104 : return 0x4082; break;
  case 105 : return 0x408b; break;
  case 106 : return 0x4094; break;
  case 107 : return 0x409d; break;
  case 108 : return 0x40a6; break;
  case 109 : return 0x40b0; break;
  case 110 : return 0x40b9; break;
  case 111 : return 0x40c3; break;
  case 112 : return 0x40cc; break;
  case 113 : return 0x40d6; break;
  case 114 : return 0x40e0; break;
  case 115 : return 0x40e9; break;
  case 116 : return 0x40f3; break;
  case 117 : return 0x40fd; break;
  case 118 : return 0x4107; break;
  case 119 : return 0x4111; break;
  case 120 : return 0x411b; break;
  case 121 : return 0x4126; break;
  case 122 : return 0x4130; break;
  case 123 : return 0x413a; break;
  case 124 : return 0x4145; break;
  case 125 : return 0x4150; break;
  case 126 : return 0x415a; break;
  case 127 : return 0x4165; break;
  case 128 : return 0x3c00; break;
  case 129 : return 0x3bf0; break;
  case 130 : return 0x3be0; break;
  case 131 : return 0x3bd1; break;
  case 132 : return 0x3bc1; break;
  case 133 : return 0x3bb2; break;
  case 134 : return 0x3ba2; break;
  case 135 : return 0x3b93; break;
  case 136 : return 0x3b84; break;
  case 137 : return 0x3b75; break;
  case 138 : return 0x3b66; break;
  case 139 : return 0x3b57; break;
  case 140 : return 0x3b49; break;
  case 141 : return 0x3b3a; break;
  case 142 : return 0x3b2c; break;
  case 143 : return 0x3b1e; break;
  case 144 : return 0x3b0f; break;
  case 145 : return 0x3b01; break;
  case 146 : return 0x3af3; break;
  case 147 : return 0x3ae5; break;
  case 148 : return 0x3ad8; break;
  case 149 : return 0x3aca; break;
  case 150 : return 0x3abd; break;
  case 151 : return 0x3aaf; break;
  case 152 : return 0x3aa2; break;
  case 153 : return 0x3a95; break;
  case 154 : return 0x3a88; break;
  case 155 : return 0x3a7b; break;
  case 156 : return 0x3a6e; break;
  case 157 : return 0x3a61; break;
  case 158 : return 0x3a54; break;
  case 159 : return 0x3a47; break;
  case 160 : return 0x3a3b; break;
  case 161 : return 0x3a2f; break;
  case 162 : return 0x3a22; break;
  case 163 : return 0x3a16; break;
  case 164 : return 0x3a0a; break;
  case 165 : return 0x39fe; break;
  case 166 : return 0x39f2; break;
  case 167 : return 0x39e6; break;
  case 168 : return 0x39da; break;
  case 169 : return 0x39cf; break;
  case 170 : return 0x39c3; break;
  case 171 : return 0x39b8; break;
  case 172 : return 0x39ac; break;
  case 173 : return 0x39a1; break;
  case 174 : return 0x3996; break;
  case 175 : return 0x398b; break;
  case 176 : return 0x3980; break;
  case 177 : return 0x3975; break;
  case 178 : return 0x396a; break;
  case 179 : return 0x395f; break;
  case 180 : return 0x3954; break;
  case 181 : return 0x394a; break;
  case 182 : return 0x393f; break;
  case 183 : return 0x3935; break;
  case 184 : return 0x392a; break;
  case 185 : return 0x3920; break;
  case 186 : return 0x3916; break;
  case 187 : return 0x390c; break;
  case 188 : return 0x3902; break;
  case 189 : return 0x38f8; break;
  case 190 : return 0x38ee; break;
  case 191 : return 0x38e4; break;
  case 192 : return 0x38da; break;
  case 193 : return 0x38d1; break;
  case 194 : return 0x38c7; break;
  case 195 : return 0x38bd; break;
  case 196 : return 0x38b4; break;
  case 197 : return 0x38ab; break;
  case 198 : return 0x38a1; break;
  case 199 : return 0x3898; break;
  case 200 : return 0x388f; break;
  case 201 : return 0x3886; break;
  case 202 : return 0x387d; break;
  case 203 : return 0x3874; break;
  case 204 : return 0x386b; break;
  case 205 : return 0x3862; break;
  case 206 : return 0x3859; break;
  case 207 : return 0x3851; break;
  case 208 : return 0x3848; break;
  case 209 : return 0x3840; break;
  case 210 : return 0x3837; break;
  case 211 : return 0x382f; break;
  case 212 : return 0x3826; break;
  case 213 : return 0x381e; break;
  case 214 : return 0x3816; break;
  case 215 : return 0x380e; break;
  case 216 : return 0x3806; break;
  case 217 : return 0x37fc; break;
  case 218 : return 0x37ec; break;
  case 219 : return 0x37dc; break;
  case 220 : return 0x37cc; break;
  case 221 : return 0x37bd; break;
  case 222 : return 0x37ad; break;
  case 223 : return 0x379e; break;
  case 224 : return 0x378f; break;
  case 225 : return 0x3780; break;
  case 226 : return 0x3771; break;
  case 227 : return 0x3762; break;
  case 228 : return 0x3753; break;
  case 229 : return 0x3745; break;
  case 230 : return 0x3736; break;
  case 231 : return 0x3728; break;
  case 232 : return 0x371a; break;
  case 233 : return 0x370b; break;
  case 234 : return 0x36fd; break;
  case 235 : return 0x36ef; break;
  case 236 : return 0x36e2; break;
  case 237 : return 0x36d4; break;
  case 238 : return 0x36c6; break;
  case 239 : return 0x36b9; break;
  case 240 : return 0x36ab; break;
  case 241 : return 0x369e; break;
  case 242 : return 0x3691; break;
  case 243 : return 0x3684; break;
  case 244 : return 0x3677; break;
  case 245 : return 0x366a; break;
  case 246 : return 0x365d; break;
  case 247 : return 0x3651; break;
  case 248 : return 0x3644; break;
  case 249 : return 0x3638; break;
  case 250 : return 0x362b; break;
  case 251 : return 0x361f; break;
  case 252 : return 0x3613; break;
  case 253 : return 0x3607; break;
  case 254 : return 0x35fb; break;
  case 255 : return 0x35ef; break;
  default : return 0xffff; break; }
}