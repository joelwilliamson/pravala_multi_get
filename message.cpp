#include "message.hpp"

#include <set>
#include <stdexcept>
#include <algorithm>
#include <iterator>

namespace message {
        std::istream& getline(std::istream& is, std::string& str)
        {
                std::istream::sentry s(is);
                char c;
                std::getline(is, str, '\r');
                if (!is.get(c) || c != '\n')
                {
                        is.setstate(std::ios_base::failbit);
                }
                return is;
        }

        void drop_newline(std::istream& is)
        {
                char c;
                if (!is.get(c) || c != '\r')
                {
                        is.setstate(std::ios_base::failbit);
                        throw std::runtime_error("Missing \\r");
                }
                if (!is.get(c) || c != '\n')
                {
                        is.setstate(std::ios_base::failbit);
                        throw std::runtime_error("Missing \\n");
                }
        }

        const std::set<ci::string>& get_valid_request_header_fields()
        {
                static const std::set<ci::string> standard_request_fields = {
                        "accept",
                        "accept-charset",
                        "accept-encoding",
                        "accept-language",
                        "accept-datetime",
                        "authorization",
                        "cache-control",
                        "connection",
                        "cookie",
                        "content-length",
                        "content-md5",
                        "content-type",
                        "date",
                        "expect",
                        "forwarded",
                        "from",
                        "host",
                        "if-match",
                        "if-modified-since",
                        "if-none-match",
                        "if-range",
                        "if-unmodified-since",
                        "max-forwards",
                        "origin",
                        "pragma",
                        "proxy-authorization",
                        "range",
                        "referer",
                        "te",
                        "user-agent",
                        "upgrade",
                        "via",
                        "warning",
                };
                return standard_request_fields;
        }

        const std::set<ci::string>& get_valid_response_header_fields()
        {
                static const std::set<ci::string> standard_response_fields = {
			"access-control-allow-origin",
			"accept-patch",
			"accept-ranges",
			"age",
			"allow",
			"alt-svc",
			"cache-control",
			"connection",
			"content-disposition",
			"content-encoding",
			"content-language",
			"content-length",
			"content-location",
			"content-md5",
			"content-range",
			"content-type",
			"date",
			"etag",
			"expires",
			"last-modified",
			"link",
			"location",
			"p3p",
			"pragma",
			"proxy-authenticate",
			"public-key-pins",
			"refresh",
			"retry-after",
			"permanent",
			"server",
			"set-cookie",
			"status",
			"strict-transport-security",
			"trailer",
			"transfer-encoding",
			"tsv",
			"tsv",
			"upgrade",
			"vary",
			"via",
			"warning",
			"www-authenticate",
			"x-frame-options",
                };
                return standard_response_fields;
        }

        bool non_standard_field(const ci::string& s)
        {
                return s.size() > 2 &&
                        s.find('X') == 0 &&
                        s.find('-') == 1;
        }

        request_field_name::request_field_name(std::string name)
                : name(ci::from_string(name))
        {

                if (!get_valid_request_header_fields().count(this->name) &&
                    !non_standard_field(this->name))
                {
                        throw std::invalid_argument(name + " is not an HTTP request header");
                }
        }

        request_field_name::request_field_name(const char* name) : request_field_name(std::string(name)) {}

        std::string request_field_name::to_string() const
        {
                return ci::to_string(name);
        }

        response_field_name::response_field_name(std::string name)
                : name(ci::from_string(name))
        {
                if (!get_valid_response_header_fields().count(this->name) &&
                    !non_standard_field(this->name))
                {
                        throw std::invalid_argument(name + " is not an HTTP response header");
                }
        }

        response_field_name::response_field_name(const char* name) : response_field_name(std::string(name)) {}

        std::string response_field_name::to_string() const
        {
                return ci::to_string(name);
        }

        bool response_field_name::operator<(const response_field_name& rhs) const
        {
                return name < rhs.name;
        }
        
        bool response_field_name::operator==(const response_field_name& rhs) const
        {
                return name == rhs.name;
        }

        std::ostream& operator<<(std::ostream& os, const response_field_name& field)
        {
                return os << field.name;
        }

        request_field::operator pair_type() const
        {
                return make_pair(name, value);
        }

        response_field::operator pair_type() const
        {
                return make_pair(name, value);
        }
        
