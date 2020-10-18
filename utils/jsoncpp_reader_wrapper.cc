#include "jsoncpp_reader_wrapper.h"
#include "Log.h"

namespace utils {

JsonReader::JsonReader() {
  reader_ = std::unique_ptr<Json::CharReader>(reader_builder_.newCharReader());
}

bool JsonReader::parse(const std::string& json, Json::Value* root) {
  LOGD("JsonReader::%s()", __func__);
  JSONCPP_STRING err;
  bool is_parsing_ok = false;
  const size_t json_len = json.length();
  try {
    is_parsing_ok =
        reader_->parse(json.c_str(), json.c_str() + json_len, root, &err);
  } catch (Json::RuntimeError& e) {
    LOGD("JsonReader::%s() Exception caught during parse json: %s", __func__, e.what());
    return false;
  }

  if (!is_parsing_ok) {
    LOGD("JsonReader::%s() Json parsing fails: %s", __func__, err.c_str());
  }
  return is_parsing_ok;
}
}  // namespace utils
