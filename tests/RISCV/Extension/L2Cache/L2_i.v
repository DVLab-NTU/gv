module L2icache(
    clk,
    rst_n,
    cache_read,
    cache_write,
    cache_addr,
    cache_rdata,
    cache_wdata,
    L2_ready,
    mem_read,
    mem_write,
    mem_addr,
    mem_rdata,
    mem_wdata,
    mem_ready
);
    parameter L1_WIDTH      = 128;
    parameter L1_ADDR_SIZE  = 28;
    parameter L2_BLOCK      = 16;
    parameter L2_WIDTH      = 128;
    parameter L2_TAG        = 24;
    parameter MEM_ADDR_SIZE = 28;
    parameter ASSO          = 2;
    
//==== input/output definition ============================
    input                       clk;
    input                       rst_n;
    // cache interface
    input                       cache_read, cache_write;
    input   [L1_ADDR_SIZE-1:0]  cache_addr;
    input   [L1_WIDTH-1:0]      cache_wdata;
    output                      L2_ready;
    output  [L1_WIDTH-1:0]      cache_rdata;
    // memory interface
    input  [L2_WIDTH-1:0]       mem_rdata;
    input                       mem_ready;
    output                      mem_read, mem_write;
    output [MEM_ADDR_SIZE-1:0]  mem_addr;
    output [L2_WIDTH-1:0]       mem_wdata;
    
//==== wire/reg definition ================================
    reg [L2_WIDTH-1:0]      L2_r        [0:L2_BLOCK*ASSO-1];
    reg [L2_WIDTH-1:0]      L2_w        [0:L2_BLOCK*ASSO-1];
    reg [L2_TAG-1:0]        tag_r       [0:L2_BLOCK*ASSO-1];
    reg [L2_TAG-1:0]        tag_w       [0:L2_BLOCK*ASSO-1];
    reg [L2_BLOCK*ASSO-1:0] valid_r;
    reg [L2_BLOCK*ASSO-1:0] valid_w;
    reg [L1_WIDTH-1:0]      cache_rd;
    reg [L2_WIDTH-1:0]      mem_wd;
    reg [MEM_ADDR_SIZE-1:0] mem_add;
    reg                     hit1, hit2;
    reg                     valid1, valid2;
    reg                     ready;
    reg                     mem_r;
    reg                     mem_w;
    reg                     state, nxt_state;
    reg [L2_BLOCK*ASSO-1:0] cnt;
    reg [L2_BLOCK*ASSO-1:0] nxt_cnt;

//==== parameter ==========================================
localparam CompareTag   = 1'd0;
localparam Allocate     = 1'd1;

//==== finite state machine ===============================
always @(posedge clk) begin
    if(~rst_n)
        state <= CompareTag;
    else
        state <= nxt_state;
