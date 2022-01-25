#include"TableOperator.h"
#include"fstream"
#include"Object/App.h"
#include"Protocol/db.pb.h"

namespace Sentry
{
    TableOperator::TableOperator(GKMysqlSocket *socket)
    {
        this->mMysqlSocket = socket;
    }

    bool TableOperator::InitDb(const std::string &db)
    {
        if (mysql_select_db(this->mMysqlSocket, db.c_str()) != 0)
        {
            const std::string sql = "CREATE DATABASE " + db;
            if (mysql_real_query(mMysqlSocket, sql.c_str(), sql.length()) != 0)
            {
                const char *err = mysql_error(mMysqlSocket);
                LOG_ERROR("create db {0} failure : {1}", db , err);
                return false;
            }
            if (mysql_select_db(this->mMysqlSocket, db.c_str()) == 0)
            {
                LOG_INFO("create db [", db, "] successful");
                return true;
            }
            return false;
        }
        return true;
    }

    bool TableOperator::InitTable(const Descriptor *typeDescriptor)
    {
        std::string sql = "desc " + typeDescriptor->name();
        if (mysql_real_query(this->mMysqlSocket, sql.c_str(), sql.length()) != 0)
        {
            if (!this->CreateMysqlTable(typeDescriptor))
            {
                LOG_ERROR("create new table", typeDescriptor->name(), "failure",
                          mysql_error(this->mMysqlSocket));
                return false;
            }
            LOG_DEBUG("create new table ",typeDescriptor->name(),  " successful");
        }
        else if (!this->UpdateMysqlTable(typeDescriptor))
        {
            return false;
        }
        return true;
    }

    bool TableOperator::UpdateMysqlTable(const Descriptor *descriptor)
    {
        MysqlQueryResult *queryResult = mysql_store_result(this->mMysqlSocket);
        if (queryResult == nullptr)
        {
            return false;
        }
        std::set<std::string> fieldSet;
        unsigned long rowCount = mysql_num_rows(queryResult);
        unsigned int fieldCount = mysql_field_count(mMysqlSocket);
        for (unsigned long count = 0; count < rowCount; count++)
        {
            MYSQL_ROW row = mysql_fetch_row(queryResult);
            unsigned long *lengths = mysql_fetch_lengths(queryResult);

            for (size_t index = 0; index < fieldCount; index++)
            {
                fieldSet.insert(std::string(row[index], (int) lengths[index]));
            }
        }

        for (int index = 1; index <= descriptor->field_count(); index++)
        {
            const FieldDescriptor *fileDesc = descriptor->FindFieldByNumber(index);
            if (fileDesc == nullptr)
            {
                continue;
            }
            auto iter = fieldSet.find(fileDesc->name());
            if (iter == fieldSet.end())
            {
                if (!this->AddNewField(descriptor->name(), fileDesc))
                {
                    LOG_ERROR("[mysql error ] {0}", mysql_error(this->mMysqlSocket));
                    LOG_ERROR("add field {0} to {1} failure", fileDesc->name(), descriptor->name());
                    return false;
                }
                LOG_DEBUG("add field {0} to {1} successful", fileDesc->name() , descriptor->name());
            }
        }
        return true;
    }

    bool TableOperator::AddNewField(const std::string &table, const FieldDescriptor *fieldDesc)
    {
        std::stringstream sqlStream;
        sqlStream << "alter table " << table << " add " << fieldDesc->name();
        switch (fieldDesc->type())
        {
            case FieldDescriptor::TYPE_INT32:
                sqlStream << " int(20) DEFAULT 0";
                break;
            case FieldDescriptor::TYPE_UINT32:
                sqlStream << " INTEGER(32) DEFAULT 0";
                break;
            case FieldDescriptor::TYPE_INT64:
            case FieldDescriptor::TYPE_UINT64:
                sqlStream << " bigint(32) DEFAULT 0";
                break;
            case FieldDescriptor::TYPE_FLOAT:
                sqlStream << " float(20) DEFAULT 0";
                break;
            case FieldDescriptor::TYPE_DOUBLE:
                sqlStream << " double(32) DEFAULT 0";
                break;
            case FieldDescriptor::TYPE_STRING:
                sqlStream << " varchar(64) DEFAULT NULL";
                break;
            case FieldDescriptor::TYPE_BYTES:
                sqlStream << " BLOB(64) DEFAULT NULL";
                break;
            default:
                return false;
        }
        const std::string sql = sqlStream.str();
        if (mysql_real_query(this->mMysqlSocket, sql.c_str(), sql.length()) != 0)
        {
            LOG_ERROR(mysql_error(mMysqlSocket));
            return false;
        }
        LOG_INFO("\n{0}", sql);
        return true;
    }

    bool TableOperator::CreateMysqlTable(const Descriptor *descriptor)
    {
        std::stringstream sqlCommand;
        sqlCommand << "create table `" << descriptor->name() << "`(\n";
        const FieldDescriptor *fileDesc = descriptor->FindFieldByNumber(1);
        for (int index = 1; index <= descriptor->field_count(); index++)
        {
            fileDesc = descriptor->FindFieldByNumber(index);
            if (fileDesc == nullptr)
            {
                continue;
            }
            sqlCommand << "`" << fileDesc->name() << "` ";
            switch (fileDesc->type())
            {
                case FieldDescriptor::TYPE_INT32:
                    sqlCommand << (index == 1 ? "INT(20) NOT NULL,\n" : "INT(20) DEFAULT 0,\n");
                    break;
                case FieldDescriptor::TYPE_UINT32:
                    sqlCommand << (index == 1 ? "INTEGER(20) NOT NULL,\n" : "INTEGER(20) DEFAULT 0,\n");
                    break;
                case FieldDescriptor::TYPE_INT64:
                case FieldDescriptor::TYPE_UINT64:
                    sqlCommand << (index == 1 ? "INTEGER(20) NOT NULL,\n" : "BIGINT(32) DEFAULT 0,\n");
                    break;
                case FieldDescriptor::CPPTYPE_FLOAT:
                LOG_CHECK_RET_FALSE(index > 1);
                    sqlCommand << "FLOAT(20) NOT NULL DEFAULT 0,\n";
                    break;
                case FieldDescriptor::TYPE_DOUBLE:
                LOG_CHECK_RET_FALSE(index > 1);
                    sqlCommand << "DOUBLE(20) NOT NULL DEFAULT 0,\n";
                    break;
                case FieldDescriptor::TYPE_STRING:
                    sqlCommand << (index == 1 ? "VARCHAR(64) NOT NULL,\n" : "VARCHAR(64) DEFAULT NULL,\n");
                    break;
                case FieldDescriptor::TYPE_BYTES:
                    sqlCommand << "BLOB(64) DEFAULT NULL,\n";
                    break;
                default:
                    return false;
            }
        }
        fileDesc = descriptor->FindFieldByNumber(1);
        sqlCommand << "PRIMARY KEY (`" << fileDesc->name() << "`)";
        sqlCommand << ")ENGINE=InnoDB DEFAULT CHARSET = utf8;";
        const std::string sql = sqlCommand.str();
        if (mysql_real_query(this->mMysqlSocket, sql.c_str(), sql.length()) != 0)
        {
            LOG_ERROR(mysql_error(mMysqlSocket));
            return false;
        }
        return true;
    }
}