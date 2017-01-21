#include "network.hpp"

#include "message.hpp"

#include <future>
#include <regex>

namespace network
{

        std::pair<std::string, std::string> parse_url(const std::string& url)
        {
                std::string regex_string = R"(^(http://)?([^/:]+)([^:]*)$)";
                // This assumes the host is either a registered name or an IPv4
                // address. IPv6 WILL NOT WORK!
                std::regex r(regex_string, std::regex::extended);
                auto url_part = std::sregex_iterator(url.cbegin(), url.cend(), r);
                if (std::distance(url_part, std::sregex_iterator()) != 1)
                {
                        throw std::runtime_error("Invalid URL");
                }
                std::string scheme = (*url_part)[1].str();
                std::string host = (*url_part)[2].str();
                std::string path = (*url_part)[3].str();
                if (path.size() == 0)
                {
                        path = "/";
                }
                return std::make_pair(std::move(host), std::move(path));
        }

        size_t download_file_parallel(
                const std::string& host, uint16_t port, const std::string& path,
                int number_requests, size_t request_size,
                std::ostream_iterator<uint8_t>& os)
        {
                std::vector<uint8_t> result_buf(number_requests * request_size);
                std::vector<std::future<size_t>> futures(number_requests);
                size_t start_byte = 0;
                size_t total_downloaded = 0;
                for (std::future<size_t>& f : futures)
                {
                        f = std::async(std::launch::async,
                                       [&host, &path, start_byte,
                                        request_size, &result_buf, port](){
                                        return make_chunk_request(
                                                host, path, start_byte,
                                                start_byte + request_size - 1,
                                                result_buf.data() + start_byte,
                                                port);}
                                );
                        start_byte += request_size;
                }
                for (std::future<size_t>& f : futures)
                {
                        total_downloaded += f.get();
                }
                // Shrink buffer to fit
                result_buf.resize(total_downloaded);
                std::copy(result_buf.cbegin(), result_buf.cend(), os);
                return total_downloaded;
        }

        size_t download_file_sequential(
                const std::string& host, uint16_t port, const std::string& path,
                int number_requests, size_t request_size,
                std::ostream_iterator<uint8_t>& os)
        {
                size_t start_byte = 0;
                size_t total_downloaded = 0;

                std::future<std::ostream_iterator<uint8_t>> f;

                for (int i = 0; i < number_requests; ++i)
                {
                        std::vector<uint8_t> buf(request_size);
                        size_t downloaded =
                                make_chunk_request(host, path, start_byte,
                                                   start_byte + request_size - 1,
                                                   buf.data(), port);;
                        buf.resize(downloaded);
                        if (f.valid())
                                f.wait();
                        f = std::async(std::launch::async, [&buf, &os]() {
                                        return std::copy(buf.cbegin(), buf.cend(),
                                                         os); });
                        total_downloaded += downloaded;
                }
                if (f.valid())
                        f.wait();
                
                return total_downloaded;
        }

        size_t make_chunk_request(
                const std::string& host, const std::string& path,
                size_t first_byte, size_t last_byte, uint8_t* buffer,
                uint16_t port)
        {
                tcp::iostream socket(host, std::to_string(port));
                std::string range_string = std::string("bytes=")
                        + std::to_string(first_byte) + "-"
                        + std::to_string(last_byte);
                
                socket << message::request_message(
                        message::method::GET, path,
                        {{"Host", host},
                                {"Range", range_string},
                                {"User-Agent", "chunking client"}});
                socket.flush();
                if (socket.error())
                {
                        throw std::runtime_error("Network error");
                }
                message::response_message response(socket);
                if (socket.error())
                {
                        throw std::runtime_error("Network error");
                }
                if (!response)
                {
                        throw std::runtime_error("Remote host " + host
                                                 + " didn't succeed.");
                }
                uint8_t* buffer_end = std::copy(response.body().cbegin(),
                                                response.body().cend(), buffer);
                return buffer_end - buffer;
        }
}
