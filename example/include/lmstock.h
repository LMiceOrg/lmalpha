/** Copyright 2018, 2019 He Hao<hehaoslj@sina.com> */

#ifndef INCLUDE_LMSTOCK_H_
#define INCLUDE_LMSTOCK_H_

#ifdef __cplusplus
#include <iostream>
#include <utility>
#include <vector>

#endif

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

  //#ifdef __cplusplus
  //  friend std::ostream& operator<<(std::ostream& os, const lmtickdata& tick);
  //#endif
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

  //#ifdef __cplusplus
  //  friend std::ostream& operator<<(std::ostream& os, const lmkdata& tick);
  //#endif
};

#pragma pack()

#ifdef __cplusplus

// typedef std::pair<int, std::vector<lmtickdata> > lmtickdayserial;
// typedef std::vector<lmtickdayserial> lmtickserial;

/** C++ ostream healer */

std::ostream& operator<<(std::ostream& os, const lmtickdata& tick) {
  os << tick.Time << " " << tick.nStockCode /** time code */
     << "\tLastPrice=" << tick.LastPrice    /** last price */
     << "\tVol=" << tick.Volume             /** vol */
     << "\tDM=" << tick.Amount              /** dm */
     << "\tbuyLevel=" << tick.buyLevel      /** buy level */
     << "\tsellLevel=" << tick.sellLevel;   /** sell level */
  return os;
}

std::ostream& operator<<(std::ostream& os, const lmkdata& kdata) {
  os << kdata.nType << " " << kdata.nCode       /** type code */
     << "\t" << kdata.nDate << kdata.nTimeBegin /** date time */
     << "\tpreClose==" << kdata.preClosePrice   /** pre close */
     << " [" << kdata.openPrice << "," << kdata.lowPrice << ","
     << kdata.highPrice << "," << kdata.closePrice
     << "]\tv=" /** open lh close */
     << kdata.volume << "\tm=" << kdata.money << "\tdc=" << kdata.dealCount;
  return os;
}

namespace lmapi {

template <class TSubClass>
struct LMBaseClass {
  typedef LMBaseClass<TSubClass> this_type;

  inline size_t bytes() const {
    const TSubClass* p = static_cast<const TSubClass*>(this);
    return p->OnBytes();
  }

  inline bool is_pod() const { return std::is_pod<TSubClass>::value; }

  inline void clear() {
    TSubClass* p = static_cast<TSubClass*>(this);
    p->OnClear();
  }

  inline int unpack(const char* buffer, size_t buffer_size) {
    TSubClass* p = static_cast<TSubClass*>(this);
    return p->OnUnpack(buffer, buffer_size);
  }

  inline int pack(char* buffer, size_t buffer_size) const {
    const TSubClass* p = static_cast<const TSubClass*>(this);
    return p->OnPack(buffer, buffer_size);
  }

  inline const char* data() const {
    // 只提供 POD类型时的访问
    static_assert(std::is_pod<TSubClass>::value, "data method");
    return reinterpret_cast<const char*>(this);
  }

  /** pod default method */
  size_t OnBytes() const {
    // 总是 返回 类型的大小
    //如果是可变长度类型，需要用户重载此函数
    static_assert(std::is_pod<TSubClass>::value, "OnBytes");
    return sizeof(TSubClass);
  }

  int OnPack(char* buffer, size_t buffer_size) const {
    int ret = -1;
    // 非POD类型，以及buffer太小情况的pack处理，由用户实现处理
    static_assert(std::is_pod<TSubClass>::value, "OnPack");
    if (bytes() <= buffer_size) {
      memcpy(buffer, (char*)this, bytes());
      ret = 0;
    }
    return ret;
  }

  int OnUnpack(const char* buffer, size_t buffer_size) {
    int ret = -1;
    // 非POD类型，以及buffer_size太小情况的unpack处理，由用户实现处理
    static_assert(std::is_pod<TSubClass>::value, "OnUnpack");
    if (bytes() <= buffer_size) {
      memcpy((char*)this, buffer, bytes());
      ret = 0;
    }

    return ret;
  }

  void OnClear() {
    //在POD类型时，调用memset初始化
    //非POD类型，用户重载此函数
    static_assert(std::is_pod<TSubClass>::value, "OnClear");
    memset(this, 0, bytes());
  }

#if __cplusplus >= 199711L
  // c++0x 标准扩展
#endif

#if __cplusplus >= 201103L
  // c++11 标准扩展
 protected:
  // 不允许直接实例化基类
  LMBaseClass() = default;
#endif
};

template <class TSubClass>
struct LMVectorData : public std::vector<TSubClass> {
  typedef LMVectorData<TSubClass> this_type;

  inline bool is_pod() const { return std::is_pod<TSubClass>::value; }
};

template <class TSubClass, bool t>
struct LMBaseVector {};

template <class TSubClass>
struct LMBaseVector<TSubClass, true> : public LMVectorData<TSubClass> {
  typedef LMBaseVector<TSubClass, true> this_type;

