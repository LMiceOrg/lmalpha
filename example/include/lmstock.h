/** Copyright 2018, 2019 He Hao<hehaoslj@sina.com> */

#ifndef INCLUDE_LMSTOCK_H_
#define INCLUDE_LMSTOCK_H_

#pragma pack(1)

struct lmtickdata {
  // 股票代码
  int nStockCode;
  // 时间 格式HHMMSS000
  int Time;
  // 买盘有效价格为几档，代表buyPriceS数组中前几位有效
  int buyLevel;
  // 卖盘有效价格为几档，代表sellPriceS数组中前几位有效
  int sellLevel;

  double LastPrice;  // 现价
  double OpenPrice;  // 开盘价
  /* double ClosePrice; 收盘价----- */
  double HighPrice;  // 最高价
  double LowPrice;   // 最低价

  double TradeCount;             // 成交笔数
  double TotalBidVolume;         // 申买总量
  double WeightedAvgBidPrice;    // 申买加权均价
  double TotalOfferVolume;       // 申卖总量
  double WeightedAvgOfferPrice;  // 申卖加权均价

  double buyPriceS[10];   //十档买单价格
  double buyVolumeS[10];  //十档买单量
  double buyCountS[10];   //十档买单笔数

  double sellPriceS[10];  //十档卖单价格
  double sellVolume[10];  //十档卖单量
  double sellCountS[10];  //十档卖单笔数

  //成交量
  double Volume;
  //成交金额
  double Amount;
};

struct lmkdata {
  int nType;
  int nCode;

  int nDate;       // yyyymmdd
  int nTimeBegin;  // HHMMSS

  double preClosePrice;
  double openPrice;
  double lowPrice;
  double highPrice;
  double closePrice;

  double volume;  //成交量 股数
  double money;   //成交金额

  double dealCount;  //成交笔数
};

#pragma pack()

#endif  // INCLUDE_LMSTOCK_H_
