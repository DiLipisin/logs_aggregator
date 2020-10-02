#include <iomanip>
#include <iostream>
#include <ctime>
#include <thread>

#include "logfile_splitter.h"
#include "utils.h"
#include "../lib/jsoncpp/include/json/json.h"

namespace {
    namespace fs = std::experimental::filesystem;

    struct InitParsedRow {
        std::string date;
        std::string fact_name;
        std::string props;
    };

    std::string TimestampToDateString(const std::uint64_t& timestamp) {
        time_t rowtime = timestamp;
        struct tm buf{};
        if (!gmtime_r(&rowtime, &buf)) {
            throw std::runtime_error("Timestamp converting error: " + std::to_string(timestamp));
        }
        std::stringstream date;
        date << std::put_time(&buf, "%Y-%m-%d");

        return date.str();
    }

    InitParsedRow ParseInitRow(const std::string& line) {
        JSONCPP_STRING err;
        Json::Value json;

        const auto line_length = static_cast<int>(line.length());

        Json::CharReaderBuilder builder;
        const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
        if (!reader->parse(line.c_str(), line.c_str() + line_length, &json, &err)) {
            throw std::runtime_error("Json line='" + line + "' parsing error: " + err);
        }

        InitParsedRow row;
        const auto& timestamp = json["ts_fact"].asUInt();
        row.date = TimestampToDateString(timestamp);
        row.fact_name = json["fact_name"].asString();
        for (int i = 0; i < 9; i++) {
            row.props += json["props"]["prop" + std::to_string(i + 1)].asString() + ", ";
        }
        row.props += json["props"]["prop10"].asString();

        return row;
    }
}

LogfileSplitter::LogfileSplitter(const std::string& infile_name, const std::string& abs_dir_path) {
    dir_path = abs_dir_path;
    infile.open(infile_name);
    if (!infile.is_open()) {
        throw std::runtime_error("IFile open error: " + infile_name);
    }
}

void LogfileSplitter::Run() {
    try {
        std::string line;
        while (std::getline(infile, line)) {
            const auto parsed_row = ParseInitRow(line);

            const auto& outfile_name = parsed_row.date;
            auto& outfile = FindOutfile(outfile_name);
            outfile << parsed_row.fact_name << "|" << parsed_row.props << std::endl;
        }
    } catch (std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
    }
}

std::ofstream& LogfileSplitter::FindOutfile(const std::string& file_name) {
    if (outfiles.find(file_name) == outfiles.end()) {
        const auto& abs_outfile_path = dir_path + "/" + file_name;
        outfiles[file_name] = std::ofstream(abs_outfile_path, std::ios::app);
    }

    auto& outfile = outfiles[file_name];
    if (!outfile.is_open()) {
        const auto& path = dir_path + "/" + file_name;
        std::cerr << "Temporary outfile " << path
                  << " opening error: " << HandleErrno(errno) << std::endl;
        throw std::runtime_error("OFile open error: " + path);
    }

    return outfile;
}
