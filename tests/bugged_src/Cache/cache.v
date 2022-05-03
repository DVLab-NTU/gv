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
    input   [29:0] proc_addr;   // word addr [31:2], cache unit can only access 4 word at a time
    input   [31:0] proc_wdata;  // data write to cache
    output         proc_stall;  
    output  [31:0] proc_rdata;
    // memory interface
    input  [127:0] mem_rdata;   
    input          mem_ready;
    output         mem_read, mem_write;
    output  [27:0] mem_addr;    // mem addr [31:4]
    output [127:0] mem_wdata;   // data write to memory
    
//==== wire/reg definition ================================
    // (block number) modulo (number of cache blocks)
    // block_addr = proc_addr mod 4
    // from cpu 30 bit = [28 bit tag + 2 bit index]
    // from cache block = 1 bit valid + (28-2) bit tag + 128 bit value
    
    wire [1:0] index;      // memory 128 bit =  32 bit value * 4
    wire way_idx;          // currently operate on which way (2-way block)
    wire [1:0] block_idx;  // cache 8 block
    
    // inside cache block
    reg [127:0] block_value_1, block_value_2, block_value_3, block_value_4, block_value_5, block_value_6, block_value_7, block_value_8;
    reg [127:0] block_value_1_nxt, block_value_2_nxt, block_value_3_nxt, block_value_4_nxt, block_value_5_nxt, block_value_6_nxt, block_value_7_nxt, block_value_8_nxt;
    reg [25:0] block_tag_1, block_tag_2, block_tag_3, block_tag_4, block_tag_5, block_tag_6, block_tag_7, block_tag_8;
    reg [25:0] block_tag_1_nxt, block_tag_2_nxt, block_tag_3_nxt, block_tag_4_nxt, block_tag_5_nxt, block_tag_6_nxt, block_tag_7_nxt, block_tag_8_nxt;
    reg block_val_1, block_val_2, block_val_3, block_val_4, block_val_5, block_val_6, block_val_7, block_val_8; 
    reg block_val_1_nxt, block_val_2_nxt, block_val_3_nxt, block_val_4_nxt, block_val_5_nxt, block_val_6_nxt, block_val_7_nxt, block_val_8_nxt; 
    reg [3:0] block_dirty_1, block_dirty_2, block_dirty_3, block_dirty_4, block_dirty_5, block_dirty_6, block_dirty_7, block_dirty_8; 
    reg [3:0] block_dirty_1_nxt, block_dirty_2_nxt, block_dirty_3_nxt, block_dirty_4_nxt, block_dirty_5_nxt, block_dirty_6_nxt, block_dirty_7_nxt, block_dirty_8_nxt; 
    reg block_way_1, block_way_3, block_way_5, block_way_7; 
    reg block_way_1_nxt, block_way_3_nxt, block_way_5_nxt, block_way_7_nxt;

    reg [25:0] read_tag1, read_tag2;
    reg [127:0] read_value1, read_value2;
    reg read_val1, read_val2;
    reg [3:0] read_dirty1, read_dirty2;
    reg read_last_way;

    reg [31:0] proc_rdata;
    reg [127:0] new_value;
    reg [3:0] new_dirty;

    wire hit;  // 1: hit, 0: miss
    
    assign index = proc_addr[1:0];
    assign block_idx = proc_addr[3:2];
    assign hit = (read_tag1 == proc_addr[29:4])? read_val1 :
                 (read_tag2 == proc_addr[29:4])? read_val2 : 
                 0 ;
    assign way_idx = (read_tag1 == proc_addr[29:4])? 1'b0 : 
                     (read_tag2 == proc_addr[29:4])? 1'b1 :
                     mem_write? read_last_way : !read_last_way;
                    //  (proc_write & mem_read)? !read_last_way : read_last_way;

    assign mem_read = !hit & (!read_dirty1) & (!read_dirty2);
    assign mem_write = (|(read_dirty1) | (|(read_dirty2))) & !hit;
    assign proc_stall = mem_read | mem_write;
    assign mem_addr = |read_dirty1? {read_tag1, proc_addr[3:2]} : 
                      |read_dirty2? {read_tag2, proc_addr[3:2]} : 
                      proc_addr[29:2]; 
    assign mem_wdata = |read_dirty1? read_value1: read_value2 ;

