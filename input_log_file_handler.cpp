#include <iomanip>
#include <iostream>
#include <ctime>
#include <thread>
#include "input_log_file_handler.h"

#include "lib/jsoncpp/include/json/json.h"
#include "exceptions.h"

#include <thread>

namespace {
    namespace fs = std::experimental::filesystem;

    struct InitParsedRaw {
        std::string date;
        std::string fact_name;
        std::string props;
    };

    std::string PrepareDate(const std::uint64_t& timestamp) {
        time_t rawtime = timestamp;
        struct tm buf{};
        if (!gmtime_r(&rawtime, &buf)) {
            std::cerr << "Timestamp converting error" << std::endl;
            throw;
        }
        std::stringstream date;
        date << std::put_time(&buf, "%Y-%m-%d");

        return date.str();
    }

    InitParsedRaw ParseInitRaw(const std::string& line) {
        JSONCPP_STRING err;
        Json::Value json;

        const auto line_length = static_cast<int>(line.length());

//        std::cout << "LINE: " << line << std::endl;
        Json::CharReaderBuilder builder;
        const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
        if (!reader->parse(line.c_str(), line.c_str() + line_length, &json,
                           &err)) {
            throw JsonParseException("Json line parsing error: " + line);
        }

        InitParsedRaw raw;
        const auto& timestamp = json["ts_fact"].asUInt();
        raw.date = PrepareDate(timestamp);
        raw.fact_name = json["fact_name"].asString();
        for (int i = 0; i < 9; i++) {
            raw.props += json["props"]["prop" + std::to_string(i + 1)].asString() + ", ";
        }
        raw.props += json["props"]["prop10"].asString();
//        std::cout << std::this_thread::get_id() << " timestamp: " << timestamp << " raw.date: " << raw.date << std::endl;
//        std::cout << std::this_thread::get_id() << " raw.fact_name: " << raw.fact_name << std::endl;
//        std::cout << std::this_thread::get_id() << " raw.props: " << raw.props << std::endl;

        return raw;
    }
}

InputLogFileHandler::InputLogFileHandler(const std::string infile_name,
                                         const std::string& abs_dir_path) {
    dir_path = abs_dir_path;
    infile.open(infile_name);

    std::string line;
    while (std::getline(infile, line)) {
        try {
            const auto& parsed_raw = ParseInitRaw(line);

            const auto& outfile_name = parsed_raw.date + "_" + parsed_raw.fact_name;
            auto* outfile = FindOutfile(outfile_name);
            if (!outfile->is_open()) {
                std::cerr << "Temporary outfile open error: " << dir_path + "/" + outfile_name << strerror(errno) << std::endl;
            }
            *outfile << parsed_raw.props << std::endl;
        } catch (JsonParseException& ex) {
            std::cerr << ex.what() << std::endl;
        }
    }
}

InputLogFileHandler::~InputLogFileHandler() {
    if (infile.is_open()) {
        infile.close();
    }

    for (auto& [file_name, file]: outfiles) {
        if (file->is_open()) {
            file->close();
        }
        delete file;
    }
}

std::ofstream* InputLogFileHandler::FindOutfile(const std::string& file_name) {
    if (outfiles.find(file_name) == outfiles.end()) {
        const auto& abs_outfile_path = dir_path + "/" + file_name;
        auto *outfile = new std::ofstream(abs_outfile_path, std::ios::app);
        outfiles[file_name] = outfile;
    }

    return outfiles[file_name];
}