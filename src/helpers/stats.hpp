#ifndef STATS_HPP
#define STATS_HPP

#include <iostream>
#include <vector>
#include <algorithm>
#include <memory>

#include "utils.hpp"

using namespace std;

using StatIndex = int;

template<typename T>
struct Stat {
    string label;
    T value;
    Stat(const string& l, T v) : label(l), value(v) {};
};

template<typename T>
class StatsPrimitive {
    private:
    vector<string> m_labels = {};
    vector<T> m_values = {};

    public:
    StatIndex registerStat(const string& label){
        m_labels.push_back(label);
        m_values.push_back(T{});
        return (StatIndex)(numStats() - 1);
    };

    vector<StatIndex> getSortPermutation() const {
        vector<StatIndex> indices = vector<StatIndex>(numStats());
        for (StatIndex i = 0; i < numStats(); i++)
            indices[i] = i;
        
        sort(indices.begin(), indices.end(), [&](const StatIndex& a, const StatIndex& b){
            return m_values[a] > m_values[b];
        });

        return indices;
    }

    int numStats() const { return (int)m_labels.size(); }
    
    void set(StatIndex i, T value){
        if (i >= numStats()) return;
        m_values[i] = value;
    }

    Stat<T> get(StatIndex i) const {
        return {
            m_labels[i],
            m_values[i]
        };
    }

    const vector<string>& getLabels() const { return m_labels; }
    const vector<T>& getValues() const { return m_values; }
};

class Stats {
    private:
    StatsPrimitive<TimerValue> timers = {};
    StatsPrimitive<CounterValue> counters = {};
    StatsPrimitive<StorageValue> storages = {};
    const StatIndex totalTimeStatIndex = 0;

    public:
    string name = "";

    Stats() {
        timers.registerStat("Total time");
        timers.set(totalTimeStatIndex, 1e-6f);
    }
    
    int numStats() const {
        return timers.numStats() + counters.numStats() + storages.numStats();
    }

    StatIndex registerTimer(const string& label)   { return timers.registerStat(label);   }
    StatIndex registerCounter(const string& label) { return counters.registerStat(label); }
    StatIndex registerStorage(const string& label) { return storages.registerStat(label); }

    void setTimer(StatIndex i, TimerValue value) {
        timers.set(totalTimeStatIndex, timers.get(totalTimeStatIndex).value + value - timers.get(i).value); 
        timers.set(i, value);
    }
    void setCounter(StatIndex i, CounterValue value) { counters.set(i, value); }
    void setStorage(StatIndex i, StorageValue value) { storages.set(i, value); }

    Stat<TimerValue>   getTimer(StatIndex i)   { return timers.get(i);   }
    Stat<CounterValue> getCounter(StatIndex i) { return counters.get(i); }
    Stat<StorageValue> getStorage(StatIndex i) { return storages.get(i); }

    const StatsPrimitive<TimerValue>&   getTimers()   { return timers; }
    const StatsPrimitive<CounterValue>& getCounters() { return counters; }
    const StatsPrimitive<StorageValue>& getStorages() { return storages; }

    const StatsPrimitive<TimerValue>&   getSortedTimers()   { return timers; }
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