        std::ostream& operator<<(std::ostream& os, method m)
        {
                switch (m)
                {
                case method::GET: { os << "GET"; break; }
                case method::HEAD: {os << "HEAD"; break; }
                case method::DELETE: {os << "DELETE"; break; }
                case method::TRACE: {os << "TRACE"; break; }
                }
                return os;
        }

        std::istream& operator>>(std::istream& is, http_version& version)
        {
                std::string version_token;
                is >> version_token;
                if (!is)
                {
                        return is;
                }
                if (version_token == "HTTP/1.0")
                {
                        version = http_version::HTTP10;
                }
                else if (version_token == "HTTP/1.1")
                {
                        version = http_version::HTTP11;
                }
                else if (version_token == "HTTP/2.0")
                {
                        version = http_version::HTTP20;
                }
                else
                {
                        is.setstate(std::ios_base::failbit);
                }
                return is;
        }

        std::ostream& operator<<(std::ostream& os, http_version version)
        {
                switch (version) {
                case http_version::HTTP10: return os << "HTTP/1.0";
                case http_version::HTTP11: return os << "HTTP/1.1";
                case http_version::HTTP20: return os << "HTTP/2.0";
                }
                return os;
        }

        request_message::request_message(method request_method, std::string path, std::vector<request_field> header_fields)
                : request_method(request_method), path(std::move(path)),
                  header_fields(std::move(header_fields))
        {
        }

        std::ostream& operator<<(std::ostream& os, request_message m)
        {
                os << m.request_method << " " << m.path << " HTTP/1.1\r\n";
                for (const auto& field : m.header_fields)
                {
                        os << field.name.to_string() << ": " << field.value << "\r\n";
                }
                os << "\r\n";
                // Message body would go here
                return os;
        }

        response_code::response_code(int code, std::string message)
                : code(code), message(std::move(message))
        {
        }

        bool response_code::operator==(const response_code& rhs) const
        {
                return code == rhs.code;
        }

        response_code::operator bool() const
        {
                return 200 <= code && code < 300;
        }

        std::istream& operator>>(std::istream& is, response_code& code)
        {
                is >> code.code;
                getline(is, code.message, '\r');
                return is;
        }

        std::ostream& operator<<(std::ostream& os, const response_code& code)
        {
                return os << code.code << " " << code.message;
        }
        
        response_message::response_message(http_version version, response_code status,
                                           std::map<response_field_name, std::string> header_fields,
                                           std::vector<uint8_t> body)
                : version(version), status(status), header_fields(std::move(header_fields)),
                  message_body(std::move(body))
        {
        }

        response_message::response_message(std::istream& is)
                : status(0,"")
        {
                if (!(is >> version)) throw std::runtime_error("Malformed HTTP version");
                if (!(is >> status)) throw std::runtime_error("Malformed HTTP status");
                for (std::string field_name, field_value; is.peek() != '\r'; )
                {
                        is >> field_name;
                        getline(is, field_value);
                        if (!is)
                        {
                                throw std::runtime_error("Malformed HTTP header field");
                        }
                        if (':' != *(field_name.end()-1))
                        {
                                throw std::runtime_error("Invalid HTTP header. Missing :");
                        }
                        else
                        {
                                field_name.pop_back();
                        }
                        header_fields.insert(std::make_pair(field_name, field_value));
                }
                // Read the message body
                auto it = header_fields.find("Content-Length");
                if (it == header_fields.end())
                {
                        throw std::runtime_error("No known content-length");
                }
                else
                {
                        drop_newline(is);
                        size_t length = std::stoul(it->second);
                        message_body.resize(length);
                        is.read(reinterpret_cast<char*>(message_body.data()), length);
                        message_body.resize(is.gcount());
                }
                        
        }

        bool response_message::operator==(const response_message& rhs) const
        {
                return version == rhs.version
                        && status == rhs.status
                        && header_fields == rhs.header_fields
                        && message_body == rhs.message_body
                        ;
        }

        response_message::operator bool() const
        {
                return status;
        }

        std::ostream& operator<<(std::ostream& os, const response_message& rhs)
        {
                os << rhs.version << " " << rhs.status << "\r\n";
                for (const auto& field : rhs.header_fields)
                {
                        os << field.first << ": " << field.second << "\r\n";
                }
                for (uint8_t byte : rhs.message_body)
                {
                        os << int(byte);
                }
                return os;
        }

        const std::vector<uint8_t>& response_message::body() const
        {
                return message_body;
        }
}

