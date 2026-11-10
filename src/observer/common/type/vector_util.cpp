/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of the Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "common/type/vector_util.h"

#include <cstdlib>
#include <cctype>
#include <sstream>

RC VectorHelper::parse_vector_string(const string &raw, vector<float> &output)
{
  output.clear();

  // 去除所有空白字符
  string s;
  s.reserve(raw.size());
  for (char c : raw) {
    if (isspace(static_cast<unsigned char>(c)) == 0) {
      s.push_back(c);
    }
  }

  if (s.size() < 2 || s.front() != '[' || s.back() != ']') {
    return RC::INVALID_ARGUMENT;
  }

  string body = s.substr(1, s.size() - 2);
  if (body.empty()) {
    return RC::INVALID_ARGUMENT;
  }

  size_t pos = 0;
  while (pos < body.size()) {
    size_t comma = body.find(',', pos);
    string token = (comma == string::npos) ? body.substr(pos) : body.substr(pos, comma - pos);
    if (token.empty()) {
      return RC::INVALID_ARGUMENT;
    }

    const char *cstr = token.c_str();
    char       *end  = nullptr;
    float       val  = strtof(cstr, &end);
    if (end == cstr || *end != '\0') {
      return RC::INVALID_ARGUMENT;
    }
    output.push_back(val);

    if (comma == string::npos) {
      break;
    }
    pos = comma + 1;
  }

  return output.empty() ? RC::INVALID_ARGUMENT : RC::SUCCESS;
}

string VectorHelper::to_vector_string(const float *data, int dim)
{
  string result = "[";
  for (int i = 0; i < dim; ++i) {
    if (i > 0) {
      result += ",";
    }
    stringstream ss;
    ss << data[i];
    result += ss.str();
  }
  result += "]";
  return result;
}
