#include <iostream>
#include <vector>
#include <map>
#include <cmath>
#include <algorithm>
#include <random>
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <set>
using namespace std;

// --- Data Structures ---
struct AttributeValue
{
    bool isNumeric;
    string categorical;
    double numeric;

    AttributeValue() : isNumeric(false), categorical(""), numeric(0.0) {}
    AttributeValue(bool num, const string &cat, double val) : isNumeric(num), categorical(cat), numeric(val) {}
};

struct Example
{
    map<string, AttributeValue> attributes;
    string label;
};

struct TreeNode
{
    string attribute;
    string label;
    map<string, TreeNode *> children;
    bool isLeaf;

    TreeNode() : attribute(""), label(""), isLeaf(false) {}
};

struct DatasetConfig
{
    bool has_header;
    vector<string> attribute_names;
    vector<bool> is_numeric;
    int label_index;
    char delimiter;
};

string trim(const string &str)
{
    size_t start = str.find_first_not_of(" \t\n\r");
    if (start == string::npos)
        return "";
    size_t end = str.find_last_not_of(" \t\n\r");
    return str.substr(start, end - start + 1);
}

string handleMissing(const string &value)
{
    string trimmed = trim(value);
    return (trimmed == "?" || trimmed.empty() || trimmed == "Unknown" || trimmed == "NA") ? "Unknown" : trimmed;
}
string discretizeNumeric(const string &attrName, double value)
{
    if (attrName == "age")
    {
        if (value < 25) return "age_<25";
        else if (value < 35) return "age_25-34";
        else if (value < 45) return "age_35-44";
        else if (value < 55) return "age_45-54";
        else if (value < 65) return "age_55-64";
        else return "age_65+";
    }
    else if (attrName == "fnlwgt")
    {
        if (value < 100000) return "low";
        else if (value < 200000) return "medium_low";
        else if (value < 300000) return "medium";
        else if (value < 400000) return "medium_high";
        else return "high";
    }
    else if (attrName == "hours-per-week")
    {
        if (value < 20) return "very_part_time";
        else if (value < 35) return "part_time";
        else if (value < 45) return "full_time";
        else if (value < 60) return "over_time";
        else return "extreme";
    }
    else if (attrName == "capital-gain")
    {
        if (value == 0) return "none";
        else if (value < 1000) return "very_low";
        else if (value < 5000) return "low";
        else if (value < 10000) return "medium";
        else return "high";
    }
    else if (attrName == "capital-loss")
    {
        if (value == 0) return "none";
        else if (value < 500) return "low";
        else if (value < 1500) return "medium";
        else return "high";
    }
    else if (attrName == "education-num")
    {
        if (value < 6) return "elementary";
        else if (value < 9) return "middle_school";
        else if (value < 12) return "high_school";
        else if (value < 14) return "college";
        else return "advanced";
    }
    else if (attrName == "sepal_length")
    {
        if (value < 5.4)
            return "small";
        else if (value < 6.3)
            return "medium";
        else
            return "large";
    }
    else if (attrName == "sepal_width")
    {
        if (value < 2.8)
            return "narrow";
        else if (value < 3.3)
            return "medium";
        else
            return "wide";
    }
    else if (attrName == "petal_length")
    {
        if (value < 2.5)
            return "short";
        else if (value < 4.8)
            return "medium";
        else
            return "long";
    }
    else if (attrName == "petal_width")
    {
        if (value < 0.8)
            return "narrow";
        else if (value < 1.6)
            return "medium";
        else
            return "wide";
    }


    
    // Default fallback
    int bucket = (int)(value / 10);
    return "bucket_" + to_string(bucket);
}


double calculateEntropy(const vector<Example> &examples)
{
    if (examples.empty())
        return 0.0;

    map<string, int> classCounts;
    for (const auto &ex : examples)
    {
        classCounts[ex.label]++;
    }

    double entropy = 0.0;
    int total = examples.size();

    for (const auto &pair : classCounts)
    {
        double p = (double)pair.second / total;
        if (p > 0)
        {
            entropy -= p * log2(p);
        }
    }

    return entropy;
}

