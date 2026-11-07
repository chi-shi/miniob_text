/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "common/log/log.h"
#include "sql/operator/group_by_vec_physical_operator.h"

GroupByVecPhysicalOperator::GroupByVecPhysicalOperator(
    vector<unique_ptr<Expression>> &&group_by_exprs, vector<Expression *> &&expressions)
{
  group_by_exprs_  = std::move(group_by_exprs);
  aggregate_exprs_ = std::move(expressions);

  value_exprs_.reserve(aggregate_exprs_.size());
  for (Expression *expr : aggregate_exprs_) {
    ASSERT(expr->type() == ExprType::AGGREGATION, "expect aggregate expression");
    auto *aggregate_expr = static_cast<AggregateExpr *>(expr);
    value_exprs_.push_back(aggregate_expr->child().get());
  }

  hash_table_ = make_unique<StandardAggregateHashTable>(aggregate_exprs_);

  int col_id = 0;
  for (auto &expr : group_by_exprs_) {
    output_chunk_.add_column(make_unique<Column>(expr->value_type(), expr->value_length()), col_id);
    col_id++;
  }
  for (Expression *expr : aggregate_exprs_) {
    output_chunk_.add_column(make_unique<Column>(expr->value_type(), expr->value_length()), col_id);
    col_id++;
  }
}

RC GroupByVecPhysicalOperator::open(Trx *trx)
{
  ASSERT(children_.size() == 1, "group by operator only support one child, but got %d", children_.size());

  PhysicalOperator &child = *children_[0];
  RC                rc    = child.open(trx);
  if (OB_FAIL(rc)) {
    LOG_INFO("failed to open child operator. rc=%s", strrc(rc));
    return rc;
  }

  while (OB_SUCC(rc = child.next(input_chunk_))) {
    Chunk groups_chunk;
    Chunk aggrs_chunk;

    for (size_t i = 0; i < group_by_exprs_.size(); i++) {
      auto group_column = make_unique<Column>();
      rc = group_by_exprs_[i]->get_column(input_chunk_, *group_column);
      if (OB_FAIL(rc)) {
        LOG_WARN("failed to get group by column. rc=%s", strrc(rc));
        return rc;
      }
      groups_chunk.add_column(std::move(group_column), i);
    }

    for (size_t i = 0; i < value_exprs_.size(); i++) {
      auto aggr_column = make_unique<Column>();
      rc = value_exprs_[i]->get_column(input_chunk_, *aggr_column);
      if (OB_FAIL(rc)) {
        LOG_WARN("failed to get aggregate value column. rc=%s", strrc(rc));
        return rc;
      }
      aggrs_chunk.add_column(std::move(aggr_column), i);
    }

    rc = hash_table_->add_chunk(groups_chunk, aggrs_chunk);
    if (OB_FAIL(rc)) {
      LOG_WARN("failed to add chunk to aggregate hash table. rc=%s", strrc(rc));
      return rc;
    }
  }

  if (rc == RC::RECORD_EOF) {
    rc = RC::SUCCESS;
  }
  if (OB_FAIL(rc)) {
    LOG_WARN("failed to scan child operator. rc=%s", strrc(rc));
    return rc;
  }

  scanner_ = make_unique<StandardAggregateHashTable::Scanner>(hash_table_.get());
  scanner_->open_scan();

  return rc;
}

RC GroupByVecPhysicalOperator::next(Chunk &chunk)
{
  output_chunk_.reset_data();
  RC rc = scanner_->next(output_chunk_);
  if (OB_FAIL(rc)) {
    return rc;
  }
  return chunk.reference(output_chunk_);
}

RC GroupByVecPhysicalOperator::close()
{
  if (scanner_ != nullptr) {
    scanner_->close_scan();
    scanner_.reset();
  }
  children_[0]->close();
  LOG_INFO("close group by operator");
  return RC::SUCCESS;
}
