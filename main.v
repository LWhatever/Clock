//Anthor:LWhatever_WHU
//FPGA CLOCK
module Display(CLOCK, set_s, set_m, set_h, rst, S0, S1, M0, M1, H0, H1, S0_ld, S1_ld, M0_ld, M1_ld, H0_ld, H1_ld);
	input CLOCK, set_s, set_m, set_h, rst;
	input [3:0] S0_ld, S1_ld, M0_ld, M1_ld, H0_ld, H1_ld;
	output [3:0] S0, S1, M0, M1, H0, H1;
	wire clk, done_s, done_m;
	reg Done_S, Done_M;
	always@(*) begin
		Done_S <= done_s;
		Done_M <= done_m;
	end
	div_clk(CLOCK,clk);
	Counter1 cnt_s(clk, rst, set_s, S0_ld, S1_ld, S0, S1, done_s);
	Counter1 cnt_m(Done_S, rst, set_m, M0_ld, M1_ld, M0, M1, done_m);
	Counter2 cnt_h(Done_M, rst, set_h, H0_ld, H1_ld, H0, H1);
endmodule

module Counter1(clk, rst, ld, d0, d1, c0, c1, done);							//0~60 counter
	input rst, clk, ld; 												// reset, clock and load
	input [3:0] d0, d1;
	output [3:0] c0, c1;
	output done;
	wire [3:0] next0 = (c0 == 4'b1001)? 4'b0000 : c0 + 4'b0001 ;	//low bit
	wire [3:0] next1 = ((c0 == 4'b1001)&(c1 == 4'b0101))? 4'b0000 : (c0 == 4'b1001)? c1 + 4'b0001 : c1 ;//hight bit
	//wire [3:0] next0 = rst? 0 : (c0 == max0)? 4'b0000 : c0 + 4'b0001 ;	//low bit
	//wire [3:0] next1 = rst? 0 : ((c0 == max0)&(c1 == max1))? 4'b0000 : (c0 == max0)? c1 + 4'b0001 : c1 ;//hight bit
	assign done = (c0 == 4'b1001) & (c1 == 4'b0101);
	D_FF #(4) count0(rst, clk, ld, d0, next0, c0) ;
	D_FF #(4) count1(rst, clk, ld, d1, next1, c1) ;
endmodule

module Counter2(clk, rst, ld, d0, d1, c0, c1);
	input rst, clk, ld; 												// reset, clock and load
	input [3:0] d0, d1;
	output [3:0] c0, c1;
	wire [3:0] next0 = ((c1 == 4'b0010)&(c0 == 4'b0011)|(c0 == 4'b1001))?4'b0000:c0 + 4'b0001;
	wire [3:0] next1 = ((c1 == 4'b0010)&(c0 == 4'b0011))?4'b0000:(c0 == 4'b1001)? c1 + 4'b0001 : c1 ;
	D_FF #(4) count0(rst, clk, ld, d0, next0, c0) ;
	D_FF #(4) count1(rst, clk, ld, d1, next1, c1) ;
endmodule

module D_FF(rst, clk, ld, d, in, out);
	parameter n = 1;
	input clk, rst, ld;
	input [n-1:0] in, d;
	output [n-1:0] out;
	reg [n-1:0] out;
	always @(negedge clk or posedge rst or posedge ld)begin
		out = rst? 0 :ld? d : in;
	end
endmodule

module div_clk(CLOCK,clk);
	input CLOCK;//50000
	output clk;
	reg clk;
	reg [15:0] Count;
	always@(posedge CLOCK)begin
		Count <= Count + 1;
		if(Count == 16'd250)
			clk <= 1;
		else if(Count == 16'd500)begin
			clk <= 0;
			Count <= 0;
		end	
	end
endmodule
