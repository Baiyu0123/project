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

#include "sql/stmt/delete_stmt.h"
#include "common/log/log.h"
#include "sql/stmt/filter_stmt.h"
#include "storage/db/db.h"
#include "storage/table/table.h"
#include "sql/parser/expression_binder.h"

DeleteStmt::DeleteStmt(Table *table, FilterStmt *filter_stmt) : table_(table), filter_stmt_(filter_stmt) {}

DeleteStmt::~DeleteStmt()
{
  if (nullptr != filter_stmt_) {
    delete filter_stmt_;
    filter_stmt_ = nullptr;
  }
}

RC DeleteStmt::create(Db *db, DeleteSqlNode &delete_sql, Stmt *&stmt)
{
  const char *table_name = delete_sql.relation_name.c_str();
  if (nullptr == db || nullptr == table_name) {
    LOG_WARN("invalid argument. db=%p, table_name=%p", db, table_name);
    return RC::INVALID_ARGUMENT;
  }

  // check whether the table exists
  Table *table = db->find_table(table_name);
  if (nullptr == table) {
    LOG_WARN("no such table. db=%s, table_name=%s", db->name(), table_name);
    return RC::SCHEMA_TABLE_NOT_EXIST;
  }
  BinderContext binder_context;

  std::unordered_map<std::string, Table *> table_map;
  table_map.insert(std::pair<std::string, Table *>(std::string(table_name), table));
  binder_context.add_table(table);

  vector<unique_ptr<Expression>> bound_condition_expressions;
  ExpressionBinder expression_binder(binder_context);
  
  for (ConditionSqlNode &condition : delete_sql.conditions) {
    unique_ptr<Expression> left_expr_(condition.left_expr);
    RC rc = expression_binder.bind_expression(left_expr_, bound_condition_expressions);
    if (OB_FAIL(rc)) {
      LOG_INFO("bind expression failed. rc=%s", strrc(rc));
      return rc;
    }
    unique_ptr<Expression> new_left_expr=std::move(bound_condition_expressions.back());
    condition.left_expr=new_left_expr.release();
    
    unique_ptr<Expression> right_expr_(condition.right_expr);
    rc = expression_binder.bind_expression(right_expr_, bound_condition_expressions);
    if (OB_FAIL(rc)) {
      LOG_INFO("bind expression failed. rc=%s", strrc(rc));
      return rc;
    }
    unique_ptr<Expression> new_right_expr=std::move(bound_condition_expressions.back());
    condition.right_expr=new_right_expr.release();
  }
  FilterStmt *filter_stmt = nullptr;
  RC          rc          = FilterStmt::create(
      db, table, &table_map, delete_sql.conditions.data(), static_cast<int>(delete_sql.conditions.size()), filter_stmt);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to create filter statement. rc=%d:%s", rc, strrc(rc));
    return rc;
  }

  stmt = new DeleteStmt(table, filter_stmt);
  return rc;
}
