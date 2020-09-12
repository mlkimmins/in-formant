#ifndef MAIN_CONTEXT_MANAGER_H
#define MAIN_CONTEXT_MANAGER_H

#include "context.h"
#include "../nodes/nodes.h"

#include <map>
#include <memory>
#include <vector>

namespace Main {

    using namespace Module;

    class ContextManager {
    public:
        ContextManager(std::unique_ptr<Context>& ctx);
        ~ContextManager();

        void initialize();
        void start();

        void loadSettings();

        void updateRendererTargetSize();
        void loadRenderingParameters(Renderer::Parameters *p);

        void createAudioNodes();
        void createAudioIOs();

        void propagateAudio();
        void processAudioNode(const std::string& nodeName, const char *in, const char *out);

        void mainBody();

    private:
        std::unique_ptr<Context> ctx;

        Freetype::FontFile *primaryFont;

        std::map<std::string, std::unique_ptr<Nodes::Node>>                nodes;
        std::map<std::string, std::vector<std::unique_ptr<Nodes::NodeIO>>> nodeIOs;
        Nodes::NodeIO **ndi, **ndo;

        int analysisDuration;
        int analysisMaxFrequency;

        int viewMinFrequency;
        int viewMaxFrequency;
        int viewMinGain;
        int viewMaxGain;
        Renderer::FrequencyScale viewFrequencyScale;

        int fftLength;
        int fftMaxFrequency;

        float preEmphasisFrequency;
        int linPredOrder;

        int spectrogramCount;

        int numFormantsToRender;
        std::vector<std::array<float, 3>> formantColors;

        std::deque<std::vector<std::array<float, 2>>>  spectrogramTrack;
        std::deque<float>                              pitchTrack;
        std::deque<std::vector<Analysis::FormantData>> formantTrack;

    };
    
}

#endif // MAIN_CONTEXT_MANAGER_H