/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of the Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#pragma once

#include "common/sys/rc.h"
#include "common/lang/string.h"
#include "common/lang/vector.h"

/**
 * @brief 向量字符串与浮点数组之间的转换工具
 * @ingroup DataType
 */
class VectorHelper
{
public:
  /// 解析 "[a,b,c]" 形式的字符串为浮点数组，格式非法时返回错误
  static RC parse_vector_string(const string &text, vector<float> &output);

  /// 把浮点数组格式化为 "[a,b,c]" 形式的字符串
  static string to_vector_string(const float *data, int dim);
};