  inline size_t bytes() const {
    const this_type* p = static_cast<const this_type*>(this);
    return sizeof(TSubClass) * p->size() + sizeof(size_t) * 2;
  }

  ///<[字节数] [数量] [数据项1],...[数据项n]
  inline int pack(char* buffer, size_t buffer_size) const {
    int ret = -1;
    size_t pos = 0;
    size_t sz = bytes();
    const this_type* p = static_cast<const this_type*>(this);

    if (sz <= buffer_size) {
      ret = 0;

      memcpy(buffer + pos, &sz, sizeof(sz));
      pos += sizeof(sz);

      sz = p->size();
      memcpy(buffer + pos, &sz, sizeof(sz));
      pos += sizeof(sz);

      if (p->size() > 0) {
        memcpy(buffer + pos, &(*p)[0], sizeof(TSubClass) * p->size());
      }
    }
    return ret;
  }

  inline int unpack(const char* buffer, size_t buffer_size) {
    size_t sz = 0;
    size_t pos = sizeof(size_t) * 2;
    size_t cnt = 0;
    this_type* p = static_cast<this_type*>(this);

    p->clear();

    if (buffer_size < pos) return -1;

    sz = *(const size_t*)buffer;
    cnt = *(const size_t*)(buffer + sizeof(size_t));

    if (sz > buffer_size) {
      return -1;
    }

    p->resize(cnt);
    memcpy(&(*p)[0], buffer + pos, sizeof(TSubClass) * cnt);

    return 0;
  }
};

template <class TSubClass>
struct LMBaseVector<TSubClass, false> : public LMVectorData<TSubClass> {
  typedef LMBaseVector<TSubClass, false> this_type;
  inline int bytes() const {
    int sz = sizeof(int) * 2;
    const this_type* p = static_cast<const this_type*>(this);
    for (size_t i = 0; i < p->size(); ++i) {
      const TSubClass* pobj = &(*p)[i];
      sz += pobj->bytes();
    }
    return sz;
  }

  inline int pack(char* buffer, size_t buffer_size) const {
    int ret = -1;
    size_t pos = 0;
    size_t sz = bytes();
    const this_type* p = static_cast<const this_type*>(this);

    if (sz <= buffer_size) {
      ret = 0;

      memcpy(buffer + pos, &sz, sizeof(sz));
      pos += sizeof(sz);

      sz = p->size();
      memcpy(buffer + pos, &sz, sizeof(sz));
      pos += sizeof(sz);

      for (size_t i = 0; i < p->size(); ++i) {
        const TSubClass& obj = (*p)[i];
        obj.pack(buffer + pos, buffer_size);
        pos += obj.bytes();
      }
    }
    return ret;
  }

  inline int unpack(const char* buffer, size_t buffer_size) {
    size_t sz = 0;
    size_t pos = sizeof(size_t) * 2;
    size_t i;
    size_t cnt = 0;
    this_type* p = static_cast<this_type*>(this);

    p->clear();

    if (buffer_size < pos) return -1;

    sz = *(const size_t*)buffer;
    cnt = *(const size_t*)(buffer + sizeof(size_t));

    if (sz > buffer_size) {
      return -1;
    }

    p->resize(cnt);

    sz -= pos;
    for (i = 0; i < cnt; ++i) {
      TSubClass& pt = *(&p)[i];
      pt.unpack(buffer + pos, sz);
      pos += pt.bytes();

      if (sz < pt.bytes()) {
        return -1;
      }
      sz -= pt.bytes();
    }

    return 0;
  }
};

//可变长类型
template <class TSubClass>
struct LMVector
    : public LMBaseVector<TSubClass, std::is_pod<TSubClass>::value> {};

struct lmkdataserial : public LMVector<lmkdata> {};

// typedef std::pair<int, std::vector<lmtickdata> > lmtickdayserial;
// typedef std::vector<lmtickdayserial> lmtickserial;

struct lmtickdayserial : public LMBaseClass<lmtickdayserial> {
  int first;
  LMVector<lmtickdata> second;

  lmtickdayserial(int dt, const std::vector<lmtickdata>& vec) {
    first = dt;
    for (size_t i = 0; i < vec.size(); ++i) {
      second.push_back(vec[i]);
    }
  }

  size_t OnBytes() const { return sizeof(int) + second.bytes(); }

  int OnUnpack(const char* buffer, size_t buffer_size) {
    size_t pos = 0;

    if (buffer_size < sizeof(first) + sizeof(size_t) * 2) return -1;

    first = *reinterpret_cast<const int*>(buffer);

    pos += sizeof(first);
    buffer_size -= 4;
    int ret = second.unpack(buffer + pos, buffer_size);
    return ret;
  }

  int OnPack(char* buffer, size_t buffer_size) const {
    if (bytes() <= buffer_size) {
      *reinterpret_cast<int*>(buffer) = first;
      second.pack(buffer + sizeof(first), buffer_size - sizeof(first));
      return 0;
    }
    return -1;
  }
};

struct lmtickserial : public LMVector<lmtickdayserial> {};

}  // namespace lmapi

#endif

#endif  // INCLUDE_LMSTOCK_H_
