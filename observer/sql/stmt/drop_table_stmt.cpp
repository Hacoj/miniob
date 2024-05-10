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
// Created by Hacoj on 2024/4/25.
//


#include "sql/stmt/drop_table_stmt.h"
#include "event/sql_debug.h"
#include "drop_table_stmt.h"


/**
 * 函数名: DropTableStmt::Drop
 * 功能: 执行删除表的操作
 * 参数:
 *   - db: 指向数据库的指针，用于执行删除操作。
 *   - drop_table: 包含要删除的表信息的DropTableSqlNode对象。
 *   - stmt: 引用，用于返回执行删除操作的语句对象。
 * 返回值: 返回操作的结果状态码，RC::SUCCESS表示成功。
 */
RC DropTableStmt::Drop(Db *db, const DropTableSqlNode &drop_table, Stmt *&stmt) 
{ 
  // 创建一个新的DropTableStmt对象
  stmt = new DropTableStmt(drop_table.relation_name);
  // 调用debug函数，输出删除表的语句信息
  sql_debug("drop table statement: table name %s", drop_table.relation_name.c_str());

  return RC::SUCCESS;
}

RC DropTableStmt::create(Db *db, const DropTableSqlNode &drop_table, Stmt *&stmt) { 
  stmt = new DropTableStmt(drop_table.relation_name);
  sql_debug("drop table statement: table name %s", drop_table.relation_name.c_str());

  return RC::SUCCESS; 
}