map<string, vector<Example>> splitData(const vector<Example> &examples, const string &attribute)
{
    map<string, vector<Example>> splits;

    // First pass: collect all non-missing values and find mode
    map<string, int> valueCounts;
    for (const auto &ex : examples)
    {
        string value;
        if (ex.attributes.at(attribute).isNumeric)
        {
            value = discretizeNumeric(attribute, ex.attributes.at(attribute).numeric);
        }
        else
        {
            value = handleMissing(ex.attributes.at(attribute).categorical);
        }

        if (value != "Unknown")
        {
            valueCounts[value]++;
        }
    }

    // Find mode (most frequent value)
    string mode = "Unknown";
    int maxCount = 0;
    for (const auto &pair : valueCounts)
    {
        if (pair.second > maxCount)
        {
            maxCount = pair.second;
            mode = pair.first;
        }
    }

    // Second pass: split data, replacing missing values with mode
    for (const auto &ex : examples)
    {
        string key;
        if (ex.attributes.at(attribute).isNumeric)
        {
            key = discretizeNumeric(attribute, ex.attributes.at(attribute).numeric);
        }
        else
        {
            string val = handleMissing(ex.attributes.at(attribute).categorical);
            key = (val == "Unknown") ? mode : val;
        }
        splits[key].push_back(ex);
    }

    return splits;
}

bool isPure(const vector<Example> &examples)
{
    if (examples.empty())
        return true;

    string firstLabel = examples[0].label;
    for (const auto &ex : examples)
    {
        if (ex.label != firstLabel)
            return false;
    }
    return true;
}

string getMajorityClass(const vector<Example> &examples)
{
    if (examples.empty())
        return "Unknown";

    map<string, int> counts;
    for (const auto &ex : examples)
    {
        counts[ex.label]++;
    }

    string majorityClass = examples[0].label;
    int maxCount = 0;
    for (const auto &pair : counts)
    {
        if (pair.second > maxCount)
        {
            maxCount = pair.second;
            majorityClass = pair.first;
        }
    }

    return majorityClass;
}

double calculateIG(const vector<Example> &examples, const string &attribute)
{
    double totalEntropy = calculateEntropy(examples);
    auto splits = splitData(examples, attribute);

    double weightedEntropy = 0.0;
    int total = examples.size();

    for (const auto &pair : splits)
    {
        double weight = (double)pair.second.size() / total;
        weightedEntropy += weight * calculateEntropy(pair.second);
    }

    return totalEntropy - weightedEntropy;
}

double calculateIntrinsicValue(const vector<Example> &examples, const string &attribute)
{
    auto splits = splitData(examples, attribute);
    double iv = 0.0;
    int total = examples.size();

    for (const auto &pair : splits)
    {
        double p = (double)pair.second.size() / total;
        if (p > 0)
        {
            iv -= p * log2(p);
        }
    }

    return iv;
}

double calculateIGR(const vector<Example> &examples, const string &attribute)
{
    double ig = calculateIG(examples, attribute);
    double iv = calculateIntrinsicValue(examples, attribute);
    return (iv == 0.0) ? 0.0 : ig / iv;
}

double calculateNWIG(const vector<Example> &examples, const string &attribute)
{
    double ig = calculateIG(examples, attribute);
    int k = splitData(examples, attribute).size();
    int S = examples.size();

    if (k == 0 || S == 0)
        return 0.0;

    double penalty1 = log2(k + 1);
    double penalty2 = 1.0 - ((double)(k - 1) / S);

    return (ig / penalty1) * penalty2;
}

