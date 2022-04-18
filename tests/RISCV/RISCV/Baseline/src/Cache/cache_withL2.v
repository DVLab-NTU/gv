module cache(
    clk,
    proc_reset,
    proc_read,
    proc_write,
    proc_addr,
    proc_rdata,
    proc_wdata,
    proc_stall,
    mem_read,
    mem_write,
    mem_addr,
    mem_rdata,
    mem_wdata,
    mem_ready
);
//==== input/output definition ============================
    input          clk;
    // processor interface
    input          proc_reset;
    input          proc_read, proc_write;
    input   [29:0] proc_addr;
    input   [31:0] proc_wdata;
    output         proc_stall;
    output  [31:0] proc_rdata;
    // memory interface
    input  [127:0] mem_rdata;
    input          mem_ready;
    output         mem_read, mem_write;
    output  [27:0] mem_addr;
    output [127:0] mem_wdata;

//==== wire/reg definition ================================
    wire           L1_read, L1_write;
    wire    [27:0] L1_addr;
    wire   [127:0] L1_wdata;
    wire           L1_ready;
    wire   [127:0] L1_rdata;
    wire           L2_read, L2_write;
    wire    [27:0] L2_addr;
    wire   [127:0] L2_wdata;
    wire           L2_ready;
    wire   [127:0] L2_rdata;

//==== combinational circuit ==============================
    L1_cache L1 (   .clk            (clk)           ,
                    .proc_reset     (proc_reset)    ,
                    .proc_read      (proc_read)     ,
                    .proc_write     (proc_write)    ,
                    .proc_addr      (proc_addr)     ,
                    .proc_rdata     (proc_rdata)    ,
                    .proc_wdata     (proc_wdata)    ,
                    .proc_stall     (proc_stall)    ,
                    .mem_read       (L2_read)       ,
                    .mem_write      (L2_write)      ,
                    .mem_addr       (L2_addr)       ,
                    .mem_rdata      (L2_rdata)      ,
                    .mem_wdata      (L2_wdata)      ,
                    .mem_ready      (L2_ready)
                );
    L2_cache L2 (   .clk            (clk)           ,
                    .reset          (proc_reset)    ,
                    .cache_read     (L1_read)     ,
                    .cache_write    (L1_write)    ,
                    .cache_addr     (L1_addr)     ,
                    .cache_rdata    (L1_rdata)    ,
                    .cache_wdata    (L1_wdata)    ,
                    .L2_ready       (L1_ready)    ,
                    .mem_read       (mem_read)       ,
                    .mem_write      (mem_write)      ,
                    .mem_addr       (mem_addr)       ,
                    .mem_rdata      (mem_rdata)      ,
                    .mem_wdata      (mem_wdata)      ,
                    .mem_ready      (mem_ready)
                );
    assign  L1_read     = L2_read;
    assign  L1_write    = L2_write;
    assign  L1_addr     = L2_addr;
    assign  L2_rdata    = L1_rdata;
    assign  L1_wdata    = L2_wdata;
    assign  L2_ready    = L1_ready;

endmodule

module L1_cache(
    clk,
    proc_reset,
    proc_read,
    proc_write,
    proc_addr,
    proc_rdata,
    proc_wdata,
    proc_stall,
    mem_read,
    mem_write,
    mem_addr,
    mem_rdata,
    mem_wdata,
    mem_ready
);
    
//==== input/output definition ============================
    input          clk;
    // processor interface
    input          proc_reset;
    input          proc_read, proc_write;
    input   [29:0] proc_addr;
    input   [31:0] proc_wdata;
    output         proc_stall;
    output  [31:0] proc_rdata;
    // memory interface
    input  [127:0] mem_rdata;
    input          mem_ready;
    output         mem_read, mem_write;
    output  [27:0] mem_addr;
    output [127:0] mem_wdata;
    
