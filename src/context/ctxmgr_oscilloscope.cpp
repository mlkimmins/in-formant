#include "contextmanager.h"
#include "../analysis/util/util.h"
#include <iostream>

using namespace Main;

void ContextManager::renderOscilloscope(RenderingContext &rctx)
{
    int iframe = 
        useFrameCursor ? std::min(std::max<int>(std::round(specMX * spectrogramCount), 0), spectrogramCount - 1)
                       : spectrogramCount - 1;

    auto& sound = soundTrack[iframe];
    auto& glot = glotTrack[iframe];

    int glotZcr = glot.size() - 1;

    for (int i = glotZcr; i >= 1; --i) {
        if (glot[i - 1] * glot[i] < 0 && glot[i - 1] > 0) {
            glotZcr = i;
            break;
        }
    }

    constexpr int numPeriods = 6;

    int glotPeriod = pitchTrack[iframe] > 0 ? 48'000 / pitchTrack[iframe] : glot.size() / numPeriods;
    int glotLength = numPeriods * glotPeriod;

    static float prevGlotLength;
    prevGlotLength = 0.95f * prevGlotLength + 0.05f * glotLength;
    glotLength = std::min<int>(std::floor(prevGlotLength), glot.size());   

    if (glotLength > glotZcr) {
        glotLength = glotZcr;
    }

    if (glot.size() == 0) {
        glotLength = 0;
    }

    int soundZcr = glot.size() > 0 ? (glotZcr * sound.size()) / glot.size() : 0;
    int soundPeriod = glot.size() > 0 ? (glotPeriod * sound.size()) / glot.size() : 0;
    int soundLength = glot.size() > 0 ? (glotLength * sound.size()) / glot.size() : 0;

    Renderer::GraphRenderData graphRender(soundLength);
    for (int i = 0; i < soundLength; ++i) {
        graphRender[i] = {
            .x = static_cast<float>(i),
            .y = sound[soundZcr - soundLength + i] * 20.0f + 5.0f,
        };
    }
    if (!graphRender.empty()) {
        rctx.renderer->renderGraph(graphRender, 0, soundLength - 1, 3.0f, 1.0f, 1.0f, 1.0f);
    }
    else {
        graphRender.push_back({.x = 0.0f, .y = 5.0f});
        graphRender.push_back({.x = 1.0f, .y = 5.0f});
        rctx.renderer->renderGraph(graphRender, 0, 1, 3.0f, 1.0f, 1.0f, 1.0f);
    }

    graphRender.resize(glotLength);

    for (int i = 0; i < glotLength; ++i) {
        graphRender[i] = {
            .x = static_cast<float>(i),
            .y = glot[glotZcr - glotLength + i] * 2.5f - 5.0f,
        };
    }
    if (!graphRender.empty()) {
        rctx.renderer->renderGraph(graphRender, 0, glotLength - 1, 3.0f, 1.0f, 0.5f, 0.0f);
    } 
    else {
        graphRender.push_back({.x = 0.0f, .y = -5.0f});
        graphRender.push_back({.x = 1.0f, .y = -5.0f});
        rctx.renderer->renderGraph(graphRender, 0, 1, 3.0f, 1.0f, 0.5f, 0.0f);
    }
}

void ContextManager::eventOscilloscope(RenderingContext &rctx)
{
}