string chooseBestAttribute(const vector<Example> &examples, const vector<string> &attributes, const string &criterion)
{
    double bestScore = -1.0;
    string bestAttribute = "";

    for (const string &attr : attributes)
    {
        double score = 0.0;

        if (criterion == "IG")
        {
            score = calculateIG(examples, attr);
        }
        else if (criterion == "IGR")
        {
            score = calculateIGR(examples, attr);
        }
        else if (criterion == "NWIG")
        {
            score = calculateNWIG(examples, attr);
        }
        else
        {
            cerr << "Unknown criterion: " << criterion << endl;
            exit(1);
        }

        if (score > bestScore)
        {
            bestScore = score;
            bestAttribute = attr;
        }
    }

    return bestAttribute;
}

TreeNode *buildTree(const vector<Example> &examples, const vector<string> &attributes,
                    const string &criterion, int depth = 0, int maxDepth = 0)
{
    TreeNode *node = new TreeNode();

    // Base case: empty examples
    if (examples.empty())
    {
        node->isLeaf = true;
        node->label = "Unknown";
        return node;
    }

    // Base case: all examples have same label
    if (isPure(examples))
    {
        node->isLeaf = true;
        node->label = examples[0].label;
        return node;
    }

    // Base case: no more attributes or max depth reached
    if (attributes.empty() || (maxDepth > 0 && depth >= maxDepth))
    {
        node->isLeaf = true;
        node->label = getMajorityClass(examples);
        return node;
    }

    // Choose best attribute
    string bestAttr = chooseBestAttribute(examples, attributes, criterion);
    if (bestAttr.empty())
    {
        node->isLeaf = true;
        node->label = getMajorityClass(examples);
        return node;
    }

    node->attribute = bestAttr;

    // Create remaining attributes list
    vector<string> remaining;
    for (const string &attr : attributes)
    {
        if (attr != bestAttr)
        {
            remaining.push_back(attr);
        }
    }

    // Split data and create children
    auto splits = splitData(examples, bestAttr);
    for (const auto &pair : splits)
    {
        if (!pair.second.empty())
        {
            node->children[pair.first] = buildTree(pair.second, remaining, criterion, depth + 1, maxDepth);
        }
    }

    // If no children were created, make this a leaf
    if (node->children.empty())
    {
        node->isLeaf = true;
        node->label = getMajorityClass(examples);
    }

    return node;
}

string predict(TreeNode *node, const Example &ex)
{
    if (node->isLeaf || node->children.empty())
    {
        return node->label;
    }

    string attrValue;
    if (ex.attributes.at(node->attribute).isNumeric)
    {
        attrValue = discretizeNumeric(node->attribute, ex.attributes.at(node->attribute).numeric);
    }
    else
    {
        attrValue = handleMissing(ex.attributes.at(node->attribute).categorical);
    }

    // Find matching child
    auto it = node->children.find(attrValue);
    if (it != node->children.end())
    {
        return predict(it->second, ex);
    }

    // If no exact match, use majority class approach
    if (!node->children.empty())
    {
        // Try to find the most similar child or use the first one
        return predict(node->children.begin()->second, ex);
    }

    return node->label;
}

double evaluate(TreeNode *tree, const vector<Example> &testData)
{
    if (testData.empty())
        return 0.0;

    int correct = 0;
    for (const auto &ex : testData)
    {
        if (predict(tree, ex) == ex.label)
        {
            correct++;
        }
    }

    return (double)correct / testData.size();
}

int countNodes(TreeNode *node)
{
    if (!node)
        return 0;

    int count = 1;
    for (const auto &child : node->children)
    {
        count += countNodes(child.second);
    }

    return count;
}

int getTreeDepth(TreeNode *node)
{
    if (!node || node->isLeaf)
        return 1;

    int maxDepth = 0;
    for (const auto &child : node->children)
    {
        maxDepth = max(maxDepth, getTreeDepth(child.second));
    }

    return maxDepth + 1;
}

void freeTree(TreeNode *node)
{
    if (!node)
        return;

    for (auto &child : node->children)
    {
        freeTree(child.second);
    }
    delete node;
}

