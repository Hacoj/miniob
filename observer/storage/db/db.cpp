/* Copyright (c) 2021 Xie Meiyi(xiemeiyi@hust.edu.cn) and OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

//
// Created by Meiyi & Longda & Wangyunlai on 2021/5/12.
//

#include "storage/db/db.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <vector>

#include "common/lang/string.h"
#include "common/log/log.h"
#include "common/os/path.h"
#include "storage/clog/clog.h"
#include "storage/common/meta_util.h"
#include "storage/table/table.h"
#include "storage/table/table_meta.h"
#include "storage/record/record.h"
#include "storage/trx/trx.h"
#include "db.h"

Db::~Db()
{
  for (auto &iter : opened_tables_) {
    delete iter.second;
  }
  LOG_INFO("Db has been closed: %s", name_.c_str());
}

RC Db::init(const char *name, const char *dbpath)
{
  if (common::is_blank(name)) {
    LOG_ERROR("Failed to init DB, name cannot be empty");
    return RC::INVALID_ARGUMENT;
  }

  if (!common::is_directory(dbpath)) {
    LOG_ERROR("Failed to init DB, path is not a directory: %s", dbpath);
    return RC::INTERNAL;
  }

  clog_manager_.reset(new CLogManager());
  if (clog_manager_ == nullptr) {
    LOG_ERROR("Failed to init CLogManager.");
    return RC::NOMEM;
  }

  RC rc = clog_manager_->init(dbpath);
  if (OB_FAIL(rc)) {
    LOG_WARN("failed to init clog manager. dbpath=%s, rc=%s", dbpath, strrc(rc));
    return rc;
  }

  name_ = name;
  path_ = dbpath;

  rc = open_all_tables();
  if (OB_FAIL(rc)) {
    LOG_WARN("failed to open all tables. dbpath=%s, rc=%s", dbpath, strrc(rc));
    return rc;
  }

  rc = recover();
  if (OB_FAIL(rc)) {
    LOG_WARN("failed to recover db. dbpath=%s, rc=%s", dbpath, strrc(rc));
    return rc;
  }
  return rc;
}

RC Db::create_table(const char *table_name, int attribute_count, const AttrInfoSqlNode *attributes)
{
  RC rc = RC::SUCCESS;
  // check table_name
  if (opened_tables_.count(table_name) != 0) {
    LOG_WARN("%s has been opened before.", table_name);
    return RC::SCHEMA_TABLE_EXIST;
  }

  // 文件路径可以移到Table模块
  std::string table_file_path = table_meta_file(path_.c_str(), table_name);
  Table      *table           = new Table();
  int32_t     table_id        = next_table_id_++;
  rc = table->create(table_id, table_file_path.c_str(), table_name, path_.c_str(), attribute_count, attributes);
  if (rc != RC::SUCCESS) {
    LOG_ERROR("Failed to create table %s.", table_name);
    delete table;
    return rc;
  }

  opened_tables_[table_name] = table;
  LOG_INFO("Create table success. table name=%s, table_id:%d", table_name, table_id);
  return RC::SUCCESS;
}


/**
 * @brief 删除指定的表
 * 
 * 此函数用于从数据库中删除指定名称的表。首先检查表是否正在被使用，如果是，则返回表不存在的错误码。接着尝试找到对应的表，
 * 如果找到，则删除该表。删除操作成功后，返回成功状态码。
 * 
 * @param table_name 指向要删除的表名称的指针。
 * @return RC 返回操作的结果状态码。成功时返回RC::SUCCESS，如果表不存在或者删除失败则返回相应的错误码。
 */
RC Db::drop_table(const char *table_name) { 
  RC rc = RC::SUCCESS;

  // 检查表是否正在被使用
  // if (opened_tables_.count(table_name) != 0) {
  //   LOG_WARN("%s is using.", table_name);
  //   return RC::SCHEMA_TABLE_NOT_EXIST;
  // }

  // 构造表的文件路径
  std::string table_file_path = table_meta_file(path_.c_str(), table_name);
  // 尝试找到对应的表
  Table       *table           = this->find_table(table_name);

  // 如果未能找到表，记录错误并返回
  if (table == nullptr) {
    LOG_ERROR("Failed to find table %s.", table_name);
    return RC::SCHEMA_TABLE_NOT_EXIST;
  }

  opened_tables_.erase(table_name);
  table->destroy();

  // 删除找到的表
  delete table;

  return rc;
}

