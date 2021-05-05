/*  
 *  Simulates an actual node inside a network with capabilities to 
 *  send and receive data packets via files.
 *  Copyright (C) 2021 Sourabh J Choure
 * 
 *  I promise that the work presented below is of my own.
 * 
 *  To make it FOSS Compliant, this software is free to use.
 */

// STL
#include <iostream>
#include <fstream>
#include <string>
// SL
#include <cstdlib>
// Unix
#include <unistd.h>

using namespace std;

#define NUMNODES 10

struct FileDescriptor
{
    // Store the name of the Files
    string inputFileName;
    string outputFileName;
    string receivedFileName;

    // File Desciptors
    fstream input;
    ofstream output;
    ofstream receivedData;
};

struct Queue
{
    // Constructor of the Queue
    Queue(int cap) : cap(cap), p(new int[cap]), f(0), r(0), n(0){};

    // Store the total capacity of the Queue
    int cap;

    // Pointer to array
    int *p;

    // Index to the front of the Queue
    int f;

    // Index to the rear of the Queue
    int r;

    // Total Number of elements in the Queue
    int n;

    // Put the elements in the Queue
    void enqueue(int);

    // Remove the elements from the Queue
    int dequeue();

    // Check if the Queue is empty
    bool empty();
};

void Queue::enqueue(int val)
{
    if (n == cap)
    {
        // Queue is Full
    }

    // Put the val
    p[r] = val;

    // Increment the rear index
    r = (r + 1) % cap;

    // Increment the total number of elements
    n++;
}

int Queue::dequeue()
{
    if (n == 0)
    {
        // Queue is Empty
    }

    // Remove the element
    int tmp = p[f];

    // Increment the front index
    f = (f + 1) % cap;

    // Decrement the number of elements
    n--;

    // Return the element
    return tmp;
}

bool Queue::empty()
{
    return (n == 0) ? true : false;
}

struct nodeLevel
{
    int level = -1;
    int dest;
};

struct Routing
{
    Routing(int dest, string dataMessage) : dest(dest), dataMessage(dataMessage)
    {
        for (size_t i = 0; i < NUMNODES; i++)
        {
            incomingNeighbors[i] = 0;
            pathToIncomingNeighbors[i] = "";

            for (size_t j = 0; j < NUMNODES; j++)
            {
                intree[i][j] = 0;
                prevIntree[i][j] = 0;
                passDataToNeighbor[i][j] = "";
            }
        }
    };

    // Destination Node
    int dest;

    // Path to Destination
    string pathToDest = "";

    // Buffer for the data to be sent
    string dataMessage;

    // Buffer for passing message to Neighbor
    string passDataToNeighbor[NUMNODES][NUMNODES];

    // Keep track of Incoming Neighbors
    int incomingNeighbors[NUMNODES];

    // In-tree of a Node
    int intree[NUMNODES][NUMNODES];

    // Previous In-tree of a Node
    int prevIntree[NUMNODES][NUMNODES];

    // Check if the Intree changed
    bool sendIntreeNow = false;

    // Store the path to the neighbor
    string pathToIncomingNeighbors[NUMNODES];

    // Check if incoming Neighbors is empty
    bool isINempty();

    // Find the path to the Incoming Neighbor
    void storePathToIncomingNeighbor(size_t, size_t, int (&)[NUMNODES][NUMNODES]);

    // buildSPT
    void buildSPT(size_t, size_t, int (&)[NUMNODES][NUMNODES]);

    // Common Function
    void extendedBFSt(size_t, size_t, int (&)[NUMNODES][NUMNODES], void (Routing::*func)(size_t, size_t, int (&)[NUMNODES][NUMNODES]));

    void extendedBFSt(size_t, size_t, int (&)[NUMNODES][NUMNODES], nodeLevel (&)[NUMNODES], void (Routing::*func)(size_t, size_t, int (&)[NUMNODES][NUMNODES], nodeLevel (&)[NUMNODES]));

    // Common Function
    void extendedBFSi(size_t, size_t, int (&)[NUMNODES][NUMNODES], void (Routing::*func)(size_t, size_t, int (&)[NUMNODES][NUMNODES]));

