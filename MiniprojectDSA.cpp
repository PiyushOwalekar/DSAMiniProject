#include <iostream>
#include <vector>
#include <cfloat>
#include <iomanip>
#include <algorithm>

using namespace std;

struct TrafficData {
    int intervalNumber;
    int arrivalRate;
};

vector<TrafficData> takeInput(int &n) {
    cout << "\n========================================\n";
    cout << "   SMART TRAFFIC SIGNAL TIMING SYSTEM   \n";
    cout << "========================================\n";

    cout << "\nEnter number of time intervals: ";
    cin >> n;

    while (n <= 0) {
        cout << "Invalid! Enter a positive number: ";
        cin >> n;
    }

    vector<TrafficData> data(n);

    cout << "\nEnter vehicle arrival rates for each interval:\n";
    cout << "------------------------------------------------\n";

    for (int i = 0; i < n; i++) {
        data[i].intervalNumber = i + 1;
        cout << "  Interval " << i + 1 << " (vehicles/sec): ";
        cin >> data[i].arrivalRate;
        
        while (data[i].arrivalRate < 0) {
            cout << "  Invalid! Enter a non-negative value: ";
            cin >> data[i].arrivalRate;
        }
    }

    cout << "\n--- Input Summary ---\n";
    cout << left << setw(12) << "Interval"
         << setw(20) << "Arrival Rate" << "\n";
    cout << "-----------------------------\n";
    for (int i = 0; i < n; i++) {
        cout << left << setw(12) << data[i].intervalNumber
             << setw(20) << data[i].arrivalRate << "\n";
    }

    return data;
}

struct SimulationResult {
    int greenTime;
    int redTime;
    double avgWaitTime;
    int maxQueueSize;
    int totalVehicles;
    int totalServed;
    int totalLeftover;
};

SimulationResult simulate(vector<TrafficData> &data, int greenTime) {
    SimulationResult result;
    result.greenTime    = greenTime;
    result.redTime      = 60 - greenTime;
    result.maxQueueSize = 0;
    result.totalServed  = 0;
    result.totalLeftover= 0;
    result.totalVehicles= 0;

    int totalWait = 0;
    int queue     = 0;
    int n         = data.size();
    
    for (int i = 0; i < n; i++) {
        queue += data[i].arrivalRate;
        result.totalVehicles += data[i].arrivalRate;

        if (queue > result.maxQueueSize)
            result.maxQueueSize = queue;

        int served = min(queue, greenTime);
        queue -= served;
        result.totalServed += served;

        result.totalLeftover += queue;
        totalWait += queue * result.redTime;
    }

    result.avgWaitTime = (double)totalWait / n;
    return result;
}

// Display detailed simulation result for a given green time
void displaySimulation(SimulationResult &res) {
    cout << "\n  Green=" << res.greenTime << "s"
         << " | Red=" << res.redTime << "s"
         << " | AvgWait=" << fixed << setprecision(2) << res.avgWaitTime << "s"
         << " | MaxQueue=" << res.maxQueueSize
         << " | Served=" << res.totalServed
         << " | Leftover=" << res.totalLeftover;
}

struct SearchStep {
    int step;
    int lo, hi, mid;
    double waitMid;
    double waitMid1;
    string direction;
};

