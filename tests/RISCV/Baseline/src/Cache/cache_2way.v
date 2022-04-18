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
