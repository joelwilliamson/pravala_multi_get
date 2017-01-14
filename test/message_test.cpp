#include "catch/single_include/catch.hpp"
#include "message.hpp"

using namespace message;

TEST_CASE("Only valid field names are allowed", "[request]") {
        REQUIRE_NOTHROW(request_field_name("Host"));
        REQUIRE_THROWS(request_field_name("Not-A-Header"));
}

TEST_CASE("Field names are case insensitive", "[request]") {
        REQUIRE_NOTHROW(request_field_name("hOst"));
}

TEST_CASE("Requests render properly", "[request]") {
        request_message request(method::GET, "/index.html", {{"Host", "www.example.org"}});
        std::ostringstream render_stream;
        render_stream << request;
        REQUIRE(render_stream.str() ==
                "GET /index.html HTTP/1.1\r\n"  \
                "Host: www.example.org\r\n" \
                "\r\n");
}

TEST_CASE("HTTP versions are read correctly", "[response]") {
        std::istringstream valid_ss("HTTP/1.0 HTTP/1.1 HTTP/2.0");
        http_version version10, version11, version20;
        valid_ss >> version10 >> version11 >> version20;
        REQUIRE(version10 == http_version::HTTP10);
        REQUIRE(version11 == http_version::HTTP11);
        REQUIRE(version20 == http_version::HTTP20);
        REQUIRE(valid_ss);
}

TEST_CASE("Invalid HTTP version are not read", "[response]") {
        std::istringstream garbage_ss("HTTP/1.5");
        http_version version;
        garbage_ss >> version;
        REQUIRE(!garbage_ss);
}

TEST_CASE("Response codes are read correctly", "[response]") {
        std::istringstream code200_ss("200 OK \r\n");
        response_code read_code, code_200(200, "OK");
        code200_ss >> read_code;
        REQUIRE(read_code == code_200);
        REQUIRE(code200_ss);
}

TEST_CASE("Responses are read correctly", "[response]") {
        std::istringstream ss(
                "HTTP/1.1 200 OK\r\n" \
                "Content-Type: text/html\r\n" \
                "Content-LEngth: 8\r\n" \
                "\r\n" \
                "12345678X");
        response_message read_message(ss);
        response_message actual_message(
                http_version::HTTP11, {200, "OK"},
                {{"Content-Type", "text/html"}, {"Content-Length", "8"}},
                {'1','2','3','4','5','6','7','8'});
        REQUIRE(read_message == actual_message);
}

TEST_CASE("Responses with not as much content as advertised work", "[response]") {
        std::istringstream ss(
                "HTTP/1.1 200 OK\r\n" \
                "Content-Type: text/html\r\n" \
                "Content-LEngth: 18\r\n" \
                "\r\n" \
                "12345678X");
        response_message read_message(ss);
        response_message actual_message(
                http_version::HTTP11, {200, "OK"},
                {{"Content-Type", "text/html"}, {"Content-Length", "18"}},
                {'1','2','3','4','5','6','7','8','X'});
        REQUIRE(read_message == actual_message);
}

TEST_CASE("Response with a message body starting with whitespace", "[response]") {
        std::istringstream ss(
                "HTTP/1.1 200 OK\r\n" \
                "Content-Type: text/html\r\n" \
                "Content-LEngth: 18\r\n" \
                "\r\n" \
                "  12345678X");
        response_message read_message(ss);
        response_message actual_message(
                http_version::HTTP11, {200, "OK"},
                {{"Content-Type", "text/html"}, {"Content-Length", "18"}},
                {' ', ' ', '1','2','3','4','5','6','7','8','X'});
        REQUIRE(read_message == actual_message);
}

TEST_CASE("Response with a non-standard field", "[response]") {
        std::istringstream ss(
                       "HTTP/2.0 206 Partial\r\n" \
                       "X-Non-Standard: True\r\n" \
                       "Content-Length: 0\r\n\r\n");
        REQUIRE_NOTHROW(response_message{ss});
}
                
