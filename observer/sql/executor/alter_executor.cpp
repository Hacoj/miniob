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
// Created by Hacoj on 2024/5/9.
//

#include "alter_executor.h"

#include "common/log/log.h"
#include "event/session_event.h"
#include "event/sql_event.h"
#include "session/session.h"
#include "sql/stmt/alter_stmt.h"
#include "storage/db/db.h"

/**
 * 执行表更改操作
 * 
 * 本函数用于执行SQL事件中的表更改操作，具体包括验证语句类型、获取会话信息、
 * 执行表更改等步骤。首先，它会验证SQL事件中的语句是否为ALTER TABLE类型，
 * 然后执行相应的表更改操作。
 * 
 * @param sql_event 指向SQL阶段事件的指针，包含了执行表更改操作所需的会话信息和语句。
 * @return 返回操作的结果状态码，RC（Result Code）。
 */
RC AlterExecutor::execute(SQLStageEvent *sql_event) { 
  // 获取当前会话
  Session *session = sql_event->session_event()->session();
  Stmt    *stmt    = sql_event->stmt();

  // 断言：确保执行的是ALTER TABLE语句
  ASSERT(stmt->type() == StmtType::ALTER, 
        "alter table executor can not run this command: %d",
        static_cast<int>(stmt->type()));

  RC rc = RC::SUCCESS;

  // 静态类型转换，获取ALTER TABLE语句对象
  AlterStmt *alter_stmt = static_cast<AlterStmt*>(stmt);

  // 获取要更改的信息
  std::string name = alter_stmt-> relation_name();
  std::vector<AttrInfoSqlNode> attr_infos_ = alter_stmt -> attr_infos();

  // 执行表更改操作
  for (AttrInfoSqlNode attr : attr_infos_) {
    rc = session->get_current_db()
                    ->alter_table(alter_stmt->relation_name().c_str(),
                                  alter_stmt->object_().c_str(),
                                  alter_stmt->operation().c_str(),
                                  &attr);
  }

  // 返回操作结果状态码
  return rc;
}