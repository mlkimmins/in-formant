#include "modules/modules.h"
#include "nodes/nodes.h"
#include "analysis/analysis.h"
#include "backtrace/backtrace.h"
#include "context/context.h"
#include "context/contextbuilder.h"
#include "context/contextmanager.h"
#include <iostream>
#include <atomic>
#include <memory>
#include <chrono>
#include <csignal>
#include <thread>

#if defined(__APPLE__)
#   include <mach-o/dyld.h>
#   include <unistd.h>
#   include <libgen.h>
#endif

using namespace Module;
using namespace std::chrono_literals;

#if ! defined(__EMSCRIPTEN__)
static std::atomic_bool signalCaught(false);
static std::atomic_int signalStatus;

static void signalHandler(int signal) {
    signalCaught = true;
    signalStatus = signal;

    switch (signal) {
    case SIGTERM:
        std::cout << "Caught signal SIGTERM" << std::endl;
        break;
    case SIGINT:
        std::cout << "Caught signal SIGINT" << std::endl;
        break;
    }

    exit(EXIT_SUCCESS);
}
#endif

int start_logger(const char *app_name);

int main(int argc, char **argv)
{
#ifdef _WIN32
    srand(time(nullptr));
#endif

    start_logger("InFormant");

#if ! defined(__EMSCRIPTEN__)
    std::signal(SIGTERM, signalHandler);
    std::signal(SIGINT, signalHandler);
    registerCrashHandler();
#endif

#if defined(__APPLE__)
    uint32_t bufSize = 32;
    std::vector<char> exePath(bufSize);
    int ret = _NSGetExecutablePath(exePath.data(), &bufSize);
    if (ret < 0) {
        exePath.resize(bufSize);
        _NSGetExecutablePath(exePath.data(), &bufSize);
    }
    chdir(dirname(exePath.data()));
#endif
    
    constexpr int testToneFrequency = 200;

    int sineTime = 0;

    auto ctxBuilder = Main::ContextBuilder<
#if defined(ANDROID) || defined(__ANDROID__)
            Audio::Oboe,
#elif defined(__linux__)
            Audio::Alsa,
#elif defined(_WIN32) || defined(__APPLE__)
            Audio::PortAudio,
#elif defined(__EMSCRIPTEN__)
            Audio::WebAudio,
#endif
            Target::SDL2,
            Renderer::Type::NanoVG>();

    ctxBuilder
        //.setPitchSolver(new Analysis::Pitch::Yin(0.4f))
        .setPitchSolver(new Analysis::Pitch::RAPT)
        //.setPitchSolver(new Analysis::Pitch::MPM)
        .setLinpredSolver(new Analysis::LP::Burg)
        .setFormantSolver(new Analysis::Formant::FilteredLP)
        .setInvglotSolver(new Analysis::Invglot::IAIF(0.99f))
        //.setInvglotSolver(new Analysis::Invglot::GFM_IAIF(0.99f))
        //.setInvglotSolver(new Analysis::Invglot::AMGIF(8))
        .setCaptureSampleRate(48000)
        .setCaptureDuration(100ms)
#ifdef __EMSCRIPTEN__
        .setPlaybackBlockDuration(100ms, 300ms)
        .setPlaybackDuration(400ms)
#else
        .setPlaybackBlockDuration(10ms, 25ms)
        .setPlaybackDuration(50ms)
#endif
        .setPlaybackSampleRate(48000)
        .setPlaybackCallback([](auto...){});

    Main::ContextManager manager(ctxBuilder.build());

    manager.initialize();
    manager.start();
    manager.terminate();

    return 0;
}