    void extendedBFSi(size_t, size_t, int (&)[NUMNODES][NUMNODES], nodeLevel (&)[NUMNODES], void (Routing::*func)(size_t, size_t, int (&)[NUMNODES][NUMNODES], nodeLevel (&)[NUMNODES]));

    // Common Function Helper: Remove TmpTree
    void removeTmpTreePath(size_t, size_t, int (&)[NUMNODES][NUMNODES]);

    // Common Function Helper: Remove InTree
    void removeInTreePath(size_t, size_t, int (&)[NUMNODES][NUMNODES]);

    // Common Function Helper: pruneNode
    void pruneNode(size_t, size_t, int (&)[NUMNODES][NUMNODES]);

    // Common Function Helper: add levels
    void addLevel(size_t, size_t, int (&)[NUMNODES][NUMNODES], nodeLevel (&)[NUMNODES]);

    // Common Function Helper: remove levels
    void removeLevel(size_t, size_t, int (&)[NUMNODES][NUMNODES], nodeLevel (&)[NUMNODES]);
};

bool Routing::isINempty()
{
    for (size_t i = 0; i < NUMNODES; i++)
    {
        if (incomingNeighbors[i])
            return false;
    }

    return true;
}

void Routing::storePathToIncomingNeighbor(size_t v, size_t rootedAt, int (&tempIntree)[NUMNODES][NUMNODES])
{
    // Add Node v to the path
    pathToIncomingNeighbors[rootedAt] = pathToIncomingNeighbors[rootedAt] + to_string(v) + " ";

    // Traverse the Graph
    for (size_t w = 0; w < NUMNODES; w++)
    {
        if (tempIntree[v][w])
        {
            storePathToIncomingNeighbor(w, rootedAt, tempIntree);
        }
    }
}

void Routing::removeTmpTreePath(size_t w, size_t v, int (&tmpIntree)[NUMNODES][NUMNODES])
{
    tmpIntree[w][v] = 0;
}

void Routing::removeInTreePath(size_t w, size_t v, int (&tmpIntree)[NUMNODES][NUMNODES])
{
    intree[w][v] = 0;
}

void Routing::pruneNode(size_t w, size_t v, int (&tmpIntree)[NUMNODES][NUMNODES])
{
    if (!tmpIntree[w][v])
    {
        intree[w][v] = 0;
    }
}

void Routing::addLevel(size_t w, size_t v, int (&tmpIntree)[NUMNODES][NUMNODES], nodeLevel (&levels)[NUMNODES])
{
    levels[w].level = levels[v].level + 1;
    levels[w].dest = v;
}

void Routing::removeLevel(size_t w, size_t v, int (&tmpIntree)[NUMNODES][NUMNODES], nodeLevel (&levels)[NUMNODES])
{
    tmpIntree[w][v] = 0;
    levels[w].level = -1;
    levels[w].dest = -1;
}

void Routing::extendedBFSt(size_t ID, size_t rootedAt, int (&tmpIntree)[NUMNODES][NUMNODES], void (Routing::*func)(size_t, size_t, int (&)[NUMNODES][NUMNODES]))
{
    // Queue to traverse
    Queue qGraph(NUMNODES);

    // Record for visited Nodes
    bool visNodes[NUMNODES] = {false};

    // Enqueue the root
    qGraph.enqueue(ID);

    // Mark the root as visited
    visNodes[ID] = true;

    // Traverse till Queue is empty
    while (!qGraph.empty())
    {
        // Remove the element from the Queue
        int v = qGraph.dequeue();

        // Scan through all the nodes in the graph
        for (int w = 0; w < NUMNODES; w++)
        {
            if (tmpIntree[w][v])
            {
                if (!visNodes[w])
                {
                    visNodes[w] = 1;
                    (this->*func)(w, v, tmpIntree);
                    qGraph.enqueue(w);
                }
            }
        }
    }
}

