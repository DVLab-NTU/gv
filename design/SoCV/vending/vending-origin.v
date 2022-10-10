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
`define NTD_50          2'b00
`define NTD_10          2'b01
`define NTD_5           2'b10
`define NTD_1           2'b11
// Coin Values
`define VALUE_NTD_50    8'd50
`define VALUE_NTD_10    8'd10
`define VALUE_NTD_5     8'd5
`define VALUE_NTD_1     8'd1
// Item Types
`define ITEM_NONE       2'b00
`define ITEM_A          2'b01
`define ITEM_B          2'b10
`define ITEM_C          2'b11
// Item Costs
`define COST_A          8'd8
`define COST_B          8'd15
`define COST_C          8'd22

module vendingMachine(
   // General I/O Ports
   clk,
   reset,
   // Input Ports
   coinInNTD_50,
   coinInNTD_10,
   coinInNTD_5,
   coinInNTD_1,
   itemTypeIn,
   // Output Ports
   coinOutNTD_50,
   coinOutNTD_10,
   coinOutNTD_5,
   coinOutNTD_1,
   itemTypeOut,
   serviceTypeOut,
   // Property Output Ports
   p1, p2, p3, p4
);

// General I/O Ports
input  clk;
input  reset;
// Input Ports
input  [1:0] coinInNTD_50;     // input number of NTD_50
input  [1:0] coinInNTD_10;     // input number of NTD_10
input  [1:0] coinInNTD_5;      // input number of NTD_5
input  [1:0] coinInNTD_1;      // input number of NTD_1
input  [1:0] itemTypeIn;       // type of an item
// Output Ports
output [2:0] coinOutNTD_50;    // output number of NTD_50
output [2:0] coinOutNTD_10;    // output number of NTD_10
output [2:0] coinOutNTD_5;     // output number of NTD_5
output [2:0] coinOutNTD_1;     // output number of NTD_1
output [1:0] itemTypeOut;      // type of an item
output [1:0] serviceTypeOut;   // type of the service
// Property Output Ports (Hugo added)
output p1, p2, p3, p4;

reg    [2:0] coinOutNTD_50;    // output number of NTD_50
reg    [2:0] coinOutNTD_10;    // output number of NTD_10
reg    [2:0] coinOutNTD_5;     // output number of NTD_5
reg    [2:0] coinOutNTD_1;     // output number of NTD_1
reg    [1:0] itemTypeOut;      // type of an item
reg    [1:0] serviceTypeOut;   // type of the service

reg    [2:0] countNTD_50;      // number of NTD_50
reg    [2:0] countNTD_10;      // number of NTD_10
reg    [2:0] countNTD_5;       // number of NTD_5
reg    [2:0] countNTD_1;       // number of NTD_1

reg    [7:0] inputValue;       // total amount of input money
reg    [7:0] serviceValue;     // total amount of service money
reg    [1:0] serviceCoinType;  // type of the coin for the service
reg          exchangeReady;    // ready for exchange in BUSY
reg          initialized;      // initialized or not (i.e. reset)

reg    [2:0] coinOutNTD_50_w;  // output number of NTD_50
reg    [2:0] coinOutNTD_10_w;  // output number of NTD_10
reg    [2:0] coinOutNTD_5_w;   // output number of NTD_5
reg    [2:0] coinOutNTD_1_w;   // output number of NTD_1
reg    [1:0] itemTypeOut_w;    // type of an item
reg    [1:0] serviceTypeOut_w; // type of the service

reg    [2:0] countNTD_50_w;    // number of NTD_50
reg    [2:0] countNTD_10_w;    // number of NTD_10
reg    [2:0] countNTD_5_w;     // number of NTD_5
reg    [2:0] countNTD_1_w;     // number of NTD_1

reg    [7:0] inputValue_w;     // total amount of input money
reg    [7:0] serviceValue_w;   // total amount of service money
reg    [1:0] serviceCoinType_w;// type of the coin for the service
reg          exchangeReady_w;  // ready for exchange in BUSY

wire   [7:0] outExchange;      // the output exchange amount, for verification

// Property Logic
/***** whether the change is right *****/
  // (1) 如果販賣機沒有零錢找時, 應退還正確的 inputValue, 否則就錯了 (含買東西不投錢, 含付錢不買東西)
  // (2) 正常情況 inputValue = 飲料錢 + outExchange, 否則就錯了
  // (3) 每次開始要算零錢時, 必須確保演算法會從最大幣值開始找錢, 以免小幣值找不夠導致經常退還錢給消費者
  // (4) 確認每次沒退錢情況下, 投出來的飲料應該要和消費者選擇的飲料一樣, 否則就錯了
assign p1 = ((initialized) && (serviceTypeOut == `SERVICE_OFF) && (itemTypeOut == `ITEM_NONE) && (outExchange != inputValue));
assign p2 = ((initialized) && (serviceTypeOut == `SERVICE_OFF) && (itemTypeOut != `ITEM_NONE) && (inputValue != outExchange + ((itemTypeOut == `ITEM_A) ? `COST_A : (itemTypeOut == `ITEM_B) ? `COST_B : (itemTypeOut == `ITEM_C) ? `COST_C : 8'd0)));
assign p3 = ((serviceTypeOut == `SERVICE_ON) && (itemTypeOut != `ITEM_NONE) && (serviceCoinType != `NTD_50));
assign p4 = ((serviceTypeOut == `SERVICE_ON) && (itemTypeOut != `ITEM_NONE) && (itemTypeOut != itemTypeIn));

