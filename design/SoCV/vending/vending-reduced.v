/* 
   Source   : vending/vending.v
   Synopsis : Written by Cheng-Yin Wu for SoCV Assignments
   Date     : 2013/02/20
   Version  : 1.0
   revised  : Simplified by Chu Chen-Kai @ 2013/02/26
*/

// Service Types
`define SERVICE_OFF     2'b00
`define SERVICE_ON      2'b01
`define SERVICE_BUSY    2'b10
// Coin Types
`define NTD_5           1'b0
`define NTD_1           1'b1
// Coin Values
`define VALUE_NTD_5     3'd5
`define VALUE_NTD_1     3'd1
// Item Types
`define ITEM_NONE       1'b0
`define ITEM_A          1'b1
// Item Costs
`define COST_A          3'd3

module vendingMachine(
   // General I/O Ports
   clk,
   reset,
   // Input Ports
   coinInNTD_5,
   coinInNTD_1,
   itemTypeIn,
   // Output Ports
   coinOutNTD_5,
   coinOutNTD_1,
   itemTypeOut,
   serviceTypeOut,
   // Property Output Ports
   p
);

// General I/O Ports
input  clk;
input  reset;
// Input Ports
input  [1:0] coinInNTD_5;      // input number of NTD_5
input  [1:0] coinInNTD_1;      // input number of NTD_1
input        itemTypeIn;       // type of an item
// Output Ports
output [1:0] coinOutNTD_5;     // output number of NTD_5
output [1:0] coinOutNTD_1;     // output number of NTD_1
output       itemTypeOut;      // type of an item
output [1:0] serviceTypeOut;   // type of the service
// Property Output Ports (Hugo added)
output p;

reg    [1:0] coinOutNTD_5;     // output number of NTD_5
reg    [1:0] coinOutNTD_1;     // output number of NTD_1
reg          itemTypeOut;      // type of an item
reg    [1:0] serviceTypeOut;   // type of the service

reg    [1:0] countNTD_5;       // number of NTD_5
reg    [1:0] countNTD_1;       // number of NTD_1

reg    [2:0] inputValue;       // total amount of input money
reg    [2:0] serviceValue;     // total amount of service money
reg          serviceCoinType;  // type of the coin for the service
reg          exchangeReady;    // ready for exchange in BUSY
reg          initialized;      // initialized or not (i.e. reset)

reg    [1:0] coinOutNTD_5_w;   // output number of NTD_5
reg    [1:0] coinOutNTD_1_w;   // output number of NTD_1
reg          itemTypeOut_w;    // type of an item
reg    [1:0] serviceTypeOut_w; // type of the service

reg    [1:0] countNTD_5_w;     // number of NTD_5
reg    [1:0] countNTD_1_w;     // number of NTD_1

reg    [2:0] inputValue_w;     // total amount of input money
reg    [2:0] serviceValue_w;   // total amount of service money
reg          serviceCoinType_w;// type of the coin for the service
reg          exchangeReady_w;  // ready for exchange in BUSY

wire   [2:0] outExchange;      // the output exchange amount, for verification

// Property Logic
/***** whether the change is right *****/
  // (1) 如果販賣機沒有零錢找時, 應退還正確的 inputValue, 否則就錯了 (含買東西不投錢, 含付錢不買東西)
  // (2) 正常情況 inputValue = 飲料錢 + outExchange, 否則就錯了
  // (3) 每次開始要算零錢時, 必須確保演算法會從最大幣值開始找錢, 以免小幣值找不夠導致經常退還錢給消費者
  // (4) 確認每次沒退錢情況下, 投出來的飲料應該要和消費者選擇的飲料一樣, 否則就錯了
