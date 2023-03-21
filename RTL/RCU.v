`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2023/03/08 16:18:59
// Design Name: 
// Module Name: RCU
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


module RCU(
    input opcode,
    input rst,
    input MAC_over,
    input div_over,
    input sign_MAC,
    input exp_MAC,
    input rm_MAC,
    input round_MAC,
    input sign_q,
    input exp_q,
    input rm_q,
    input round_q,
    input clk,
    output data_o,
    output opcode_o,
    output output_up
    );
    
    wire[2:0]opcode;
    wire rst;
    wire MAC_over;
    wire div_over;
    wire sign_MAC;
    wire signed [6:0]exp_MAC;
    wire [11:0] rm_MAC;
    wire round_MAC;
    wire sign_q;
    wire signed [6:0]exp_q;
    wire [11:0] rm_q;
    wire round_q;
    wire clk;
    reg [2:0]opcode_o;
    reg [15:0]data_o;
    reg output_up;
    
    
    reg sign2;
    reg signed [6:0]exp2;
    reg [11:0] rm2;
    reg round_flag2;
    reg denormal_over;
    reg [2:0]opcode2;
    wire sign;
    wire signed [6:0]exp;
    wire [11:0]rm;
    wire round_flag;
    assign sign = MAC_over ? sign_MAC : sign_q;
    assign exp = MAC_over ? exp_MAC : exp_q;
    assign rm = MAC_over ? rm_MAC : rm_q;
    assign round_flag = MAC_over ? round_MAC : round_q;
    
    always@(posedge clk)    begin
        if(rst) begin                   //rst
            exp2 <= 0;
            rm2 <= 12'd0;
            sign2 <= 1'b0;
            round_flag2 <= 1'b0;
            denormal_over <= 1'b0;
            opcode2 <= 2'd0;
        end
        else  if(MAC_over || div_over)  begin       //work
            opcode2 <= MAC_over ? opcode : 3'd2;
            denormal_over <= 1'b1;
            sign2 <= sign;
            if(exp < 1) begin
                case(exp)
                    0 : begin
                        round_flag2 <= rm[0];
                        rm2 <= rm >> 1;
                    end
                    -1 : begin
                        round_flag2 <= rm[1];
                        rm2 <= rm >> 2;
                    end
                    -2 : begin
                        round_flag2 <= rm[2];
                        rm2 <= rm >> 3;
                    end
                    -3 : begin
                        round_flag2 <= rm[3];
                        rm2 <= rm >> 4;
                    end
                    -4 : begin
                        round_flag2 <= rm[4];
                        rm2 <= rm >> 5;
                    end
                    -5 : begin
                        round_flag2 <= rm[5];
                        rm2 <= rm >> 6;
                    end
                    -6 : begin
                        round_flag2 <= rm[6];
                        rm2 <= rm >> 7;
                    end
                    -7 : begin
                        round_flag2 <= rm[7];
                        rm2 <= rm >> 8;
                    end
                    -8 : begin
                        round_flag2 <= rm[8];
                        rm2 <= rm >> 9;
                    end
                    -9 : begin
                        round_flag2 <= rm[9];
                        rm2 <= rm >> 10;
                    end
                    -10 : begin
                        round_flag2 <= rm[10];
                        rm2 <= rm >> 11;
                    end
                    default : begin
                        round_flag2 <= 1'b0;
                        rm2 <= 12'd0;
                    end
                endcase
                exp2 <= 0;
            end
            else    begin
                exp2 <= exp;
                rm2 <= rm;
                round_flag2 <= round_flag;
            end
        end
        else       begin                                 //idle
            opcode2 <= opcode2;
            denormal_over <= 1'b0;
            sign2 <= sign2;
            exp2 <= exp2;
            rm2 <= rm2;
            round_flag2 <= round_flag2;
        end
    end
    
    
    //round and carry
    reg sign3;
    reg signed [6:0]exp3;
    reg [11:0]rm3;
    reg [11:0]rm3_cache;
    reg [2:0]opcode3;
    reg RC_over;
    
    always @(posedge clk)   begin
        if(rst) begin                                   //rst
            opcode3 <= 3'd0;
            RC_over <= 1'b0;
            rm3 <= 12'd0;
            exp3 <= 0;
            sign3 <= 1'b0;
        end
        else  if  (denormal_over)   begin               //work
            opcode3<= opcode2;
            RC_over <= 1'b1;
            sign3 <= sign2;
            rm3_cache = rm2 + round_flag2;
            if(exp2 == 0)   begin
                rm3 <= rm3_cache;
                if(rm3_cache[10])
                    exp3 <= 1;
                else
                    exp3 <= exp2;
            end
            else    begin
                if(rm3_cache[11])   begin
                    exp3 <= exp2 + 1;
                    rm3 <= rm3_cache >> 1;
                end
                else    begin
                    exp3 <= exp2;
                    rm3 <= rm3_cache;
                end
            end
        end 
        else    begin                                   //idle
            RC_over <= 1'b0;
            opcode3 <= opcode3;
            sign3 <= sign3;
            exp3 <= exp3;
            rm3 <= rm3;
        end
    end
    
    //output
    always @(posedge  clk)begin
        if(rst) begin                           //rst
            data_o <= 16'd0;
            output_up <= 1'b0;
            opcode_o <= 3'd0;
        end
        else if(RC_over)    begin               //work
            output_up <= 1'b1;
            opcode_o <= opcode3;
            if(exp3 > 30)    
                data_o <= {sign3,15'h7bff};
            else if(rm3 == 12'd0)
                data_o <= 16'd0;
            else
                data_o <= {sign3,exp3[4:0],rm3[9:0]};
        end
        else    begin                           //idle
            output_up <= 1'b0;
            opcode_o <= opcode_o;
            data_o <= data_o;
        end
    end
endmodule
