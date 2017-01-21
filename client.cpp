#include <fstream>
#include "network.hpp"
#include <iostream>
#include <unistd.h>

#include <boost/program_options.hpp>

int main(int argc, const char* argv[]) {

        namespace po = boost::program_options;

        po::options_description desc("Options");
        desc.add_options()
                ("chunk-size", po::value<size_t>()->default_value(1024*1024), "the size of chunk to download")
                ("chunk-number", po::value<int>()->default_value(4), "the number of chunks to download")
                ("url", po::value<std::string>()->required(), "where to download from")
                ("outfile", po::value<std::string>()->default_value("download"), "the path to write the downloaded file to")
                ("serial", po::bool_switch()->default_value(false), "use serial download instead of the default parallel")
                ;
        po::variables_map vars;
        try
        {
                po::store(po::parse_command_line(argc, argv, desc), vars);
                po::notify(vars);
        }
        catch (std::exception& e)
        {
                std::cerr << "Bad options: " << e.what() << '\n';
                std::cerr << desc << '\n';
                return 1;
        }
        
        std::string outfile(vars["outfile"].as<std::string>());
        std::string host, path;
        std::tie(host, path) = network::parse_url(vars["url"].as<std::string>());
        
        std::ofstream fs(outfile, std::ios::out | std::ios::binary | std::ios::trunc);
        std::ostream_iterator<uint8_t> os(fs);
        if (vars["serial"].as<bool>())
        {
                network::download_file_sequential(
                        host, 80, path,
                        vars["chunk-number"].as<int>(),
                        vars["chunk-size"].as<size_t>(),
                        os);
        }
        else
        {
                network::download_file_parallel(
                        host, 80, path,
                        vars["chunk-number"].as<int>(),
                        vars["chunk-size"].as<size_t>(),
                        os);
        }
}