//==== paramterers ================================
    parameter blockSize     = 4 * 32;
    parameter tagSize       = 26;
    parameter validSize     = 1;
    parameter set           = 4;
    parameter way           = 2;

    parameter S_IDLE        = 2'd0;
    parameter S_ALLOCATE    = 2'd1;
    parameter S_WRITE       = 2'd2;

//==== wire/reg definition ================================
    
    reg [1:0] state_r, state_w;
    
    reg [blockSize * way - 1 : 0] store_r [0 : set - 1];
    reg [blockSize * way - 1 : 0] store_w [0 : set - 1];
    
    reg [way - 1 : 0] dirty_r [0 : set - 1];
    reg [way - 1 : 0] dirty_w [0 : set - 1];

    reg [way - 1 : 0] valid_r [0 : set - 1];
    reg [way - 1 : 0] valid_w [0 : set - 1];

    reg [way - 1 : 0] used_r [0 : set - 1];
    reg [way - 1 : 0] used_w [0 : set - 1];

    reg [tagSize * way - 1 : 0]  tag_r [0 : set - 1];
    reg [tagSize * way - 1 : 0]  tag_w [0 : set - 1];

    reg         mem_read_r, mem_read_w;
    reg         mem_write_r, mem_write_w;
    reg [27:0]  mem_addr_r, mem_addr_w;
    reg [127:0] mem_wdata_r, mem_wdata_w;
    
    reg [31:0]  proc_rdata_r, proc_rdata_w;
    reg         proc_stall_r, proc_stall_w; 

    integer i;

    // ===== wires =====
    reg [25:0]  tag_field;
    reg [1:0]   block_id;
    reg [1:0]   block_offset;
    reg         hit_index;
    reg         valid;
    reg         hit0, hit1;
    reg         hit;   
    reg         dirty;

    // ===== input buffers ======
    reg mem_ready_buf;

    assign mem_read     = mem_read_w;
    assign mem_write    = mem_write_w;
    assign mem_addr     = mem_addr_w;
    assign mem_wdata    = mem_wdata_w;

    assign proc_stall   = proc_stall_w;
    assign proc_rdata   = proc_rdata_w;  