void Routing::extendedBFSt(size_t ID, size_t rootedAt, int (&tmpIntree)[NUMNODES][NUMNODES], nodeLevel (&levels)[NUMNODES], void (Routing::*func)(size_t, size_t, int (&)[NUMNODES][NUMNODES], nodeLevel (&)[NUMNODES]))
{
    // Mark the levels as zero
    levels[ID].level = 0;
    levels[ID].dest = -1;

    // Queue to traverse
    Queue qGraph(NUMNODES);

    // Record for visited Nodes
    bool visNodes[NUMNODES] = {false};

    // Enqueue the root
    qGraph.enqueue(ID);

    // Mark the root as visited
    visNodes[ID] = true;

    // Traverse till Queue is empty
    while (!qGraph.empty())
    {
        // Remove the element from the Queue
        int v = qGraph.dequeue();

        // Scan through all the nodes in the graph
        for (int w = 0; w < NUMNODES; w++)
        {
            if (tmpIntree[w][v])
            {
                if (!visNodes[w])
                {
                    visNodes[w] = 1;
                    (this->*func)(w, v, tmpIntree, levels);
                    qGraph.enqueue(w);
                }
            }
        }
    }
}

void Routing::extendedBFSi(size_t ID, size_t rootedAt, int (&tmpIntree)[NUMNODES][NUMNODES], void (Routing::*func)(size_t, size_t, int (&)[NUMNODES][NUMNODES]))
{
    // Queue to traverse
    Queue qGraph(NUMNODES);

    // Record for visited Nodes
    bool visNodes[NUMNODES] = {false};

    // Enqueue the root
    qGraph.enqueue(rootedAt);

    // Mark the root as visited
    visNodes[rootedAt] = true;

    // Traverse till Queue is empty
    while (!qGraph.empty())
    {
        // Remove the element from the Queue
        int v = qGraph.dequeue();

        // Scan through all the nodes in the graph
        for (int w = 0; w < NUMNODES; w++)
        {
            if (intree[w][v])
            {
                if (!visNodes[w])
                {
                    visNodes[w] = 1;
                    (this->*func)(w, v, tmpIntree);
                    qGraph.enqueue(w);
                }
            }
        }
    }
}

void Routing::extendedBFSi(size_t ID, size_t rootedAt, int (&tmpIntree)[NUMNODES][NUMNODES], nodeLevel (&levels)[NUMNODES], void (Routing::*func)(size_t, size_t, int (&)[NUMNODES][NUMNODES], nodeLevel (&)[NUMNODES]))
{
    // Mark the levels as zero
    levels[rootedAt].level = 0;
    levels[rootedAt].dest = -1;

    // Queue to traverse
    Queue qGraph(NUMNODES);

    // Record for visited Nodes
    bool visNodes[NUMNODES] = {false};

    // Enqueue the root
    qGraph.enqueue(rootedAt);

    // Mark the root as visited
    visNodes[rootedAt] = true;

    // Traverse till Queue is empty
    while (!qGraph.empty())
    {
        // Remove the element from the Queue
        int v = qGraph.dequeue();

        // Scan through all the nodes in the graph
        for (int w = 0; w < NUMNODES; w++)
        {
            if (intree[w][v])
            {
                if (!visNodes[w])
                {
                    visNodes[w] = 1;
                    (this->*func)(w, v, tmpIntree, levels);
                    qGraph.enqueue(w);
                }
            }
        }
    }
}