vector<Example> loadDataset(const string &filename, const DatasetConfig &config)
{
    vector<Example> dataset;
    ifstream file(filename);
    string line;

    if (!file.is_open())
    {
        cerr << "Error: Cannot open file " << filename << endl;
        return dataset;
    }

    if (config.has_header)
    {
        getline(file, line); // Skip header
    }

    while (getline(file, line))
    {
        if (line.empty())
            continue;

        Example ex;
        stringstream ss(line);
        string token;
        vector<string> tokens;

        while (getline(ss, token, config.delimiter))
        {
            tokens.push_back(trim(token));
        }

        if (tokens.size() <= static_cast<size_t>(config.label_index))
            continue;

        // Process attributes based on dataset type
        if (config.attribute_names.size() == 4 && config.label_index == 5) // Iris dataset
        {
            // Skip Id column (index 0), process sepal_length (1), sepal_width (2), petal_length (3), petal_width (4)
            vector<int> iris_indices = {1, 2, 3, 4};
            for (size_t i = 0; i < config.attribute_names.size(); ++i)
            {
                const string &attr = config.attribute_names[i];
                int token_idx = iris_indices[i];

                if (token_idx < tokens.size())
                {
                    try
                    {
                        double val = stod(tokens[token_idx]);
                        ex.attributes[attr] = AttributeValue(true, "", val);
                    }
                    catch (...)
                    {
                        ex.attributes[attr] = AttributeValue(true, "", 0.0);
                    }
                }
                else
                {
                    ex.attributes[attr] = AttributeValue(true, "", 0.0);
                }
            }
        }
        else // Adult dataset
        {
            // Process attributes sequentially
            for (size_t i = 0; i < config.attribute_names.size(); ++i)
            {
                const string &attr = config.attribute_names[i];

                if (i < tokens.size())
                {
                    if (config.is_numeric[i])
                    {
                        try
                        {
                            double val = stod(tokens[i]);
                            ex.attributes[attr] = AttributeValue(true, "", val);
                        }
                        catch (...)
                        {
                            ex.attributes[attr] = AttributeValue(true, "", 0.0);
                        }
                    }
                    else
                    {
                        ex.attributes[attr] = AttributeValue(false, handleMissing(tokens[i]), 0.0);
                    }
                }
                else
                {
                    if (config.is_numeric[i])
                    {
                        ex.attributes[attr] = AttributeValue(true, "", 0.0);
                    }
                    else
                    {
                        ex.attributes[attr] = AttributeValue(false, "Unknown", 0.0);
                    }
                }
            }
        }

        // Set label
        if (static_cast<size_t>(config.label_index) < tokens.size())
        {
            ex.label = trim(tokens[config.label_index]);
        }
        else
        {
            ex.label = "Unknown";
        }

        dataset.push_back(ex);
    }

    file.close();
    return dataset;
}

DatasetConfig getAdultConfig()
{
    DatasetConfig config;
    config.has_header = false;
    config.delimiter = ',';
    config.label_index = 14;
    config.attribute_names = {"age", "workclass", "fnlwgt", "education", "education-num",
                              "marital-status", "occupation", "relationship", "race", "sex",
                              "capital-gain", "capital-loss", "hours-per-week", "native-country"};
    config.is_numeric = {true, false, true, false, true, false, false, false, false, false,
                         true, true, true, false};
    return config;
}

DatasetConfig getIrisConfig()
{
    DatasetConfig config;
    config.has_header = true;
    config.delimiter = ',';
    config.label_index = 5; // Species column is at index 5 (0-based: Id=0, SepalLength=1, SepalWidth=2, PetalLength=3, PetalWidth=4, Species=5)
    config.attribute_names = {"sepal_length", "sepal_width", "petal_length", "petal_width"};
    config.is_numeric = {true, true, true, true};
    return config;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        cerr << "Usage: ./decision_tree <criterion> <maxDepth>\n";
        cerr << "Criterion: IG, IGR, or NWIG\n";
        cerr << "MaxDepth: integer (0 for no pruning)\n";
        cerr << "Example: ./decision_tree IG 3\n";
        return 1;
    }

    string criterion = argv[1];
    int maxDepth = stoi(argv[2]);

    if (criterion != "IG" && criterion != "IGR" && criterion != "NWIG")
    {
        cerr << "Error: Invalid criterion. Use IG, IGR, or NWIG\n";
        return 1;
    }

    // Dataset selection
    cout << "Select dataset:\n1. Adult Income\n2. Iris\nEnter choice (1 or 2): ";
    int choice;
    cin >> choice;

    DatasetConfig config;
    string filename;
