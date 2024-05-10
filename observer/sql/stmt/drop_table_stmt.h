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
// Created by Wangyunlai on 2023/6/13.
//

#pragma once

#include <string>
#include <vector>

#include "sql/stmt/stmt.h"

class Db;

/**
 * @brief 表示删除表的语句
 * @ingroup Statement
 * @details 虽然解析成了stmt，但是与原始的SQL解析后的数据也差不多
 */
class DropTableStmt : public Stmt
{
public:
  /**
   * @brief 构造函数
   * @param table_name 要删除的表的名称
   */
  DropTableStmt(const std::string &table_name)
      : table_name_(table_name) {}

  /**
   * @brief 虚拟析构函数
   */
  virtual ~DropTableStmt() = default;

  /**
   * @brief 获取语句类型
   * @return 返回语句类型，此处为StmtType::DROP_TABLE
   */
  StmtType type() const override { return StmtType::DROP_TABLE; }

  /**
   * @brief 获取要删除的表的名称
   * @return 返回表的名称的引用
   */
  const std::string                  &table_name() const { return table_name_; }

  /**
   * @brief 执行删除表的操作
   * @param db 指向数据库的指针
   * @param drop_table 删除表的SQL节点
   * @param stmt 删除操作成功后，返回的语句指针
   * @return 返回操作的结果码
   */
  static RC Drop(Db *db, const DropTableSqlNode &drop_table, Stmt *&stmt);

  static RC create(Db *db, const DropTableSqlNode &create_table, Stmt *&stmt);
private:
  std::string                  table_name_; // 要删除的表的名称
};