assign outExchange = (`VALUE_NTD_50 * {5'd0, coinOutNTD_50}) + 
                     (`VALUE_NTD_10 * {5'd0, coinOutNTD_10}) + 
                     (`VALUE_NTD_5  * {5'd0, coinOutNTD_5 }) + 
                     (`VALUE_NTD_1  * {5'd0, coinOutNTD_1 });

always @ (*) begin
  coinOutNTD_50_w   = coinOutNTD_50;
  coinOutNTD_10_w   = coinOutNTD_10;
  coinOutNTD_5_w    = coinOutNTD_5;
  coinOutNTD_1_w    = coinOutNTD_1;
  itemTypeOut_w     = itemTypeOut;
  serviceTypeOut_w  = serviceTypeOut;
  countNTD_50_w     = countNTD_50;
  countNTD_10_w     = countNTD_10;
  countNTD_5_w      = countNTD_5;
  countNTD_1_w      = countNTD_1;
  inputValue_w      = inputValue;
  serviceValue_w    = serviceValue;
  serviceCoinType_w = serviceCoinType;
  exchangeReady_w   = exchangeReady;

  case (serviceTypeOut)
    `SERVICE_ON   : begin
      if (itemTypeIn != `ITEM_NONE) begin  // valid request
        coinOutNTD_50_w   = 3'd0;
        coinOutNTD_10_w   = 3'd0;
        coinOutNTD_5_w    = 3'd0;
        coinOutNTD_1_w    = 3'd0;
        itemTypeOut_w     = itemTypeIn;
        serviceTypeOut_w  = `SERVICE_BUSY;
        countNTD_50_w     = (({1'b0, countNTD_50} + {2'd0, coinInNTD_50}) >= {1'b0, 3'b111}) ? 
                             3'b111 : (countNTD_50 + {1'b0, coinInNTD_50});
        countNTD_10_w     = (({1'b0, countNTD_10} + {2'd0, coinInNTD_10}) >= {1'b0, 3'b111}) ? 
                             3'b111 : (countNTD_10 + {1'b0, coinInNTD_10});
        countNTD_5_w      = (({1'b0, countNTD_5 } + {2'd0, coinInNTD_5 }) >= {1'b0, 3'b111}) ? 
                             3'b111 : (countNTD_5  + {1'b0, coinInNTD_5 });
        countNTD_1_w      = (({1'b0, countNTD_1 } + {2'd0, coinInNTD_1 }) >= {1'b0, 3'b111}) ? 
                             3'b111 : (countNTD_1  + {1'b0, coinInNTD_1 });
        inputValue_w      = (`VALUE_NTD_50 * {6'd0, coinInNTD_50}) + 
                             (`VALUE_NTD_10 * {6'd0, coinInNTD_10}) + 
                             (`VALUE_NTD_5  * {6'd0, coinInNTD_5 }) + 
                             (`VALUE_NTD_1  * {6'd0, coinInNTD_1 });
        serviceValue_w    = (itemTypeIn == `ITEM_A) ? `COST_A : 
                             (itemTypeIn == `ITEM_B) ? `COST_B : 
                             (itemTypeIn == `ITEM_C) ? `COST_C : 8'd0; 
        serviceCoinType_w = `NTD_50;
        exchangeReady_w   = 1'b0;
      end
    end
    `SERVICE_OFF  : begin  // Output change and item
      coinOutNTD_50_w   = 3'd0;
      coinOutNTD_10_w   = 3'd0;
      coinOutNTD_5_w    = 3'd0;
      coinOutNTD_1_w    = 3'd0;
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
          `NTD_50 : begin
            if (serviceValue >= `VALUE_NTD_50) begin
              if (countNTD_50 == 3'd0) begin
                serviceCoinType_w = `NTD_10;
              end else begin
                coinOutNTD_50_w = coinOutNTD_50 + 3'd1;
                countNTD_50_w   = countNTD_50 - 3'd1;
                serviceValue_w  = serviceValue - `VALUE_NTD_50;
              end
            end else begin
              serviceCoinType_w = `NTD_10;
            end
          end
          `NTD_10 : begin
            if (serviceValue >= `VALUE_NTD_10) begin
              if (countNTD_10 == 3'd0) begin
                serviceCoinType_w = `NTD_5;
              end else begin
                coinOutNTD_10_w = coinOutNTD_10 + 3'd1;
                countNTD_10_w   = countNTD_10 - 3'd1;
                serviceValue_w  = serviceValue - `VALUE_NTD_10;
              end
            end else begin
              serviceCoinType_w = `NTD_5;
            end
          end
          `NTD_5  : begin
            if (serviceValue >= `VALUE_NTD_5) begin
              if (countNTD_5 == 3'd0) begin
                serviceCoinType_w = `NTD_1;
              end else begin
                coinOutNTD_5_w = coinOutNTD_5 + 3'd1;
                countNTD_5_w   = countNTD_5 - 3'd1;
                serviceValue_w = serviceValue - `VALUE_NTD_5;
              end
            end else begin
              serviceCoinType_w = `NTD_1;
            end
          end
          `NTD_1  : begin
            if (serviceValue >= `VALUE_NTD_1) begin
              if (countNTD_1 == 3'd0) begin
                serviceValue_w    = inputValue;
                itemTypeOut_w     = `ITEM_NONE;
                serviceCoinType_w = `NTD_50;
                countNTD_50_w     = countNTD_50 + coinOutNTD_50;
                countNTD_10_w     = countNTD_10 + coinOutNTD_10;
                countNTD_5_w      = countNTD_5 + coinOutNTD_5;
                countNTD_1_w      = countNTD_1 + coinOutNTD_1;
                coinOutNTD_50_w   = 3'd0;
                coinOutNTD_10_w   = 3'd0;
                coinOutNTD_5_w    = 3'd0;
                coinOutNTD_1_w    = 3'd0;
                serviceTypeOut_w  = `SERVICE_OFF;   // bug 1 
              end else begin
                coinOutNTD_1_w = coinOutNTD_1 + 3'd1;
                countNTD_1_w   = countNTD_1 - 3'd1;
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
      coinOutNTD_50     <= 3'd0;
      coinOutNTD_10     <= 3'd0;
      coinOutNTD_5      <= 3'd0;
      coinOutNTD_1      <= 3'd0;
      itemTypeOut       <= `ITEM_NONE;
      serviceTypeOut    <= `SERVICE_ON;
      countNTD_50       <= 3'd2;
      countNTD_10       <= 3'd2;
      countNTD_5        <= 3'd2;
      countNTD_1        <= 3'd2;
      inputValue        <= 8'd0;
      serviceValue      <= 8'd0;
      serviceCoinType   <= `NTD_50;
      exchangeReady     <= 1'b0;
      initialized       <= 1'b1;
   end
   else begin
      coinOutNTD_50     <= coinOutNTD_50_w;
      coinOutNTD_10     <= coinOutNTD_10_w;
      coinOutNTD_5      <= coinOutNTD_5_w;
      coinOutNTD_1      <= coinOutNTD_1_w;
      itemTypeOut       <= itemTypeOut_w;
      serviceTypeOut    <= serviceTypeOut_w;
      countNTD_50       <= countNTD_50_w;
      countNTD_10       <= countNTD_10_w;
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
