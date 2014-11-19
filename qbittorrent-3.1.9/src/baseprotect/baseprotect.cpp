#include <queue>
#include <exception>
#include <sstream>
#include <fstream>
#include <string>

#include <stdlib.h>
#include <tchar.h>

#ifndef Q_MOC_RUN

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <boost/thread/thread.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <boost/chrono.hpp>
#include <boost/format.hpp>
#include <boost/foreach.hpp>

#endif

#include <libtorrent/session.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/session_settings.hpp>

#include <eh.h>
#include <soci-sqlite3.h>

#include "qbtsession.h"
#include "baseprotect.hpp"

#include <QDir>
#include <QString>

using boost::shared_ptr;
using boost::mutex;
using boost::condition_variable;
using std::string;
using std::exception;

using libtorrent::torrent_handle;
using libtorrent::peer_info;
using libtorrent::peer_id;
using libtorrent::peer_disconnected_alert;

using namespace boost::gregorian;
using namespace boost::posix_time;

using namespace sqlite_api;

namespace baseprotect
{

namespace log {

void write(const std::exception& e){
    try
    {
        std::ofstream log("qbtlog.log", std::ios::app);
        log << "[" << posix_time::second_clock::universal_time()<< "]"
            << "ERROR: "  << e.what() << std::endl;
    }
    catch(...){}
}

void write(QString s){
    try{
        write(s.toStdString());
    }catch(...){}
}

}

soci::session& tracer::ensure_db()
{
    string create_table = utils::read_whole_file("scripts/create_table_script.sql");
    string check = utils::read_whole_file("scripts/check_table_script.sql");
    string create_index = utils::read_whole_file("scripts/create_index_script.sql");

    using soci::into;

    string c = check;

    int table_exsist = 0;
    (*db_) << check, into(table_exsist);

    if(table_exsist == 0)
    {
        (*db_) << create_table;
        //(*db_) << create_index;
    }

    return *db_;

}

void tracer::thread_main()
{
    boost::unique_lock<boost::mutex> lock(m, boost::try_to_lock);
    log::write("Starting BASEPROTECT thread");

    try
    {
        while(running)
        {
            if(lock.owns_lock())
            {
                log::write(QString("Queue size: %1").arg(queue_.size()).toStdString());

                while(!queue_.empty() && running)
                {
                    log::write("Processing event");

                    peer_hit_ptr hit;
                    hit = queue_.front();
                    queue_.pop();
                    write_to_database(hit);
                }

                log::write("Entries processed. Waiting for events.");
                queue_not_empty_.timed_wait(
                    lock, posix_time::seconds(1)
                );
            }

            utc = sync_time();
        }

        log::write("Exiting BASEPROTECT thread");
        thread_stopped_.notify_one();
    }
    catch(std::exception& e)
    {
        log::write(e);
    }
}

boost::posix_time::ptime tracer::sync_time()
{
    return libtorrent::sync_time();
}

void tracer::stop()
{
    boost::unique_lock<boost::mutex> lock(m, boost::try_to_lock);

    log::write("Stoping BASEPROTECT thread");
    running = false;
    job_.detach();
    //thread_stopped_.timed_wait(lock, posix_time::seconds(5));
    log::write("Thread BASEPROTECT  stopped");
}

void tracer::insert(bp_entry_t entry)
{
    boost::unique_lock<boost::mutex> lock(m, boost::try_to_lock);
    log::write("Insert new entry");

    if(!lock.owns_lock())
        return;  

    queue_.push(make_hit(entry));

    if(queue_.size() > 0){
        queue_not_empty_.notify_one();
    }
}

peer_hit_ptr make_hit(peer_disconnected_alert* alert)
{
    peer_hit_ptr hit = boost::make_shared<peer_disconnect>();
    hit->peer_id = alert->pid;

    return hit;
}

peer_hit_ptr make_hit(const bp_entry_t& entry)
{
    log::write("Making hit");

    peer_hit_ptr hit = boost::make_shared<peer_hit>();

    hit->ip = entry.ip;
    hit->port = entry.port;

    hit->peer_id = entry.pid;

    hit->local_ip = entry.local_ip;
    hit->local_port = entry.local_port;

    hit->block = entry.pb.block_index;
    hit->piece = entry.pb.piece_index;

    hit->t_hash = entry.t_hash;
    hit->b_hash = entry.b_hash;
    hit->p_hash = entry.p_hash;

    hit->client = entry.client;
    hit->filesize = entry.filesize;
    hit->filename = entry.filename;
    hit->path = entry.path;

    hit->piece_size = entry.piece_size;
    hit->block_size = entry.block_size;

    return hit;
}

void tracer::write_to_database(peer_disconnected_alert* alert)
{
    posix_time::ptime pt = posix_time::second_clock::universal_time();

    auto hit = make_hit(alert);
    hit->execute_command(*db_, hit);
}

peer_hit_ptr tracer::write_to_database(peer_hit_ptr hit)
{
    log::write("Writing to DB.");

    if(!is_valid_hit(hit))
    {
        log::write("Invalid hit.");
        return peer_hit_ptr();
    }

    hit->time = utc;
    hit->execute_command(*db_, hit);

    return hit;
}

bool tracer::is_valid_hit(peer_hit_ptr hit)
{
    return hit != NULL && hit->is_valid();
}

command peer_hit::execute_command(soci::session& db, peer_hit_ptr hit)
{
    try
    {
        log::write("Creating DB entry");

        using soci::use;

        std::string insert = utils::read_whole_file("scripts/insert_script.sql");

        const gregorian::date& d = hit->time.date();
        const posix_time::time_duration& t = hit->time.time_of_day();

        const string& ip = hit->ip.to_string();
        const string& local_ip = utils::get_local_ip();

        const string& guid = utils::obj_to_string(hit->peer_id);
        const string& t_hash = utils::obj_to_string(hit->t_hash);
        const string& p_hash = utils::obj_to_string(hit->p_hash);
        const string& b_hash = utils::obj_to_string(hit->b_hash);

        command cmd = (
                    db.prepare << insert,

                       use(ip,                      "ip"),
                       use(local_ip,                "self_ip"),

                       use((unsigned short)d.year(),"year"),
                       use(d.month().as_number(), 	"month"),
                       use(d.day().as_number(),     "day"),
                       use(t.hours(), 				"hour"),
                       use(t.minutes(),             "minute"),
                       use(t.seconds(), 			"second"),

                       use(guid,				 	"guid"),
                       use(hit->client, 			"client"),

                       use(t_hash,				 	"t_hash"),
                       use(p_hash,				 	"p_hash"),
                       use(b_hash,				 	"b_hash"),

                       use(hit->port,   			"port"),
                       use(hit->local_port,         "self_port"),

                       use(hit->filename,           "filename"),
                       use(hit->path,               "path"),
                       use(hit->filesize,           "filesize"),

                       use(hit->block_size,         "block_size"),
                       use(hit->piece_size,         "piece_size"),

                       use(hit->block,              "block"),
                       use(hit->piece,              "piece"),

                       use(std::string("qbt3.1.9"),      "sig")
                      );

        log::write("Writing DB entry");
        cmd.execute(true);
        log::write("Entry written");

        return cmd;

    }catch(std::exception e){
        log::write(e);
        log::write("Reconnecting");
        db.reconnect();
    }
}

command peer_disconnect::execute_command(soci::session& db,  peer_hit_ptr hit)
{
    using soci::use;

    const char* update = "update hits set disconnect_time=:time where disconnect_time is NULL and guid=:guid";

    string time = to_simple_string(posix_time::second_clock::universal_time());

    command cmd = (db.prepare << update,
                   use(time, "time"),
                   use(utils::obj_to_string(hit->peer_id), "guid"));

    cmd.execute(true);

    return cmd;
}

std::string tracer::make_path(std::string dir, std::string name)
{
    boost::format fmt("%1%_%2%_qBt_Hits.db");
    posix_time::ptime pt = posix_time::second_clock::universal_time();
    const gregorian::date& d = pt.date();

    string dbpath = (fmt % name % d.week_number()).str();
    return QDir(dir.c_str()).absoluteFilePath(dbpath.c_str()).toStdString();
}

std::string tracer::start_tracking(std::string dir, std::string name)
{
    log::write("Start tracking");
    std::string dbpath = make_path(dir, name);
    db_.reset(new soci::session(soci::sqlite3, dbpath));
    return dbpath;
}

namespace utils
{

void sleep(int msecs)
{
    SleepThread::msleep(msecs);
}

std::string file_size_format(size_t bytes)
{
    size_t kilobyte = 1024;
    size_t megabyte = 1024 * kilobyte;
    size_t gigabyte = 1024 * megabyte;

    if (bytes > gigabyte) return (boost::format("%1% GB") % (bytes / gigabyte)).str();
    else if (bytes > megabyte) return (boost::format("%1% MB") % (bytes / megabyte)).str();
    else if (bytes > kilobyte) return (boost::format("%1% KB") % (bytes / kilobyte)).str();
    else return (boost::format("%1% B") % (bytes)).str();
}

std::string read_whole_file(const char* fn)
{
        return std::string((std::istreambuf_iterator<char>(std::ifstream(fn))),
                 std::istreambuf_iterator<char>());
}

std::string get_local_ip()
{
    WSAData wsaData;
    if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0) {
        return "0.0.0.0";
    }

    char ac[80];
    if (gethostname(ac, sizeof(ac)) == SOCKET_ERROR) {
        WSACleanup();
        return "0.0.0.0";
    }

    struct hostent *phe = gethostbyname(ac);
    if (phe == 0) {
        return ac;
    }

    for (int i = 0; phe->h_addr_list[i] != 0; ++i) {
        struct in_addr addr;
        memcpy(&addr, phe->h_addr_list[i], sizeof(struct in_addr));

        std::string ip = inet_ntoa(addr);
        if(!boost::algorithm::starts_with(ip, "127") &&
           !boost::algorithm::starts_with(ip, "192"))
        {
            WSACleanup();
            return ip;
        }
    }

    WSACleanup();
    return ac;
}

int get_service_status(std::string client)
{
    return libtorrent::get_service_status(client);
}


} // utils

} //baseprotect
