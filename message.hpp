#ifndef MESSAGE_HPP
#define MESSAGE_HPP
#include <string>
#include <vector>
#include <map>
#include <ostream>
#include <istream>

#include "ci_string.hpp"

// This module provides classes to represent HTTP requests and responses, and to
// stream the classes using iostream interfaces.

// In retrospect, making this class so elaborate was unnecessary. It certainly
// made the rest of the implementation very straightforward, but for something
// of this size, maintaining invariants is easy enough without classes for
// support.
namespace message {

        // request/response_field_name represent the key of a single header
        // key/value pair in an HTTP message.
        class request_field_name {
                // Using an enumeration instead of a string here could save some memory, but it
                // would be a bit of a pain to code the conversion to string. (Maybe could be
                // done with some regexes.)
                ci::string name;
        public:
                request_field_name(std::string name);
                request_field_name(const char* name);
                std::string to_string() const;
        };

        class response_field_name {
                ci::string name;
        public:
                response_field_name(std::string name);
                response_field_name(const char* name);
                std::string to_string() const;
                bool operator<(const response_field_name& rhs) const;
                bool operator==(const response_field_name& rhs) const;
                friend std::ostream& operator<<(std::ostream& os, const response_field_name& field);
        };        
        
        // method should represent any HTTP method. Since I haven't implemented
        // request bodies, the methods that use bodies are disabled.
        enum class method {
                GET,
                HEAD,
                // POST,
                // PUT,
                DELETE,
                // CONNECT,
                // OPTIONS,
                TRACE,
                // PATCH,
        };
        std::ostream& operator<<(std::ostream& os, method m);
        
        // request/response_field represent a key/value pair in a message header
        struct request_field {
                request_field_name name;
                std::string value;
                using pair_type = std::pair<request_field_name, std::string>;
                operator pair_type() const;
        };

        struct response_field {
                response_field_name name;
                std::string value;
                using pair_type = std::pair<response_field_name, std::string>;
                operator pair_type() const;
        };
        
        
        // request_message represents a restricted type of HTTP requests. Only
        // requests with no bodies are possible, and only HTTP/1.1 is supported.
        class request_message {
                method request_method;
                std::string path;
                // Only allow HTTP/1.1 messages
                
                std::vector<request_field> header_fields;
                
                // No need for message bodies yet.
        public:
                request_message(method request_method, std::string path, std::vector<request_field> header_fields);
                friend std::ostream& operator<<(std::ostream& os, request_message message);
        };
        
        enum class http_version {
                HTTP10,
                HTTP11,
                HTTP20,
        };
        
        std::istream& operator>>(std::istream& is, http_version& version);
        
        // A response_code represents whether a request succeeded or failed.
        // A status code of 2xx represents a success, anything else is a failure
        // of some sort.
        class response_code
        {
                int code;
                std::string message;
        public:
                response_code(int code, std::string message);
                response_code() = default;
                bool operator==(const response_code& rhs) const;
                operator bool() const;
                friend std::istream& operator>>(std::istream& is, response_code& code);
                friend std::ostream& operator<<(std::ostream& os,
                                                const response_code& code);
        };
                
        // A response_message is the result of the request.
        class response_message {
                http_version version;
                response_code status;
                
                std::map<response_field_name, std::string> header_fields;
                
                std::vector<uint8_t> message_body;
        public:
                response_message(http_version version, response_code status,
                                 std::map<response_field_name, std::string> header_fields,
                                 std::vector<uint8_t> body);
                response_message(std::istream& is);
                bool operator==(const response_message& rhs) const;
                operator bool() const;
                friend std::ostream& operator<<(std::ostream& os, const response_message& rhs);
                const std::vector<uint8_t>& body() const;
        };
}

#endif
