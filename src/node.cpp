/*  
 *  Simulates an actual node inside a network with capabilities to 
 *  send and receive data packets via files.
 *  Copyright (C) 2021 Sourabh J Choure
 * 
 *  I promise that the work presented below is of my own.
 * 
 *  To make it FOSS Compliant, this software is free to use.
 */

#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
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
    Routing(int dest, string dataMessage) : dest(dest), dataMessage(dataMessage){};

    // Destination Node
    int dest;

    // Path to Destination
    string pathToDest = "";

    // Buffer for the data to be sent
    string dataMessage;

    // Buffer for passing message to Neighbor
    string passDataToNeighbor[NUMNODES] = {""};

    // Keep track of Incoming Neighbors
    int incomingNeighbors[NUMNODES] = {0};

    // Keep track of Outgoing Neighbors
    int outgoingNeighbors[NUMNODES] = {0};

    // In-tree of a Node
    int intree[NUMNODES][NUMNODES] = {{0}};

    // Store the path to the neighbor
    string pathToIncomingNeighbors[NUMNODES] = {""};

    // Check if incoming Neighbors is empty
    bool isINempty();

    // Find the path to the Incoming Neighbor
    void storePathToIncomingNeighbor(size_t, size_t, int (&)[NUMNODES][NUMNODES]);

    // BFS Traversal for the graph
    void BFS(size_t, size_t, int (&)[NUMNODES][NUMNODES]);
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

void Routing::BFS(size_t ID, size_t rootedAt, int (&tmpIntree)[NUMNODES][NUMNODES])
{
    {
        // Queue to remove
        Queue qTmpRmv(NUMNODES);

        // Visited for remove
        bool visRmv[NUMNODES] = {false};

        // Modify the intree of the incoming neighbor
        tmpIntree[ID][rootedAt] = 0;

        // Put the remove subtree in Queue
        qTmpRmv.enqueue(ID);

        // Mark the remove node as visited
        visRmv[ID] = true;

        // Remove the subtree
        while (!qTmpRmv.empty())
        {
            // Remove the element from the Queue
            int v = qTmpRmv.dequeue();

            // Scan through all the nodes in the graph
            for (int w = 0; w < NUMNODES; w++)
            {
                if (tmpIntree[w][v])
                {
                    if (!visRmv[w])
                    {
                        visRmv[w] = 1;
                        tmpIntree[w][v] = 0;
                        qTmpRmv.enqueue(w);
                    }
                }
            }
        }
    }

    tmpIntree[rootedAt][ID] = 1;

    // Queues for both the tree
    Queue qCurNode(NUMNODES);
    Queue qTmpNode(NUMNODES);

    // Visited array for both the trees
    bool visCur[NUMNODES] = {false};
    bool visTmp[NUMNODES] = {false};

    // Mark all the nodes as unvisited at the start
    nodeLevel levelCur[NUMNODES];
    nodeLevel levelTmp[NUMNODES];

    // Mark both the nodes of tree as visited
    visCur[ID] = true;
    visTmp[ID] = true;

    // Put them in Queue
    qCurNode.enqueue(ID);
    qTmpNode.enqueue(ID);

    // Mark the level of both trees
    levelCur[ID].level = 0;
    levelCur[ID].dest = -1;
    levelTmp[ID].level = 0;
    levelTmp[ID].dest = -1;

    //cout << "Intree Node " << ID << endl;

    // Check if a Current Node Queue is empty
    while (!qCurNode.empty())
    {
        // Remove the element from the Queue
        int v = qCurNode.dequeue();
        //cout << " \tCurNode: " << v << " Level: " << levelCur[v].level << endl;
        // Scan through all the nodes in the graph
        for (int w = 0; w < NUMNODES; w++)
        {
            if (intree[w][v])
            {
                if (!visCur[w])
                {
                    visCur[w] = 1;
                    levelCur[w].level = levelCur[v].level + 1;
                    levelCur[w].dest = v;
                    qCurNode.enqueue(w);
                }
            }
        }
    }

    // Check if the Temp Node Queue is empty
    while (!qTmpNode.empty())
    {
        // Remove the element from the Queue
        int v = qTmpNode.dequeue();
        //cout << " \tTmpNode: " << v << " Level: " << levelTmp[v].level << endl;
        // Scan through all the nodes in the graph
        for (int w = 0; w < NUMNODES; w++)
        {
            if (tmpIntree[w][v])
            {
                if (!visTmp[w])
                {
                    visTmp[w] = 1;
                    levelTmp[w].level = levelTmp[v].level + 1;
                    levelTmp[w].dest = v;
                    qTmpNode.enqueue(w);
                }
            }
        }
    }

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

                    // Queue to remove
                    Queue qTmpRmv(NUMNODES);

                    // Visited for remove
                    bool visRmv[NUMNODES] = {false};

                    // Modify the intree of the incoming neighbor
                    tmpIntree[cmpLvl][levelTmp[cmpLvl].dest] = 0;

                    // Put the remove subtree in Queue
                    qTmpRmv.enqueue(cmpLvl);

                    // Mark the remove node as visited
                    visRmv[cmpLvl] = true;

                    // Remove the subtree
                    while (!qTmpRmv.empty())
                    {
                        // Remove the element from the Queue
                        int v = qTmpRmv.dequeue();

                        // Scan through all the nodes in the graph
                        for (int w = 0; w < NUMNODES; w++)
                        {
                            if (tmpIntree[w][v])
                            {
                                if (!visRmv[w])
                                {
                                    visRmv[w] = 1;
                                    tmpIntree[w][v] = 0;
                                    levelTmp[w].level = -1;
                                    levelTmp[w].dest = -1;
                                    qTmpRmv.enqueue(w);
                                }
                            }
                        }
                    }
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

                    // Queue to remove
                    Queue qTmpRmv(NUMNODES);

                    // Visited for remove
                    bool visRmv[NUMNODES] = {false};

                    // Modify the intree of the incoming neighbor
                    intree[cmpTmp][levelCur[cmpTmp].dest] = 0;

                    // Put the remove subtree in Queue
                    qTmpRmv.enqueue(cmpTmp);

                    // Mark the remove node as visited
                    visRmv[cmpTmp] = true;

                    // Remove the subtree
                    while (!qTmpRmv.empty())
                    {
                        // Remove the element from the Queue
                        int v = qTmpRmv.dequeue();

                        // Scan through all the nodes in the graph
                        for (int w = 0; w < NUMNODES; w++)
                        {
                            if (intree[w][v])
                            {
                                if (!visRmv[w])
                                {
                                    visRmv[w] = 1;
                                    intree[w][v] = 0;
                                    levelCur[w].level = -1;
                                    levelCur[w].dest = -1;
                                    qTmpRmv.enqueue(w);
                                }
                            }
                        }
                    }
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
    void computeHello();

    // Compute the intree Messages
    void computeIntree();

    // Compute the Data Messages
    void computeData();
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
    //cout << "Bad:" << channel.input.bad() << " EOF:" << channel.input.eof() << " Fail:" << channel.input.fail() << " GOOD:"<< channel.input.good();
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
    for(size_t i = 0; i < NUMNODES; i++)
    {
        if(msg.passDataToNeighbor[i] != "")
        {
            channel.output << msg.passDataToNeighbor[i] << endl;
            channel.output.flush();
            msg.passDataToNeighbor[i] = "";
        }
    }
    
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

        // Debug
        // cout << msg.pathToDest << " and " << msg.dataMessage << endl;

        // Find the Incoming Neighbor
        size_t len = msg.pathToDest.length();
        char in = msg.pathToDest[len - 4];

        if (msg.pathToIncomingNeighbors[in - '0'] == "")
            return;

        // Send the data to the Incoming Neighbor
        channel.output << "Data " << ID << " " << msg.dest << " " << msg.pathToIncomingNeighbors[in - '0'].erase(0, 2) << "begin " << msg.dataMessage << endl;
        channel.output.flush();
    }
}

