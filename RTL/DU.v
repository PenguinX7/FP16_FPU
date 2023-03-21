`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2022/11/30 17:56:22
// Design Name: 
// Module Name: FP16_div
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module FP16_div(
    input data_dividend,
    input data_divisor,
    input input_valid,
    input clk,
    input rst,
    output sign_o,
    output exp_o,
    output rm_o,
    output output_update,
    output idle,
    output round_o
    );
    
    wire [15:0]data_dividend;
    wire [15:0]data_divisor;
    wire input_valid;
    wire clk;
    wire rst;
    reg output_update;
    reg idle;
    reg sign_o;
    reg signed [6:0]exp_o;
    reg [11:0]rm_o;
    reg round_o;
    
    reg [2:0]state;
    reg overflow;
    reg sign_x;
    reg sign_d;
    reg signed [6:0]exp_x;
    reg signed [6:0]exp_d;
    reg signed [6:0]exp_q;
    reg [44:0]rm_x;
    reg [44:0]rm_x_cache1;
    reg [44:0]rm_x_cache2;
    reg [43:0]rm_d;
    reg round_flag;
    reg [5:0]n;
    
    always@(posedge clk or posedge rst) begin       //state control
        if(rst) begin
            state <= 3'd0;
        end
        else    begin
            case (state)
                3'd0 : begin               //input_check =>E,S,cal
                    if(input_valid)
                        state <= 1;
                    else
                        state <= 0;
                end
                3'd1 : begin               //E,S,cal =>output or M,cal
                    if(overflow)
                        state <= 6;
                    else
                        state <= 2;
                end
                3'd2 : begin               //M,cal
                    if(n == 0)
                        state <= 3;
                    else
                        state <= 2;
                end
                3'd3 : state <= 0;         //nor_pre =>idle
            endcase
        end
    end
    
    
    always@(posedge clk)    begin
      if(rst)   begin
        output_update <= 1'b0;
        idle <= 1'b0;
        sign_o <= 1'b0;
        exp_o <= 0;
        rm_o <= 12'd0;
        overflow <= 1'b0;
        sign_x <= 1'b0;
        sign_d <= 1'b0;
        exp_x <= 0;
        exp_d <= 0;
        rm_d <= 44'd0;
        rm_x <= 45'd0;
        round_flag <= 1'b0;
        n <= 6'd32;
      end
      else  begin
        case(state)
            3'd0 : begin
                output_update <= 1'b0;
                if(input_valid) begin
                    idle <= 1'b0;
                    sign_x <= data_dividend[15];
                    rm_x[9:0] <= data_dividend[9:0];
                    if(data_dividend[14:10] == 5'd0)    begin
                        if(data_dividend[9:0] == 10'd0) begin   //0
                            exp_x <= data_dividend[14:10];
                            rm_x[44:10] <= 35'd0;
                        end
                        else    begin                           //denormal
                            exp_x <= 7'd1;
                            rm_x[44:10] <= 35'd0;
                        end
                    end
                    else    begin                               //normal
                        exp_x <= data_dividend[14:10];
                        rm_x[44:10] <= 35'd1;
                    end
                    sign_d <= data_divisor[15];
                    rm_d[9:0]  <= data_divisor[9:0];
                    if(data_divisor[14:10] == 5'd0)    begin                           //denormal
                            exp_d <= 7'd1;
                            rm_d[43:10] <= 34'd0;
                            overflow <= 1'b0;
                    end
                    else    begin                               //normal
                        exp_d <= data_divisor[14:10];
                        rm_d[43:10] <= 34'd1;
                        overflow <= 1'b0;
                    end
                end
                else    begin
                    idle <= 1'b1;
                    sign_x <= sign_x;
                    exp_x <= exp_x;
                    rm_x <= rm_x;
                    sign_d <= sign_d;
                    exp_d <= exp_d;
                    rm_d <= rm_d;
                    overflow <= overflow;
                end
            end
            3'd1 : begin
                exp_q <= exp_x - exp_d + 15;
                sign_o <= sign_x ^ sign_d;
                rm_x <= rm_x << 22;
                rm_d <= rm_d << 33;
                n <= 6'd32;
            end
            3'd2 : begin
                if(rm_x[44])    begin
                    rm_x_cache1 = rm_x << 1;
                    rm_x_cache2 = rm_x_cache1 + rm_d;
                end
                else    begin
                    rm_x_cache1 = rm_x << 1;
                    rm_x_cache2 = rm_x_cache1 - rm_d;
                end
                if(rm_x_cache2[44]) begin
                    rm_x[44:1] <= rm_x_cache2[44:1];
                    rm_x[0] <= 1'b0;
                end
                else    begin
                    rm_x[44:1] <= rm_x_cache2[44:1];
                    rm_x[0] <= 1'b1;
                end
                n <= n - 1;
            end
            3'd3 : begin
                output_update <= 1'b1;
                casex(rm_x[32:0])
                    33'b1_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx :begin
                        rm_o <= rm_x[32:22];
                        round_o <= rm_x[21];
                        exp_o <= exp_q + 10;
                    end
                    33'b0_1xxx_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx :begin
                        rm_o <= rm_x[31:21];
                        round_o <= rm_x[20];
                        exp_o <= exp_q + 9;
                    end
                    33'b0_01xx_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx :begin
                        rm_o <= rm_x[30:20];
                        round_o <= rm_x[19];
                        exp_o <= exp_q + 8;
                    end
                    33'b0_001x_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx :begin
                        rm_o <= rm_x[29:19];
                        round_o <= rm_x[18];
                        exp_o <= exp_q + 7;
                    end
                    33'b0_0001_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx :begin
                        rm_o <= rm_x[28:18];
                        round_o <= rm_x[17];
                        exp_o <= exp_q + 6;
                    end
                    33'b0_0000_1xxx_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx :begin
                        rm_o <= rm_x[27:17];
                        round_o <= rm_x[16];
                        exp_o <= exp_q + 5;
                    end
                    33'b0_0000_01xx_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx :begin
                        rm_o <= rm_x[26:16];
                        round_o <= rm_x[15];
                        exp_o <= exp_q + 4;
                    end
                    33'b0_0000_001x_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx :begin
                        rm_o <= rm_x[25:15];
                        round_o <= rm_x[14];
                        exp_o <= exp_q + 3;
                    end
                    33'b0_0000_0001_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx :begin
                        rm_o <= rm_x[24:14];
                        round_o <= rm_x[13];
                        exp_o <= exp_q + 2;
                    end
                    33'b0_0000_0000_1xxx_xxxx_xxxx_xxxx_xxxx_xxxx :begin
                        rm_o <= rm_x[23:13];
                        round_o <= rm_x[12];
                        exp_o <= exp_q + 1;
                    end
                    33'b0_0000_0000_01xx_xxxx_xxxx_xxxx_xxxx_xxxx :begin
                        rm_o <= rm_x[22:12];
                        round_o <= rm_x[11];
                        exp_o <= exp_q;
                    end
                    33'b0_0000_0000_001x_xxxx_xxxx_xxxx_xxxx_xxxx :begin
                        rm_o <= rm_x[21:11];
                        round_o <= rm_x[10];
                        exp_o <= exp_q - 1;
                    end
                    33'b0_0000_0000_0001_xxxx_xxxx_xxxx_xxxx_xxxx :begin
                        rm_o <= rm_x[20:10];
                        round_o <= rm_x[9];
                        exp_o <= exp_q - 2;
                    end
                    33'b0_0000_0000_0000_1xxx_xxxx_xxxx_xxxx_xxxx :begin
                        rm_o <= rm_x[19:9];
                        round_o <= rm_x[8];
                        exp_o <= exp_q - 3;
                    end
                    33'b0_0000_0000_0000_01xx_xxxx_xxxx_xxxx_xxxx :begin
                        rm_o <= rm_x[18:8];
                        round_o <= rm_x[7];
                        exp_o <= exp_q - 4;
                    end
                    33'b0_0000_0000_0000_001x_xxxx_xxxx_xxxx_xxxx :begin
                        rm_o <= rm_x[17:7];
                        round_o <= rm_x[6];
                        exp_o <= exp_q - 5;
                    end
                    33'b0_0000_0000_0000_0001_xxxx_xxxx_xxxx_xxxx :begin
                        rm_o <= rm_x[16:6];
                        round_o <= rm_x[5];
                        exp_o <= exp_q - 6;
                    end
                    33'b0_0000_0000_0000_0000_1xxx_xxxx_xxxx_xxxx :begin
                        rm_o <= rm_x[15:5];
                        round_o <= rm_x[4];
                        exp_o <= exp_q - 7;
                    end
                    33'b0_0000_0000_0000_0000_01xx_xxxx_xxxx_xxxx :begin
                        rm_o <= rm_x[14:4];
                        round_o <= rm_x[3];
                        exp_o <= exp_q - 8;
                    end
                    33'b0_0000_0000_0000_0000_001x_xxxx_xxxx_xxxx :begin
                        rm_o <= rm_x[13:3];
                        round_o <= rm_x[2];
                        exp_o <= exp_q - 9;
                    end
                    33'b0_0000_0000_0000_0000_0001_xxxx_xxxx_xxxx :begin
                        rm_o <= rm_x[12:2];
                        round_o <= rm_x[1];
                        exp_o <= exp_q - 10;
                    end
                    33'b0_0000_0000_0000_0000_0000_1xxx_xxxx_xxxx :begin
                        rm_o <= rm_x[11:1];
                        round_o <= rm_x[0];
                        exp_o <= exp_q - 11;
                    end
                    default : begin
                        rm_o <= 12'd0;
                        round_o <= 1'b0;
                        exp_o <= 0;
                    end
                endcase
            end
        endcase
      end
    end
    
endmodule
