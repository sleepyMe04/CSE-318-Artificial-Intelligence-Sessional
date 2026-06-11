#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <random>
#include <unordered_set>
#include <unordered_map>
#include <string>
#include <sstream>
#include <codecvt>
#include <locale>
#include <windows.h>
#include <climits>
#include <cmath>
#include <stdexcept>
#include <ctime>
#include <iomanip>
#include <string>

struct Edge
{
    int src; // 1-based index
    int dest;
    int weight;

    Edge(int s, int d, int w) : src(s), dest(d), weight(w) {}
};

class CompleteMaxCut
{
private:
    int numVertices;
    std::vector<Edge> edges;
    std::vector<std::vector<Edge>> adjacencyList;
    std::mt19937 random;

public:
    void shuffleArray(std::vector<int> &array)
    {
        for (int i = array.size() - 1; i > 0; i--)
        {
            std::uniform_int_distribution<> dis(0, i);
            int j = dis(random);
            std::swap(array[i], array[j]);
        }
    }

    int calculateCut(const std::vector<bool> &partition)
    {
        int cut = 0;
        for (const Edge &e : edges)
        {
            if (partition[e.src] != partition[e.dest])
            {
                cut += e.weight;
            }
        }
        return cut;
    }

    CompleteMaxCut(int numNodes, const std::vector<Edge> &edgeList)
        : numVertices(numNodes), edges(edgeList), adjacencyList(numNodes + 1)
    {

        std::random_device rd;
        random.seed(rd());

        for (const Edge &e : edges)
        {
            adjacencyList[e.src].push_back(e);
            adjacencyList[e.dest].push_back(Edge(e.dest, e.src, e.weight));
        }
    }

    double randomizedMaxCut(int numRuns)
    {
        double total = 0;
        std::vector<bool> partition(numVertices + 1);
        std::uniform_int_distribution<> dis(0, 1);

        for (int i = 0; i < numRuns; i++)
        {
            for (int v = 1; v <= numVertices; v++)
            {
                partition[v] = dis(random);
            }
            total += calculateCut(partition);
        }
        return total / numRuns;
    }

    int greedyMaxCut()
    {
        std::vector<bool> partition(numVertices + 1);
        if (edges.empty())
            return 0;

        auto maxEdge = *std::max_element(edges.begin(), edges.end(),
                                         [](const Edge &a, const Edge &b)
                                         { return a.weight < b.weight; });
        partition[maxEdge.src] = true;
        partition[maxEdge.dest] = false;

        for (int v = 1; v <= numVertices; v++)
        {
            if (v == maxEdge.src || v == maxEdge.dest)
                continue;

            int sigmaX = 0, sigmaY = 0;
            for (const Edge &e : adjacencyList[v])
            {
                if (partition[e.dest])
                {
                    sigmaY += e.weight;
                }
                else
                {
                    sigmaX += e.weight;
                }
            }
            partition[v] = (sigmaX > sigmaY);
        }
        return calculateCut(partition);
    }

    std::vector<bool> semiGreedyMaxCut(double alpha)
    {
        std::vector<bool> partition(numVertices + 1, false);
        std::vector<bool> assigned(numVertices + 1, false);
        std::vector<int> unassigned;

        for (int v = 1; v <= numVertices; ++v)
        {
            unassigned.push_back(v);
        }

        shuffleArray(unassigned); // Randomize initial order

        for (int step = 0; step < numVertices; ++step)
        {
            std::vector<std::tuple<int, int, int>> candidates; // vertex, gainX, gainY

            for (int v : unassigned)
            {
                int gainX = 0, gainY = 0;
                for (const Edge &e : adjacencyList[v])
                {
                    if (assigned[e.dest])
                    {
                        if (partition[e.dest])
                            gainY += e.weight;
                        else
                            gainX += e.weight;
                    }
                }
                candidates.emplace_back(v, gainX, gainY);
            }

            if (candidates.empty())
                break;

            // Evaluate greedy function (gain difference)
            std::vector<std::pair<int, int>> scores; // vertex, gain diff
            int maxGain = INT_MIN, minGain = INT_MAX;

            for (const auto &t : candidates)
            {
                int v = std::get<0>(t);
                int gx = std::get<1>(t);
                int gy = std::get<2>(t);
                int score = std::abs(gx - gy);
                scores.emplace_back(v, score);
                maxGain = std::max(maxGain, score);
                minGain = std::min(minGain, score);
            }

            // Build Restricted Candidate List (RCL)
            std::vector<int> RCL;
            for (const auto &p : scores)
            {
                int v = p.first;
                int score = p.second;
                if (score >= maxGain - alpha * (maxGain - minGain))
                {
                    RCL.push_back(v);
                }
            }

            // Pick randomly from RCL
            std::uniform_int_distribution<> dis(0, RCL.size() - 1);
            int chosen = RCL[dis(random)];

            // Assign to the better partition
            int gainX = 0, gainY = 0;
            for (const Edge &e : adjacencyList[chosen])
            {
                if (assigned[e.dest])
                {
                    if (partition[e.dest])
                        gainY += e.weight;
                    else
                        gainX += e.weight;
                }
            }
            partition[chosen] = (gainX > gainY);
            assigned[chosen] = true;
            unassigned.erase(std::remove(unassigned.begin(), unassigned.end(), chosen), unassigned.end());
        }

        return partition;
    }

