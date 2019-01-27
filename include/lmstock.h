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

#ifdef __cplusplus
#include <vector>
// typedef std::pair<int, std::vector<lmtickdata> > lmtickdayserial;
// typedef std::vector<lmtickdayserial> lmtickserial;

namespace lmapi {

template <class TSubClass>
struct LMBaseClass {
  typedef LMBaseClass<TSubClass> this_type;

  inline size_t size() const {
    const TSubClass* p = static_cast<const TSubClass*>(this);
    return p->OnSize();
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
  size_t OnSize() const {
    // 总是 返回 类型的大小
    //如果是可变长度类型，需要用户重载此函数
    static_assert(std::is_pod<TSubClass>::value, "OnSize");
    return sizeof(TSubClass);
  }

  int OnPack(char* buffer, size_t buffer_size) const {
    int ret = -1;
    // 非POD类型，以及buffer太小情况的pack处理，由用户实现处理
    static_assert(std::is_pod<TSubClass>::value, "OnPack");
    if (size() <= buffer_size) {
      memcpy(buffer, (char*)this, size());
      ret = 0;
    }
    return ret;
  }

  int OnUnpack(const char* buffer, size_t buffer_size) {
    int ret = -1;
    // 非POD类型，以及buffer_size太小情况的unpack处理，由用户实现处理
    static_assert(std::is_pod<TSubClass>::value, "OnUnpack");
    if (size() <= buffer_size) {
      memcpy((char*)this, buffer, size());
      ret = 0;
    }

    return ret;
  }

  void OnClear() {
    //在POD类型时，调用memset初始化
    //非POD类型，用户重载此函数
    static_assert(std::is_pod<TSubClass>::value, "OnClear");
    memset(this, 0, size());
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
struct LMVectorData {
  inline size_t count() const { return m_vec.size(); }

  inline bool is_pod() const { return std::is_pod<TSubClass>::value; }

  inline void clear() { m_vec.clear(); }

  inline void push_back(const TSubClass& o) { m_vec.push_back(o); }

  inline const TSubClass& operator[](size_t pos) const { return m_vec[pos]; }

  inline TSubClass& operator[](size_t pos) { return m_vec[pos]; }

  std::vector<TSubClass> m_vec;
};

template <class TSubClass, bool t>
struct LMBaseVector {};

template <class TSubClass>
struct LMBaseVector<TSubClass, true> : public LMVectorData<TSubClass> {
  typedef LMBaseVector<TSubClass, true> this_type;

  inline size_t size() const {
    const this_type* p = static_cast<const this_type*>(this);
    return sizeof(TSubClass) * p->count() + sizeof(size_t) * 2;
  }

  ///<[字节数] [数量] [数据项1],...[数据项n]
  inline int pack(char* buffer, size_t buffer_size) const {
    int ret = -1;
    size_t pos = 0;
    size_t sz = size();
    const this_type* p = static_cast<const this_type*>(this);

    if (sz <= buffer_size) {
      ret = 0;

      memcpy(buffer + pos, &sz, sizeof(sz));
      pos += sizeof(sz);

      sz = p->count();
      memcpy(buffer + pos, &sz, sizeof(sz));
      pos += sizeof(sz);

      if (p->m_vec.size() > 0) {
        memcpy(buffer + pos, &(p->m_vec)[0],
               sizeof(TSubClass) * p->m_vec.size());
      }
    }
    return ret;
  }

  inline int unpack(const char* buffer, size_t buffer_size) {
    size_t sz = 0;
    size_t pos = sizeof(size_t) * 2;
    size_t cnt = 0;
    this_type* p = static_cast<this_type*>(this);

    p->m_vec.clear();

    if (buffer_size < pos) return -1;

    sz = *(const size_t*)buffer;
    cnt = *(const size_t*)(buffer + sizeof(size_t));

    if (sz > buffer_size) {
      return -1;
    }

    p->m_vec.resize(cnt);
    memcpy(&(p->m_vec)[0], buffer + pos, sizeof(TSubClass) * cnt);

    return 0;
  }
};

template <class TSubClass>
struct LMBaseVector<TSubClass, false> : public LMVectorData<TSubClass> {
  typedef LMBaseVector<TSubClass, false> this_type;
  inline int size() const {
    int sz = sizeof(int) * 2;
    const this_type* p = static_cast<const this_type*>(this);
    for (size_t i = 0; i < p->m_vec.size(); ++i) {
      const TSubClass* pobj = &(p->m_vec)[i];
      sz += pobj->size();
    }
    return sz;
  }

  inline int pack(char* buffer, size_t buffer_size) const {
    int ret = -1;
    size_t pos = 0;
    size_t sz = size();
    const this_type* p = static_cast<const this_type*>(this);

    if (sz <= buffer_size) {
      ret = 0;

      memcpy(buffer + pos, &sz, sizeof(sz));
      pos += sizeof(sz);

      sz = p->count();
      memcpy(buffer + pos, &sz, sizeof(sz));
      pos += sizeof(sz);

      for (size_t i = 0; i < p->m_vec.size(); ++i) {
        const TSubClass& obj = p->m_vec[i];
        obj.pack(buffer + pos, buffer_size);
        pos += obj.size();
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

    p->m_vec.clear();

    if (buffer_size < pos) return -1;

    sz = *(const size_t*)buffer;
    cnt = *(const size_t*)(buffer + sizeof(size_t));

    if (sz > buffer_size) {
      return -1;
    }

    p->m_vec.resize(cnt);

    sz -= pos;
    for (i = 0; i < cnt; ++i) {
      TSubClass& p = p->m_vec[i];
      p.unpack(buffer + pos, sz);
      pos += p.size();

      if (sz < p.size()) {
        return -1;
      }
      sz -= p.size();
      // m_vec.push_back(p);
    }

    return 0;
  }
};

//可变长类型
template <class TSubClass>
struct LMVector
    : public LMBaseVector<TSubClass, std::is_pod<TSubClass>::value> {};

struct lmkdatas : public LMBaseClass<lmkdatas> {
  LMVector<lmkdata> m_kdata;
  int i;

  size_t OnSize() const { return m_kdata.size(); }

  int OnUnpack(const char* buffer, size_t buffer_size) {
    int ret = m_kdata.unpack(buffer, buffer_size);
    return ret;
  }

  int OnPack(char* buffer, size_t buffer_size) const {
    if (size() <= buffer_size) {
      m_kdata.pack(buffer, buffer_size);
      return 0;
    }
    return -1;
  }
};

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

  size_t OnSize() const { return sizeof(int) + second.size(); }

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
    if (size() <= buffer_size) {
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
