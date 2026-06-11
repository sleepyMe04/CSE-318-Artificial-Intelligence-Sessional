#include <iostream>
#include <string>
#include <queue>
#include <unordered_set>
#include <vector>
#include <bits/stdc++.h>
using namespace std;
class Node
{
public:
    Node *prevNode; // Reference to previous node
    int **grid;
    int k;              // k*k grid
    int required_moves; // Moves required to reach this configuration
    pair<int, int> zeroPos;
    Node(int k, int **grid)
    {
        this->k = k;
        this->required_moves = 0;
        this->prevNode = NULL;
        this->grid = new int *[k];

        for (int i = 0; i < k; i++)
        {

            this->grid[i] = new int[k];

            for (int j = 0; j < k; j++)
            {
                this->grid[i][j] = grid[i][j];
                if (this->grid[i][j] == 0)
                {
                    this->zeroPos = make_pair(i, j);
                }
            }
        }
    }
    Node(int **grid, int k, int required_moves, pair<int, int> zeroPos, Node &prevNode)
    {
        this->grid = grid;
        this->k = k;
        this->required_moves = required_moves;
        this->zeroPos = zeroPos;
        this->prevNode = &prevNode;
    }
   
    bool isEqual(Node *node)
    {
        for (int i = 0; i < k; i++)
        {
            for (int j = 0; j < k; j++)
            {
                if (node->grid[i][j] != this->grid[i][j])
                {
                    return false;
                }
            }
        }
        return true;
    }
    bool isGoal(Node *node)
    {
        int temp = 1;
        for (int i = 0; i < k; i++)
        {
            for (int j = 0; j < k; j++)
            {
                if (node->grid[i][j] != temp)
                {
                    return false;
                }

                temp++;
                if (temp == k * k)
                {
                    break;
                }
            }
        }
        return true;
    }
    void printGrid()
    {
        for (int i = 0; i < k; i++)
        {
            for (int j = 0; j < k; j++)
            {
                cout << grid[i][j] << " ";
            }
            cout << endl;
        }
    }
    int **getGrid()
    {
        return this->grid;
    }
    int getK()
    {
        return this->k;
    }
    int getReqMoves()
    {
        return this->required_moves;
    }
    pair<int, int> getZeroPos()
    {
        return this->zeroPos;
    }
    Node *getPrevNode()
    {
        return this->prevNode;
    }
    Node *getCurrNode()
    {
        return this;
    }
    vector<Node *> getChildren()
    {
        vector<Node *> children;
        int i = this->zeroPos.first;
        int j = this->zeroPos.second;

        // move up
        if (i > 0)
        {
            int **grid = new int *[k];
            for (int i = 0; i < k; i++)
            {
                grid[i] = new int[k];
                for (int j = 0; j < k; j++)
                {
                    grid[i][j] = this->grid[i][j];
                }
            }
            grid[i][j] = grid[i - 1][j];
            grid[i - 1][j] = 0;
            Node *node = new Node(grid, k, this->required_moves + 1, make_pair(i - 1, j), *this);
            children.push_back(node);
            // node->print_grid();
        }

        // move down
        if (i < k - 1)
        {
            int **grid = new int *[k];
            for (int i = 0; i < k; i++)
            {
                grid[i] = new int[k];
                for (int j = 0; j < k; j++)
                {
                    grid[i][j] = this->grid[i][j];
                }
            }
            grid[i][j] = grid[i + 1][j];
            grid[i + 1][j] = 0;
            Node *node = new Node(grid, k, this->required_moves + 1, make_pair(i + 1, j), *this);
            children.push_back(node);
            // node->print_grid();
        }

        // move left
        if (j > 0)
        {
            int **grid = new int *[k];
            for (int i = 0; i < k; i++)
            {
                grid[i] = new int[k];
                for (int j = 0; j < k; j++)
                {
                    grid[i][j] = this->grid[i][j];
                }
            }
            grid[i][j] = grid[i][j - 1];
            grid[i][j - 1] = 0;
            Node *node = new Node(grid, k, this->required_moves + 1, make_pair(i, j - 1), *this);
            children.push_back(node);
            // node->print_grid();
        }

        // move right
        if (j < k - 1)
        {
            int **grid = new int *[k];
            for (int i = 0; i < k; i++)
            {
                grid[i] = new int[k];
                for (int j = 0; j < k; j++)
                {
                    grid[i][j] = this->grid[i][j];
                }
            }
            grid[i][j] = grid[i][j + 1];
            grid[i][j + 1] = 0;
            Node *node = new Node(grid, k, this->required_moves + 1, make_pair(i, j + 1), *this);
            children.push_back(node);
            // node->print_grid();
        }
        return children;
    }

    // destructor
    ~Node()
    {
        for (int i = 0; i < k; i++)
        {
            delete[] grid[i];
        }
        delete[] grid;
    }
};

