#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <boost/asio.hpp>

namespace network
{

        // Download a file using the provided chunk size and count. If the size
        // of the file is less than number_requests * request_size it will not
        // be fully downloaded. The parallel download will use multiple threads
        // to make requests and buffer all the results in memory until the
        // entire file is downloaded. The sequential download will run
        // everything on the main thread
        // Return the amount of data downloaded.
        size_t download_file_parallel(
                const std::string& host, uint16_t port, const std::string& path,
                int number_requests, size_t request_size,
                std::ostream_iterator<uint8_t>& os);

        size_t download_file_sequential(
                const std::string& host, uint16_t port, const std::string& path,
                int number_requests, size_t request_size,
                std::ostream_iterator<uint8_t>& os);

        using boost::asio::ip::tcp;

        // Download a chunk of the file between the given bounds.
        // Returns the amount of data downloaded.
        size_t make_chunk_request(
                const std::string& host, const std::string& path,
                size_t first_byte, size_t last_byte, uint8_t* buffer,
                uint16_t port = 80);
}

#endif

