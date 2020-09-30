#include <iomanip>
#include <iostream>
#include <ctime>
#include <thread>

#include "logfile_splitter.h"
#include "utils.h"
#include "../lib/jsoncpp/include/json/json.h"

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
            throw std::runtime_error("Timestamp converting error: " + std::to_string(timestamp));
        }
        std::stringstream date;
        date << std::put_time(&buf, "%Y-%m-%d");

        return date.str();
    }

    InitParsedRaw ParseInitRaw(const std::string& line) {
        JSONCPP_STRING err;
        Json::Value json;

        const auto line_length = static_cast<int>(line.length());

        Json::CharReaderBuilder builder;
        const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
        if (!reader->parse(line.c_str(), line.c_str() + line_length, &json,
                           &err)) {
            throw std::runtime_error("Json line='" + line + "' parsing error: " + err);
        }

        InitParsedRaw raw;
        const auto& timestamp = json["ts_fact"].asUInt();
        raw.date = PrepareDate(timestamp);
        raw.fact_name = json["fact_name"].asString();
        for (int i = 0; i < 9; i++) {
            raw.props += json["props"]["prop" + std::to_string(i + 1)].asString() + ", ";
        }
        raw.props += json["props"]["prop10"].asString();

        return raw;
    }
}

LogfileSplitter::LogfileSplitter(const std::string& infile_name,
                                         const std::string& abs_dir_path) {
    dir_path = abs_dir_path;
    infile.open(infile_name);

    std::string line;
    try {
        while (std::getline(infile, line)) {
            const auto& parsed_raw = ParseInitRaw(line);

            const auto& outfile_name = parsed_raw.date;
            auto* outfile = FindOutfile(outfile_name);
            if (!outfile->is_open()) {
                const auto& path = dir_path + "/" + outfile_name;
                std::cerr << "Temporary outfile " << path
                    << " opening error: " << HandleErrno(errno) << std::endl;
                throw std::runtime_error("OFile open error: " + path);
            }

            *outfile << parsed_raw.fact_name << "|" << parsed_raw.props << std::endl;
        }
    } catch (std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
    }
}

LogfileSplitter::~LogfileSplitter() {
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

std::ofstream* LogfileSplitter::FindOutfile(const std::string& file_name) {
    if (outfiles.find(file_name) == outfiles.end()) {
        const auto& abs_outfile_path = dir_path + "/" + file_name;
        auto *outfile = new std::ofstream(abs_outfile_path, std::ios::app);
        outfiles[file_name] = outfile;
    }

    return outfiles[file_name];
}