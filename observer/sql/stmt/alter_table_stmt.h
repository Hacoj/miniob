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


#pragma once

#include <string>
#include <vector>

#include "sql/stmt/stmt.h"
#include 

/**
 * @brief 修改表的语句
 * @ingroup Statement
 * @details 虽然解析成了stmt，但是与原始的SQL解析后的数据也差不多
 */
class ALterTableStmt : public Stmt
{
public:
  ALterTableStmt(const std::string &table_name, const std::vector<AttrInfoSqlNode> &attr_infos) : table_name_(table_name) {}
  virtual ~ALterTableStmt() = default;

  StmtType type() const override { return StmtType::ALTER_TABLE; }

  const std::string &table_name() const { return table_name_; }
  const std::vector<AttrInfoSqlNode> &attr_infos() const { return attr_infos_; }

private:
  std::vector<AttrInfoSqlNode> attr_infos_;
  std::string table_name_;
};
