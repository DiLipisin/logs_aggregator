#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <time.h>
#include <experimental/filesystem>

#include "input_log_file_handler.h"

#include "lib/jsoncpp/include/json/json.h"

namespace {
    namespace fs = std::experimental::filesystem;

    struct InitParsedRaw {
        std::string date;
        std::string fact_name;
        std::string props;
    };

    std::string PrepareDate(const std::uint64_t& timestamp) {
        time_t rawtime;
        struct tm * ptm;
        time(&rawtime);
        ptm = gmtime(&rawtime);

        std::stringstream date;
        date << std::to_string(ptm->tm_year) <<
            "-" << std::setfill('0') << std::setw(2) << std::to_string(ptm->tm_mon) <<
            "-" <<std::setfill('0') << std::setw(2) << std::to_string(ptm->tm_mday);

        return date.str();
    }

    InitParsedRaw ParseInitRaw(std::string& line) {
        JSONCPP_STRING err;
        Json::Value json;

        const auto line_length = static_cast<int>(line.length());

        Json::CharReaderBuilder builder;
        const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
        if (!reader->parse(line.c_str(), line.c_str() + line_length, &json,
                           &err)) {
            std::cerr << "error: " << err << std::endl;
//            TODO: add exc
        }

        InitParsedRaw raw;
        raw.date = PrepareDate(json["ts_fact"].asUInt());
        std::cout << "raw.date: " << raw.date << std::endl;
        raw.fact_name = json["fact_name"].asString();
        std::cout << "raw.fact_name: " << raw.fact_name << std::endl;
        for (int i = 0; i < 9; i++) {
            raw.props += json["props"]["prop" + std::to_string(i + 1)].asString() + ", ";
        }
        raw.props += json["props"]["prop10"].asString();
        std::cout << "raw.props: " << raw.props << std::endl;

        return raw;
    }
}

//TODO: const int
InputLogFileHandler::InputLogFileHandler(const std::string& infile_name,
                                         const std::experimental::filesystem::path& abs_dir_path) {
    std::cout << "## infile_name: " << infile_name << " abs_dir_path:" << abs_dir_path << std::endl;
    dir_path = abs_dir_path;
    infile.open(infile_name);

    std::string line;

    while (infile >> line) {
        std::cout << "## line: " << line << std::endl;
        const auto& parsed_raw = ParseInitRaw(line);

        const auto& outfile_name = parsed_raw.date + "_" + parsed_raw.fact_name + ".txt";
        auto* outfile = FindOutfile(outfile_name);
        if (outfile->is_open()) {
            *outfile << parsed_raw.props << std::endl;
        } else {
            std::cerr << "Outfile open error: " << outfile_name << std::endl;
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
    }
}

std::ofstream* InputLogFileHandler::FindOutfile(const std::string& file_name) {
    if (outfiles.find(file_name) == outfiles.end()) {
        const auto& abs_outfile_path = dir_path.string() + "/" + file_name;
        std::cout << "abs_outfile_path: " << abs_outfile_path << std::endl;
        auto *outfile = new std::ofstream(abs_outfile_path, std::ios::app);
        outfiles[file_name] = outfile;
    }

    return outfiles[file_name];
}