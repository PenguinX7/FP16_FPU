`timescale 1ns / 1ps

module FP16_FPU(
    input clk,
    input rst,
    input input_valid,
    input data1,
    input data2,
    input data3,
    input opcode,
    output idle,
    output output_up,
    output data_o,
    output opcode_o
    );
    
    wire clk;
    wire rst;
    wire input_valid;
    wire [15:0]data1;
    wire [15:0]data2;
    wire [15:0]data3;
    wire [2:0]opcode;
    reg idle;
    reg output_up;
    reg [15:0]data_o;
    reg [2:0]opcode_o;
    
    reg [4:0]state;
    reg count;
    reg [2:0]op;
    //for ex
    reg sign_ex;
    reg signed [6:0]exp_ex;
    reg [23:0]rm_ex;
    reg [2:0]rm_a;
    reg [2:0]comp;
    reg [15:0]y_exp;
    reg [15:0]y_a;
    reg sign_res;
    reg [4:0]exp_res;
    reg [11:0]rm_res;
    reg [11:0]rm_res_cache;
    //for lnx
    reg signed [6:0]exp_lnx;
    reg [15:0]rm_lnx;
    reg signed [6:0]E_lnxo;
    reg S_lnxo;
    reg [10:0]M_lnxo;
    reg [15:0]lnxo;
    
    //instantiate table1
    reg [7 : 0] addra_table2;
    wire [15 : 0] douta_table2;
    
    //instantiate table1
    reg [6 : 0] addra_table1;
    wire [31 : 0] douta_table1;
    
    //instantiate MACU
    reg [15:0] MAC_k;
    reg [15:0] MAC_x;
    reg [15:0] MAC_b;
    reg MAC_rst;
    reg MAC_input_valid;
    reg [2:0]MAC_opcode;
    wire MAC_sign;
    wire signed [6:0]MAC_exp;
    wire [11:0]MAC_rm;
    wire MAC_over;
    wire [2:0]MAC_opcode_o;
    wire MAC_round;
    
    //instantiate DU
    reg [15:0] D_dividend;
    reg [15:0] D_divisor;
    reg D_input_valid;
    reg D_rst;
    wire D_sign;
    wire signed [6:0]D_exp;
    wire [11:0]D_rm;
    wire D_over;
    wire D_idle;
    wire D_round;
    
    //instantiate RCU
    reg RCU_rst;
    wire [15:0]RCU_o;
    wire [2:0]RCU_opcode_o;
    wire RCU_over;
    
    always @(posedge clk)   begin
        if(rst) begin
            state <= 0;
        end
        else   begin
            case(state)
                5'd0 : begin
                    if(input_valid) begin
                        case(opcode)
                            2 : state <= 18;         //div   //to wait
                            4 : state <= 1;         //ex    //ex input_check => shift
                            5 : state <= 10;         //lnx   //to lnx input_check
                            default : state <= 0;   //invalid opcode or +-*/
                        endcase
                    end
                    else
                        state <= 0;
                end
                5'd1 : state <= 2;                       //ex shift =>table_search
                5'd2 : state <= 3;                     //ex table_search =>
                5'd3 : state <= 4;
                5'd4 : state <= 5;
                5'd5 : state <= 6;
                5'd6 : begin
                    if(~count && RCU_over && (RCU_opcode_o == 3'd4))
                        state <= 7;
                    else if(count && RCU_over && (RCU_opcode_o == 3'd4))
                        state <= 8;
                    else
                        state <= 6;
                end
                5'd7 : state <= 6;
                5'd8 : state <= 9;
                5'd9 : state <= 0;//独立输出,回到空闲
                5'd10 : state <= 11;                       //lnx
                5'd11 : state <= 12;
                5'd12 : state <= 13;
                5'd13 : state <= 14;
                5'd14 : state <= 15;
                5'd15 : begin
                    if(~count && RCU_over && (RCU_opcode_o == 3'd5))
                        state <= 16;
                    else if(count && RCU_over && (RCU_opcode_o == 3'd5))
                        state <= 17;
                    else
                        state <= 15;
                end
                5'd16 : state <= 15;
                5'd17 : state <= 8;
                5'd18 : begin
                    if(RCU_over && (RCU_opcode_o == 3'd2))
                        state <= 0;
                    else
                        state <= 18;
                end
            endcase
        end
    end
    
    always @(posedge clk)   begin
        if(rst) begin
            MAC_rst <= 1'b1;
            D_rst <= 1'b1;
            idle <= 1'b1;
            RCU_rst <= 1'b1;
            sign_ex <= 1'b0;
            exp_ex <= 0;
            rm_ex <= 24'd0;
            rm_a <= 3'd0;
            comp <= 3'd0;
            y_exp <= 16'd0;
            y_a <= 16'd0;
            sign_res <= 1'b0;
            exp_res <= 5'd0;
            rm_res <= 12'd0;
            rm_res_cache <= 12'd0;
            exp_lnx <= 0;
            rm_lnx <= 16'd0; 
        end
        else    begin
            case(state) 
                5'd0 : begin
                    MAC_rst <= 1'b0;
                    D_rst <= 1'b0;
                    RCU_rst <= 1'b0;
                    if(input_valid) begin
                        op <= opcode;
                        case(opcode)
                            0 : begin       //add
                                idle <= 1'b1;
                                MAC_input_valid <= 1'b1;
                                D_input_valid <= 1'b0;
                                MAC_k <= data1;
                                MAC_x <= 16'h3c00;
                                MAC_b <= data2;
                                MAC_opcode <= opcode;
                            end
                            1 : begin       //mcl
                                idle <= 1'b1;
                                MAC_input_valid <= 1'b1;
                                D_input_valid <= 1'b0;
                                MAC_k <= data1;
                                MAC_x <= data2;
                                MAC_b <= 16'h0000;
                                MAC_opcode <= opcode;
                            end
                            2 : begin       //div
                                idle <= 1'b0;
                                MAC_input_valid <= 1'b0;
                                D_input_valid <= 1'b1;
                                D_dividend <= data1;
                                D_divisor <= data2;
                            end
                            3 : begin       //MAC
                                idle <= 1'b1;
                                MAC_input_valid <= 1'b1;
                                D_input_valid <= 1'b0;
                                MAC_k <= data1;
                                MAC_x <= data2;
                                MAC_b <= data3;
                                MAC_opcode <= opcode;
                            end
                            4 : begin       //ex input_check
                                MAC_input_valid <= 1'b0;
                                D_input_valid <= 1'b0;
                                idle <= 1'b0;
                                count <= 1'b0;
                                if((data1[14:0] > 15'h498c) && ~data1[15])  begin
                                    sign_ex <= 1'b0;
                                    exp_ex <= 7'b0010010;
                                    rm_ex <= 24'b011000110000000000000000;
                                end
                                else if((data1[14:0] > 15'h48da) && data1[15])  begin
                                    sign_ex <= 1'b1;
                                    exp_ex <= 7'b0010010;
                                    rm_ex <= 24'b001101101000000000000000;
                                end
                                else    begin
                                    sign_ex <= data1[15];
                                    exp_ex <= data1[14:10];
                                    rm_ex <= {data1[9:0],14'd0};
                                end
                            end
                            5 : begin       //lnx input_check
                                MAC_input_valid <= 1'b0;
                                D_input_valid <= 1'b0;
                                idle <= 1'b0;
                                count <= 1'b0;
                                if(data1[15] || (data1[14:0] == 15'd0)) begin
                                    exp_lnx <= 7'b0000001;
                                    rm_lnx <= 16'h3c00;
                                end
                                else    begin
                                    exp_lnx <= data1[14:10];
                                    rm_lnx <= {6'b001111,data1[9:0]};
                                end
                            end
                            default : begin
                                idle <= 1'b1;
                            end
                        endcase
                    end
                    else    begin
                        idle <= 1'b1;
                        MAC_input_valid <= 1'b0;
                        D_input_valid <= 1'b0;
                    end
                end
                5'd1 : begin                               //ex
                    case(exp_ex)
                    6'd18 : begin
                        rm_a <= rm_ex[23:21];
                        rm_ex <= rm_ex << 3;
                    end
                    6'd17 : begin
                        rm_a <= rm_ex[23:22];
                        rm_ex <= rm_ex << 2;
                    end
                    6'd16 : begin
                        rm_a <= rm_ex[23];
                        rm_ex <= rm_ex << 1;
                    end
                    6'd15 : begin
                        rm_a <= 3'd0;
                        rm_ex <= rm_ex;
                    end
                    6'd14 : begin
                        rm_a <= 3'd0;
                        rm_ex <= rm_ex >> 1;
                    end
                    6'd13 : begin
                        rm_a <= 3'd0;
                        rm_ex <= rm_ex >> 2;
                    end
                    6'd12 : begin
                        rm_a <= 3'd0;
                        rm_ex <= rm_ex >> 3;
                    end
                    6'd11 : begin
                        rm_a <= 3'd0;
                        rm_ex <= rm_ex >> 4;
                    end
                    6'd10 : begin
                        rm_a <= 3'd0;
                        rm_ex <= rm_ex >> 5;
                    end
                    6'd9 : begin
                        rm_a <= 3'd0;
                        rm_ex <= rm_ex >> 6;
                    end
                    6'd8 : begin
                        rm_a <= 3'd0;
                        rm_ex <= rm_ex >> 7;
                    end
                    6'd7 : begin
                        rm_a <= 3'd0;
                        rm_ex <= rm_ex >> 8;
                    end
                    6'd6 : begin
                        rm_a <= 3'd0;
                        rm_ex <= rm_ex >> 9;
                    end
                    6'd5 : begin
                        rm_a <= 3'd0;
                        rm_ex <= rm_ex >> 10;
                    end
                    6'd4 : begin
                        rm_a <= 3'd0;
                        rm_ex <= rm_ex >> 11;
                    end
                    6'd3 : begin
                        rm_a <= 3'd0;
                        rm_ex <= rm_ex >> 12;
                    end
                    6'd2 : begin
                        rm_a <= 3'd0;
                        rm_ex <= rm_ex >> 13;
                    end
                    6'd1 : begin
                        rm_a <= 3'd0;
                        rm_ex <= rm_ex >> 14;
                    end
                    default : begin
                        rm_a <= 3'd0;
                        rm_ex <= 24'd0;
                    end
                    endcase
                    addra_table1 <= sign_ex ? exp_ex + 6'd18 : exp_ex;
                end
                5'd2 : begin
                    comp <= rm_ex[16:14];
                    addra_table2 <= sign_ex ? rm_ex[23:17] + 8'd128 : rm_ex[23:17];
                end
                5'd3 : begin
                    addra_table1 <= sign_ex ? rm_a + 6'd41 : rm_a + 6'd37;
                    y_exp <= douta_table1[15:0];
                end
                5'd4 : begin
                    MAC_input_valid <= 1'b1;
                    MAC_opcode <= 3'd4;
                    MAC_k <= y_exp;
                    MAC_x <= douta_table2;
                    MAC_b <= 16'h0000;
                end
                5'd5 : begin
                    y_a <= douta_table1[15:0];
                    MAC_input_valid <= 1'b0;
                end
                5'd6 : begin
                    if(RCU_over && ~count && (RCU_opcode_o == 3'd4))   begin
                        MAC_k <= y_a;
                        MAC_x <= RCU_o;
                        MAC_b <= 16'h0000;
                        MAC_input_valid <= 1'b1;
                        MAC_opcode <= 3'd4;
                    end
                    else if(RCU_over && count && (RCU_opcode_o == 3'd4)) begin
                        sign_res <= RCU_o[15];
                        exp_res <= RCU_o[14:10];
                        rm_res_cache = {1'b1,RCU_o[9:0]};                //logic 
                        if(sign_ex && (exp_ex > 7'd1))    begin
                            case(comp)
                                3'd1 : rm_res <= rm_res_cache - 12'd1;
                                3'd2 : rm_res <= rm_res_cache - 12'd2;
                                3'd3 : rm_res <= rm_res_cache - 12'd3;
                                3'd4 : rm_res <= rm_res_cache - 12'd5;
                                3'd5 : rm_res <= rm_res_cache - 12'd6;
                                3'd6 : rm_res <= rm_res_cache - 12'd8;
                                3'd7 : rm_res <= rm_res_cache - 12'd10;
                                default :  rm_res <= rm_res_cache;
                            endcase
                        end
                        else  if(~sign_ex && (exp_ex <= 7'h11))  begin
                            case(comp)
                                3'd1 : rm_res <= rm_res_cache + 12'd1;
                                3'd2 : rm_res <= rm_res_cache + 12'd2;
                                3'd3 : rm_res <= rm_res_cache + 12'd3;
                                3'd4 : rm_res <= rm_res_cache + 12'd5;
                                3'd5 : rm_res <= rm_res_cache + 12'd6;
                                3'd6 : rm_res <= rm_res_cache + 12'd8;
                                3'd7 : rm_res <= rm_res_cache + 12'd10;
                                default :  rm_res <= rm_res_cache;
                            endcase
                        end
                        else
                            rm_res <= rm_res_cache;
                    end
                end
                5'd7 : begin
                    MAC_input_valid <= 1'b0;
                    count <= 2'd1;
                end
                5'd8 : begin
                    if(rm_res[11])  begin
                        exp_res <= exp_res + 5'd1;
                        rm_res <= rm_res >> 1;
                    end
                    else if(rm_res[10:9] == 2'b01) begin
                        exp_res <= exp_res - 5'd1;
                        rm_res <= rm_res << 1;
                    end
                    else    begin
                        exp_res <= exp_res;
                        rm_res <= rm_res;
                    end
                end
                5'd9 : idle <= 1'b1;
                5'd10 : begin                                  //lnx
                    if(rm_lnx < 16'h3c0a)
                        addra_table1 <= 45;
                    else if(rm_lnx < 16'h3c14)
                        addra_table1 <= 46;
                    else if(rm_lnx < 16'h3c1f)
                        addra_table1 <= 47;
                    else if(rm_lnx < 16'h3c33)
                        addra_table1 <= 48;
                    else if(rm_lnx < 16'h3c48)
                        addra_table1 <= 49;
                    else if(rm_lnx < 16'h3c66)
                        addra_table1 <= 50;
                    else if(rm_lnx < 16'h3c85)
                        addra_table1 <= 51;
                    else if(rm_lnx < 16'h3ca4)
                        addra_table1 <= 52;
                    else if(rm_lnx < 16'h3ccd)
                        addra_table1 <= 53;
                    else if(rm_lnx < 16'h3d00)
                        addra_table1 <= 54;
                    else if(rm_lnx < 16'h3d33)
                        addra_table1 <= 55;
                    else if(rm_lnx < 16'h3d66)
                        addra_table1 <= 56;
                    else if(rm_lnx < 16'h3d9a)
                        addra_table1 <= 57;
                    else if(rm_lnx < 16'h3dd7)
                        addra_table1 <= 58;
                    else if(rm_lnx < 16'h3e14)
                        addra_table1 <= 59;
                    else if(rm_lnx < 16'h3e52)
                        addra_table1 <= 60;
                    else if(rm_lnx < 16'h3e9a)
                        addra_table1 <= 61;
                    else if(rm_lnx < 16'h3ecd)
                        addra_table1 <= 62;
                    else if(rm_lnx < 16'h3f00)
                        addra_table1 <= 63;
                    else if(rm_lnx < 16'h3f33)
                        addra_table1 <= 64;
                    else if(rm_lnx < 16'h3f66)
                        addra_table1 <= 65;
                    else if(rm_lnx < 16'h3f9a)
                        addra_table1 <= 66;
                    else if(rm_lnx < 16'h3fb8)
                        addra_table1 <= 67;
                    else if(rm_lnx < 16'h3fcd)
                        addra_table1 <= 68;
                    else if(rm_lnx < 16'h3fe1)
                        addra_table1 <= 69;
                    else if(rm_lnx < 16'h3fec)
                        addra_table1 <= 70;
                    else if(rm_lnx < 16'h3ff6)
                        addra_table1 <= 71;
                    else 
                        addra_table1 <= 72;
                end
                5'd12 : begin
                    addra_table1 <= exp_lnx + 73;
                end
                5'd13 : begin
                    MAC_k <= douta_table1[31:16];
                    MAC_x <= rm_lnx;
                    MAC_b <= douta_table1[15:0];
                    MAC_input_valid <= 1'b1;
                    MAC_opcode <= 3'd5;
                end
                5'd14 : MAC_input_valid <= 1'b0;
                5'd15 : begin
                    if(~count && RCU_over && (RCU_opcode_o == 3'd5))begin 
                        count <= 1;
                        MAC_k <= douta_table1[15:0];
                        MAC_x <= 16'h398c;
                        MAC_b <= RCU_o;
                        MAC_input_valid <= 1'b1;
                        MAC_opcode <= 3'd5;
                    end
                    else if(count && RCU_over && (RCU_opcode_o == 3'd5))
                        S_lnxo <= RCU_o[15];
                        E_lnxo <= RCU_o[14:10];
                        M_lnxo <= {1'b1,RCU_o[9:0]};
                        lnxo <= RCU_o;
                end
                5'd16 : MAC_input_valid <= 1'b0;
                5'd17 : begin
                    exp_res <= E_lnxo;
                    sign_res <= S_lnxo;
                    if((lnxo>=16'ha8c0) && (lnxo <= 16'haaa0))
                        rm_res <= M_lnxo - 16'h0016;
                    else if((lnxo>=16'ha420) && (lnxo <= 16'ha5e0))
                        rm_res <= M_lnxo - 16'h0013;
                    else if((lnxo>=16'h9c00) && (lnxo <= 16'h9d80))
                        rm_res <= M_lnxo - 16'h007e;
                    else if((lnxo <= 16'h9b00) && (lnxo >= 16'h9800))
                        rm_res <= M_lnxo - 16'h00ff;
                    else if((lnxo == 16'h9600) || (lnxo == 16'h9400))
                        rm_res <= M_lnxo - 16'h0200;
                    else
                        rm_res <= M_lnxo;
                end
                5'd18 : begin
                    if(RCU_over && (RCU_opcode_o == 3'd2))
                        idle <= 1'b1;
                    else
                        idle <= 1'b0;
                end
            endcase
        end
    end
    
    always@(posedge clk)    begin
        if(rst) begin
            output_up <= 1'b0;
            data_o <= 16'd0;
            opcode_o <= 3'd0;
        end
        else    begin
            if(RCU_over && ~RCU_opcode_o[2])   begin    //+*/
                output_up <= 1'b1;
                data_o <= RCU_o;
                opcode_o <= RCU_opcode_o;
            end
            else if(state == 9) begin                   //ex or lnx
                output_up <= 1'b1;
                data_o <= {sign_res,exp_res[4:0],rm_res[9:0]};
                opcode_o <= op;
            end
//            else if((count == 2'd1) && RCU_over && (RCU_opcode_o == 3'd5))  begin
//                output_up <= 1'b1;
//                data_o <= RCU_o;
//                opcode_o <= RCU_opcode_o;
//            end
            else    begin
                output_up <= 1'b0;
                data_o <= data_o;
                opcode_o <= opcode_o;
            end
        end
    end
    
    blk_mem_gen_1 table2 (
    .clka(clk),    // input wire clka
    .addra(addra_table2),  // input wire [7 : 0] addra
    .douta(douta_table2)  // output wire [15 : 0] douta
    );
    
    blk_mem_gen_0 table1 (
    .clka(clk),    // input wire clka
    .addra(addra_table1),  // input wire [6 : 0] addra
    .douta(douta_table1)  // output wire [31 : 0] douta
    );
    
    RCU RCU(
    .opcode(MAC_opcode_o),
    .rst(RCU_rst),
    .MAC_over(MAC_over),
    .div_over(D_over),
    .sign_MAC(MAC_sign),
    .exp_MAC(MAC_exp),
    .rm_MAC(MAC_rm),
    .round_MAC(MAC_round),
    .sign_q(D_sign),
    .exp_q(D_exp),
    .rm_q(D_rm),
    .round_q(D_round),
    .clk(clk),
    .data_o(RCU_o),
    .opcode_o(RCU_opcode_o),
    .output_up(RCU_over)
    );
    
    FP16_div DU (
    .data_dividend(D_dividend),
    .data_divisor(D_divisor),
    .input_valid(D_input_valid),
    .clk(clk),
    .rst(D_rst),
    .sign_o(D_sign),
    .exp_o(D_exp),
    .rm_o(D_rm),
    .output_update(D_over),
    .idle(D_idle),
    .round_o(D_round)
    );
    
    FP16_MAC_pipeline MACU (
    .clk(clk),
    .data_k(MAC_k),
    .data_x(MAC_x),
    .data_b(MAC_b),
    .rst(MAC_rst),
    .input_valid(MAC_input_valid),
    .opcode(MAC_opcode),
    .sign_o(MAC_sign),
    .exp_o(MAC_exp),
    .rm_o(MAC_rm),
    .output_up(MAC_over),
    .opcode_o(MAC_opcode_o),
    .round_o(MAC_round)
    );
    
endmodule