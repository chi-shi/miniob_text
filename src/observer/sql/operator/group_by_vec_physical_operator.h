/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#pragma once

#include "common/lang/memory.h"
#include "sql/expr/aggregate_hash_table.h"
#include "sql/operator/physical_operator.h"

/**
 * @brief Group By 物理算子(vectorized)
 * @ingroup PhysicalOperator
 */
class GroupByVecPhysicalOperator : public PhysicalOperator
{
public:
  GroupByVecPhysicalOperator(vector<unique_ptr<Expression>> &&group_by_exprs, vector<Expression *> &&expressions);

  virtual ~GroupByVecPhysicalOperator() = default;

  PhysicalOperatorType type() const override { return PhysicalOperatorType::GROUP_BY_VEC; }

  RC open(Trx *trx) override;
  RC next(Chunk &chunk) override;
  RC close() override;

private:
  vector<unique_ptr<Expression>> group_by_exprs_;  ///< 分组表达式
  vector<Expression *>           aggregate_exprs_;  ///< 聚合表达式（不持有所有权）
  vector<Expression *>           value_exprs_;      ///< 聚合函数的输入表达式
  unique_ptr<StandardAggregateHashTable> hash_table_;
  unique_ptr<AggregateHashTable::Scanner> scanner_;

  Chunk input_chunk_;
  Chunk output_chunk_;
};
