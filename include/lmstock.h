#ifndef INCLUDE_LMSTOCK_H_
#define INCLUDE_LMSTOCK_H_

/** Copyright 2018 He Hao<hehaoslj@sina.com> */

#pragma pack(1)

struct lmtickdata {
  // 股票代码
  int nStockCode;
  // 时间 格式HHMMSS
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

  // 成交量
  double Volume;
  // 成交金额
  double Amount;

  // 买盘 改为向量化
  // PriceLevel buyPriceS[10];
  double buyPriceS[10];
  double buyVolumes[10];

  // 卖盘 改为向量化
  // PriceLevel sellPriceS[10];
  double sellPriceS[10];
  double sellVolumeS[10];
};

struct lmkdata {
  enum eKType {
    Min_1,
    Min_5,
    Min_15,
    Min_30,
    Min_60,
    Min_120,
    DayK,
  };

  eKType nType;
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
