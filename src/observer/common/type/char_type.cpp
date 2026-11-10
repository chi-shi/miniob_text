/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "common/lang/comparator.h"
#include "common/log/log.h"
#include "common/type/char_type.h"
#include "common/type/vector_util.h"
#include "common/value.h"

#include <cstdio>

int CharType::compare(const Value &left, const Value &right) const
{
  ASSERT(left.attr_type() == AttrType::CHARS && right.attr_type() == AttrType::CHARS, "invalid type");
  return common::compare_string(
      (void *)left.value_.pointer_value_, left.length_, (void *)right.value_.pointer_value_, right.length_);
}

RC CharType::set_value_from_str(Value &val, const string &data) const
{
  val.set_string(data.c_str());
  return RC::SUCCESS;
}

RC CharType::cast_to(const Value &val, AttrType type, Value &result) const
{
  switch (type) {
    case AttrType::DATES: {
      // 将 'YYYY-MM-DD' 字符串转成 YYYYMMDD 整数，并校验闰年、大小月；非法则返回错误
      const char *raw = val.value_.pointer_value_;
      if (raw == nullptr) {
        return RC::SCHEMA_FIELD_TYPE_MISMATCH;
      }
      // 去除首尾空白，与 MySQL 对日期字符串的宽容处理保持一致
      string s(raw);
      size_t first = s.find_first_not_of(" \t\r\n");
      if (first == string::npos) {
        return RC::SCHEMA_FIELD_TYPE_MISMATCH;
      }
      size_t last = s.find_last_not_of(" \t\r\n");
      s = s.substr(first, last - first + 1);
      int  y = 0, m = 0, d = 0;
      char extra = 0;
      // 末尾加 %c：若日期后面还有多余字符，sscanf 会返回 4，借此把非法格式也拦掉
      int matched = sscanf(s.c_str(), "%d-%d-%d%c", &y, &m, &d, &extra);
      if (matched != 3) {
        return RC::SCHEMA_FIELD_TYPE_MISMATCH;
      }
      static const int month_days[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
      bool leap = (y % 400 == 0) || (y % 4 == 0 && y % 100 != 0);
      if (y < 1 || y > 9999 || m < 1 || m > 12 || d < 1
          || d > month_days[m] + (m == 2 && leap ? 1 : 0)) {
        return RC::SCHEMA_FIELD_TYPE_MISMATCH;
      }
      int date_value = y * 10000 + m * 100 + d;
      result.set_int(date_value);
      result.set_type(AttrType::DATES);
      return RC::SUCCESS;
    }
    case AttrType::INTS: {
      // 字符串转整数，取前导数字，无法解析时按 0 处理，与 MySQL 隐式转换一致
      result.set_int(val.get_int());
      return RC::SUCCESS;
    }
    case AttrType::FLOATS: {
      result.set_float(val.get_float());
      return RC::SUCCESS;
    }
    case AttrType::VECTORS: {
      // 解析 "[a,b,c]" 形式的字符串为向量
      vector<float> values;
      RC rc = VectorHelper::parse_vector_string(val.get_string(), values);
      if (rc != RC::SUCCESS) {
        return rc;
      }
      result.set_vector(values.data(), static_cast<int>(values.size()));
      return RC::SUCCESS;
    }
    default: return RC::UNIMPLEMENTED;
  }
  return RC::SUCCESS;
}

int CharType::cast_cost(AttrType type)
{
  if (type == AttrType::CHARS) {
    return 0;
  }
  if (type == AttrType::DATES) {
    return 1;
  }
  if (type == AttrType::INTS || type == AttrType::FLOATS) {
    return 2;
  }
  if (type == AttrType::VECTORS) {
    return 2;
  }
  return INT32_MAX;
}

RC CharType::to_string(const Value &val, string &result) const
{
  stringstream ss;
  ss << val.value_.pointer_value_;
  result = ss.str();
  return RC::SUCCESS;
}