vector<string> selected_attributes;

if (choice == 1)
{
    config = getAdultConfig();
    filename = "adult.data";
    selected_attributes = config.attribute_names;  // Use all attributes
}
else if (choice == 2)
{
    config = getIrisConfig();
    filename = "iris.csv";
    selected_attributes = config.attribute_names;  // Already set for Iris
}
else
{
    cerr << "Invalid choice. Using Iris dataset by default.\n";
    config = getIrisConfig();
    filename = "iris.csv";
    selected_attributes = config.attribute_names;
}

// Load dataset
vector<Example> dataset = loadDataset(filename, config);
cout << "Loaded " << dataset.size() << " examples from " << filename << endl;

// Log unique values for each attribute (for debugging discretization issues)
if (choice == 1)
{
    for (const string &attr : selected_attributes)
    {
        set<string> uniqueVals;
        for (const Example &ex : dataset)
        {
            string val = ex.attributes.at(attr).isNumeric
                         ? discretizeNumeric(attr, ex.attributes.at(attr).numeric)
                         : handleMissing(ex.attributes.at(attr).categorical);
            uniqueVals.insert(val);
        }
        cout << "Attribute: " << attr << " has " << uniqueVals.size() << " unique values in dataset.\n";
    }
}

    if (dataset.empty())
    {
        cerr << "Error: Dataset loading failed. Check the file path or format.\n";
        return 1;
    }

    // Run experiments
    const int numRuns = 20;
    const double trainRatio = 0.8;
    double totalAccuracy = 0.0;
    double totalNodes = 0.0;
    double totalDepth = 0.0;
    vector<double> accuracies;

    cout << "\nRunning " << numRuns << " experiments with " << criterion
         << " and maxDepth " << maxDepth << "...\n";
    cout << string(50, '-') << endl;

    for (int run = 0; run < numRuns; ++run)
    {
        // Shuffle dataset with strong RNG
        vector<Example> shuffled = dataset;
        mt19937 rng(chrono::high_resolution_clock::now().time_since_epoch().count() + run);
        shuffle(shuffled.begin(), shuffled.end(), rng);

        // Split into train and test
        size_t splitPoint = shuffled.size() * trainRatio;
        vector<Example> trainData(shuffled.begin(), shuffled.begin() + splitPoint);
        vector<Example> testData(shuffled.begin() + splitPoint, shuffled.end());

        // Build tree
        TreeNode *tree = buildTree(trainData, config.attribute_names, criterion, 0, maxDepth);

        // Evaluate
        double accuracy = evaluate(tree, testData);
        int nodeCount = countNodes(tree);
        int treeDepth = getTreeDepth(tree);

        accuracies.push_back(accuracy);
        totalAccuracy += accuracy;
        totalNodes += nodeCount;
        totalDepth += treeDepth;

        cout << "Run " << setw(2) << (run + 1) << "/" << numRuns
             << " - Accuracy: " << fixed << setprecision(2) << (accuracy * 100) << "%"
             << ", Nodes: " << setw(3) << nodeCount
             << ", Depth: " << setw(2) << treeDepth << endl;

        freeTree(tree);
    }


    // Calculate statistics
    double avgAccuracy = totalAccuracy / numRuns;
    double avgNodes = totalNodes / numRuns;
    double avgDepth = totalDepth / numRuns;

    double minAccuracy = *min_element(accuracies.begin(), accuracies.end());
    double maxAccuracy = *max_element(accuracies.begin(), accuracies.end());


    cout << string(50, '=') << endl;
    cout << "FINAL RESULTS - " << criterion << " with maxDepth " << maxDepth << endl;
    cout << string(50, '=') << endl;
    cout << "Average Accuracy: " << fixed << setprecision(4) << (avgAccuracy * 100) << "%" << endl;

    cout << "Min Accuracy: " << fixed << setprecision(2) << (minAccuracy * 100) << "%" << endl;
    cout << "Max Accuracy: " << fixed << setprecision(2) << (maxAccuracy * 100) << "%" << endl;
    cout << "Average Nodes: " << fixed << setprecision(1) << avgNodes << endl;
    cout << "Average Depth: " << fixed << setprecision(1) << avgDepth << endl;

    return 0;
}





