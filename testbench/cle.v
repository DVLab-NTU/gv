`timescale 1ns/10ps

module cle(
    input             clk,
    input             reset,
    input      [7:0]  rom_q,
    input      [7:0]  mem_out,
    output reg [6:0]  rom_a,
    output reg [9:0]  sram_a,
    output reg [7:0]  sram_d,
    output reg        sram_wen,
    output reg        finish,
    output reg        mem_wen,
    output reg [7:0]  mem_addr,
    output reg [7:0]  mem_in
);

// ---------------------------------------------------------------------------
// Parameter Definition
// ---------------------------------------------------------------------------

localparam IDLE                 = 6'd0;
localparam FILL_ZERO            = 6'd1;
localparam NEW_LABEL_INIT       = 6'd2;
localparam NEW_LABEL_SCAN       = 6'd3;
localparam NEW_LABEL_ADDR       = 6'd4;
localparam NEW_LABEL_READ       = 6'd5;
localparam NEW_LABEL_GEN        = 6'd6;
localparam NEW_LABEL_WRITE      = 6'd7;
localparam LABELED_INIT         = 6'd8;
localparam LABELED_SCAN         = 6'd9;
localparam LABELED_ADDR         = 6'd10;
localparam LABELED_READ         = 6'd11;
localparam CHECK_LEFTUP_ADDR    = 6'd12;
localparam CHECK_LEFTUP_READ    = 6'd13;
localparam CHECK_LEFTUP_WRITE   = 6'd14;
localparam CHECK_UP_ADDR        = 6'd15;
localparam CHECK_UP_READ        = 6'd16;
localparam CHECK_UP_WRITE       = 6'd17;
localparam CHECK_RIGHTUP_ADDR   = 6'd18;
localparam CHECK_RIGHTUP_READ   = 6'd19;
localparam CHECK_RIGHTUP_WRITE  = 6'd20;
localparam CHECK_LEFT_ADDR      = 6'd21;
localparam CHECK_LEFT_READ      = 6'd22;
localparam CHECK_LEFT_WRITE     = 6'd23;
localparam CHECK_RIGHT_ADDR     = 6'd24;
localparam CHECK_RIGHT_READ     = 6'd25;
localparam CHECK_RIGHT_WRITE    = 6'd26;
localparam CHECK_LEFTDOWN_ADDR  = 6'd27;
localparam CHECK_LEFTDOWN_READ  = 6'd28;
localparam CHECK_LEFTDOWN_WRITE = 6'd29;
localparam CHECK_DOWN_ADDR      = 6'd30;
localparam CHECK_DOWN_READ      = 6'd31;
localparam CHECK_DOWN_WRITE     = 6'd32;
localparam CHECK_RIGHTDOWN_ADDR = 6'd33;
localparam CHECK_RIGHTDOWN_READ = 6'd34;
localparam CHECK_RIGHTDOWN_WRITE= 6'd35;
localparam GAME_OVER            = 6'd36;

// ---------------------------------------------------------------------------
// Wires, Registers and Module
// ---------------------------------------------------------------------------

// Wires
//reg        mem_wen;
//reg  [7:0] mem_addr;
//reg  [7:0] mem_in;
//wire [7:0] mem_out;
reg  [5:0] next_state;
reg        rom_pixel;
reg        mem_pixel;
wire [6:0] pixel_addr;
reg  [9:0] index_offset;
wire [9:0] pixel_index_offset;
reg  [9:0] pixel_index_w;
reg  [7:0] mem_in_labeled;

// Registers
(* fsm_encoding = "auto" *) reg [5:0] curr_state;
reg [9:0] pixel_index;
reg [4:0] label_index;

// Module
/*
sram_256x8 sram (
    .CLK(clk),
    .CEN(1'b0),
    .WEN(mem_wen),
    .A(mem_addr),
    .D(mem_in),
    .Q(mem_out)
);
*/

/*
reg init = 1;
always @(posedge clk) begin
    if (init)
        assume (reset);
    init <= 0;
end
*/


always begin
    assert ( curr_state != GAME_OVER || finish == 1 );
end

// ---------------------------------------------------------------------------
// Combinational Blocks
// ---------------------------------------------------------------------------

assign pixel_index_offset = pixel_index + index_offset;
assign pixel_addr = pixel_index_offset[9:3];

always@(*) begin
    case(pixel_index_offset[2:0])
    3'd7 : mem_in_labeled = {mem_out[7:1], 1'b1};
    3'd6 : mem_in_labeled = {mem_out[7:2], 1'b1, mem_out[0:0]};
    3'd5 : mem_in_labeled = {mem_out[7:3], 1'b1, mem_out[1:0]};
    3'd4 : mem_in_labeled = {mem_out[7:4], 1'b1, mem_out[2:0]};
    3'd3 : mem_in_labeled = {mem_out[7:5], 1'b1, mem_out[3:0]};
    3'd2 : mem_in_labeled = {mem_out[7:6], 1'b1, mem_out[4:0]};
    3'd1 : mem_in_labeled = {mem_out[7:7], 1'b1, mem_out[5:0]};
    3'd0 : mem_in_labeled = {              1'b1, mem_out[6:0]};
    endcase
end

always@(*) begin
    case(pixel_index_offset[2:0])
        3'd7 : begin
            rom_pixel = rom_q[0];
            mem_pixel = mem_out[0];
        end
        3'd6 : begin
            rom_pixel = rom_q[1];
            mem_pixel = mem_out[1];
        end
        3'd5 : begin
            rom_pixel = rom_q[2];
            mem_pixel = mem_out[2];
        end
        3'd4 : begin
            rom_pixel = rom_q[3];
            mem_pixel = mem_out[3];
        end
        3'd3 : begin
            rom_pixel = rom_q[4];
            mem_pixel = mem_out[4];
        end
        3'd2 : begin
            rom_pixel = rom_q[5];
            mem_pixel = mem_out[5];
        end
        3'd1 : begin
            rom_pixel = rom_q[6];
            mem_pixel = mem_out[6];
        end
        3'd0 : begin
            rom_pixel = rom_q[7];
            mem_pixel = mem_out[7];
        end
    endcase
end

// FSM next state logic
always@(*) begin
    case(curr_state)
        IDLE            :begin
            next_state = FILL_ZERO;
        end
        FILL_ZERO       :begin
            if( pixel_index != 127) begin
                next_state = FILL_ZERO;
            end
            else begin
                next_state = NEW_LABEL_INIT;
            end
        end
        NEW_LABEL_INIT  :begin
            next_state = NEW_LABEL_ADDR;
        end
        NEW_LABEL_SCAN  :begin
            if( pixel_index == 127) begin
                next_state = GAME_OVER;
            end
            else begin
                next_state = NEW_LABEL_ADDR;
            end
        end
        NEW_LABEL_ADDR  :begin
            next_state = NEW_LABEL_READ;
        end
        NEW_LABEL_READ  :begin
            if( rom_pixel == 1'b1 && mem_pixel == 1'b0) begin
                next_state = NEW_LABEL_GEN;
            end
            else begin
                next_state = NEW_LABEL_SCAN;
            end
        end
        NEW_LABEL_GEN :begin
            next_state = NEW_LABEL_WRITE;
        end
        NEW_LABEL_WRITE :begin
            next_state = LABELED_INIT;
        end
        LABELED_INIT :begin
            next_state = LABELED_ADDR;
        end
        LABELED_SCAN :begin
            if( pixel_index == 10'd127 ) begin
                next_state = NEW_LABEL_INIT;
            end
            else begin
                next_state = LABELED_ADDR;
            end
        end
        LABELED_ADDR :begin
            next_state = LABELED_READ;
        end
        LABELED_READ :begin
            if( rom_pixel == 1'b1 && mem_pixel == 1'b1) begin
                next_state = (pixel_index[4:0] != 5'd0 && pixel_index[9:5] != 5'd0)?CHECK_LEFTUP_ADDR:
                                                                                    CHECK_RIGHT_ADDR;
            end
            else begin
                next_state = LABELED_SCAN;
            end
        end

        CHECK_LEFTUP_ADDR :begin
            next_state = CHECK_LEFTUP_READ;
        end
        CHECK_LEFTUP_READ :begin
            if( rom_pixel == 1'b1 && mem_pixel == 1'b0) begin
                next_state = CHECK_LEFTUP_WRITE;
            end
            else begin
                next_state = CHECK_UP_ADDR;
            end
        end
        CHECK_LEFTUP_WRITE :begin
            next_state = LABELED_INIT;
        end

        CHECK_UP_ADDR :begin
            next_state = CHECK_UP_READ;
        end
        CHECK_UP_READ :begin
            if( rom_pixel == 1'b1 && mem_pixel == 1'b0) begin
                next_state = CHECK_UP_WRITE;
            end
            else begin
            next_state = CHECK_UP_ADDR;
                next_state = (pixel_index[4:0] != 5'd31)?CHECK_RIGHTUP_ADDR:
                                                         CHECK_LEFT_ADDR;
            end
        end
        CHECK_UP_WRITE :begin
            next_state = LABELED_INIT;
        end
        CHECK_RIGHTUP_ADDR :begin
            next_state = CHECK_RIGHTUP_READ;
        end
        CHECK_RIGHTUP_READ :begin
            if( rom_pixel == 1'b1 && mem_pixel == 1'b0) begin
                next_state = CHECK_RIGHTUP_WRITE;
            end
            else begin
                next_state = (pixel_index[4:0] != 5'd0 )?CHECK_LEFT_ADDR:
                                                         CHECK_RIGHT_ADDR;
            end
        end
        CHECK_RIGHTUP_WRITE :begin
            next_state = LABELED_INIT;
        end
        CHECK_LEFT_ADDR :begin
            next_state = CHECK_LEFT_READ;
        end
        CHECK_LEFT_READ :begin
            if( rom_pixel == 1'b1 && mem_pixel == 1'b0) begin
                next_state = CHECK_LEFT_WRITE;
            end
            else begin
                next_state = (pixel_index[4:0] != 5'd31)?CHECK_RIGHT_ADDR:
                             (pixel_index[9:5] != 5'd31)?CHECK_LEFTDOWN_ADDR:
                                                         LABELED_SCAN;
            end
        end
        CHECK_LEFT_WRITE :begin
            next_state = LABELED_INIT;
        end
        CHECK_RIGHT_ADDR :begin
            next_state = CHECK_RIGHT_READ;
        end
        CHECK_RIGHT_READ :begin
            if( rom_pixel == 1'b1 && mem_pixel == 1'b0) begin
                next_state = CHECK_RIGHT_WRITE;
            end
            else begin
                next_state = (pixel_index[9:5] == 5'd31)?LABELED_SCAN:
                             (pixel_index[4:0] != 5'd0 )?CHECK_LEFTDOWN_ADDR:
                                                         CHECK_DOWN_ADDR;
            end
        end
        CHECK_RIGHT_WRITE :begin
            next_state = LABELED_INIT;
        end
        CHECK_LEFTDOWN_ADDR :begin
            next_state = CHECK_LEFTDOWN_READ;
        end
        CHECK_LEFTDOWN_READ :begin
            if( rom_pixel == 1'b1 && mem_pixel == 1'b0) begin
                next_state = CHECK_LEFTDOWN_WRITE;
            end
            else begin
                next_state = CHECK_DOWN_ADDR;
            end
        end
        CHECK_LEFTDOWN_WRITE :begin
            next_state = LABELED_INIT;
        end
        CHECK_DOWN_ADDR :begin
            next_state = CHECK_DOWN_READ;
        end
        CHECK_DOWN_READ :begin
            if( rom_pixel == 1'b1 && mem_pixel == 1'b0) begin
                next_state = CHECK_DOWN_WRITE;
            end
            else begin
                next_state = (pixel_index[4:0] != 5'd31)?CHECK_RIGHTDOWN_ADDR:
                                                         LABELED_SCAN;
            end
        end
        CHECK_DOWN_WRITE :begin
            next_state = LABELED_INIT;
        end
        CHECK_RIGHTDOWN_ADDR :begin
            next_state = CHECK_RIGHTDOWN_READ;
        end
        CHECK_RIGHTDOWN_READ :begin
            if( rom_pixel == 1'b1 && mem_pixel == 1'b0) begin
                next_state = CHECK_RIGHTDOWN_WRITE;
            end
            else begin
                next_state = LABELED_SCAN;
            end
        end
        CHECK_RIGHTDOWN_WRITE :begin
            next_state = LABELED_INIT;
        end
        GAME_OVER  :begin
            next_state = GAME_OVER;
        end
        default         :begin
            next_state = IDLE;
        end
    endcase
end

// FSM output logic
always@(*) begin
    case(curr_state)
        IDLE            :begin
            pixel_index_w = pixel_index;
            rom_a         = 7'd0;
            sram_a        = 10'd0;
            sram_d        = 8'd0;
            sram_wen      = 1'd1;
            mem_addr      = 8'd0;
            mem_in        = 8'd0;
            mem_wen       = 1'b1;
            index_offset  = 10'd0;

        end
        FILL_ZERO        :begin
            pixel_index_w = pixel_index + 10'd1;
            rom_a         = 7'd0;
            sram_a        = pixel_index;
            sram_d        = 8'd0;
            sram_wen      = 1'd0;
            mem_addr      = pixel_index[9:2];
            mem_in        = 8'd0;
            mem_wen       = 1'b0;
            index_offset  = 10'd0;

        end
        NEW_LABEL_INIT  :begin
            pixel_index_w = 10'd0;
            rom_a         = 7'd0;
            sram_a        = 10'd0;
            sram_d        = 8'd0;
            sram_wen      = 1'b1;
            mem_addr      = 8'd0;
            mem_in        = 8'd0;
            mem_wen       = 1'b1;
            index_offset  = 10'd0;

        end
        NEW_LABEL_SCAN  :begin
            pixel_index_w = pixel_index + 10'd1;
            rom_a         = 7'd0;
            sram_a        = 10'd0;
            sram_d        = 8'd0;
            sram_wen      = 1'b1;
            mem_addr      = 8'd0;
            mem_in        = 8'd0;
            mem_wen       = 1'b1;
            index_offset  = 10'd0;

        end
        NEW_LABEL_ADDR  :begin
            pixel_index_w = pixel_index;
            rom_a         = pixel_addr;
            sram_a        = 10'd0;
            sram_d        = 8'd0;
            sram_wen      = 1'd1;
            mem_addr      = {1'b0, pixel_addr};
            mem_in        = 8'd0;
            mem_wen       = 1'b1;
            index_offset  = 10'd0;

        end
        NEW_LABEL_READ  :begin
            pixel_index_w = pixel_index;
            rom_a         = 7'd0;
            sram_a        = 10'd0;
            sram_d        = 8'd0;
            sram_wen      = 1'd1;
            mem_addr      = 8'd0;
            mem_in        = 8'd0;
            mem_wen       = 1'b1;
            index_offset  = 10'd0;
            
        end
        GAME_OVER       :begin
            pixel_index_w = 10'd0;
            rom_a         = 7'd0;
            sram_a        = 10'd0;
            sram_d        = 8'd0;
            sram_wen      = 1'd1;
            mem_addr      = 8'd0;
            mem_in        = 8'd0;
            mem_wen       = 1'b1;
            index_offset  = 10'd0;
        end
        NEW_LABEL_GEN  :begin
            pixel_index_w = pixel_index;
            rom_a         = 7'd0;
            sram_a        = 10'd0;
            sram_d        = 8'd0;
            sram_wen      = 1'd1;
            mem_addr      = {1'b0, pixel_addr};
            mem_in        = 8'd0;
            mem_wen       = 1'b1;
            index_offset  = 10'd0;
        end
        NEW_LABEL_WRITE  :begin
            pixel_index_w = pixel_index;
            rom_a         = 7'd0;
            sram_a        = pixel_index;
            sram_d        = {3'b000, label_index};
            sram_wen      = 1'd0;
            mem_addr      = {1'b0, pixel_addr};
            mem_in        = mem_in_labeled;
            mem_wen       = 1'b0;
            index_offset  = 10'd0;
        end
        LABELED_INIT  :begin
            pixel_index_w = 10'd0;
            rom_a         = 7'd0;
            sram_a        = 10'd0;
            sram_d        = 8'd0;
            sram_wen      = 1'd1;
            mem_addr      = 8'd0;
            mem_in        = 8'd0;
            mem_wen       = 1'b1;
            index_offset  = 10'd0;
        end
        LABELED_SCAN  :begin
            pixel_index_w = pixel_index + 10'd1;
            rom_a         = 7'd0;
            sram_a        = 10'd0;
            sram_d        = 8'd0;
            sram_wen      = 1'd1;
            mem_addr      = 8'd0;
            mem_in        = 8'd0;
            mem_wen       = 1'b1;
            index_offset  = 10'd0;
        end
        LABELED_ADDR  :begin
            pixel_index_w = pixel_index;
            rom_a         = pixel_addr;
            sram_a        = 10'd0;
            sram_d        = 8'd0;
            sram_wen      = 1'd1;
            mem_addr      = {1'b0, pixel_addr};
            mem_in        = 8'd0;
            mem_wen       = 1'b1;
            index_offset  = 10'd0;
        end
        LABELED_READ  :begin
            pixel_index_w = pixel_index;
            rom_a         = 7'd0;
            sram_a        = 10'd0;
            sram_d        = 8'd0;
            sram_wen      = 1'd1;
            mem_addr      = 8'd0;
            mem_in        = 8'd0;
            mem_wen       = 1'b1;
            index_offset  = 10'd0;
        end
        CHECK_LEFTUP_ADDR  :begin
            pixel_index_w = pixel_index;
            rom_a         = pixel_addr;
            sram_a        = 10'd0;
            sram_d        = 8'd0;
            sram_wen      = 1'd1;
            mem_addr      = {1'b0, pixel_addr};
            mem_in        = 8'd0;
            mem_wen       = 1'b1;
            index_offset  = 10'd991;
        end
        CHECK_LEFTUP_READ  :begin
            pixel_index_w = pixel_index;
            rom_a         = pixel_addr;
            sram_a        = 10'd0;
            sram_d        = 8'd0;
            sram_wen      = 1'd1;
            mem_addr      = {1'b0, pixel_addr};
            mem_in        = 8'd0;
            mem_wen       = 1'b1;
            index_offset  = 10'd991;
        end
        CHECK_LEFTUP_WRITE  :begin
            pixel_index_w = pixel_index;
            rom_a         = 7'd0;
            sram_a        = pixel_index+10'd991;
            sram_d        = {3'b0, label_index};
            sram_wen      = 1'd0;
            mem_addr      = {1'b0, pixel_addr};
            mem_in        = mem_in_labeled;
            mem_wen       = 1'b0;
            index_offset  = 10'd991;
        end
        CHECK_UP_ADDR  :begin
            pixel_index_w = pixel_index;
            rom_a         = pixel_addr;
            sram_a        = 10'd0;
            sram_d        = 8'd0;
            sram_wen      = 1'd1;
            mem_addr      = {1'b0, pixel_addr};
            mem_in        = 8'd0;
            mem_wen       = 1'b1;
            index_offset  = 10'd992;
        end
        CHECK_UP_READ  :begin
            pixel_index_w = pixel_index;
            rom_a         = pixel_addr;
            sram_a        = 10'd0;
            sram_d        = 8'd0;
            sram_wen      = 1'd1;
            mem_addr      = {1'b0, pixel_addr};
            mem_in        = 8'd0;
            mem_wen       = 1'b1;
            index_offset  = 10'd992;
        end
        CHECK_UP_WRITE  :begin
            pixel_index_w = pixel_index;
            rom_a         = 7'd0;
            sram_a        = pixel_index+10'd992;
            sram_d        = {3'b000, label_index};
            sram_wen      = 1'd0;
            mem_addr      = {1'b0, pixel_addr};
            mem_in        = mem_in_labeled;
            mem_wen       = 1'b0;
            index_offset  = 10'd992;
        end
        CHECK_RIGHTUP_ADDR  :begin
            pixel_index_w = pixel_index;
            rom_a         = pixel_addr;
            sram_a        = 10'd0;
            sram_d        = 8'd0;
            sram_wen      = 1'd1;
            mem_addr      = {1'b0, pixel_addr};
            mem_in        = 8'd0;
            mem_wen       = 1'b1;
            index_offset  = 10'd993;
        end
        CHECK_RIGHTUP_READ  :begin
            pixel_index_w = pixel_index;
            rom_a         = pixel_addr;
            sram_a        = 10'd0;
            sram_d        = 8'd0;
            sram_wen      = 1'd1;
            mem_addr      = {1'b0, pixel_addr};
            mem_in        = 8'd0;
            mem_wen       = 1'b1;
            index_offset  = 10'd993;
        end
        CHECK_RIGHTUP_WRITE  :begin
            pixel_index_w = pixel_index;
            rom_a         = 7'd0;
            sram_a        = pixel_index+10'd993;
            sram_d        = {3'b000, label_index};
            sram_wen      = 1'd0;
            mem_addr      = {1'b0, pixel_addr};
            mem_in        = mem_in_labeled;
            mem_wen       = 1'b0;
            index_offset  = 10'd993;
        end
        CHECK_LEFT_ADDR  :begin
            pixel_index_w = pixel_index;
            rom_a         = pixel_addr;
            sram_a        = 10'd0;
            sram_d        = 8'd0;
            sram_wen      = 1'd1;
            mem_addr      = {1'b0, pixel_addr};
            mem_in        = 8'd0;
            mem_wen       = 1'b1;
            index_offset  = 10'd127;
        end
        CHECK_LEFT_READ  :begin
            pixel_index_w = pixel_index;
            rom_a         = pixel_addr;
            sram_a        = 10'd0;
            sram_d        = 8'd0;
            sram_wen      = 1'd1;
            mem_addr      = {1'b0, pixel_addr};
            mem_in        = 8'd0;
            mem_wen       = 1'b1;
            index_offset  = 10'd127;
        end
        CHECK_LEFT_WRITE  :begin
            pixel_index_w = pixel_index;
            rom_a         = 7'd0;
            sram_a        = pixel_index+10'd127;
            sram_d        = {3'b000, label_index};
            sram_wen      = 1'd0;
            mem_addr      = {1'b0, pixel_addr};
            mem_in        = mem_in_labeled;
            mem_wen       = 1'b0;
            index_offset  = 10'd127;
        end
        CHECK_RIGHT_ADDR  :begin
            pixel_index_w = pixel_index;
            rom_a         = pixel_addr;
            sram_a        = 10'd0;
            sram_d        = 8'd0;
            sram_wen      = 1'd1;
            mem_addr      = {1'b0, pixel_addr};
            mem_in        = 8'd0;
            mem_wen       = 1'b1;
            index_offset  = 10'd1;
        end
        CHECK_RIGHT_READ  :begin
            pixel_index_w = pixel_index;
            rom_a         = pixel_addr;
            sram_a        = 10'd0;
            sram_d        = 8'd0;
            sram_wen      = 1'd1;
            mem_addr      = {1'b0, pixel_addr};
            mem_in        = 8'd0;
            mem_wen       = 1'b1;
            index_offset  = 10'd1;
        end
        CHECK_RIGHT_WRITE  :begin
            pixel_index_w = pixel_index;
            rom_a         = 7'd0;
            sram_a        = pixel_index+10'd1;
            sram_d        = {3'b000, label_index};
            sram_wen      = 1'd0;
            mem_addr      = {1'b0, pixel_addr};
            mem_in        = mem_in_labeled;
            mem_wen       = 1'b0;
            index_offset  = 10'd1;
        end
        CHECK_LEFTDOWN_ADDR  :begin
            pixel_index_w = pixel_index;
            rom_a         = pixel_addr;
            sram_a        = 10'd0;
            sram_d        = 8'd0;
            sram_wen      = 1'd1;
            mem_addr      = {1'b0, pixel_addr};
            mem_in        = 8'd0;
            mem_wen       = 1'b1;
            index_offset  = 10'd31;
        end
        CHECK_LEFTDOWN_READ  :begin
            pixel_index_w = pixel_index;
            rom_a         = pixel_addr;
            sram_a        = 10'd0;
            sram_d        = 8'd0;
            sram_wen      = 1'd1;
            mem_addr      = {1'b0, pixel_addr};
            mem_in        = 8'd0;
            mem_wen       = 1'b1;
            index_offset  = 10'd31;            
        end
        CHECK_LEFTDOWN_WRITE  :begin
            pixel_index_w = pixel_index;
            rom_a         = 7'd0;
            sram_a        = pixel_index + 10'd31;
            sram_d        = {3'b000, label_index};
            sram_wen      = 1'd0;
            mem_addr      = {1'b0, pixel_addr};
            mem_in        = mem_in_labeled;
            mem_wen       = 1'b0;
            index_offset  = 10'd31;
        end
        CHECK_DOWN_ADDR  :begin
            pixel_index_w = pixel_index;
            rom_a         = pixel_addr;
            sram_a        = 10'd0;
            sram_d        = 8'd0;
            sram_wen      = 1'd1;
            mem_addr      = {1'b0, pixel_addr};
            mem_in        = 8'd0;
            mem_wen       = 1'b1;
            index_offset  = 10'd32; 
        end
        CHECK_DOWN_READ  :begin
            pixel_index_w = pixel_index;
            rom_a         = pixel_addr;
            sram_a        = 10'd0;
            sram_d        = 8'd0;
            sram_wen      = 1'd1;
            mem_addr      = {1'b0, pixel_addr};
            mem_in        = 8'd0;
            mem_wen       = 1'b1;
            index_offset  = 10'd32; 
        end
        CHECK_DOWN_WRITE  :begin
            pixel_index_w = pixel_index;
            rom_a         = 7'd0;
            sram_a        = pixel_index + 10'd32;
            sram_d        = {3'b000, label_index};
            sram_wen      = 1'd0;
            mem_addr      = {1'b0, pixel_addr};
            mem_in        = mem_in_labeled;
            mem_wen       = 1'b0;
            index_offset  = 10'd32;
        end
        CHECK_RIGHTDOWN_ADDR  :begin
            pixel_index_w = pixel_index;
            rom_a         = pixel_addr;
            sram_a        = 10'd0;
            sram_d        = 8'd0;
            sram_wen      = 1'd1;
            mem_addr      = {1'b0, pixel_addr};
            mem_in        = 8'd0;
            mem_wen       = 1'b1;
            index_offset  = 10'd33; 
        end
        CHECK_RIGHTDOWN_READ  :begin
            pixel_index_w = pixel_index;
            rom_a         = pixel_addr;
            sram_a        = 10'd0;
            sram_d        = 8'd0;
            sram_wen      = 1'd1;
            mem_addr      = {1'b0, pixel_addr};
            mem_in        = 8'd0;
            mem_wen       = 1'b1;
            index_offset  = 10'd33; 
        end
        CHECK_RIGHTDOWN_WRITE  :begin
            pixel_index_w = pixel_index;
            rom_a         = 7'd0;
            sram_a        = pixel_index + 10'd33;
            sram_d        = {3'b000, label_index};
            sram_wen      = 1'd0;
            mem_addr      = {1'b0, pixel_addr};
            mem_in        = mem_in_labeled;
            mem_wen       = 1'b0;
            index_offset  = 10'd33;
        end
        default         :begin
            pixel_index_w = 10'd0;
            rom_a         = 7'd0;
            sram_a        = 10'd0;
            sram_d        = 8'd0;
            sram_wen      = 1'd1;
            mem_addr      = 8'd0;
            mem_in        = 8'd0;
            mem_wen       = 1'b1;
            index_offset  = 10'd0;
        end
    endcase
end

// ---------------------------------------------------------------------------
// Sequential Block
// ---------------------------------------------------------------------------

// FSM state register
always@(posedge clk or posedge reset) begin
    if(reset)
        curr_state      <= IDLE;
    else 
        curr_state      <= next_state;
end

// count register
always@(posedge clk or posedge reset) begin
    if(reset) begin
        pixel_index   <= 10'd0;
    end
    else begin
        pixel_index   <= pixel_index_w;
    end
end

// index register
always@(posedge clk or posedge reset) begin
    if(reset) begin
        label_index   <= 5'd0;
    end
    else if(curr_state == NEW_LABEL_GEN) begin
        label_index   <= label_index + 5'd1;
    end
end

// finish register
always@(posedge clk or posedge reset) begin
    if(reset) begin
        finish   <= 1'd0;
    end
    else if(curr_state == GAME_OVER)begin
        finish   <= 1'd1;
    end
end

endmodule