class Heuristics
{
public:
    int HammingDistance(Node *node)
    {
        int distance = 0;
        int temp = 1;
        int k = node->getK();
        int **grid = node->getGrid();
        for (int i = 0; i < k; i++)
        {
            for (int j = 0; j < k; j++)
            {
                if (grid[i][j] != temp)
                {
                    distance++;
                }
                temp++;
                if (temp == k * k)
                {
                    break;
                }
            }
        }
        return distance;
    }

    int ManhattanDistance(Node *node)
    {

        int distance = 0;

        int k = node->getK();
        int **grid = node->getGrid();
        for (int i = 0; i < k; i++)
        {
            for (int j = 0; j < k; j++)
            {
                int value = grid[i][j];
                if (value != 0)
                {
                    int goalRow = (value - 1) / k;
                    int goalCol = (value - 1) % k;

                    distance += abs(i - goalRow) + abs(j - goalCol);
                }
            }
        }
        return distance;
    }
    int EuclideanDistance(Node *node)
    {
        int **grid = node->getGrid();
        int k = node->getK();
        double distance = 0.0;
        for (int i = 0; i < k; i++)
        {
            for (int j = 0; j < k; j++)
            {
                int value = grid[i][j];
                if (value != 0)
                {
                    int goalRow = (value - 1) / k;
                    int goalCol = (value - 1) % k;

                    distance += sqrt(pow((i - goalRow), 2) + pow((j - goalCol), 2));
                }
            }
        }
        return distance;
    }
    int LinearConflict(Node *node)
    {
        int k = node->getK();
        int **grid = node->getGrid();
        int manhattan = this->ManhattanDistance(node);
        int linearConflicts = 0;

        // Linear Conflicts in Rows
        for (int row = 0; row < k; row++)
        {
            for (int i = 0; i < k; i++)
            {
                int val1 = grid[row][i];
                if (val1 == 0)
                    continue;

                int goalRow1 = (val1 - 1) / k;
                int goalCol1 = (val1 - 1) % k;

                if (goalRow1 == row)
                { // It should be in this row
                    for (int j = i + 1; j < k; j++)
                    {
                        int val2 = grid[row][j];
                        if (val2 == 0)
                            continue;

                        int goalRow2 = (val2 - 1) / k;
                        int goalCol2 = (val2 - 1) % k;

                        if (goalRow2 == row && goalCol1 > goalCol2)
                        {
                            linearConflicts++;
                        }
                    }
                }
            }
        }

        // Linear Conflicts in Columns
        for (int col = 0; col < k; col++)
        {
            for (int i = 0; i < k; i++)
            {
                int val1 = grid[i][col];
                if (val1 == 0)
                    continue;

                int goalRow1 = (val1 - 1) / k;
                int goalCol1 = (val1 - 1) % k;

                if (goalCol1 == col)
                { // It should be in this column
                    for (int j = i + 1; j < k; j++)
                    {
                        int val2 = grid[j][col];
                        if (val2 == 0)
                            continue;

                        int goalRow2 = (val2 - 1) / k;
                        int goalCol2 = (val2 - 1) % k;

                        if (goalCol2 == col && goalRow1 > goalRow2)
                        {
                            linearConflicts++;
                        }
                    }
                }
            }
        }

        return manhattan + 2 * linearConflicts;
    }

    // calculate the hamming distance of the current node and adds it to the number of moves of the current node
    int hammingDistanceWithMoves(Node *node)
    {
        return this->HammingDistance(node) + node->getReqMoves();
    }

    // calculate the manhattan distance of the current node and adds it to the number of moves of the current node
    int manhattanDistanceWithMoves(Node *node)
    {
        return this->ManhattanDistance(node) + node->getReqMoves();
    }
    // calculate the Eucledian distance of the current node and adds it to the number of moves of the current node
    int euclideanDistanceWithMoves(Node *node)
    {
        return this->EuclideanDistance(node) + node->getReqMoves();
    }
    int linearConflictWithMoves(Node *node)
    {
        return this->LinearConflict(node) + node->getReqMoves();
    }
};

class Puzzle
{
public:
   
   
    struct nodeComparator
    {
        bool operator()(Node *node1, Node *node2) const
        {
            // if all elements of the grid of node1 are equal to the elements of the grid of node2
            if (node1->isEqual(node2))
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    };

    // struct for node hash
    struct nodeHash
    {
        size_t operator()(Node *node) const
        {
            int32_t hash = 0;
            string str = "";

            for (int i = 0; i < node->getK(); i++)
            {
                for (int j = 0; j < node->getK(); j++)
                {
                    str += to_string(node->getGrid()[i][j]);
                }
            }

            for (int i = 0; i < str.length(); i++)
            {
                hash = hash * 31 + str[i];
            }

            return hash;
        }
    };
    /// INVERSION COUNT TO CHECK SOLVABILITY
    int countInversions(Node *node)
    {
        //  cout<<"Hola";
        int k = node->getK();
        int **grid = node->getGrid();
        vector<int> FlattenedGrid;

        // Flatten the grid in row-major order, skipping the blank tile (0)
        for (int i = 0; i < k; i++)
        {
            for (int j = 0; j < k; j++)
            {
                if (grid[i][j] != 0)
                    FlattenedGrid.push_back(grid[i][j]);
            }
        }

        int inversions = 0;
        int size = FlattenedGrid.size();

        /*for (int i = 0; i < k*k; i++)
       {

               cout<<FlattenedGrid[i];


       }*/
        for (int i = 0; i < k * k; i++)
        {
            for (int j = i + 1; j < k * k; j++)
            {

                if (FlattenedGrid[i] != 0 && FlattenedGrid[j] != 0 && FlattenedGrid[i] > FlattenedGrid[j])
                {

                    // cout<<inversions<<endl;
                    // cout<<FlattenedGrid[i];
                    inversions++;
                }
            }
        }
        return inversions;
    }
    // SOLVABOLITY CHECK

