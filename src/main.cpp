#include "common.hpp"
#include "simulation.hpp"

#include <argparse/argparse.hpp>
#include <filesystem>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

using std::cerr;
using std::cout;
using std::endl;
namespace fs = std::filesystem;

/**
 * @brief entry point of the program
 *
 * @param argc number of arguments
 * @param argv array of arguments
 * @return return code
 */
int main(int argc, char** argv) {
    argparse::ArgumentParser program("mps");
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::info);
    auto error_sink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
    error_sink->set_level(spdlog::level::err);
    spdlog::logger logger("console", {console_sink, error_sink});
    spdlog::set_default_logger(std::make_shared<spdlog::logger>(logger));

    program.add_argument("-s", "--setting")
        .required()
        .help("path to setting file")
        .action([](const std::string& value) {
            if (!fs::exists(value)) {
                spdlog::error("The setting file {} does not exist", value);
                exit(-1);
            }
            spdlog::info("Setting file: {}", value);
            return value;
        });

    program.add_argument("-o", "--output")
        .required()
        .help("path to output directory")
        .action([](const std::string& value) {
            if (!fs::exists(value)) {
                spdlog::error("The output directory {} does not exist", value);
                exit(-1);
            }
            spdlog::error("Output directory: {}", value);
            fs::create_directories(value);
            return value;
        });

    // Although the use of exeptions is prohibited by the guidelines of this project,
    // the following process is shown in the document of the argpase library,
    // so we use here as an execptional calse.
    try {
        program.parse_args(argc, argv);
    } catch (const std::exception& err) {
        cout << err.what() << endl;
        cerr << err.what() << endl;
        cerr << program;
        exit(-1);
    }

    // auto settingPath = fs::path(argv[1]);
    auto settingPath     = fs::path(program.get<std::string>("--setting"));
    auto outputDirectory = fs::path(program.get<std::string>("--output"));
    Simulation simulation(settingPath, outputDirectory);
    simulation.run();

    return 0;
}
