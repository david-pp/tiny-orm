#ifndef TINYWORLD_TINY_DB_H
#define TINYWORLD_TINY_DB_H

#include <memory>
#include <soci.h>
#include <soci-mysql.h>
#include "tinylogger.h"

class SOCIPool {
public:
    static SOCIPool& instance() {
        static SOCIPool pool_;
        return pool_;
    }

    bool connect(const std::string& url, size_t poolsize = 1) {
        soci::register_factory_mysql();

        cpool_size_ = poolsize;
        cpool_ = std::make_shared<soci::connection_pool>(poolsize);

        try {
            for (size_t i = 0; i < cpool_size_; ++i) {
                soci::session &sql = cpool_->at(i);
                sql.open(url);
            }
            return true;
        }
        catch (const soci::mysql_soci_error &e) {
            LOG_ERROR("SOCI", "Connect Failed, SOCI Error:%s", e.what());
        }
        catch (const std::exception &e) {
            LOG_ERROR("SOCI", "Connect Failed, Error:%s", e.what());
        }

        return false;
    }

    soci::connection_pool* pool() { return cpool_.get(); }

private:
    size_t cpool_size_ = 0;
    std::shared_ptr<soci::connection_pool> cpool_;
};

#endif //TINYWORLD_TINY_DB_H
