#include "common.hpp"
#include "simulation.hpp"

#include <argparse/argparse.hpp>
#include <filesystem>

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

    program.add_argument("-s", "--setting")
        .required()
        .help("path to setting file")
        .action([](const std::string& value) {
            if (!fs::exists(value)) {
                cout << "ERROR: The setting file " << value << " does not exist" << endl;
                cerr << "ERROR: The setting file " << value << " does not exist" << endl;
                exit(-1);
            }
            cout << "Setting file: " << value << endl;
            return value;
        });

    program.add_argument("-o", "--output")
        .required()
        .help("path to output directory")
        .action([](const std::string& value) {
            if (!fs::exists(value)) {
                cout << "ERROR: The output directory " << value << " does not exist" << endl;
                cerr << "ERROR: The output directory " << value << " does not exist" << endl;
                exit(-1);
            }
            cout << "Output directory: " << value << endl;
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
