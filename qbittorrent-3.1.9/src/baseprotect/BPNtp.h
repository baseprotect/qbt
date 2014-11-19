#ifndef NTPCLIENT_HPP
#define NTPCLIENT_HPP

#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/array.hpp>
#include <boost/optional.hpp>
#include <boost/thread.hpp>

using boost::asio::ip::tcp;

using namespace boost;
using namespace boost::asio;
using namespace boost::system;

using boost::optional;
using boost::asio::ip::tcp;
 
void set_result(optional<error_code>* a, error_code b);

template <typename MutableBufferSequence> 
error_code read_with_timeout(tcp::socket& sock, 
      const MutableBufferSequence& buffers,
	  posix_time::time_duration timeout) 
{ 
    optional<error_code> timer_result; 
    
	deadline_timer timer(sock.get_io_service()); 
    timer.expires_from_now(timeout);
	 
    timer.async_wait(boost::bind(set_result, &timer_result, _1)); 
    optional<error_code> read_result; 
    
	async_read(sock, buffers, 
        boost::bind(set_result, &read_result, _1)); 
  
    sock.get_io_service().reset(); 
    while (sock.get_io_service().run_one()) 
    { 
      if (read_result) 
        timer.cancel(); 
      else if (timer_result) 
        sock.cancel(); 
    } 

    return *read_result; 
} 

class ntp{
	posix_time::ptime time;
	mutex time_mutex;
public:
	posix_time::ptime get_time()
	{
		//unique_lock<mutex> lock(time_mutex);
		time = boost::posix_time::second_clock::universal_time();
        return time;
	}
	  
    posix_time::ptime async_get_time(const char* server)
    {
		try
		{
			asio::io_service io_service;

			tcp::resolver resolver(io_service);
			tcp::resolver::query query(server, "2013");
			tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

			tcp::socket socket(io_service);
			asio::connect(socket, endpoint_iterator);

			for(;;)
			{
				array<char, 15> buf;
				system::error_code error;
			  
				error = read_with_timeout(socket, 
							asio::buffer(buf), 
							posix_time::seconds(3));
			 
				if (error == asio::error::eof)
					break;
				else if (error)
					throw system::system_error(error);
				else
				{
					posix_time::ptime pt = posix_time::from_iso_string(buf.data());
			   
					unique_lock<mutex> lock(time_mutex);
					time = pt;
				}
			}
		}
		catch(std::exception& e)
		{
			unique_lock<mutex> lock(time_mutex);
			time = boost::posix_time::second_clock::universal_time();
		}
	}
};

#endif