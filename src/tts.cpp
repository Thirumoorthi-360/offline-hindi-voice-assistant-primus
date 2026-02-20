/*
 * ============================================================
 *  PRIMUS AI v2.0 — TTS with Litter Smoother
 *  espeak-ng → WAV → DSP chain → aplay
 *
 *  DSP Chain:
 *   1. Litter Smoother   — removes tiny noise spikes
 *   2. Gaussian Smooth   — softens harshness
 *   3. Bass Boost        — deeper male voice
 *   4. Noise Gate        — clears silence gaps
 *   5. Normalize         — consistent volume
 *   6. Soft Limiter      — tanh, no clipping
 *   7. Normalize         — final level
 *   8. Fade In/Out       — removes click/pop
 * ============================================================
 */

#include "tts.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <numeric>

using namespace std;

#define SAMPLE_RATE 22050

/* =========================
   CONSTRUCTOR
========================= */

TTS::TTS(float g, int s, int p){
    gain  = g;
    speed = s;
    pitch = p;
}

void TTS::setTone(float g, int s, int p){
    gain  = g;
    speed = s;
    pitch = p;
}

/* =========================
   READ WAV
========================= */

static vector<double> readWav(const string& filename){
    ifstream file(filename, ios::binary);
    if(!file) return {};
    file.seekg(44);
    vector<double> samples;
    int16_t s;
    while(file.read(reinterpret_cast<char*>(&s), sizeof(s)))
        samples.push_back(s / 32768.0);
    return samples;
}

/* =========================
   WRITE WAV
========================= */

static void writeWav(const string& filename, const vector<double>& samples){
    fstream file(filename, ios::in | ios::out | ios::binary);
    if(!file) return;
    file.seekp(44);
    for(double s : samples){
        s = max(-1.0, min(1.0, s));
        int16_t out = static_cast<int16_t>(s * 32767);
        file.write(reinterpret_cast<char*>(&out), sizeof(out));
    }
    file.close();
}

/* =========================
   0. LITTER SMOOTHER
   Removes tiny random noise
   spikes (litter) that are
   shorter than minDuration
   samples and smaller than
   threshold — without
   touching the real voice.

   Algorithm:
   - Scan for isolated spikes
   - If spike width < window
     AND neighbors are near 0
     → zero it out (it's litter)
   - Real voice has sustained
     energy so it passes through
========================= */

static vector<double> litterSmoother(const vector<double>& in,
                                      int    window    = 8,
                                      double threshold = 0.04)
{
    vector<double> out = in;
    int n = in.size();

    for(int i = window; i < n - window; i++){

        double cur = fabs(in[i]);

        // Only look at samples above threshold
        if(cur < threshold) continue;

        // Check if neighbors (window samples each side) are quiet
        double leftEnergy  = 0.0;
        double rightEnergy = 0.0;

        for(int j = 1; j <= window; j++){
            leftEnergy  += fabs(in[i - j]);
            rightEnergy += fabs(in[i + j]);
        }

        leftEnergy  /= window;
        rightEnergy /= window;

        // If both sides are near silence → isolated spike = litter
        if(leftEnergy < threshold * 0.5 &&
           rightEnergy < threshold * 0.5)
        {
            // Smooth it out with neighbors instead of hard zero
            out[i] = (in[i-1] + in[i+1]) * 0.5;
        }
    }

    return out;
}

/* =========================
   1. GAUSSIAN SMOOTH
   Removes harshness
========================= */

static vector<double> gaussianSmooth(const vector<double>& in, int radius = 2){
    int n = in.size();
    vector<double> out = in;

    vector<double> kernel(2*radius+1);
    double sigma = radius / 2.0;
    double sum   = 0;
    for(int i = -radius; i <= radius; i++){
        kernel[i+radius] = exp(-(i*i)/(2.0*sigma*sigma));
        sum += kernel[i+radius];
    }
    for(auto& k : kernel) k /= sum;

    for(int i = radius; i < n - radius; i++){
        double val = 0;
        for(int j = -radius; j <= radius; j++)
            val += in[i+j] * kernel[j+radius];
        out[i] = val;
    }
    return out;
}

