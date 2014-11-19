#pragma once

#include <QHostInfo>
#include <QThread>

#include <queue>
#include <sstream>
#include <fstream>
#include <set>

#ifndef Q_MOC_RUN
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian_types.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/foreach.hpp>

#endif

#include "libtorrent/session.hpp"
#include "libtorrent/peer_info.hpp"
#include "libtorrent/torrent.hpp"
#include "libtorrent/peer_connection.hpp"
#include "libtorrent/alert_types.hpp"

#include <soci.h>
#include <soci-sqlite3.h>

#include <stdexcept>
#include <strstream>

using namespace boost;
using namespace libtorrent;


class QBtSession;

#define _A(x) decltype(*std::begin(x))

namespace baseprotect {

namespace log {

void write(const std::exception& e);
void write(QString s);

template<class T>
void write(T obj){
    std::ofstream log("qbtlog.log", std::ios::app);
    log << "[" << posix_time::second_clock::universal_time() << "]"
        << "LOG: " << obj << std::endl;
}

} // log

class peer_hit_base;

typedef boost::shared_ptr<peer_hit_base> peer_hit_ptr;
typedef soci::statement command;

struct peer_hit_base
{
    posix_time::ptime time;

    libtorrent::address ip;
    libtorrent::address local_ip;

    int port;
    int local_port;

    std::string country;
    std::string filename;
    std::string client;
    std::string path;

    sha1_hash t_hash;
    sha1_hash p_hash;
    sha1_hash b_hash;

    int block;
    int piece;

    peer_id peer_id;

    size_t filesize;
    size_t block_size;
    size_t piece_size;

    bool disconnect;

    virtual command execute_command(soci::session&, peer_hit_ptr) = 0;

    bool is_valid() const
    {
        return !t_hash.is_all_zeros()
            && !peer_id.is_all_zeros();
    }
};

struct peer_hit : public peer_hit_base
{
    command execute_command(soci::session&, peer_hit_ptr);
};

struct peer_disconnect : public peer_hit_base
{
    command execute_command(soci::session&, peer_hit_ptr);
};

class tracer
{
public:

    tracer(QBtSession* session)
         : session_(session)
         , running(true)
         , job_(boost::bind(&tracer::thread_main, this))
    {}

    /**/
    soci::session& ensure_db();

    /**/
    void insert(bp_entry_t hit);

    /**/
    void write_to_database(libtorrent::peer_disconnected_alert* alert);

    /**/
    peer_hit_ptr write_to_database(peer_hit_ptr hit);

    /**/
    std::string start_tracking(std::string, std::string name);

    /**/
    std::string make_path(std::string dir, std::string name);

    /**/
    bool is_valid_hit(peer_hit_ptr hit);

    /**/
    void stop();

private:

    /**/
    void thread_main();

    /**/
    boost::posix_time::ptime sync_time();

private:
    bool running;
    QBtSession* session_;
    boost::thread job_;
    boost::mutex m;
    std::unique_ptr<soci::session> db_;
    std::queue<peer_hit_ptr> queue_;
    boost::condition_variable queue_not_empty_;
    boost::condition_variable thread_stopped_;

    boost::mutex time_mutex;
    boost::posix_time::ptime utc;
};

peer_hit_ptr make_hit(libtorrent::peer_disconnected_alert* alert);

peer_hit_ptr make_hit(const bp_entry_t& entry);


namespace utils
{

class SleepThread : public QThread {
public:
   static inline void msleep(unsigned long msecs) {
       QThread::msleep(msecs);
   }
};

void sleep(int msecs);

std::string get_local_ip();
std::string file_size_format(size_t);
std::string read_whole_file(const char* fn);
int get_service_status(std::string client);

template<class T>
std::string obj_to_string(const T& obj){
    std::stringstream s;
    s << obj;
    return s.str();
}

}

}