//==== combinational circuit ==============================
always @(*) begin
    tag_field       = proc_addr[29:4];
    block_id        = proc_addr[3:2];
    block_offset    = proc_addr[1:0];
    hit0            = valid_r[block_id][0] & (tag_r[block_id][tagSize - 1 -: tagSize] == tag_field);
    hit1            = valid_r[block_id][1] & (tag_r[block_id][2 * tagSize - 1 -: tagSize] == tag_field);
    hit             = hit0 | hit1;
    hit_index       = hit0 ? 1'b0 : 1'b1;
    valid           = valid_r[block_id][hit_index];
    dirty           = dirty_r[block_id][hit_index];
    
    proc_rdata_w    = proc_rdata_r;
    for(i = 0; i < set; i = i + 1) begin
        store_w[i] = store_r[i];
        used_w[i] = used_r[i];
    end
        
    case (state_r)
        S_IDLE: begin
            if(proc_read) begin
                if(hit) begin // read hit !!
                    if(hit_index == 1'b0) begin
                        used_w[block_id][0] = 1'b1;
                        used_w[block_id][1] = 1'b0;
                    end
                    else begin
                        used_w[block_id][0] = 1'b0;
                        used_w[block_id][1] = 1'b1;
                    end

                    if(block_offset == 2'd0)
                        proc_rdata_w = store_r[block_id][31 + blockSize * hit_index -: 32];
                    else if(block_offset == 2'd1)
                        proc_rdata_w = store_r[block_id][63 + blockSize * hit_index -: 32];
                    else if(block_offset == 2'd2)
                        proc_rdata_w = store_r[block_id][95 + blockSize * hit_index -: 32];
                    else
                        proc_rdata_w = store_r[block_id][127 + blockSize * hit_index -: 32];
                end
            end
            else if(proc_write) begin
                if(hit) begin // write hit !!
                    if(hit_index == 1'b0) begin
                        used_w[block_id][0]     = 1'b1;
                        used_w[block_id][1]     = 1'b0;
                    end
                    else begin
                        used_w[block_id][0]     = 1'b0;
                        used_w[block_id][1]     = 1'b1;
                    end

                    if(block_offset == 2'd0)
                        store_w[block_id][31 + blockSize * hit_index -: 32] = proc_wdata;
                    else if(block_offset == 2'd1)
                        store_w[block_id][63 + blockSize * hit_index -: 32] = proc_wdata;
                    else if(block_offset == 2'd2)
                        store_w[block_id][95 + blockSize * hit_index -: 32] = proc_wdata;
                    else
                        store_w[block_id][127 + blockSize * hit_index -: 32] = proc_wdata;
                end
            end
        end
        S_ALLOCATE: begin
            if(mem_ready_buf) begin
                if(proc_read) begin // read operation
                    if(used_r[block_id][0]) begin
                        store_w[block_id][2 * blockSize - 1 -: 128]   = mem_rdata;
                        used_w[block_id][0] = 1'b0;
                        used_w[block_id][1] = 1'b1;
                    end
                    else begin
                        store_w[block_id][blockSize - 1 -: 128]   = mem_rdata;
                        used_w[block_id][0] = 1'b1;
                        used_w[block_id][1] = 1'b0;
                    end

                    if(block_offset == 2'd0)
                        proc_rdata_w        = mem_rdata[31:0];
                    else if(block_offset == 2'd1)
                        proc_rdata_w        = mem_rdata[63:32];
                    else if(block_offset == 2'd2)
                        proc_rdata_w        = mem_rdata[95:64];
                    else
                        proc_rdata_w        = mem_rdata[127:96];
                end
                else begin // write operation
                    if(used_r[block_id][0]) begin
                        used_w[block_id][0] = 1'b0;
                        used_w[block_id][1] = 1'b1;
                        if(block_offset == 2'd0) begin
                            store_w[block_id][31 + blockSize -: 32]     = proc_wdata;
                            store_w[block_id][127 + blockSize -: 96]   = mem_rdata[127:32];
                        end
                        else if(block_offset == 2'd1) begin
                            store_w[block_id][63 + blockSize -: 32]    = proc_wdata;
                            store_w[block_id][31 + blockSize -: 32]     = mem_rdata[31:0];
                            store_w[block_id][127 + blockSize -: 64]   = mem_rdata[127:64];
                        end
                        else if(block_offset == 2'd2) begin
                            store_w[block_id][95 + blockSize -: 32]    = proc_wdata;
                            store_w[block_id][63 + blockSize -: 64]     = mem_rdata[63:0];
                            store_w[block_id][127 + blockSize -: 32]   = mem_rdata[127:96];
                        end                        
                        else begin
                            store_w[block_id][127 + blockSize -: 32]   = proc_wdata;
                            store_w[block_id][95 + blockSize -: 96]     = mem_rdata[95:0];
                        end
                    end
                    else begin
                        used_w[block_id][0] = 1'b1;
                        used_w[block_id][1] = 1'b0;
                        if(block_offset == 2'd0) begin
                            store_w[block_id][31:0]     = proc_wdata;
                            store_w[block_id][127:32]   = mem_rdata[127:32];
                        end
                        else if(block_offset == 2'd1) begin
                            store_w[block_id][63:32]    = proc_wdata;
                            store_w[block_id][31:0]     = mem_rdata[31:0];
                            store_w[block_id][127:64]   = mem_rdata[127:64];
                        end
                        else if(block_offset == 2'd2) begin
                            store_w[block_id][95:64]    = proc_wdata;
                            store_w[block_id][63:0]     = mem_rdata[63:0];
                            store_w[block_id][127:96]   = mem_rdata[127:96];
                        end                        
                        else begin
                            store_w[block_id][127:96]   = proc_wdata;
                            store_w[block_id][95:0]     = mem_rdata[95:0];
                        end
                    end
                    
                end
            end
        end
    endcase
end
// ========== valid bit ============ //
always @(*) begin
    for(i = 0; i < set; i = i + 1) begin
        valid_w[i] = valid_r[i];
        dirty_w[i] = dirty_r[i];
        tag_w[i] = tag_r[i];
    end
    case (state_r)
        S_IDLE: begin
            if(proc_write && hit) begin
                dirty_w[block_id][hit_index] = 1'b1;
            end
        end
        S_ALLOCATE: begin
            if(mem_ready_buf) begin
                if(used_r[block_id][0]) begin
                    valid_w[block_id][1]   = 1'b1;
                    tag_w[block_id][2 * tagSize - 1 -: tagSize] = tag_field;
                    if(proc_read)
                        dirty_w[block_id][1]   = 1'b0;
                    else
                        dirty_w[block_id][1]   = 1'b1;    
                end
                else begin
                    valid_w[block_id][0]   = 1'b1;
                    tag_w[block_id][1 * tagSize - 1 -: tagSize] = tag_field;
                    if(proc_read)
                        dirty_w[block_id][0]   = 1'b0;
                    else
                        dirty_w[block_id][0]   = 1'b1;
                end
            end
        end
    endcase
end
// ========== memory logic ============ //
always @(*) begin
    mem_write_w         = mem_write_r;
    mem_wdata_w         = mem_wdata_r;
    mem_read_w          = mem_read_r;
    mem_addr_w          = mem_addr_r;    
    case (state_r)
        S_IDLE: begin
            if((proc_read || proc_write) && !hit) begin
                if(dirty) begin
                    mem_write_w         = 1'b1;
                    mem_read_w          = 1'b0;
                    
                    if(used_r[block_id][0]) begin
                        mem_wdata_w         = store_r[block_id][2 * blockSize - 1 -: 128];
                        mem_addr_w          = {tag_r[block_id][2 * tagSize - 1 -: tagSize], block_id};
                    end
                    else begin
                        mem_wdata_w         = store_r[block_id][blockSize - 1 -: 128];
                        mem_addr_w          = {tag_r[block_id][tagSize - 1 -: tagSize], block_id};
                    end
                end
                else begin
                    mem_write_w         = 1'b0;
                    mem_wdata_w         = mem_wdata_r;
                    mem_read_w          = 1'b1;
                    mem_addr_w          = proc_addr[29:2];    
                end
            end
            else begin
                mem_write_w         = 1'b0;
                mem_wdata_w         = mem_wdata_r;
                mem_read_w          = 1'b0;
                mem_addr_w          = mem_addr_r;
            end
        end
        S_WRITE: begin
            if(mem_ready_buf) begin
                mem_write_w         = 1'b0;
                mem_wdata_w         = mem_wdata_r;
                mem_read_w          = 1'b1;
                mem_addr_w          = proc_addr[29:2];
            end
            else begin
                mem_write_w         = 1'b1;
                mem_wdata_w         = mem_wdata_r;
                mem_read_w          = 1'b0;
                mem_addr_w          = mem_addr_r;
            end
        end
        S_ALLOCATE: begin
            if(mem_ready_buf) begin
                mem_write_w         = 1'b0;
                mem_wdata_w         = mem_wdata_r;
                mem_read_w          = 1'b0;
                mem_addr_w          = mem_addr_r;
            end
            else begin
                mem_write_w         = 1'b0;
                mem_wdata_w         = mem_wdata_r;
                mem_read_w          = 1'b1;
                mem_addr_w          = mem_addr_r;
            end
        end
    endcase
end
// =========== state logic ============= //
always @(*) begin
    state_w = state_r;
    case (state_r)
        S_IDLE: begin
            if(proc_read || proc_write) begin
                if(hit) //  read/write hit !!
                    state_w      = S_IDLE;
                else begin
                    if(dirty)  // block dirty -> write back
                        state_w             = S_WRITE;
                    else                   // block clean -> allocate memory
                        state_w             = S_ALLOCATE;
                end
            end
            else begin
                state_w     = S_IDLE;  
            end
        end
        S_WRITE: begin
            if(mem_ready_buf)
                state_w     = S_ALLOCATE;
            else
                state_w     = S_WRITE;
        end
        S_ALLOCATE: begin
            if(mem_ready_buf)
                state_w     = S_IDLE;
            else
                state_w     = S_ALLOCATE;
        end
    endcase
end
// ========= pcocessor stall logic ========= //
always @(*) begin
    proc_stall_w = proc_stall_r;
    case (state_r)
        S_IDLE: begin
            if(proc_read || proc_write) begin
                if(hit) //  read/write hit !!
                    proc_stall_w    = 1'b0;
                else 
                    proc_stall_w    = 1'b1;
            end
            else begin
                proc_stall_w    = 1'b0;
            end
        end
        S_WRITE: begin
            proc_stall_w    = 1'b1;
        end
        S_ALLOCATE: begin
            if(mem_ready_buf)
                proc_stall_w    = 1'b0;
            else
                proc_stall_w    = 1'b1;
        end
    endcase
end

//==== sequential circuit =================================
always@( posedge clk ) begin
    if( proc_reset ) begin
        for(i = 0; i < set; i = i + 1) begin
            store_r[i]  <= {way{128'd0}};
            tag_r[i]    <= {way{26'd0}};
            valid_r[i]  <= {way{1'b0}};
            dirty_r[i]  <= {way{1'b0}};
            used_r[i]   <= {way{1'b0}};
        end
        state_r         <= S_IDLE;
        mem_read_r      <= 1'b0;
        mem_write_r     <= 1'b0;
        mem_wdata_r     <= 128'd0;
        mem_addr_r      <= 28'd0;
        proc_rdata_r    <= 32'd0;
        proc_stall_r    <= 1'b0;
        mem_ready_buf   <= 1'b0;
    end
    else begin
        for(i = 0; i < set; i = i + 1) begin
            store_r[i]  <= store_w[i];
            tag_r[i]    <= tag_w[i];
            valid_r[i]  <= valid_w[i];
            dirty_r[i]  <= dirty_w[i];
            used_r[i]   <= used_w[i];
        end
        state_r         <= state_w;
        mem_read_r      <= mem_read_w;
        mem_write_r     <= mem_write_w;
        mem_wdata_r     <= mem_wdata_w;
        mem_addr_r      <= mem_addr_w;
        proc_rdata_r    <= proc_rdata_w;
        proc_stall_r    <= proc_stall_w;
        mem_ready_buf   <= mem_ready;
    end
end

endmodule

module L2_cache(
    clk,
    reset,
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
    input                       reset;
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
    reg [L2_WIDTH-1:0]      cache_r     [0:L2_BLOCK*ASSO-1];
    reg [L2_WIDTH-1:0]      cache_w     [0:L2_BLOCK*ASSO-1];
    reg [L2_TAG-1:0]        tag_r       [0:L2_BLOCK*ASSO-1];
    reg [L2_TAG-1:0]        tag_w       [0:L2_BLOCK*ASSO-1];
    reg [L2_BLOCK*ASSO-1:0] valid_r;
    reg [L2_BLOCK*ASSO-1:0] valid_w;
    reg [L1_WIDTH-1:0]      cache_rd_r;
    reg [L1_WIDTH-1:0]      cache_rd_w;
    reg [L2_WIDTH-1:0]      mem_wd;
    reg [MEM_ADDR_SIZE-1:0] mem_add;
    reg                     hit1, hit2;
    reg                     valid1, valid2;
    reg                     ready;
    reg                     mem_r;
    reg                     mem_w;
    reg [1:0]               state, nxt_state;
    reg [L2_BLOCK*ASSO-1:0] cnt;
    reg [L2_BLOCK*ASSO-1:0] nxt_cnt;

//==== parameter ==========================================
localparam CompareTag   = 2'd0;
localparam Allocate     = 2'd1;
localparam WriteThrough = 2'd2;

//==== finite state machine ===============================
always @(posedge clk) begin
    if(reset)
        state <= CompareTag;
    else
        state <= nxt_state;
end
always @(*) begin
    nxt_state = CompareTag;
    case(state)
    CompareTag : begin
        if(~(cache_read|cache_write))begin
            nxt_state = CompareTag;
        end
        else if((valid1&hit1)|(valid2&hit2))begin
            if (cache_read)
                nxt_state = CompareTag;
            else
                nxt_state = WriteThrough;
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
    WriteThrough : begin
        if(mem_ready)
            nxt_state = CompareTag;
        else
            nxt_state = WriteThrough;
    end
    default : nxt_state = CompareTag;
    endcase
end

//==== combinational circuit ==============================
assign L2_ready  = ready;
assign cache_rdata  = cache_rd_r;
assign mem_addr     = mem_add;
assign mem_wdata    = mem_wd;
assign mem_read     = mem_r;
assign mem_write    = mem_w;
integer i;

always @(*) begin
    mem_wd   = 128'd0;
    mem_add     = 28'd0;
    ready       = 1'b0;
    mem_r       = 1'b0;
    mem_w       = 1'b0;
    hit1        = 1'b0;
    valid1      = 1'b0;
    hit2        = 1'b0;
    valid2      = 1'b0;
    cache_rd_w  = cache_rd_r;
    for (i = 0; i < L2_BLOCK*ASSO; i = i + 1)begin
        cache_w[i]  = cache_r[i];
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
                    cache_rd_w= cache_r[0];
                end
                4'd1 : begin
                    cache_rd_w= cache_r[2];
                end
                4'd2 : begin
                    cache_rd_w= cache_r[4];
                end
                4'd3 : begin
                    cache_rd_w= cache_r[6];
                end
                4'd4 : begin
                    cache_rd_w= cache_r[8];
                end
                4'd5 : begin
                    cache_rd_w= cache_r[10];
                end
                4'd6 : begin
                    cache_rd_w= cache_r[12];
                end
                4'd7 : begin
                    cache_rd_w= cache_r[14];
                end
                4'd8 : begin
                    cache_rd_w= cache_r[16];
                end
                4'd9 : begin
                    cache_rd_w= cache_r[18];
                end
                4'd10 : begin
                    cache_rd_w= cache_r[20];
                end
                4'd11 : begin
                    cache_rd_w= cache_r[22];
                end
                4'd12 : begin
                    cache_rd_w= cache_r[24];
                end
                4'd13 : begin
                    cache_rd_w= cache_r[26];
                end
                4'd14 : begin
                    cache_rd_w= cache_r[28];
                end
                4'd15 : begin
                    cache_rd_w= cache_r[30];
                end
                default : cache_rd_w= 128'b0;
            endcase
        end
        else if(cache_write)begin
            ready = 1'b0;   
            case(cache_addr[3:0])
                4'd0 : begin
                    cache_w[0] = cache_wdata;
                end
                4'd1 : begin
                    cache_w[2] = cache_wdata;
                end
                4'd2 : begin
                    cache_w[4] = cache_wdata;
                end
                4'd3 : begin
                    cache_w[6] = cache_wdata;
                end
                4'd4 : begin
                    cache_w[8] = cache_wdata;
                end
                4'd5 : begin
                    cache_w[10] = cache_wdata;
                end
                4'd6 : begin
                    cache_w[12] = cache_wdata;
                end
                4'd7 : begin
                    cache_w[14] = cache_wdata;
                end
                4'd8 : begin
                    cache_w[16] = cache_wdata;
                end
                4'd9 : begin
                    cache_w[18] = cache_wdata;
                end
                4'd10 : begin
                    cache_w[20] = cache_wdata;
                end
                4'd11 : begin
                    cache_w[22] = cache_wdata;
                end
                4'd12 : begin
                    cache_w[24] = cache_wdata;
                end
                4'd13 : begin
                    cache_w[26] = cache_wdata;
                end
                4'd14 : begin
                    cache_w[28] = cache_wdata;
                end
                4'd15 : begin
                    cache_w[30] = cache_wdata;
                end
                default : cache_w[0] = cache_r[0];
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
                    cache_rd_w = cache_r[1];
                end
                4'd1 : begin
                    cache_rd_w = cache_r[3];
                end
                4'd2 : begin
                    cache_rd_w = cache_r[5];
                end
                4'd3 : begin
                    cache_rd_w = cache_r[7];
                end
                4'd4 : begin
                    cache_rd_w = cache_r[9];
                end
                4'd5 : begin
                    cache_rd_w = cache_r[11];
                end
                4'd6 : begin
                    cache_rd_w = cache_r[13];
                end
                4'd7 : begin
                    cache_rd_w = cache_r[15];
                end
                4'd8 : begin
                    cache_rd_w = cache_r[17];
                end
                4'd9 : begin
                    cache_rd_w = cache_r[19];
                end
                4'd10 : begin
                    cache_rd_w = cache_r[21];
                end
                4'd11 : begin
                    cache_rd_w = cache_r[23];
                end
                4'd12 : begin
                    cache_rd_w = cache_r[25];
                end
                4'd13 : begin
                    cache_rd_w = cache_r[27];
                end
                4'd14 : begin
                    cache_rd_w = cache_r[29];
                end
                4'd15 : begin
                    cache_rd_w = cache_r[31];
                end
                default : cache_rd_w = 128'b0;
            endcase
        end
        if(cache_write)begin
            ready = 1'b0;
            case(cache_addr[3:0])
                4'd0 : begin
                    cache_w[1] = cache_wdata;
                end
                4'd1 : begin
                    cache_w[3] = cache_wdata;
                end
                4'd2 : begin
                    cache_w[5] = cache_wdata;
                end
                4'd3 : begin
                    cache_w[7] = cache_wdata;
                end
                4'd4 : begin
                    cache_w[9] = cache_wdata;
                end
                4'd5 : begin
                    cache_w[11] = cache_wdata;
                end
                4'd6 : begin
                    cache_w[13] = cache_wdata;
                end
                4'd7 : begin
                    cache_w[15] = cache_wdata;
                end
                4'd8 : begin
                    cache_w[17] = cache_wdata;
                end
                4'd9 : begin
                    cache_w[19] = cache_wdata;
                end
                4'd10 : begin
                    cache_w[21] = cache_wdata;
                end
                4'd11 : begin
                    cache_w[23] = cache_wdata;
                end
                4'd12 : begin
                    cache_w[25] = cache_wdata;
                end
                4'd13 : begin
                    cache_w[27] = cache_wdata;
                end
                4'd14 : begin
                    cache_w[29] = cache_wdata;
                end
                4'd15 : begin
                    cache_w[31] = cache_wdata;
                end
                default : cache_w[0] = cache_r[0];
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
            mem_add = cache_addr;
            ready = 1'b0;
        end
    end
    else begin
        if(~mem_ready)begin
            mem_r = 1'b0;
            mem_w = 1'b1;
            ready = 1'b0;
            case(cache_addr[3:2])
                4'd0 : begin
                    mem_add = (valid1&hit1)?{tag_r[0], 4'd0}:{tag_r[1], 4'd0};
                    mem_wd = (valid1&hit1)?cache_r[0]:cache_r[1];
                end
                4'd1 : begin
                    mem_add = (valid1&hit1)?{tag_r[2], 4'd1}:{tag_r[3], 4'd1};
                    mem_wd = (valid1&hit1)?cache_r[2]:cache_r[3];
                end
                4'd2 : begin
                    mem_add = (valid1&hit1)?{tag_r[4], 4'd2}:{tag_r[5], 4'd2};
                    mem_wd = (valid1&hit1)?cache_r[4]:cache_r[5];
                end
                4'd3 : begin
                    mem_add = (valid1&hit1)?{tag_r[6], 4'd3}:{tag_r[7], 4'd3};
                    mem_wd = (valid1&hit1)?cache_r[6]:cache_r[7];
                end
                4'd4 : begin
                    mem_add = (valid1&hit1)?{tag_r[8], 4'd4}:{tag_r[9], 4'd4};
                    mem_wd = (valid1&hit1)?cache_r[8]:cache_r[9];
                end
                4'd5 : begin
                    mem_add = (valid1&hit1)?{tag_r[10], 4'd5}:{tag_r[11], 4'd5};
                    mem_wd = (valid1&hit1)?cache_r[10]:cache_r[11];
                end
                4'd6 : begin
                    mem_add = (valid1&hit1)?{tag_r[12], 4'd6}:{tag_r[13], 4'd6};
                    mem_wd = (valid1&hit1)?cache_r[12]:cache_r[13];
                end
                4'd7 : begin
                    mem_add = (valid1&hit1)?{tag_r[14], 4'd7}:{tag_r[15], 4'd7};
                    mem_wd = (valid1&hit1)?cache_r[14]:cache_r[15];
                end
                4'd8 : begin
                    mem_add = (valid1&hit1)?{tag_r[16], 4'd8}:{tag_r[17], 4'd8};
                    mem_wd = (valid1&hit1)?cache_r[16]:cache_r[17];
                end
                4'd9 : begin
                    mem_add = (valid1&hit1)?{tag_r[18], 4'd9}:{tag_r[19], 4'd9};
                    mem_wd = (valid1&hit1)?cache_r[18]:cache_r[19];
                end
                4'd10 : begin
                    mem_add = (valid1&hit1)?{tag_r[20], 4'd10}:{tag_r[21], 4'd10};
                    mem_wd = (valid1&hit1)?cache_r[20]:cache_r[21];
                end
                4'd11 : begin
                    mem_add = (valid1&hit1)?{tag_r[22], 4'd11}:{tag_r[23], 4'd11};
                    mem_wd = (valid1&hit1)?cache_r[22]:cache_r[23];
                end
                4'd12 : begin
                    mem_add = (valid1&hit1)?{tag_r[24], 4'd12}:{tag_r[25], 4'd12};
                    mem_wd = (valid1&hit1)?cache_r[24]:cache_r[25];
                end
                4'd13 : begin
                    mem_add = (valid1&hit1)?{tag_r[26], 4'd13}:{tag_r[27], 4'd13};
                    mem_wd = (valid1&hit1)?cache_r[26]:cache_r[27];
                end
                4'd14 : begin
                    mem_add = (valid1&hit1)?{tag_r[28], 4'd14}:{tag_r[29], 4'd14};
                    mem_wd = (valid1&hit1)?cache_r[28]:cache_r[29];
                end
                4'd15 : begin
                    mem_add = (valid1&hit1)?{tag_r[31], 4'd15}:{tag_r[31], 4'd15};
                    mem_wd = (valid1&hit1)?cache_r[31]:cache_r[31];
                end
                default : begin
                    mem_add = 28'd0;
                    mem_wd = 128'd0;
                end
            endcase
        end
        else begin
            ready = 1'b1;
        end
    end
end

//==== sequential circuit =================================
always@( posedge clk ) begin
    cache_rd_r <= cache_rd_w;
    for (i = 0; i < L2_BLOCK*ASSO; i = i + 1)begin
        cache_r[i] <= cache_w[i];
        tag_r[i] <= tag_w[i];
    end
    if( reset ) begin
        cache_rd_r <= 128'd0;
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