// int main(int argc, char *argv[])
// {
    

//     cout << "Select dataset:\n1. Adult Income\n2. Iris\nEnter choice (1 or 2): ";
//     int choice;
//     cin >> choice;

//     DatasetConfig config;
//     string filename;
//     string dataset_name;
//     if (choice == 1)
//     {
//         config = getAdultConfig();
//         filename = "adult.data";
//         dataset_name = "Adult";
//     }
//     else if (choice == 2)
//     {
//         config = getIrisConfig();
//         filename = "iris.csv";
//         dataset_name = "Iris";
//     }
//     else
//     {
//         cerr << "Invalid choice. Using Iris dataset by default.\n";
//         config = getIrisConfig();
//         filename = "iris.csv";
//         dataset_name = "Iris";
//     }

//     vector<Example> dataset = loadDataset(filename, config);

//     if (dataset.empty())
//     {
//         cerr << "Error: Dataset loading failed.\n";
//         return 1;
//     }

//     const int numRuns = 20;
//     const double trainRatio = 0.8;

//     vector<string> criteria = {"IG", "IGR", "NWIG"};
//     vector<int> depths;
//     for (int d = 1; d <= 10; ++d) depths.push_back(d);

//     ofstream csv("results.csv");
//     csv << "dataset,criterion,max_depth,avg_accuracy,avg_nodes\n";

//     for (const string &criterion : criteria)
//     {
//         for (int maxDepth : depths)
//         {
//             double totalAccuracy = 0.0;
//             double totalNodes = 0.0;

//             for (int run = 0; run < numRuns; ++run)
//             {
//                 vector<Example> shuffled = dataset;
//                 mt19937 rng(chrono::high_resolution_clock::now().time_since_epoch().count() + run);
//                 shuffle(shuffled.begin(), shuffled.end(), rng);

//                 size_t splitPoint = (size_t)(shuffled.size() * trainRatio);
//                 vector<Example> trainData(shuffled.begin(), shuffled.begin() + splitPoint);
//                 vector<Example> testData(shuffled.begin() + splitPoint, shuffled.end());

//                 TreeNode *tree = buildTree(trainData, config.attribute_names, criterion, 0, maxDepth);

//                 double accuracy = evaluate(tree, testData);
//                 int nodeCount = countNodes(tree);

//                 totalAccuracy += accuracy;
//                 totalNodes += nodeCount;

//                 freeTree(tree);
//             }

//             double avgAccuracy = totalAccuracy / numRuns;
//             double avgNodes = totalNodes / numRuns;

//             cout << "Dataset: " << dataset_name
//                  << ", Criterion: " << criterion
//                  << ", MaxDepth: " << maxDepth
//                  << ", Avg Accuracy: " << avgAccuracy * 100 << "%"
//                  << ", Avg Nodes: " << avgNodes << endl;

//             csv << dataset_name << "," << criterion << "," << maxDepth << ","
//                 << avgAccuracy << "," << avgNodes << "\n";
//         }
//     }

//     csv.close();

//     cout << "Experiment results saved to results.csv\n";

//     return 0;
// }
 