RC Db::alter_table(const char *table_name, const char *operation,
     const char *object_, const AttrInfoSqlNode *attributes) { 
  if (opened_tables_.count(table_name) == 0) {
    LOG_WARN("%s does not exist.", table_name);
    return RC::SCHEMA_TABLE_NOT_EXIST;
  }

  Table *table = opened_tables_[table_name]; 
  const TableMeta &table_meta = table->table_meta();

  std::vector<FieldMeta> field(*(table_meta.field_metas()));

  if (std::string(object_) == "COLUMN"){
    if (std::string(operation) == "ADD") {

      FieldMeta field_mata = FieldMeta(
        attributes->name.c_str(),
        attributes->type,
        0,
        attributes->length,
        true
      );

      field.push_back((field_mata));
    }
  }

  AttrInfoSqlNode *attrs = new AttrInfoSqlNode[field.size()];

  for (long unsigned int i = 0; i < field.size(); i++)
  {
    attrs[i] = *new AttrInfoSqlNode();
    attrs[i].length = field.at(i).len();
    attrs[i].name   = field.at(i).name();
    attrs[i].type   = field.at(i).type();
  }
  

  TableMeta *new_table_meta = new TableMeta();
  new_table_meta->init(table_meta.table_id(),
                       table_meta.name(),
                       field.size(),
                       attrs);
  
  table->set_table_mete(*new_table_meta);


  return RC::SUCCESS;
}

Table *Db::find_table(const char *table_name) const
{
  std::unordered_map<std::string, Table *>::const_iterator iter = opened_tables_.find(table_name);
  if (iter != opened_tables_.end()) {
    return iter->second;
  }
  return nullptr;
}

Table *Db::find_table(int32_t table_id) const
{
  for (auto pair : opened_tables_) {
    if (pair.second->table_id() == table_id) {
      return pair.second;
    }
  }
  return nullptr;
}

RC Db::open_all_tables()
{
  std::vector<std::string> table_meta_files;
  int                      ret = common::list_file(path_.c_str(), TABLE_META_FILE_PATTERN, table_meta_files);
  if (ret < 0) {
    LOG_ERROR("Failed to list table meta files under %s.", path_.c_str());
    return RC::IOERR_READ;
  }

  RC rc = RC::SUCCESS;
  for (const std::string &filename : table_meta_files) {
    Table *table = new Table();
    rc           = table->open(filename.c_str(), path_.c_str());
    if (rc != RC::SUCCESS) {
      delete table;
      LOG_ERROR("Failed to open table. filename=%s", filename.c_str());
      return rc;
    }

    if (opened_tables_.count(table->name()) != 0) {
      delete table;
      LOG_ERROR("Duplicate table with difference file name. table=%s, the other filename=%s",
          table->name(), filename.c_str());
      return RC::INTERNAL;
    }

    if (table->table_id() >= next_table_id_) {
      next_table_id_ = table->table_id() + 1;
    }
    opened_tables_[table->name()] = table;
    LOG_INFO("Open table: %s, file: %s", table->name(), filename.c_str());
  }

  LOG_INFO("All table have been opened. num=%d", opened_tables_.size());
  return rc;
}

const char *Db::name() const { return name_.c_str(); }

void Db::all_tables(std::vector<std::string> &table_names) const
{
  for (const auto &table_item : opened_tables_) {
    table_names.emplace_back(table_item.first);
  }
}

RC Db::sync()
{
  RC rc = RC::SUCCESS;
  for (const auto &table_pair : opened_tables_) {
    Table *table = table_pair.second;
    rc           = table->sync();
    if (rc != RC::SUCCESS) {
      LOG_ERROR("Failed to flush table. table=%s.%s, rc=%d:%s", name_.c_str(), table->name(), rc, strrc(rc));
      return rc;
    }
    LOG_INFO("Successfully sync table db:%s, table:%s.", name_.c_str(), table->name());
  }
  LOG_INFO("Successfully sync db. db=%s", name_.c_str());
  return rc;
}

RC Db::recover() { return clog_manager_->recover(this); }

CLogManager *Db::clog_manager() { return clog_manager_.get(); }