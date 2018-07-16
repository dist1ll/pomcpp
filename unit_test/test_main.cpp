#define CATCH_CONFIG_RUNNER

#include <thread>

#include "catch.hpp"

bool THREADING;
uint THREAD_COUNT;

int main( int argc, char* argv[] )
{
    Catch::Session session; // There must be exactly one instance

    THREAD_COUNT = 1; // Some user variable you want to be able to set
    uint maxThreads = std::thread::hardware_concurrency(); // max nr conc th.

    // Build a new parser on top of Catch's
    using namespace Catch::clara;
    auto cli
        = session.cli() // Get Catch's composite command line parser
          | Opt( THREAD_COUNT, "threads" ) // bind variable to a new option, with a hint string
          ["-t"]["--threads"]    // the option names it will respond to
          ("How many threads for testing performance?");

    // Now pass the new composite back to Catch so it uses that
    session.cli( cli );

    // Let Catch (using Clara) parse the command line
    int returnCode = session.applyCommandLine( argc, argv );
    if( returnCode != 0 ) // Indicates a command line error
        return returnCode;


    // if set on the command line then 'height' is now set at this point
    THREADING = THREAD_COUNT > 1;
    if(THREADING)
    {
        std::cout << std::endl
                  << "Activated Multi-Threading for performance tests. " << std::endl
                  << "\tThread count:            " << THREAD_COUNT << std::endl
                  << "\tMax supported threads:   " << maxThreads << std::endl;
    }
    return session.run();
}