assign p = ((initialized && (serviceTypeOut == `SERVICE_OFF) && (itemTypeOut == `ITEM_NONE) && (outExchange != inputValue)) ||
            (initialized && (serviceTypeOut == `SERVICE_OFF) && (itemTypeOut != `ITEM_NONE) && (inputValue != outExchange + ((itemTypeOut == `ITEM_A) ? `COST_A : 4'd0))) ||
            ((serviceTypeOut == `SERVICE_ON) && (itemTypeOut != `ITEM_NONE) && (serviceCoinType != `NTD_5)) ||
            ((serviceTypeOut == `SERVICE_ON) && (itemTypeOut != `ITEM_NONE) && (itemTypeOut != itemTypeIn)));
  
assign outExchange = (`VALUE_NTD_5  * {1'd0, coinOutNTD_5 }) + 
                     (`VALUE_NTD_1  * {1'd0, coinOutNTD_1 }); 
                     

always @ (*) begin
  coinOutNTD_5_w    = coinOutNTD_5;
  coinOutNTD_1_w    = coinOutNTD_1;
  itemTypeOut_w     = itemTypeOut;
  serviceTypeOut_w  = serviceTypeOut;
  countNTD_5_w      = countNTD_5;
  countNTD_1_w      = countNTD_1;
  inputValue_w      = inputValue;
  serviceValue_w    = serviceValue;
  serviceCoinType_w = serviceCoinType;
  exchangeReady_w   = exchangeReady;

  case (serviceTypeOut)
    `SERVICE_ON   : begin
      if (itemTypeIn != `ITEM_NONE) begin  // valid request
        coinOutNTD_5_w    = 2'd0;
        coinOutNTD_1_w    = 2'd0;
        itemTypeOut_w     = itemTypeIn;
        serviceTypeOut_w  = `SERVICE_BUSY;
        countNTD_5_w      = (({1'b0, countNTD_5 } + {1'd0, coinInNTD_5 }) >= {1'b0, 2'b11}) ? 
                             2'b11 : (countNTD_5  + {1'b0, coinInNTD_5 });
        countNTD_1_w      = (({1'b0, countNTD_1 } + {1'd0, coinInNTD_1 }) >= {1'b0, 2'b11}) ? 
                             2'b11 : (countNTD_1  + {1'b0, coinInNTD_1 });
        inputValue_w      =  (`VALUE_NTD_5  * {1'd0, coinInNTD_5 }) + 
                             (`VALUE_NTD_1  * {1'd0, coinInNTD_1 }); 
        serviceValue_w    = (itemTypeIn == `ITEM_A) ? `COST_A : 3'd0;
        serviceCoinType_w = `NTD_5;
        exchangeReady_w   = 1'b0;
      end
    end
    `SERVICE_OFF  : begin  // Output change and item
      coinOutNTD_5_w    = 2'd0;
      coinOutNTD_1_w    = 2'd0;
      itemTypeOut_w     = `ITEM_NONE;
      serviceTypeOut_w  = `SERVICE_ON;
    end
    default       : begin  // check change for the item
      if (!exchangeReady) begin
        if (inputValue < serviceValue) begin  // too few money for the item
          serviceValue_w  = inputValue;
          itemTypeOut_w   = `ITEM_NONE;
          exchangeReady_w = 1'b1;
        end else begin
          serviceValue_w  = inputValue - serviceValue;
          exchangeReady_w = 1'b1;
        end
      end else begin
        case (serviceCoinType)
          `NTD_5  : begin
            if (serviceValue >= `VALUE_NTD_5) begin
              if (countNTD_5 == 2'd0) begin
                serviceCoinType_w = `NTD_1;
              end else begin
                coinOutNTD_5_w = coinOutNTD_5 + 2'd1;
                countNTD_5_w   = countNTD_5 - 2'd1;
                serviceValue_w = serviceValue - `VALUE_NTD_5;
              end
            end else begin
              serviceCoinType_w = `NTD_1;
            end
          end
          `NTD_1  : begin
            if (serviceValue >= `VALUE_NTD_1) begin
              if (countNTD_1 == 2'd0) begin
                serviceValue_w    = inputValue;
                itemTypeOut_w     = `ITEM_NONE;
                serviceCoinType_w = `NTD_5;
                countNTD_5_w      = countNTD_5 + coinOutNTD_5;
                countNTD_1_w      = countNTD_1 + coinOutNTD_1;
                coinOutNTD_5_w    = 2'd0;
                coinOutNTD_1_w    = 2'd0;
                serviceTypeOut_w  = `SERVICE_OFF;   // bug 1 
              end else begin
                coinOutNTD_1_w = coinOutNTD_1 + 2'd1;
                countNTD_1_w   = countNTD_1 - 2'd1;
                serviceValue_w = serviceValue - `VALUE_NTD_1;
              end
            end else begin  // exchangeable
              serviceTypeOut_w = `SERVICE_OFF;
            end
          end
        endcase
      end
    end
  endcase
end

always @ (posedge clk) begin
   if (!reset) begin
      coinOutNTD_5      <= 2'd0;
      coinOutNTD_1      <= 2'd0;
      itemTypeOut       <= `ITEM_NONE;
      serviceTypeOut    <= `SERVICE_ON;
      countNTD_5        <= 2'd2;
      countNTD_1        <= 2'd2;
      inputValue        <= 3'd0;
      serviceValue      <= 3'd0;
      serviceCoinType   <= `NTD_5;
      exchangeReady     <= 1'b0;
      initialized       <= 1'b1;
   end
   else begin
      coinOutNTD_5      <= coinOutNTD_5_w;
      coinOutNTD_1      <= coinOutNTD_1_w;
      itemTypeOut       <= itemTypeOut_w;
      serviceTypeOut    <= serviceTypeOut_w;
      countNTD_5        <= countNTD_5_w;
      countNTD_1        <= countNTD_1_w;
      inputValue        <= inputValue_w;
      serviceValue      <= serviceValue_w;
      serviceCoinType   <= serviceCoinType_w;
      exchangeReady     <= exchangeReady_w;
      initialized       <= initialized;
   end
end

endmodule