    bool Solvable(Node *node)
    {
        int inv_count = this->countInversions(node);
       // cout << "Inversion count " << inv_count << endl;
        int k = node->getK();
        int blankRow = node->getZeroPos().first;
        int blankCol = node->getZeroPos().second;
        int blankRowFromBottom = k - blankRow;

        /////////////k odd && inversion count even///
        if (k % 2 != 0)
        {
            return inv_count % 2 == 0;
        }
        else
        {
            return (blankRowFromBottom % 2 == 0) ? (inv_count % 2 != 0) : (inv_count % 2 == 0);
        }
        // return false;
    }
    void printPath(Node *node)

    {
        if (node == NULL)
        {
            return;
        }
        printPath(node->getPrevNode());

        node->printGrid();
        cout << endl;
    }
    void solve(Node* StartNode, string& heuristic) {
        // Convert to lowercase for consistent comparison
        transform(heuristic.begin(), heuristic.end(), heuristic.begin(), ::tolower);
        
      //  cout << "Solving with " << heuristic << " Heuristic" << endl;
    
        if (!Solvable(StartNode)) {
            cout << "This puzzle is not solvable" << endl;
            return;
        }
    
        Heuristics h;
        int (Heuristics::*heuristicFunc)(Node*) = nullptr;
          
        if (heuristic == "hamming") {
           
            heuristicFunc = &Heuristics::hammingDistanceWithMoves;
            cout<< "Hamming distance heuristic selected." << endl;
        } else if (heuristic == "manhattan") {
            heuristicFunc = &Heuristics::manhattanDistanceWithMoves;
            cout<< "Manhattan distance heuristic selected." << endl;
        } else if (heuristic == "euclidean") {
            heuristicFunc = &Heuristics::euclideanDistanceWithMoves;
            cout<< "Euclidean distance heuristic selected." << endl;
        } else if (heuristic == "linear") {
            heuristicFunc = &Heuristics::linearConflictWithMoves;
            cout<< "Linear conflict heuristic selected." << endl;
        } else {
            cout << "Unknown heuristic! Defaulting to Manhattan." << endl;
            heuristicFunc = &Heuristics::manhattanDistanceWithMoves;
        }
    
        auto comparator = [&h, heuristicFunc](Node* a, Node* b) {
            return (h.*heuristicFunc)(a) > (h.*heuristicFunc)(b);
        };
    
        priority_queue<Node*, vector<Node*>, decltype(comparator)> open_list(comparator);
        unordered_set<Node*, nodeHash, nodeComparator> closed_list;
    
        open_list.push(StartNode);
        int expanded_nodes = 0;
        int explored_nodes = 1;
    
        while (!open_list.empty()) {
            Node* current = open_list.top();
            open_list.pop();
            
            if (closed_list.find(current) != closed_list.end()) continue;
            
            closed_list.insert(current);
            expanded_nodes++;
    
            if (current->isGoal(current)) {
                cout << "Minimum moves: " << current->getReqMoves() << endl;
                cout << "Expanded nodes: " << expanded_nodes << endl;
                cout << "Explored nodes: " << explored_nodes << endl;
                printPath(current);
                return;
            }
    
            for (Node* child : current->getChildren()) {
                if (closed_list.find(child) == closed_list.end()) {
                    open_list.push(child);
                    explored_nodes++;
                }
            }
        }
    }
};
int main() {
    int k;
    cout << "Enter grid size (k): ";
    cin >> k;

    int **grid = new int *[k];
    cout << "Enter board configuration (row-wise, use 0 for blank):\n";
    for (int i = 0; i < k; i++) {
        grid[i] = new int[k];
        for (int j = 0; j < k; j++) {
            cin >> grid[i][j];
        }
    }

    Node *start = new Node(k, grid);
    Puzzle puzzle;

    cout << "Choose heuristic (hamming / manhattan / euclidean / linear): ";
    string choice;
    cin >> choice;

    // Convert to lowercase for case-insensitive comparison
    transform(choice.begin(), choice.end(), choice.begin(), ::tolower);

    puzzle.solve(start, choice);

    // Clean up dynamically allocated memory
    for (int i = 0; i < k; i++) {
        delete[] grid[i];
    }
    delete[] grid;
    delete start;

    return 0;
}