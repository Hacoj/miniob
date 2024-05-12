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

/**
 * @brief 修改表的语句
 * @ingroup Statement
 * @details 虽然解析成了stmt，但是与原始的SQL解析后的数据也差不多
 */
class AlterStmt : public Stmt
{
public:
    AlterStmt(const std::string &relation_name, const std::string &object, const std::string &operation, const std::vector<AttrInfoSqlNode> &attr_infos)
        : relation_name_(relation_name), object__(object), operation_(operation), attr_infos_(attr_infos)  {}

    virtual ~AlterStmt() = default;

    StmtType type() const override { return StmtType::ALTER; }

    const std::string &relation_name() const { return relation_name_; }
    const std::string &object_() const { return object__; }
    const std::string &operation() const { return operation_; }
    const std::vector<AttrInfoSqlNode> &attr_infos() const { return attr_infos_; }

    static RC create(Db *db, const AlterSqlNode &alter, Stmt *&stmt);

private:
    std::string relation_name_;
    std::string object__;
    std::string operation_;
    std::vector<AttrInfoSqlNode> attr_infos_;
};