void Routing::buildSPT(size_t ID, size_t rootedAt, int (&tmpIntree)[NUMNODES][NUMNODES])
{
    // Store the Previous Intree
    for(size_t i = 0; i < NUMNODES; i++)
    {
        for(size_t j = 0; j < NUMNODES; j++)
        {
            prevIntree[i][j] = intree[i][j];
        }
    }

    // Modify the intree of the incoming neighbor
    for (size_t i = 0; i < NUMNODES; i++)
    {
        tmpIntree[ID][i] = 0;
    }

    extendedBFSt(ID, rootedAt, tmpIntree, &Routing::removeTmpTreePath);

    tmpIntree[rootedAt][ID] = 1;

    // Prune the dead nodes from the intree by comparing it with the last tempintree
    extendedBFSi(ID, rootedAt, tmpIntree, &Routing::pruneNode);

    // Mark all the nodes as unvisited at the start
    nodeLevel levelCur[NUMNODES];
    nodeLevel levelTmp[NUMNODES];

    extendedBFSi(rootedAt, ID, tmpIntree, levelCur, &Routing::addLevel);

    extendedBFSt(ID, rootedAt, tmpIntree, levelTmp, &Routing::addLevel);

    int mergeTree[NUMNODES][NUMNODES] = {0};

    // Merge the levels
    for (int hop = 1; hop < NUMNODES; hop++)
    {
        int count = NUMNODES;
        while (count--)
        {
            int cmpLvl = -1;
            int cmpTmp = -1;

            for (size_t curLvl = 0; curLvl < NUMNODES; curLvl++)
            {
                if (levelCur[curLvl].level == hop)
                {
                    cmpLvl = curLvl;
                    break;
                }
            }

            for (size_t tmpLvl = 0; tmpLvl < NUMNODES; tmpLvl++)
            {
                if (levelTmp[tmpLvl].level == hop)
                {
                    cmpTmp = tmpLvl;
                    break;
                }
            }

            if (cmpLvl == -1 && cmpTmp == -1)
            {
                break;
            }

            else if ((cmpLvl != -1 && cmpTmp == -1) || (cmpLvl != -1 && cmpTmp != -1 && cmpLvl < cmpTmp))
            {
                int dest = levelCur[cmpLvl].dest;

                if (levelTmp[cmpLvl].level == -1)
                {
                    mergeTree[cmpLvl][dest] = 1;
                }
                else
                {
                    mergeTree[cmpLvl][dest] = 1;

                    // Modify the intree of the incoming neighbor
                    tmpIntree[cmpLvl][levelTmp[cmpLvl].dest] = 0;

                    extendedBFSt(cmpLvl, rootedAt, tmpIntree, levelTmp, &Routing::removeLevel);
                }

                levelCur[cmpLvl].level = -1;
                levelCur[cmpLvl].dest = -1;

                levelTmp[cmpLvl].level = -1;
                levelTmp[cmpLvl].dest = -1;
            }

            else if ((cmpLvl == -1 && cmpTmp != -1) || (cmpLvl != -1 && cmpTmp != -1 && cmpLvl > cmpTmp))
            {
                int dest = levelTmp[cmpTmp].dest;

                if (levelCur[cmpTmp].level == -1)
                {
                    mergeTree[cmpTmp][dest] = 1;
                }
                else
                {
                    mergeTree[cmpTmp][dest] = 1;

                    // Modify the intree of the myself
                    intree[cmpTmp][levelCur[cmpTmp].dest] = 0;

                    extendedBFSi(ID, cmpTmp, tmpIntree, levelCur, &Routing::removeLevel);
                }

                levelTmp[cmpTmp].level = -1;
                levelTmp[cmpTmp].dest = -1;

                levelCur[cmpTmp].level = -1;
                levelCur[cmpTmp].dest = -1;
            }

            else if (cmpLvl != -1 && cmpTmp != -1 && cmpLvl == cmpTmp)
            {
                int dest = levelCur[cmpLvl].dest;

                mergeTree[cmpLvl][dest] = 1;

                levelCur[cmpLvl].level = -1;
                levelCur[cmpLvl].dest = -1;

                levelTmp[cmpTmp].level = -1;
                levelTmp[cmpTmp].dest = -1;
            }
        }
    }

    // copy to intree
    for (size_t i = 0; i < NUMNODES; i++)
        for (size_t j = 0; j < NUMNODES; j++)
            intree[i][j] = mergeTree[i][j];

    // Check if the intree changed to push it immediately
    for(size_t i = 0; i < NUMNODES; i++)
    {
        for(size_t j = 0; j < NUMNODES; j++)
        {
            if(prevIntree[i][j] != intree[i][j])
            {
                sendIntreeNow = true;
            }
        }
    }
}