void Node::computeHello()
{
    // Refresh the Incoming Neighbors
    fill(msg.incomingNeighbors, msg.incomingNeighbors + NUMNODES, 0);

    // Read the Input file to check for the message
    // and then update the incoming neighbors
    string line;
    streampos oldpos;
    while (oldpos = channel.input.tellg(), (line = readFile(channel.input)) != "")
    {
        // Check if the line read is a Hello Message or not, if not then put the line back to the file and break.
        if (line[0] != 'H')
        {
            channel.input.seekg(oldpos);
            break;
        }

        //Store the node number in char form
        char c = line[6];

        // Update the Incoming Neighbors
        msg.incomingNeighbors[c - '0'] = 1;
    }

    // cout << "Hello Node " << ID << endl;
    // cout << "\t";
    // for (size_t i = 0; i < NUMNODES; i++)
    //     cout << msg.incomingNeighbors[i] << " ";
    // cout << endl;
}

void Node::computeIntree()
{
    // Keep record of who sent the intree message
    bool gotIntree[NUMNODES] = {0};

    // Read the input file
    // Update the Intree Graph
    string line;
    streampos oldpos;
    while (oldpos = channel.input.tellg(), (line = readFile(channel.input)) != "")
    {
        // Check if the line read is an Intree Message or not, if not then put the line back to the file and break.
        if (line[0] != 'I')
        {
            channel.input.seekg(oldpos);
            break;
        }

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

        //cout << "Node " << rootedAt << " Sent Intree with length " << len << " and body " << body << endl;

        // Create a temporary Intree Graph of the received Intree message
        int tempIntree[NUMNODES][NUMNODES] = {{0}};

        // Extract the node numbers from the message
        for (size_t i = 0; i < body; i += 5)
        {
            char r = line[10 + i];
            char c = line[10 + i + 2];

            // Place a directed edge here
            tempIntree[r - '0'][c - '0'] = 1;
        }

        //Refresh the Contents in Path To Incoming Neighbor
        msg.pathToIncomingNeighbors[rootedAt] = "";
        // Find the path to the Incoming Neighbor
        msg.storePathToIncomingNeighbor(ID, rootedAt, tempIntree);
        // Check if string was empty or not
        string tempCheck = to_string(ID) + " ";
        if (msg.pathToIncomingNeighbors[rootedAt] == tempCheck)
            msg.pathToIncomingNeighbors[rootedAt] = "";

        // Debug
        //cout << "Node " << ID << " Path: " << msg.pathToIncomingNeighbors[rootedAt] << endl;

        // Merge the two trees
        msg.BFS(ID, rootedAt, tempIntree);
    }

    // Keep with the neighbors who sen the Intree
    for (size_t i = 0; i < NUMNODES; i++)
    {
        if (msg.incomingNeighbors[i] == 1 && gotIntree[i] == 0)
        {
            // Remove the subtree
            // Queue to remove
            Queue qTmpRmv(NUMNODES);

            // Visited for remove
            bool visRmv[NUMNODES] = {false};

            // Modify the intree of the Node
            msg.intree[i][ID] = 0;

            // Put the remove subtree in Queue
            qTmpRmv.enqueue(i);

            // Mark the remove node as visited
            visRmv[i] = true;

            // Remove the subtree
            while (!qTmpRmv.empty())
            {
                // Remove the element from the Queue
                int v = qTmpRmv.dequeue();

                // Scan through all the nodes in the graph
                for (int w = 0; w < NUMNODES; w++)
                {
                    if (msg.intree[w][v])
                    {
                        if (!visRmv[w])
                        {
                            visRmv[w] = 1;
                            msg.intree[w][v] = 0;
                            qTmpRmv.enqueue(w);
                        }
                    }
                }
            }

            // Remove it from the Incoming Neighbor
            msg.incomingNeighbors[i] = 0;
        }
        else if (msg.incomingNeighbors[i] == 0 && gotIntree[i] == 1)
        {
            // Add it to the incoming Neighbors
            msg.incomingNeighbors[i] = 1;
        }
    }

    // cout << " Node Intree " << ID << endl;
    // for(int i = 0; i < NUMNODES; i++)
    // {
    //     for(int j = 0; j < NUMNODES; j++)
    //         cout << msg.intree[i][j] << " ";
    //     cout << endl;
    // }
}