end
always @(*) begin
    nxt_state = CompareTag;
    case(state)
    CompareTag : begin
        if(~(cache_read)begin
            nxt_state = CompareTag;
        end
        else if((valid1&hit1)|(valid2&hit2))begin
            nxt_state = CompareTag;
        end
        else
            nxt_state = Allocate;
    end
    Allocate : begin
        if(mem_ready)
            nxt_state = CompareTag;
        else
            nxt_state = Allocate;
    end
    default : nxt_state = CompareTag;
    endcase
end

//==== combinational circuit ==============================
assign L2_ready     = ready;
assign cache_rdata  = cache_rd;
assign mem_addr     = mem_add;
assign mem_wdata    = mem_wd;
assign mem_read     = mem_r;
assign mem_write    = mem_w;
integer i;

always @(*) begin
    cache_rd    = 128'd0;
    wdata_out   = 128'd0;
    mem_add     = 28'd0;
    ready       = 1'b0;
    mem_r       = 1'b0;
    mem_w       = 1'b0;
    hit1        = 1'b0;
    valid1      = 1'b0;
    hit2        = 1'b0;
    valid2      = 1'b0;
    stall       = 1'b0;
    for (i = 0; i < L2_BLOCK*ASSO; i = i + 1)begin
        L2_w[i]     = L2_r[i];
        valid_w[i]  = valid_r[i];
        tag_w[i]    = tag_r[i];
        nxt_cnt[i]  = cnt[i];
    end
    case(cache_addr[3:0])
        4'd0 : begin
            hit1    = (cache_addr[27:4] == tag_r[0]);
            valid1  = valid_r[0];
            hit2    = (cache_addr[27:4] == tag_r[1]);
            valid2  = valid_r[1];
        end
        4'd1 : begin
            hit1    = (cache_addr[27:4] == tag_r[2]);
            valid1  = valid_r[2];
            hit2    = (cache_addr[27:4] == tag_r[3]);
            valid2  = valid_r[3];
        end
        4'd2 : begin
            hit1    = (cache_addr[27:4] == tag_r[4]);
            valid1  = valid_r[4];
            hit2    = (cache_addr[27:4] == tag_r[5]);
            valid2  = valid_r[5];
        end
        4'd3 : begin
            hit1    = (cache_addr[27:4] == tag_r[6]);
            valid1  = valid_r[6];
            hit2    = (cache_addr[27:4] == tag_r[7]);
            valid2  = valid_r[7];
        end
        4'd4 : begin
            hit1    = (cache_addr[27:4] == tag_r[8]);
            valid1  = valid_r[8];
            hit2    = (cache_addr[27:4] == tag_r[9]);
            valid2  = valid_r[9];
        end
        4'd5 : begin
            hit1    = (cache_addr[27:4] == tag_r[10]);
            valid1  = valid_r[10];
            hit2    = (cache_addr[27:4] == tag_r[11]);
            valid2  = valid_r[11];
        end
        4'd6 : begin
            hit1    = (cache_addr[27:4] == tag_r[12]);
            valid1  = valid_r[12];
            hit2    = (cache_addr[27:4] == tag_r[13]);
            valid2  = valid_r[13];
        end
        4'd7 : begin
            hit1    = (cache_addr[27:4] == tag_r[14]);
            valid1  = valid_r[14];
            hit2    = (cache_addr[27:4] == tag_r[15]);
            valid2  = valid_r[15];
        end
        4'd8 : begin
            hit1    = (cache_addr[27:4] == tag_r[16]);
            valid1  = valid_r[16];
            hit2    = (cache_addr[27:4] == tag_r[17]);
            valid2  = valid_r[17];
        end
        4'd9 : begin
            hit1    = (cache_addr[27:4] == tag_r[18]);
            valid1  = valid_r[18];
            hit2    = (cache_addr[27:4] == tag_r[19]);
            valid2  = valid_r[19];
        end
        4'd10 : begin
            hit1    = (cache_addr[27:4] == tag_r[20]);
            valid1  = valid_r[20];
            hit2    = (cache_addr[27:4] == tag_r[21]);
            valid2  = valid_r[21];
        end
        4'd11 : begin
            hit1    = (cache_addr[27:4] == tag_r[22]);
            valid1  = valid_r[22];
            hit2    = (cache_addr[27:4] == tag_r[23]);
            valid2  = valid_r[23];
        end
        4'd12 : begin
            hit1    = (cache_addr[27:4] == tag_r[24]);
            valid1  = valid_r[24];
            hit2    = (cache_addr[27:4] == tag_r[25]);
            valid2  = valid_r[25];
        end
        4'd13 : begin
            hit1    = (cache_addr[27:4] == tag_r[26]);
            valid1  = valid_r[26];
            hit2    = (cache_addr[27:4] == tag_r[27]);
            valid2  = valid_r[27];
        end
        4'd14 : begin
            hit1    = (cache_addr[27:4] == tag_r[28]);
            valid1  = valid_r[28];
            hit2    = (cache_addr[27:4] == tag_r[29]);
            valid2  = valid_r[29];
        end
        4'd15 : begin
            hit1    = (cache_addr[27:4] == tag_r[30]);
            valid1  = valid_r[30];
            hit2    = (cache_addr[27:4] == tag_r[31]);
            valid2  = valid_r[31];
        end
        default : begin
            hit1    = 1'b0;
            valid1  = 1'b0;
            hit2    = 1'b0;
            valid2  = 1'b0;
        end
    endcase
    if(state == CompareTag)begin
    if(valid1&hit1)begin
        case(cache_addr[3:0])
            4'd0 : begin
                nxt_cnt[0] = 1'b0;
                nxt_cnt[1] = 1'b1;
            end
            4'd1 : begin
                nxt_cnt[2] = 1'b0;
                nxt_cnt[3] = 1'b1;
            end
            4'd2 : begin
                nxt_cnt[4] = 1'b0;
                nxt_cnt[5] = 1'b1;
            end
            4'd3 : begin
                nxt_cnt[6] = 1'b0;
                nxt_cnt[7] = 1'b1;
            end
            4'd4 : begin
                nxt_cnt[8] = 1'b0;
                nxt_cnt[9] = 1'b1;
            end
            4'd5 : begin
                nxt_cnt[10] = 1'b0;
                nxt_cnt[11] = 1'b1;
            end
            4'd6 : begin
                nxt_cnt[12] = 1'b0;
                nxt_cnt[13] = 1'b1;
            end
            4'd7 : begin
                nxt_cnt[14] = 1'b0;
                nxt_cnt[15] = 1'b1;
            end
            4'd8 : begin
                nxt_cnt[16] = 1'b0;
                nxt_cnt[17] = 1'b1;
            end
            4'd9 : begin
                nxt_cnt[18] = 1'b0;
                nxt_cnt[19] = 1'b1;
            end
            4'd10 : begin
                nxt_cnt[20] = 1'b0;
                nxt_cnt[21] = 1'b1;
            end
            4'd11 : begin
                nxt_cnt[22] = 1'b0;
                nxt_cnt[23] = 1'b1;
            end
            4'd12 : begin
                nxt_cnt[24] = 1'b0;
                nxt_cnt[25] = 1'b1;
            end
            4'd13 : begin
                nxt_cnt[26] = 1'b0;
                nxt_cnt[27] = 1'b1;
            end
            4'd14 : begin
                nxt_cnt[28] = 1'b0;
                nxt_cnt[29] = 1'b1;
            end
            4'd15 : begin
                nxt_cnt[30] = 1'b0;
                nxt_cnt[31] = 1'b1;
            end
            default : nxt_cnt[0] = cnt[0];
        endcase
        if(cache_read)begin
            ready = 1'b1;
            case(cache_addr[3:0])
                4'd0 : begin
                    cache_rd = cache_r[0];
                end
                4'd1 : begin
                    cache_rd = cache_r[2];
                end
                4'd2 : begin
                    cache_rd = cache_r[4];
                end
                4'd3 : begin
                    cache_rd = cache_r[6];
                end
                4'd4 : begin
                    cache_rd = cache_r[8];
                end
                4'd5 : begin
                    cache_rd = cache_r[10];
                end
                4'd6 : begin
                    cache_rd = cache_r[12];
                end
                4'd7 : begin
                    cache_rd = cache_r[14];
                end
                4'd8 : begin
                    cache_rd = cache_r[16];
                end
                4'd9 : begin
                    cache_rd = cache_r[18];
                end
                4'd10 : begin
                    cache_rd = cache_r[20];
                end
                4'd11 : begin
                    cache_rd = cache_r[22];
                end
                4'd12 : begin
                    cache_rd = cache_r[24];
                end
                4'd13 : begin
                    cache_rd = cache_r[26];
                end
                4'd14 : begin
                    cache_rd = cache_r[28];
                end
                4'd15 : begin
                    cache_rd = cache_r[30];
                end
                default : cache_rd = 128'b0;
            endcase
        end
    end
    else if(valid2&hit2)begin
        case(cache_addr[3:0])
            4'd0 : begin
                nxt_cnt[0] = 1'b1;
                nxt_cnt[1] = 1'b0;
            end
            4'd1 : begin
                nxt_cnt[2] = 1'b1;
                nxt_cnt[3] = 1'b0;
            end
            4'd2 : begin
                nxt_cnt[4] = 1'b1;
                nxt_cnt[5] = 1'b0;
            end
            4'd3 : begin
                nxt_cnt[6] = 1'b1;
                nxt_cnt[7] = 1'b0;
            end
            4'd4 : begin
                nxt_cnt[8] = 1'b1;
                nxt_cnt[9] = 1'b0;
            end
            4'd5 : begin
                nxt_cnt[10] = 1'b1;
                nxt_cnt[11] = 1'b0;
            end
            4'd6 : begin
                nxt_cnt[12] = 1'b1;
                nxt_cnt[13] = 1'b0;
            end
            4'd7 : begin
                nxt_cnt[14] = 1'b1;
                nxt_cnt[15] = 1'b0;
            end
            4'd8 : begin
                nxt_cnt[16] = 1'b1;
                nxt_cnt[17] = 1'b0;
            end
            4'd9 : begin
                nxt_cnt[18] = 1'b1;
                nxt_cnt[19] = 1'b0;
            end
            4'd10 : begin
                nxt_cnt[20] = 1'b1;
                nxt_cnt[21] = 1'b0;
            end
            4'd11 : begin
                nxt_cnt[22] = 1'b1;
                nxt_cnt[23] = 1'b0;
            end
            4'd12 : begin
                nxt_cnt[24] = 1'b1;
                nxt_cnt[25] = 1'b0;
            end
            4'd13 : begin
                nxt_cnt[26] = 1'b1;
                nxt_cnt[27] = 1'b0;
            end
            4'd14 : begin
                nxt_cnt[28] = 1'b1;
                nxt_cnt[29] = 1'b0;
            end
            4'd15 : begin
                nxt_cnt[30] = 1'b1;
                nxt_cnt[31] = 1'b0;
            end
            default : nxt_cnt[0] = cnt[0];
        endcase
        if(cache_read)begin
            ready = 1'b1;
            case(cache_addr[3:0])
                4'd0 : begin
                    cache_rd = cache_r[1];
                end
                4'd1 : begin
                    cache_rd = cache_r[3];
                end
                4'd2 : begin
                    cache_rd = cache_r[5];
                end
                4'd3 : begin
                    cache_rd = cache_r[7];
                end
                4'd4 : begin
                    cache_rd = cache_r[9];
                end
                4'd5 : begin
                    cache_rd = cache_r[11];
                end
                4'd6 : begin
                    cache_rd = cache_r[13];
                end
                4'd7 : begin
                    cache_rd = cache_r[15];
                end
                4'd8 : begin
                    cache_rd = cache_r[17];
                end
                4'd9 : begin
                    cache_rd = cache_r[19];
                end
                4'd10 : begin
                    cache_rd = cache_r[21];
                end
                4'd11 : begin
                    cache_rd = cache_r[23];
                end
                4'd12 : begin
                    cache_rd = cache_r[25];
                end
                4'd13 : begin
                    cache_rd = cache_r[27];
                end
                4'd14 : begin
                    cache_rd = cache_r[29];
                end
                4'd15 : begin
                    cache_rd = cache_r[31];
                end
                default : cache_rd = 128'b0;
            endcase
        end
    end
    else begin
        mem_r = 1'b1;
        mem_w = 1'b0;
        mem_add = cache_addr;
        ready = 1'b0;
    end
    end
    else if(state == Allocate)begin
        ready = 1'b0;
        if(mem_ready)begin
            case(cache_addr[3:0])
            4'd0 : begin
                if(cnt[0])begin
                    cache_w[0] = mem_rdata;
                    tag_w[0] = cache_addr[27:4];
                    valid_w[0] = 1'b1;
                end
                else begin
                    cache_w[1] = mem_rdata;
                    tag_w[1] = cache_addr[27:4];
                    valid_w[1] = 1'b1;
                end
            end
            4'd1 : begin
                if(cnt[2])begin
                    cache_w[2] = mem_rdata;
                    tag_w[2] = cache_addr[27:4];
                    valid_w[2] = 1'b1;
                end
                else begin
                    cache_w[3] = mem_rdata;
                    tag_w[3] = cache_addr[27:4];
                    valid_w[3] = 1'b1;
                end
            end
            4'd2 : begin
                if(cnt[4])begin
                    cache_w[4] = mem_rdata;
                    tag_w[4] = cache_addr[27:4];
                    valid_w[4] = 1'b1;
                end
                else begin
                    cache_w[5] = mem_rdata;
                    tag_w[5] = cache_addr[27:4];
                    valid_w[5] = 1'b1;
                end
            end
            4'd3 : begin
                if(cnt[6])begin
                    cache_w[6] = mem_rdata;
                    tag_w[6] = cache_addr[27:4];
                    valid_w[6] = 1'b1;
                end
                else begin
                    cache_w[7] = mem_rdata;
                    tag_w[7] = cache_addr[27:4];
                    valid_w[7] = 1'b1;
                end
            end
            4'd4 : begin
                if(cnt[8])begin
                    cache_w[8] = mem_rdata;
                    tag_w[8] = cache_addr[27:4];
                    valid_w[8] = 1'b1;
                end
                else begin
                    cache_w[9] = mem_rdata;
                    tag_w[9] = cache_addr[27:4];
                    valid_w[9] = 1'b1;
                end
            end
            4'd5 : begin
                if(cnt[10])begin
                    cache_w[10] = mem_rdata;
                    tag_w[10] = cache_addr[27:4];
                    valid_w[10] = 1'b1;
                end
                else begin
                    cache_w[11] = mem_rdata;
                    tag_w[11] = cache_addr[27:4];
                    valid_w[11] = 1'b1;
                end
            end
            4'd6 : begin
                if(cnt[12])begin
                    cache_w[12] = mem_rdata;
                    tag_w[12] = cache_addr[27:4];
                    valid_w[12] = 1'b1;
                end
                else begin
                    cache_w[13] = mem_rdata;
                    tag_w[13] = cache_addr[27:4];
                    valid_w[13] = 1'b1;
                end
            end
            4'd7 : begin
                if(cnt[14])begin
                    cache_w[14] = mem_rdata;
                    tag_w[14] = cache_addr[27:4];
                    valid_w[14] = 1'b1;
                end
                else begin
                    cache_w[15] = mem_rdata;
                    tag_w[15] = cache_addr[27:4];
                    valid_w[15] = 1'b1;
                end
            end
            4'd8 : begin
                if(cnt[16])begin
                    cache_w[16] = mem_rdata;
                    tag_w[16] = cache_addr[27:4];
                    valid_w[16] = 1'b1;
                end
                else begin
                    cache_w[17] = mem_rdata;
                    tag_w[17] = cache_addr[27:4];
                    valid_w[17] = 1'b1;
                end
            end
            4'd9 : begin
                if(cnt[18])begin
                    cache_w[18] = mem_rdata;
                    tag_w[18] = cache_addr[27:4];
                    valid_w[18] = 1'b1;
                end
                else begin
                    cache_w[19] = mem_rdata;
                    tag_w[19] = cache_addr[27:4];
                    valid_w[19] = 1'b1;
                end
            end
            4'd10 : begin
                if(cnt[20])begin
                    cache_w[20] = mem_rdata;
                    tag_w[20] = cache_addr[27:4];
                    valid_w[20] = 1'b1;
                end
                else begin
                    cache_w[21] = mem_rdata;
                    tag_w[21] = cache_addr[27:4];
                    valid_w[21] = 1'b1;
                end
            end
            4'd11 : begin
                if(cnt[22])begin
                    cache_w[22] = mem_rdata;
                    tag_w[22] = cache_addr[27:4];
                    valid_w[22] = 1'b1;
                end
                else begin
                    cache_w[23] = mem_rdata;
                    tag_w[23] = cache_addr[27:4];
                    valid_w[23] = 1'b1;
                end
            end
            4'd12 : begin
                if(cnt[24])begin
                    cache_w[24] = mem_rdata;
                    tag_w[24] = cache_addr[27:4];
                    valid_w[24] = 1'b1;
                end
                else begin
                    cache_w[25] = mem_rdata;
                    tag_w[25] = cache_addr[27:4];
                    valid_w[25] = 1'b1;
                end
            end
            4'd13 : begin
                if(cnt[26])begin
                    cache_w[26] = mem_rdata;
                    tag_w[26] = cache_addr[27:4];
                    valid_w[26] = 1'b1;
                end
                else begin
                    cache_w[27] = mem_rdata;
                    tag_w[27] = cache_addr[27:4];
                    valid_w[27] = 1'b1;
                end
            end
            4'd14 : begin
                if(cnt[28])begin
                    cache_w[28] = mem_rdata;
                    tag_w[28] = cache_addr[27:4];
                    valid_w[28] = 1'b1;
                end
                else begin
                    cache_w[29] = mem_rdata;
                    tag_w[29] = cache_addr[27:4];
                    valid_w[29] = 1'b1;
                end
            end
            4'd15 : begin
                if(cnt[30])begin
                    cache_w[30] = mem_rdata;
                    tag_w[30] = cache_addr[27:4];
                    valid_w[30] = 1'b1;
                end
                else begin
                    cache_w[31] = mem_rdata;
                    tag_w[31] = cache_addr[27:4];
                    valid_w[31] = 1'b1;
                end
            end
            default : begin
                cache_w[0] = cache_r[0];
                tag_w[0] = tag_r[0];
                valid_w[0] = 1'b0;
            end
            endcase
        end
        else begin
            mem_r = 1'b1;
            mem_w = 1'b0;
            mem_add = cache_addr[29:2];
            ready = 1'b0;
        end
    end
end

//==== sequential circuit =================================
always@( posedge clk ) begin
    for (i = 0; i < L2_BLOCK*ASSO; i = i + 1)begin
        cache_r[i] <= cache_w[i];
        tag_r[i] <= tag_w[i];
    end
    if( ~rst_n ) begin
        for (i = 0; i < L2_BLOCK*ASSO; i = i + 1)begin
            valid_r[i] <= 1'b0;
            cnt[i] <= 1'b0;
        end
    end
    else begin
        for (i = 0; i < L2_BLOCK*ASSO; i = i + 1)begin
            valid_r[i] <= valid_w[i];
            cnt[i] <= nxt_cnt[i];
        end
    end
end

endmodule