/* =========================
   2. BASS BOOST
   Deeper male voice
========================= */

static vector<double> bassBoost(const vector<double>& in, double amount = 0.28){
    vector<double> lowpass = in;
    double alpha = 0.15;
    for(size_t i = 1; i < in.size(); i++)
        lowpass[i] = alpha * in[i] + (1.0 - alpha) * lowpass[i-1];
    vector<double> out(in.size());
    for(size_t i = 0; i < in.size(); i++)
        out[i] = in[i] + amount * lowpass[i];
    return out;
}

/* =========================
   3. NOISE GATE
   Cuts silence hiss
========================= */

static vector<double> noiseGate(const vector<double>& in, double threshold = 0.012){
    vector<double> out = in;
    for(size_t i = 0; i < in.size(); i++)
        if(fabs(in[i]) < threshold)
            out[i] = 0.0;
    return out;
}

/* =========================
   4. NORMALIZE
========================= */

static vector<double> normalize(const vector<double>& in, double target = 0.92){
    double peak = 0.0;
    for(double s : in) peak = max(peak, fabs(s));
    if(peak < 1e-9) return in;
    vector<double> out(in.size());
    double g = target / peak;
    for(size_t i = 0; i < in.size(); i++)
        out[i] = in[i] * g;
    return out;
}

/* =========================
   5. SOFT LIMITER
   tanh saturation, no clip
========================= */

static vector<double> softLimit(const vector<double>& in, double drive = 1.8){
    vector<double> out(in.size());
    double td = tanh(drive);
    for(size_t i = 0; i < in.size(); i++)
        out[i] = tanh(drive * in[i]) / td;
    return out;
}

/* =========================
   6. FADE IN/OUT
   Removes click at edges
========================= */

static vector<double> fadeInOut(const vector<double>& in, int fadeMs = 15){
    vector<double> out = in;
    int fadeSamples = (SAMPLE_RATE * fadeMs) / 1000;
    fadeSamples = min(fadeSamples, (int)in.size() / 4);
    for(int i = 0; i < fadeSamples; i++){
        double env = (double)i / fadeSamples;
        out[i]                  *= env;
        out[out.size()-1-i]     *= env;
    }
    return out;
}

/* =========================
   PROCESS WAV — Full chain
========================= */

void TTS::processWav(const string& filename){

    vector<double> samples = readWav(filename);
    if(samples.empty()) return;

    // Full DSP chain
    samples = litterSmoother(samples, 8,   0.04);  // 0. litter smoother
    samples = gaussianSmooth(samples, 2);           // 1. smooth harshness
    samples = bassBoost     (samples, 0.28);        // 2. deeper voice
    samples = noiseGate     (samples, 0.012);       // 3. kill hiss
    samples = normalize     (samples, 0.90);        // 4. normalize
    samples = softLimit     (samples, 1.8);         // 5. soft limit
    samples = normalize     (samples, 0.92);        // 6. final normalize
    samples = fadeInOut     (samples, 15);          // 7. clean edges

    writeWav(filename, samples);
}

/* =========================
   SPEAK
========================= */

void TTS::speak(const string& text){

    string safeText = text;
    for(char& c : safeText)
        if(c == '"' || c == '`') c = '\'';

    string wavFile = "/tmp/primus_tts.wav";

    string cmd =
        "espeak-ng -v hi"
        " -s " + to_string(speed) +
        " -p " + to_string(pitch) +
        " -a 180"
        " -g 6"
        " -w " + wavFile +
        " \"" + safeText + "\" 2>/dev/null";

    system(cmd.c_str());

    processWav(wavFile);

    string playCmd =
        "aplay -q -f S16_LE -r " +
        to_string(SAMPLE_RATE) +
        " -c 1 " + wavFile + " 2>/dev/null";

    system(playCmd.c_str());
}