void Node::computeData()
{
    // Parse the input file
    string line;
    streampos oldpos;
    while (oldpos = channel.input.tellg(), (line = readFile(channel.input)) != "")
    {
        // Check if the line read is a Data Message or not, if not then put the line back to the file and break.
        if (line[0] != 'D')
        {
            channel.input.seekg(oldpos);
            break;
        }

        // Extract the Intermediate node
        char dataInterDest = line[9];

        // Check if it is destined to me
        if (unsigned(dataInterDest - '0') != ID)
            continue;

        // Extract the Destination Node
        char dataDest = line[7];

        // Extract the Source Node
        char dataSrc = line[5];

        if (unsigned(dataDest - '0') == ID)
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
                cout << "Node " << ID << " Path " << intermediateNode << endl;
                // Check if string was empty or not
                string tempCheck = dataDest + " ";
                if (intermediateNode == tempCheck)
                {
                    intermediateNode = "";

                    continue;
                }

                // Find the Incoming Neighbor
                size_t len = intermediateNode.length();
                char in = intermediateNode[len - 4];

                if (msg.pathToIncomingNeighbors[in - '0'] == "")
                    continue;
                cout << "Node " << ID << " IN " << msg.pathToIncomingNeighbors[in - '0'] << endl;
                msg.passDataToNeighbor[dataSrc - '0'] = "Data " + to_string((dataSrc-'0')) + " " + to_string((dataDest - '0')) + " " + msg.pathToIncomingNeighbors[in - '0'].erase(0,2) + "begin " + message;
                
            }
            else
            {
                // Remove myself from the intermediate nodes
                line.erase(line.begin()+9);
                line.erase(line.begin()+9);
                msg.passDataToNeighbor[dataSrc - '0'] = line;
            }
        }
    }
}

void Node::processInputFile()
{
    // Read the message after 2 second delay, to avoid race condition. The Controller is also delayed by 1 second
    // Counter to check when it is time to expect input
    static size_t timer = 0;

    if (timer >= 1)
    {
        // Check for Hello Message
        if ((timer - 1) % 30 == 0)
        {
            computeHello();
        }

        // Check for Intree Message
        if ((timer - 1) % 10 == 0)
        {
            computeIntree();
        }

        if ((timer - 1) % 15 == 0)
        {
            computeData();
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