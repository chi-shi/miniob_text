/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of the Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "common/type/vector_type.h"

#include "common/value.h"
#include "common/type/vector_util.h"

#include <cmath>

int VectorType::compare(const Value &left, const Value &right) const
{
  if (right.attr_type() != AttrType::VECTORS) {
    return INT32_MAX;
  }

  int left_dim = left.length();
  int right_dim = right.length();
  if (left_dim != right_dim) {
    return 1;
  }

  const float *l = left.get_vector();
  const float *r = right.get_vector();
  for (int i = 0; i < left_dim; ++i) {
    if (l[i] != r[i]) {
      return 1;
    }
  }
  return 0;
}

RC VectorType::cast_to(const Value &val, AttrType type, Value &result) const
{
  switch (type) {
    case AttrType::VECTORS: {
      result = val;
      return RC::SUCCESS;
    }
    case AttrType::CHARS: {
      string str;
      RC rc = to_string(val, str);
      if (rc != RC::SUCCESS) {
        return rc;
      }
      result.set_string(str.c_str());
      return RC::SUCCESS;
    }
    default: return RC::UNSUPPORTED;
  }
}

int VectorType::cast_cost(AttrType type)
{
  if (type == AttrType::VECTORS || type == AttrType::CHARS) {
    return 1;
  }
  return INT32_MAX;
}

RC VectorType::to_string(const Value &val, string &result) const
{
  result = VectorHelper::to_vector_string(val.get_vector(), val.length());
  return RC::SUCCESS;
}
