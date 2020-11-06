#ifndef UTILS_JSONCPP_READER_WRAPPER_
#define UTILS_JSONCPP_READER_WRAPPER_

#include <memory>
#include <string>
#include "json/reader.h"

namespace utils {

class JsonReader {
 public:
  JsonReader();
  bool parse(const std::string& json, Json::Value* root);

 private:
  std::unique_ptr<Json::CharReader> reader_;
  Json::CharReaderBuilder reader_builder_;
};
}  // namespace utils
#endif  // UTILS_JSONCPP_READER_WRAPPER_