class Node
{
public:
    Node(size_t ID, size_t duration, int dest, string dataMessage) : ID(ID), duration(duration), msg(dest, dataMessage)
    {
        setChannels();
    };
    ~Node();

    // ID of the node
    size_t ID;

    // Duration
    size_t duration;

    // Hello Message Sender
    void helloProtocol();

    // Intree Protocol
    void intreeProtocol();

    // Data Protocol
    void dataProtocol();

    // Process Input File
    void processInputFile();

private:
    // Keep record of who sent the intree message
    bool gotIntree[NUMNODES] = {0};

    // Channels of the Node
    FileDescriptor channel;

    // Routing Data Structure
    Routing msg;

    // init the channels
    void setChannels();

    // read the file contents line by line
    string readFile(fstream &);

    // Return the path to the destination
    void findPathToDest(int, string &);

    // Compute the Hello Messages
    void computeHello(string &);

    // Compute the intree Messages
    void computeIntree(string &);

    // Compute the Data Messages
    void computeData(string &);
};

Node::~Node()
{
    // Close the channels
    channel.input.close();
    channel.output.close();
    channel.receivedData.close();
}

void Node::setChannels()
{
    channel.inputFileName = string("input_") + char('0' + ID);
    channel.outputFileName = string("output_") + char('0' + ID);
    channel.receivedFileName = char('0' + ID) + string("_received");

    channel.input.open(channel.inputFileName.c_str(), ios::out);
    channel.input.close();

    channel.input.open(channel.inputFileName.c_str(), ios::in);
    channel.output.open(channel.outputFileName.c_str(), ios::out | ios::app);
    channel.receivedData.open(channel.receivedFileName.c_str(), ios::out | ios::app);

    if (channel.input.fail())
    {
        cout << "Node " << ID << ": No input file" << endl;
        exit(1);
    }
    if (channel.output.fail())
    {
        cout << "Node " << ID << ": No output file" << endl;
        exit(1);
    }
    if (channel.receivedData.fail())
    {
        cout << "Node " << ID << ": No receivedData file" << endl;
        exit(1);
    }
}

string Node::readFile(fstream &fd)
{
    string line = "";
    getline(fd, line);
    if (fd.eof())
    {
        fd.clear();
        line = "";
    }

    return line;
}

void Node::helloProtocol()
{
    // Send the Hello Message on the Output file for the controller to read
    channel.output << "Hello " << ID << endl;
    channel.output.flush();
}

void Node::intreeProtocol()
{
    // Check the status of incoming Neighbors
    if (msg.isINempty())
    {
        channel.output << "Intree " << ID << endl;
        channel.output.flush();
        return;
    }

    // Create a buffer for the message to send
    string buffer = "Intree " + to_string(ID) + " ";

    // Traverse the Intree
    Queue qCurNode(NUMNODES);

    // Visit Node
    bool visCur[NUMNODES] = {false};

    // Enqueue the Node
    qCurNode.enqueue(ID);

    // Mark it visited
    visCur[ID] = true;

    // Check if a Current Node Queue is empty
    while (!qCurNode.empty())
    {
        // Remove the element from the Queue
        int v = qCurNode.dequeue();

        // Scan through all the nodes in the graph
        for (int w = 0; w < NUMNODES; w++)
        {
            if (msg.intree[w][v])
            {
                if (!visCur[w])
                {
                    visCur[w] = 1;
                    qCurNode.enqueue(w);
                    buffer = buffer + "(" + to_string(w) + " " + to_string(v) + ")";
                }
            }
        }
    }

    // write to the file
    channel.output << buffer << endl;
    channel.output.flush();
}

void Node::findPathToDest(int v, string &path)
{
    // Store the path
    path = path + to_string(v) + " ";

    for (size_t w = 0; w < NUMNODES; w++)
    {
        if (msg.intree[v][w])
        {
            findPathToDest(w, path);
        }
    }
}

