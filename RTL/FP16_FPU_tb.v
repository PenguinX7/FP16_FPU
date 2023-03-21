`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2023/03/09 17:20:00
// Design Name: 
// Module Name: FP16_FPU_tb
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


module FP16_FPU_tb(

    );
    reg clk;
    reg [15:0]data1;
    reg [15:0]data2;
    reg [15:0]data3;
    reg rst;
    reg input_valid;
    reg [2:0]opcode;
    wire idle;
    wire [2:0]opcode_o;
    wire [15:0]data_o;
    wire output_up;
    reg [19:0]count;
    reg [31:0]mem_lnx[30719:0];                 //for lnx
    reg [31:0]mem_ex[37478:0];                  //for ex
    reg [47:0]mem[1045505:0];               //for div
    
    always #5 clk = ~clk;
    
    initial begin
//        opcode = 5;
//        opcode = 4;     //ex
//        opcode = 2;     //div
        opcode = 0;         //choose the op to test
        case(opcode)
            5 : $readmemh("test_lnx.txt",mem_lnx);
            4 : $readmemh("test_ex.txt",mem_ex);
            2 : $readmemh("test_div.txt",mem);
            1 : begin
                $readmemh("test_mcl.txt",mem);
            end
            0 : begin
                $readmemh("test_add.txt",mem);
            end
        endcase
        clk = 1;
        rst = 1;
        input_valid = 0;
        count = 0;
        #10 rst = 0;
//          input_valid = 1;
//           data1 = 16'h00be;
//           data2 = 16'hfbb2;
        
    end
    
    always@(posedge clk)    begin
        case(opcode)
            0 : begin
                if(idle && ~output_up)    begin
                    data1 = mem[count][47:32];
                    data2 = mem[count][31:16];
                    count = count + 1;
                    input_valid = 1;
                end
                else if(idle && output_up)   begin
                    data1 = mem[count][47:32];
                    data2 = mem[count][31:16];
                    if(data_o != mem[count - 13][15:0])  begin
                        $display("error,data_1 = %h,data_2=%h,output=%h,should=%h",mem[count - 13][47:32],mem[count - 13][31:16],data_o,mem[count-13][15:0]);
                        $stop;
                    end
                    input_valid = 1;
                    if(count == 1045505)  begin
                        $display("test over\n");
                        $finish;
                    end
                    else
                        count = count + 1;
                end
                else if(input_valid)
                    input_valid = 0;
            end
            1 : begin
                if(idle && ~output_up)    begin
                    data1 = mem[count][47:32];
                    data2 = mem[count][31:16];
                    count = count + 1;
                    input_valid = 1;
                end
                else if(idle && output_up)   begin
                    data1 = mem[count][47:32];
                    data2 = mem[count][31:16];
                    if(data_o != mem[count - 13][15:0])  begin
                        $display("error,data_1 = %h,data_2=%h,output=%h,should=%h",mem[count - 13][47:32],mem[count - 13][31:16],data_o,mem[count-13][15:0]);
                        $stop;
                    end
                    input_valid = 1;
                    if(count == 1045505)  begin
                        $display("test over\n");
                        $finish;
                    end
                    else
                        count = count + 1;
                end
                else if(input_valid)
                    input_valid = 0;
            end
            2 : begin
                if(idle && ~output_up)    begin
                    data1 = mem[count][47:32];
                    data2 = mem[count][31:16];
                    count = count + 1;
                    input_valid = 1;
                end
                else if(idle && output_up)   begin
                    data1 = mem[count][47:32];
                    data2 = mem[count][31:16];
                    if(data_o != mem[count - 2][15:0])
                        $display("error,data_x = %h,data_d=%h,output=%h,should=%h",mem[count - 1][47:32],mem[count - 1][31:16],data_o,mem[count-2][15:0]);
                    input_valid = 1;
                    if(count == 1045505)  begin
                        $display("test over\n");
                        $finish;
                    end
                    else
                        count = count + 1;
                end
                else if(input_valid)
                    input_valid = 0;
            end
            4 : begin
                if(idle && ~output_up)    begin
                    data1 = mem_ex[count][31:16];
                    count = count + 1;
                    input_valid = 1;
                end
                else if(idle && output_up)   begin
                    data1 = mem_ex[count][31:16];
                    if(data_o != mem_ex[count - 1][15:0])
                        $display("error,count = %d,data_i = %h\n,output = %h",count - 1,mem_ex[count - 1][31:16],data_o);
                    input_valid = 1;
                    if(count == 37478)  begin
                        $display("test over\n");
                        $finish;
                    end
                    else
                        count = count + 1;
                end
                else if(input_valid)
                    input_valid = 0;
            end
            5 : begin
                if(idle && ~output_up)    begin
                    data1 = mem_lnx[count][31:16];
                    count = count + 1;
                    input_valid = 1;
                end
                else if(idle && output_up)   begin
                    data1 = mem_lnx[count][31:16];
                    if(data_o != mem_lnx[count - 1][15:0])
                        $display("error,count = %d,data_i = %h\n,output = %h",count - 1,mem_lnx[count - 1][31:16],data_o);
                    input_valid = 1;
                    if(count == 30719)  begin
                        $display("test over\n");
                        $finish;
                    end
                    else
                        count = count + 1;
                end
                else if(input_valid)
                    input_valid = 0;
            end
        endcase
    end
    
    
    FP16_FPU DUV (
    .clk(clk),
    .data1(data1),
    .data2(data2),
    .data3(data3),
    .rst(rst),
    .input_valid(input_valid),
    .opcode(opcode),
    .idle(idle),
    .opcode_o(opcode_o),
    .data_o(data_o),
    .output_up(output_up)
    );
endmodule
