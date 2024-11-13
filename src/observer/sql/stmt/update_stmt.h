/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

//
// Created by Wangyunlai on 2022/5/22.
//

#pragma once

#include "common/rc.h"
#include "sql/stmt/stmt.h"
#include "storage/field/field_meta.h"
#include "common/log/log.h"
#include "sql/stmt/filter_stmt.h"
#include "storage/db/db.h"
#include "storage/table/table.h"
class Table;

/**
 * @brief 更新语句
 * @ingroup Statement
 */
class UpdateStmt : public Stmt
{
public:
  UpdateStmt() = default;
  //UpdateStmt(Table *table, Value *values, int value_amount, FilterStmt *filter_stmt);
  UpdateStmt(Table *table,const FieldMeta* field,const Value value, FilterStmt *filter_stmt)
    : table_(table), field_(field), value_(value), filter_stmt_(filter_stmt)
  {}
  // ~UpdateStmt()
  // {
  //   if (nullptr != filter_stmt_) {
  //     delete filter_stmt_;
  //     filter_stmt_ = nullptr;
  //   }
  // }
  StmtType type() const override { return StmtType::UPDATE; }
public:
  static RC create(Db *db, const UpdateSqlNode &update_sql, Stmt *&stmt);

public:
  Table *table() const { return table_; }
  //Value *values() const { return values_; }
  //int    value_amount() const { return value_amount_; }
  const FieldMeta *field() const {return field_;}
  Value value() const {return value_;}
  FilterStmt *filter_stmt() const { return filter_stmt_; }
  

private:
  Table *table_        = nullptr;
  //Value *values_       = nullptr;
  //int    value_amount_ = 0;
  const FieldMeta* field_     = nullptr;
  Value value_;
  FilterStmt *filter_stmt_ = nullptr;
};