    int localSearch(std::vector<bool> &partition)
    {
        int currentCut = calculateCut(partition);
        bool improved;
        std::vector<int> vertices(numVertices);
        for (int i = 0; i < numVertices; i++)
            vertices[i] = i + 1;

        do
        {
            improved = false;
            shuffleArray(vertices);

            for (int v : vertices)
            {
                int delta = 0;
                for (const Edge &e : adjacencyList[v])
                {
                    if (partition[v] == partition[e.dest])
                        delta += e.weight;
                    else
                        delta -= e.weight;
                }

                if (delta > 0)
                {
                    partition[v] = !partition[v];
                    currentCut += delta;
                    improved = true;
                }
            }

        } while (improved);

        return currentCut;
    }

    int graspMaxCut(double alpha, int iterations)
    {
        int bestCut = 0;
        std::vector<bool> bestPartition;

        for (int i = 0; i < iterations; i++)
        {
            auto partition = semiGreedyMaxCut(alpha);
            int cut = localSearch(partition);

            if (cut > bestCut)
            {
                bestCut = cut;
                bestPartition = partition;
            }
        }
        return bestCut;
    }
};

std::vector<Edge> readGraph(const std::string &filename, int &numVertices, int &numEdges)
{
    std::vector<Edge> edges;
    std::ifstream file(filename);
    if (!file.is_open())
        throw std::runtime_error("Cannot open file: " + filename);

    file >> numVertices >> numEdges;
    edges.reserve(numEdges);

    int src, dest, weight;
    while (file >> src >> dest >> weight)
    {
        edges.emplace_back(src, dest, weight);
    }
    return edges;
}

std::vector<std::string> getGraphFiles(const std::string &folder)
{
    std::vector<std::string> files;
    WIN32_FIND_DATAW findFileData;

    std::wstring wideFolder = std::wstring(folder.begin(), folder.end()) + L"\\*.rud";
    HANDLE hFind = FindFirstFileW(wideFolder.c_str(), &findFileData);

    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                // Convert wide filename to UTF-8 string
                std::wstring wname(findFileData.cFileName);
                std::string fname(wname.begin(), wname.end());
                files.push_back(folder + "\\" + fname);
            }
        } while (FindNextFileW(hFind, &findFileData));
        FindClose(hFind);
    }

    // Sort files numerically based on number after 'g'
    std::sort(files.begin(), files.end(), [](const std::string &a, const std::string &b)
              {
                  int aNum = std::stoi(a.substr(a.find_last_of("g") + 1));
                  int bNum = std::stoi(b.substr(b.find_last_of("g") + 1));
                  return aNum < bNum; });

    return files;
}
// int main() {
//     std::string folderPath = "set1";
//     std::vector<std::string> graphFiles = getGraphFiles(folderPath);

//     // Open CSV file for writing
//     std::ofstream outputFile("maxcut_results.csv");
//     if (!outputFile.is_open()) {
//         std::cerr << "Error: Could not create output file!" << std::endl;
//         return 1;
//     }

//     // Known best solutions from the benchmark data
//     std::unordered_map<std::string, int> bestSolutions = {
//         {"G1", 12078}, {"G2", 12084}, {"G3", 12077}, {"G11", 627}, {"G12", 621}, {"G13", 645},
//         {"G14", 3187}, {"G15", 3169}, {"G16", 3172}, {"G22", 14123}, {"G23", 14129}, {"G24", 14131},
//         {"G32", 1560}, {"G33", 1537}, {"G34", 1541}, {"G35", 8000}, {"G36", 7996}, {"G37", 8009},
//         {"G43", 7027}, {"G44", 7022}, {"G45", 7020}, {"G48", 6000}, {"G49", 6000}, {"G50", 5988}
//     };

//     // Write CSV header
//     outputFile << "Problem,Constructive algorithm,Local search,GRASP,Known best solution or upper bound\n";
//     outputFile << "Name,|V| or n,|E| or m,Simple Randomized or Randomized-1,Simple Greedy or Greedy-1,Semi-greedy-1,Simple local or local-1,GRASP-1,\n";
//     outputFile << ",,,,,No. of iterations,Average value,No. of iterations,Best value,\n";

//     for (const auto& filename : graphFiles) {
//         try {
//             int numVertices, numEdges;
//             auto edges = readGraph(filename, numVertices, numEdges);
//             CompleteMaxCut solver(numVertices, edges);

//             // Extract graph name
//             size_t start = filename.find_last_of("\\") + 1;
//             size_t end = filename.find_last_of(".");
//             std::string graphName = filename.substr(start, end - start);