SimulationResult binarySearchOptimal(vector<TrafficData> &data) {
    int lo = 10, hi = 50;
    double bestWait    = DBL_MAX;
    int optimalGreen   = lo;
    int stepCount      = 0;

    vector<SearchStep> searchLog;

    cout << "\n--- Binary Search Progress ---\n";
    cout << left
         << setw(6)  << "Step"
         << setw(6)  << "Lo"
         << setw(6)  << "Hi"
         << setw(6)  << "Mid"
         << setw(12) << "Wait@Mid"
         << setw(12) << "Wait@Mid+1"
         << setw(12) << "Direction" << "\n";
    cout << string(60, '-') << "\n";

    while (lo <= hi) {
        stepCount++;
        int mid = (lo + hi) / 2;

        SimulationResult resMid  = simulate(data, mid);
        SimulationResult resMid1 = simulate(data, mid + 1);

        if (resMid.avgWaitTime < bestWait) {
            bestWait     = resMid.avgWaitTime;
            optimalGreen = mid;
        }

        string direction;
        if (resMid1.avgWaitTime < resMid.avgWaitTime) {
            lo        = mid + 1;
            direction = "RIGHT →";
        } else {
            hi        = mid - 1;
            direction = "← LEFT";
        }

        cout << left
             << setw(6)  << stepCount
             << setw(6)  << lo
             << setw(6)  << hi
             << setw(6)  << mid
             << setw(12) << fixed << setprecision(1) << resMid.avgWaitTime
             << setw(12) << fixed << setprecision(1) << resMid1.avgWaitTime
             << setw(12) << direction << "\n";
    }

    cout << "\nTotal Search Steps: " << stepCount
         << " (out of possible 40 linear steps)\n";

    return simulate(data, optimalGreen);
}

void displaySignalBar(int greenTime) {
    int redTime = 60 - greenTime;
    cout << "\nSignal Cycle (60s):\n  [";

    for (int i = 0; i < greenTime / 2; i++) cout << "G";
    cout << "|";

    for (int i = 0; i < redTime / 2; i++) cout << "R";
    cout << "]\n";
    cout << "  |←── " << greenTime << "s Green ──→|←── "
         << redTime << "s Red ──→|\n";
}

void displayEfficiency(SimulationResult &optimal,
                       SimulationResult &worst) {
    double improvement = 0;
    if (worst.avgWaitTime > 0)
        improvement = ((worst.avgWaitTime - optimal.avgWaitTime)
                       / worst.avgWaitTime) * 100;

    cout << "\n--- Efficiency Metrics ---\n";
    cout << "  Worst case wait  : " << fixed << setprecision(2)
         << worst.avgWaitTime << "s  (green=10s)\n";
    cout << "  Optimal wait     : " << fixed << setprecision(2)
         << optimal.avgWaitTime << "s  (green=" << optimal.greenTime << "s)\n";
    cout << "  Improvement      : " << fixed << setprecision(1)
         << improvement << "% reduction in wait time\n";
    cout << "  Vehicles served  : " << optimal.totalServed << "\n";
    cout << "  Vehicles leftover: " << optimal.totalLeftover << "\n";
    cout << "  Peak queue size  : " << optimal.maxQueueSize << "\n";
}

void displayResults(SimulationResult &optimal,
                    SimulationResult &worst) {
    cout << "\n\n========================================\n";
    cout << "             FINAL RESULTS              \n";
    cout << "========================================\n";
    cout << "  Optimal Green Duration : " << optimal.greenTime << " seconds\n";
    cout << "  Red Signal Duration    : " << optimal.redTime   << " seconds\n";
    cout << fixed << setprecision(2);
    cout << "  Min Avg Waiting Time   : " << optimal.avgWaitTime << " seconds\n";

    displaySignalBar(optimal.greenTime);
    displayEfficiency(optimal, worst);

    cout << "\n========================================\n";
    cout << "  Status: ";
    if (optimal.avgWaitTime == 0)
        cout << "✅ No congestion — all vehicles pass!\n";
    else if (optimal.avgWaitTime < 50)
        cout << "⚡ Moderate wait — traffic manageable.\n";
    else
        cout << "⚠️  High wait — consider adding lanes.\n";
    cout << "========================================\n";
}

int main() {
    int n;
    vector<TrafficData> data = takeInput(n);

    cout << "\n\n--- Simulating Traffic Flow ---\n";
    SimulationResult optimal = binarySearchOptimal(data);

    SimulationResult worst = simulate(data, 10);

    displayResults(optimal, worst);
    return 0;
}