//==== combinational circuit ==============================
    always @(*) begin
        case(block_idx)
            2'b00: begin
                read_dirty1 = block_dirty_1;
                read_dirty2 = block_dirty_2;
                read_val1 = block_val_1;
                read_val2 = block_val_2;
                read_tag1 = block_tag_1;
                read_tag2 = block_tag_2;
                read_value1 = block_value_1;
                read_value2 = block_value_2;
                read_last_way = block_way_1;
            end
            2'b01: begin
                read_dirty1 = block_dirty_3;
                read_dirty2 = block_dirty_4;
                read_val1 = block_val_3; 
                read_val2 = block_val_4; 
                read_tag1 = block_tag_3;
                read_tag2 = block_tag_4;
                read_value1 = block_value_3;
                read_value2 = block_value_4;
                read_last_way = block_way_3;
            end
            2'b10: begin
                read_dirty1 = block_dirty_5;
                read_dirty2 = block_dirty_6;
                read_val1 = block_val_5;
                read_val2 = block_val_6;
                read_tag1 = block_tag_5;
                read_tag2 = block_tag_6;
                read_value1 = block_value_5;
                read_value2 = block_value_6;
                read_last_way = block_way_5;
            end
            2'b11: begin
                read_dirty1 = block_dirty_7;
                read_dirty2 = block_dirty_8;
                read_val1 = block_val_7;
                read_val2 = block_val_8;
                read_tag1 = block_tag_7;
                read_tag2 = block_tag_8;
                read_value1 = block_value_7;
                read_value2 = block_value_8;
                read_last_way = block_way_7;
            end
        endcase
    end
    
    always @(*) begin
        if(proc_read) begin
            // read cache value according to index (4 word block)
            case(index) 
                2'b00: proc_rdata = way_idx? read_value2[31:0] : read_value1[31:0];
                2'b01: proc_rdata = way_idx? read_value2[63:32] : read_value1[63:32];
                2'b10: proc_rdata = way_idx? read_value2[95:64] : read_value1[95:64];
                2'b11: proc_rdata = way_idx? read_value2[127:96] : read_value1[127:96];
            endcase
        end else begin
            proc_rdata = 32'b0;
        end
    end
    
    always @(*) begin
        if(proc_write) begin
            // write cache value according to index (4 word block)
            case(index) 
                2'b00: begin
                    new_value = way_idx? {read_value2[127:32],proc_wdata} : {read_value1[127:32],proc_wdata};
                    new_dirty = way_idx? {read_dirty2[3:1], 1'b1} : {read_dirty1[3:1], 1'b1};
                end
                2'b01: begin
                    new_value = way_idx? {read_value2[127:64], proc_wdata, read_value2[31:0]} : {read_value1[127:64], proc_wdata, read_value1[31:0]};
                    new_dirty = way_idx? {read_dirty2[3:2], 1'b1, read_dirty2[0]} : {read_dirty1[3:2], 1'b1, read_dirty1[0]};
                end
                2'b10: begin
                    new_value = way_idx? {read_value2[127:96], proc_wdata, read_value2[63:0]} : {read_value1[127:96], proc_wdata, read_value1[63:0]};
                    new_dirty = way_idx? {read_dirty2[3], 1'b1, read_dirty2[1:0]} : {read_dirty1[3], 1'b1, read_dirty1[1:0]};
                end
                2'b11: begin
                    new_value = way_idx? {proc_wdata, read_value2[95:0]} : {proc_wdata, read_value1[95:0]};
                    new_dirty = way_idx? {1'b1, read_dirty2[2:0]} : {1'b1, read_dirty1[2:0]};
                end
            endcase
        end else begin
            new_value = way_idx? read_value2 : read_value1;
            new_dirty = 4'b0;
        end
    end

    always @(*) begin
        if(block_idx==2'b00) begin
            if(!read_dirty1 & !read_dirty2) begin
                block_tag_1_nxt =  (mem_ready & !way_idx) ? mem_addr[27:2] : block_tag_1;
                block_tag_2_nxt =  (mem_ready & way_idx) ? mem_addr[27:2] : block_tag_2;
            end else begin
                block_tag_1_nxt = block_tag_1;
                block_tag_2_nxt = block_tag_2;
            end               
        end else begin
            block_tag_1_nxt = block_tag_1;
            block_tag_2_nxt = block_tag_2;
        end  
        if(block_idx==2'b01) begin
            if(!read_dirty1 & !read_dirty2) begin
                block_tag_3_nxt =  (mem_ready & !way_idx) ? mem_addr[27:2] : block_tag_3;
                block_tag_4_nxt =  (mem_ready & way_idx) ? mem_addr[27:2] : block_tag_4;
            end else begin
                block_tag_3_nxt = block_tag_3;
                block_tag_4_nxt = block_tag_4;
            end               
        end else begin
            block_tag_3_nxt = block_tag_3;
            block_tag_4_nxt = block_tag_4;
        end  
        if(block_idx==2'b10) begin
            if(!read_dirty1 & !read_dirty2) begin
                block_tag_5_nxt =  (mem_ready & !way_idx) ? mem_addr[27:2] : block_tag_5;
                block_tag_6_nxt =  (mem_ready & way_idx) ? mem_addr[27:2] : block_tag_6;
            end else begin
                block_tag_5_nxt = block_tag_5;
                block_tag_6_nxt = block_tag_6;
            end               
        end else begin
            block_tag_5_nxt = block_tag_5;
            block_tag_6_nxt = block_tag_6;
        end  
        if(block_idx==2'b11) begin
            if(!read_dirty1 & !read_dirty2) begin
                block_tag_7_nxt =  (mem_ready & !way_idx) ? mem_addr[27:2] : block_tag_7;
                block_tag_8_nxt =  (mem_ready & way_idx) ? mem_addr[27:2] : block_tag_8;
            end else begin
                block_tag_7_nxt = block_tag_7;
                block_tag_8_nxt = block_tag_8;
            end               
        end else begin
            block_tag_7_nxt = block_tag_7;
            block_tag_8_nxt = block_tag_8;
        end  
    end

    always @(*) begin
        block_value_1_nxt = (mem_ready & (block_idx==2'b00) & !way_idx) ? mem_rdata : 
                            (proc_write & hit & (block_idx==2'b00) & !way_idx)? new_value : 
                            block_value_1;
        block_val_1_nxt = (mem_ready & (block_idx==2'b00) & !way_idx) ? 1 : block_val_1;
        block_dirty_1_nxt = (!(block_idx==2'b00) | way_idx) ? block_dirty_1 :
                            hit? new_dirty :
                            mem_ready? 4'b0 :
                            block_dirty_1;
        block_way_1_nxt = (mem_ready & (block_idx==2'b00))? way_idx : block_way_1;
        
        block_value_2_nxt = (mem_ready & (block_idx==2'b00) & way_idx) ? mem_rdata : 
                            (proc_write & hit & (block_idx==2'b00) & way_idx)? new_value : 
                            block_value_2;
        block_val_2_nxt =  (mem_ready & (block_idx==2'b00) & way_idx) ?1 : block_val_2;
        block_dirty_2_nxt = (!(block_idx==2'b00) | !way_idx) ? block_dirty_2:
                            hit? new_dirty : 
                            mem_ready? 4'b0 :
                            block_dirty_2;

        block_value_3_nxt = (mem_ready & (block_idx==2'b01) & !way_idx) ? mem_rdata : 
                            (proc_write & hit & (block_idx==2'b01) & !way_idx)? new_value : 
                            block_value_3;
        block_val_3_nxt = (mem_ready & (block_idx==2'b01) & !way_idx) ? 1 : block_val_3;
        block_dirty_3_nxt = (!(block_idx==2'b01) | way_idx) ? block_dirty_3:
                            hit? new_dirty :
                            mem_ready? 4'b0 :
                            block_dirty_3;
        block_way_3_nxt = (mem_ready & (block_idx==2'b01))? way_idx : block_way_3;

        block_value_4_nxt = (mem_ready & (block_idx==2'b01) & way_idx) ? mem_rdata : 
                            (proc_write & hit & (block_idx==2'b01) & way_idx)? new_value : 
                            block_value_4;
        block_val_4_nxt = (mem_ready & (block_idx==2'b01) & way_idx) ? 1 : block_val_4;
        block_dirty_4_nxt = (!(block_idx==2'b01) | !way_idx) ? block_dirty_4 : 
                            hit? new_dirty :
                            mem_ready? 4'b0 :
                            block_dirty_4;

        block_value_5_nxt = (mem_ready & (block_idx==2'b10) & !way_idx) ? mem_rdata : 
                            (proc_write & hit & (block_idx==2'b10) & !way_idx)? new_value : 
                            block_value_5;
        block_val_5_nxt = (mem_ready & (block_idx==2'b10) & !way_idx) ? 1 : block_val_5;
        block_dirty_5_nxt = (!(block_idx==2'b10) | way_idx) ? block_dirty_5 :
                            hit? new_dirty :
                            mem_ready? 4'b0 :
                            block_dirty_5;
        block_way_5_nxt = (mem_ready & (block_idx==2'b10))? way_idx : block_way_5;

        block_value_6_nxt = (mem_ready & (block_idx==2'b10) & way_idx) ? mem_rdata : 
                            (proc_write & hit & (block_idx==2'b10) & way_idx)? new_value : 
                            block_value_6;
        block_val_6_nxt = (mem_ready & (block_idx==2'b10) & way_idx) ? 1 : block_val_6;
        block_dirty_6_nxt = (!(block_idx==2'b10) | !way_idx) ?  block_dirty_6 :
                            hit? new_dirty :
                            mem_ready? 4'b0 :
                            block_dirty_6;

        block_value_7_nxt = (mem_ready & (block_idx==2'b11) & !way_idx) ? mem_rdata : 
                            (proc_write & hit & (block_idx==2'b11) & !way_idx)? new_value : 
                            block_value_7;
        block_val_7_nxt = (mem_ready & (block_idx==2'b11) & !way_idx) ? 1 : block_val_7;
        block_dirty_7_nxt = (!(block_idx==2'b11) | way_idx) ? block_dirty_7 :
                            hit? new_dirty : 
                            mem_ready? 4'b0 :
                            block_dirty_7;
        block_way_7_nxt = (mem_ready & (block_idx==2'b11))? way_idx : block_way_7;

        block_value_8_nxt = (mem_ready & (block_idx==2'b11) & way_idx) ? mem_rdata : 
                            (proc_write & hit & (block_idx==2'b11) & way_idx)? new_value :
                            block_value_8;
        block_val_8_nxt = (mem_ready & (block_idx==2'b11) & way_idx) ? 1 : block_val_8;
        block_dirty_8_nxt = (!(block_idx==2'b11) | !way_idx) ?  block_dirty_8:
                            hit? new_dirty : 
                            mem_ready? 4'b0 :
                            block_dirty_8;
    end

//==== sequential circuit =================================
always@( posedge clk ) begin
    if( proc_reset ) begin
        block_dirty_1 <= 1'b0;
        block_value_1 <= 128'b0;
        block_tag_1 <= 25'b0;
        block_val_1 <= 1'b0;
        block_way_1 <= 1'b0;

        block_dirty_2 <= 1'b0;
        block_value_2 <= 128'b0;
        block_tag_2 <= 25'b0;
        block_val_2 <= 1'b0;

        block_dirty_3 <= 1'b0;
        block_value_3 <= 128'b0;
        block_tag_3 <= 25'b0;
        block_val_3 <= 1'b0;
        block_way_3 <= 1'b0;

        block_dirty_4 <= 1'b0;
        block_value_4 <= 128'b0;
        block_tag_4 <= 25'b0;
        block_val_4 <= 1'b0;

        block_dirty_5 <= 1'b0;
        block_value_5 <= 128'b0;
        block_tag_5 <= 25'b0;
        block_val_5 <= 1'b0;
        block_way_5 <= 1'b0;

        block_dirty_6 <= 1'b0;
        block_value_6 <= 128'b0;
        block_tag_6 <= 25'b0;
        block_val_6 <= 1'b0;
        
        block_dirty_7 <= 1'b0;
        block_value_7 <= 128'b0;
        block_tag_7 <= 25'b0;
        block_val_7 <= 1'b0;
        block_way_7 <= 1'b0;

        block_dirty_8 <= 1'b0;
        block_value_8 <= 128'b0;
        block_tag_8 <= 25'b0;
        block_val_8 <= 1'b0;
    end
    else begin
        block_dirty_1 <= block_dirty_1_nxt;
        block_value_1 <= block_value_1_nxt;
        block_tag_1 <= block_tag_1_nxt;
        block_val_1 <= block_val_1_nxt;
        block_way_1 <= block_way_1_nxt;

        block_dirty_2 <= block_dirty_2_nxt;
        block_value_2 <= block_value_2_nxt;
        block_tag_2 <= block_tag_2_nxt;
        block_val_2 <= block_val_2_nxt;

        block_dirty_3 <= block_dirty_3_nxt;
        block_value_3 <= block_value_3_nxt;
        block_tag_3 <= block_tag_3_nxt;
        block_val_3 <= block_val_3_nxt;
        block_way_3 <= block_way_3_nxt;

        block_dirty_4 <= block_dirty_4_nxt;
        block_value_4 <= block_value_4_nxt;
        block_tag_4 <= block_tag_4_nxt;
        block_val_4 <= block_val_4_nxt;

        block_dirty_5 <= block_dirty_5_nxt;
        block_value_5 <= block_value_5_nxt;
        block_tag_5 <= block_tag_5_nxt;
        block_val_5 <= block_val_5_nxt;
        block_way_5 <= block_way_5_nxt;

        block_dirty_6 <= block_dirty_6_nxt;
        block_value_6 <= block_value_6_nxt;
        block_tag_6 <= block_tag_6_nxt;
        block_val_6 <= block_val_6_nxt;

        block_dirty_7 <= block_dirty_7_nxt;
        block_value_7 <= block_value_7_nxt;
        block_tag_7 <= block_tag_7_nxt;
        block_val_7 <= block_val_7_nxt;
        block_way_7 <= block_way_7_nxt;

        block_dirty_8 <= block_dirty_8_nxt;
        block_value_8 <= block_value_8_nxt;
        block_tag_8 <= block_tag_8_nxt;
        block_val_8 <= block_val_8_nxt;

    end
end
endmodule