//             // Get known best solution (0 if not available)
//             int bestSolution = bestSolutions.count(graphName) ? bestSolutions[graphName] : 0;

//             // Run algorithms
//             double randomized = solver.randomizedMaxCut(100);  // 100 iterations for average
//             int greedy = solver.greedyMaxCut();

//             // Semi-greedy with alpha=0.5, 20 iterations
//             auto semiGreedyPartition = solver.semiGreedyMaxCut(0.5);
//             int semiGreedy = solver.calculateCut(semiGreedyPartition);

//             // Local search
//             int localSearchValue = solver.localSearch(semiGreedyPartition);

//             // GRASP with alpha=0.5, 50 iterations
//             int grasp = solver.graspMaxCut(0.5, 50);

//             // Write results to CSV
//             outputFile << graphName << ","
//                       << numVertices << ","
//                       << numEdges << ","
//                       << randomized << ","
//                       << greedy << ","
//                       << semiGreedy << ","
//                       << localSearchValue << ","
//                       << grasp << ","
//                       << bestSolution << "\n";

//             std::cout << "Processed: " << graphName << std::endl;
//         }
//         catch (const std::exception& e) {
//             std::cerr << "Error processing " << filename << ": " << e.what() << "\n";
//         }
//     }

//     outputFile.close();
//     std::cout << "Results saved to maxcut_results.csv" << std::endl;
//     return 0;
// }

int main()
{
    std::string folderPath = "set1";
    std::vector<std::string> graphFiles = getGraphFiles(folderPath);

    std::ofstream outputFile("maxcut_results.csv");
    if (!outputFile.is_open())
    {
        std::cerr << "Error: Could not create output file!" << std::endl;
        return 1;
    }

    std::unordered_map<std::string, int> bestSolutions = {
        {"g1", 12078}, {"g2", 12084}, {"g3", 12077}, {"g11", 627}, {"g12", 621}, {"g13", 645}, {"g14", 3187}, {"g15", 3169}, {"g16", 3172}, {"g22", 14123}, {"g23", 14129}, {"g24", 14131}, {"g32", 1560}, {"g33", 1537}, {"g34", 1541}, {"g35", 8000}, {"g36", 7996}, {"g37", 8009}, {"g43", 7027}, {"g44", 7022}, {"g45", 7020}, {"g48", 6000}, {"g49", 6000}, {"g50", 5988}};

    // Write CSV headers
    outputFile << "Problem,,,Constructive algorithm,,,Local search,,GRASP,,Known best solution or upper bound\n";
    outputFile << "Name,|V| or n,|E| or m,Simple Randomized or Randomized-1,Simple Greedy or Greedy-1,Semi-greedy-1,"
                  "No. of iterations,Average value,No. of iterations,Best value,\n";

    const int RANDOMIZED_ITERATIONS = 100;
    const int LOCAL_SEARCH_ITERATIONS = 10;
    const int GRASP_ITERATIONS = 50;

    for (const auto &filename : graphFiles)
    {
        try
        {
            int numVertices, numEdges;
            auto edges = readGraph(filename, numVertices, numEdges);
            CompleteMaxCut solver(numVertices, edges);

            // Extract graph name (without folder and extension)
            std::string graphName = filename.substr(filename.find_last_of("/\\") + 1);
            graphName = graphName.substr(0, graphName.find_last_of("."));

            // Get known best solution or "N/A"
            std::string bestSolutionStr = "N/A";
            auto it = bestSolutions.find(graphName);
            if (it != bestSolutions.end())
            {
                bestSolutionStr = std::to_string(it->second);
            }

            // Run algorithms
            double randomizedAvg = solver.randomizedMaxCut(RANDOMIZED_ITERATIONS);
            int greedy = solver.greedyMaxCut();

            auto semiGreedyPartition = solver.semiGreedyMaxCut(0.5);
            int semiGreedy = solver.calculateCut(semiGreedyPartition);

            double localSearchTotal = 0;
            for (int i = 0; i < LOCAL_SEARCH_ITERATIONS; ++i)
            {
                auto partition = solver.semiGreedyMaxCut(0.5);
                localSearchTotal += solver.localSearch(partition);
            }
            double localSearchAvg = localSearchTotal / LOCAL_SEARCH_ITERATIONS;

            int graspBest = solver.graspMaxCut(0.5, GRASP_ITERATIONS);

            // Output to CSV
            outputFile << graphName << "," << numVertices << "," << numEdges << ","
                       << randomizedAvg << "," << greedy << "," << semiGreedy << ","
                       << LOCAL_SEARCH_ITERATIONS << "," << localSearchAvg << ","
                       << GRASP_ITERATIONS << "," << graspBest << ","
                       << bestSolutionStr << "\n";

            std::cout << "Processed: " << graphName << std::endl;
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error processing " << filename << ": " << e.what() << "\n";
        }
    }

    outputFile.close();
    std::cout << "Results saved to maxcut_results.csv" << std::endl;
    return 0;
}