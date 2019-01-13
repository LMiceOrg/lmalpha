#include "lmapi.h"

namespace lmapi {

struct serial_internal {
  enum serial_type {
    tick_type,
    min1_type,
    min5_type,
    min15_type,
    min30_type,
    min60_type,
    min120_type
  };
  int type;
  std::vector<std::string> root_paths;
};

serial_dataset::serial_dataset(int tp, const std::string& instrument,
                               int start_date, int end_date) {}

serial_dataset::~serial_dataset() {}

}  // namespace lmapi