void Node::dataProtocol()
{
    // Send the Data Message if the destination is not -1
    if (msg.dest != -1)
    {
        // Clear the old path
        msg.pathToDest = "";

        // Find the new path
        findPathToDest(msg.dest, msg.pathToDest);

        // Check if string was empty or not
        string tempCheck = to_string(msg.dest) + " ";
        if (msg.pathToDest == tempCheck)
        {
            msg.pathToDest = "";

            //exit
            return;
        }

        // Find the Incoming Neighbor
        size_t len = msg.pathToDest.length();
        char in = msg.pathToDest[len - 4];

        if (msg.pathToIncomingNeighbors[in - '0'] == "")
            return;

        string path = msg.pathToIncomingNeighbors[in - '0'];

        path.erase(0, 2);

        // Send the data to the Incoming Neighbor
        channel.output << "Data " << ID << " " << msg.dest << " " << path << "begin " << msg.dataMessage << endl;
        channel.output.flush();
    }
}

void Node::computeHello(string &line)
{
    // Read the Input file to check for the message
    // and then update the incoming neighbors

    //Store the node number in char form
    char c = line[6];

    // Update the Incoming Neighbors
    msg.incomingNeighbors[c - '0'] = 1;
}

void Node::computeIntree(string &line)
{

    // Read the input file
    // Update the Intree Graph
    // Make the Intree Graph with the help of the Intree message and Incoming neighbors
    // Parse the Intree Message
    // Calculate the total length of the message
    size_t len = line.length();

    // Find who sent this message
    char c = line[7];
    // Let's convert it to int;
    int rootedAt = c - '0';
    // Store in the who sent Intree
    gotIntree[rootedAt] = true;

    // Calculate the length of body part of the message
    size_t body;
    if (len == 8)
    {
        // No body to calculate
        body = 0;
    }
    else
    {
        // Size of the header plus extra the space, starts from '(' and ends at ')'
        body = len - 9;
    }

    // Create a temporary Intree Graph of the received Intree message
    int tmpIntree[NUMNODES][NUMNODES] = {{0}};

    // Extract the node numbers from the message
    for (size_t i = 0; i < body; i += 5)
    {
        char r = line[10 + i];
        char c = line[10 + i + 2];

        // Place a directed edge here
        tmpIntree[r - '0'][c - '0'] = 1;
    }

    //Refresh the Contents in Path To Incoming Neighbor
    msg.pathToIncomingNeighbors[rootedAt] = "";
    // Find the path to the Incoming Neighbor
    msg.storePathToIncomingNeighbor(ID, rootedAt, tmpIntree);
    // Check if string was empty or not
    string tmpCheck = to_string(ID) + " ";
    if (msg.pathToIncomingNeighbors[rootedAt] == tmpCheck)
        msg.pathToIncomingNeighbors[rootedAt] = "";

    // Merge the two trees
    msg.buildSPT(ID, rootedAt, tmpIntree);
}

void Node::computeData(string &line)
{
    // Parse the input file
    // Extract the Intermediate node
    char dataInterDest = line[9];

    // Check if it is destined to me
    if (unsigned(dataInterDest - '0') != ID)
        return;

    // Extract the Destination Node
    char dataDest = line[7];

    // Extract the Source Node
    char dataSrc = line[5];

    if (unsigned(dataDest - '0') == ID && line[11] == 'b')
    {
        // Extract the data Message
        string message = line.erase(0, 17);

        // Add the data to the received file
        channel.receivedData << "Message from " << dataSrc << " to " << dataDest << " : " << message << endl;
    }
    else
    {
        if (line[11] == 'b')
        {

            // Extract the data Message
            string message = line.erase(0, 17);

            //Pass to Neighbor
            string intermediateNode = "";

            // Find the new path
            findPathToDest((dataDest - '0'), intermediateNode);

            // Check if string was empty or not
            string tempCheck = dataDest + " ";
            if (intermediateNode == tempCheck)
            {
                intermediateNode = "";
                return;
            }

            // Find the Incoming Neighbor
            size_t len = intermediateNode.length();
            char in = intermediateNode[len - 4];

            if (msg.pathToIncomingNeighbors[in - '0'] == "")
                return;

            string path = msg.pathToIncomingNeighbors[in - '0'];

            path.erase(0, 2);

            for (size_t j = 0; j < NUMNODES; j++)
            {
                if (msg.passDataToNeighbor[dataSrc - '0'][j] == "")
                {
                    msg.passDataToNeighbor[dataSrc - '0'][j] = "Data " + to_string((dataSrc - '0')) + " " + to_string((dataDest - '0')) + " " + path + "begin " + message;
                    break;
                }
            }
        }
        else
        {
            // Remove myself from the intermediate nodes
            line.erase(line.begin() + 9);
            line.erase(line.begin() + 9);

            for (size_t j = 0; j < NUMNODES; j++)
            {
                if (msg.passDataToNeighbor[dataSrc - '0'][j] == "")
                {
                    msg.passDataToNeighbor[dataSrc - '0'][j] = line;
                    break;
                }
            }
        }
    }
}

