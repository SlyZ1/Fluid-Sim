#ifndef STATS_HPP
#define STATS_HPP

#include <iostream>
#include <vector>
#include "utils.hpp"
#include "algorithm"

using namespace std;

struct Stat {
    string label;
    float time;
};

class Stats {
    private:
    vector<string> labels = {};
    vector<float> times = {};

    public:
    string name = "";

    void add(string label){
        labels.push_back(label);
        times.push_back(0);
    };

    void set(string label, float time){
        auto statIndex = find(labels.begin(), labels.end(), label) - labels.begin();
        if (statIndex >= numStats()) return;
        times[statIndex] = time;
    }

    int numStats() const { return (int)labels.size(); }

    Stat get(int index) const {
        return {
            labels[index],
            times[index]
        };
    }

    Stat get(string label) const {
        auto statIndex = find(labels.begin(), labels.end(), label) - labels.begin();
        if (statIndex >= numStats()) return Stat();
        return {
            labels[statIndex],
            times[statIndex]
        };
    }

    const vector<string>& getLabels() const { return labels; }
    const vector<float>& getTimes() const { return times; }
};

class IStatsProvider {
    protected:
    shared_ptr<Stats> m_stats;

    public:
    IStatsProvider(string name) {
        m_stats = make_shared<Stats>();
        m_stats->name = name;
    };
    virtual ~IStatsProvider() {};
    shared_ptr<Stats> getStats() const { return m_stats; }
};

#endif