void Node::processInputFile()
{
    static size_t timer = 0;
    string line = "";
    while ((line = readFile(channel.input)) != "")
    {
        // Check for Hello Message
        if (line[0] == 'H')
            computeHello(line);

        // Check for Intree Message
        if (line[0] == 'I')
            computeIntree(line);

        // Check for Data Message
        if (line[0] == 'D')
            computeData(line);
    }

    if (timer > 0 && ((timer - 2) % 10) == 0)
    {
        // Keep with the neighbors who sent the Intree
        for (size_t i = 0; i < NUMNODES; i++)
        {
            if (msg.incomingNeighbors[i] == 1 && gotIntree[i] == false)
            {
                cout << "Node " << ID << ": oh no! Node " << i << " got killed! Time to adapt my peers!" << endl;

                // Modify the intree of the Node
                msg.intree[i][ID] = 0;

                // Remove the subtree
                msg.extendedBFSi(ID, i, msg.intree, &Routing::removeInTreePath);

                // Remove it from the Incoming Neighbor
                msg.incomingNeighbors[i] = 0;

                // Push the intree message Immediately
                msg.sendIntreeNow = true;
            }
            else if (msg.incomingNeighbors[i] == 0 && gotIntree[i] == true)
            {
                // Add it to the incoming Neighbors
                msg.incomingNeighbors[i] = 1;
            }

            gotIntree[i] = false;
        }
    }

    // Push the In-tree Immediately
    if(msg.sendIntreeNow)
    {
        intreeProtocol();
        msg.sendIntreeNow = false;
    }

    // Pass the Data Message to the Neighbor
    for (size_t i = 0; i < NUMNODES; i++)
    {
        for (size_t j = 0; j < NUMNODES; j++)
        {
            if (msg.passDataToNeighbor[i][j] != "")
            {
                channel.output << msg.passDataToNeighbor[i][j] << endl;
                channel.output.flush();
                msg.passDataToNeighbor[i][j] = "";
            }
        }
    }

    timer++;
}

int main(int argc, char *argv[])
{
    //Check number of arguments
    if (argc < 4 || argc > 5)
    {
        cout << "too " << (argc < 4 ? "few " : "many ") << "arguments passed" << endl;
        cout << "Requires: ID, Duration, Destination, Message(if Destination !=-1)" << endl;
        return -1;
    }

    //Convert Char Array to long int
    long int arg[3];
    for (int i = 0; i < 3; i++)
        arg[i] = strtol(argv[i + 1], NULL, 10);

    //Check if a node is going to send data or not
    string data;
    if (arg[2] == -1)
        data = "";
    else
        data = argv[4];

    //Create a node
    Node node(arg[0], arg[1], arg[2], data);

    for (size_t i = 0; i < node.duration; i++)
    {
        // Send Hello Message every 30 seconds
        if (i % 30 == 0)
            node.helloProtocol();

        // Send In tree message every 10 seconds
        if (i % 10 == 0)
            node.intreeProtocol();

        // Send Data message every 15 seconds
        if (i % 15 == 0)
            node.dataProtocol();

        // Read the Input file and update the received file if neccessary
        node.processInputFile();

        // Sleep for One second
        sleep(1);
    }

    cout << "Node " << node.ID << " Done" << endl;

    return 0;
}