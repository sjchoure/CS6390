/*  
 *  Simulates a fictional component inside a network
 *  to enable forwarding of data packets from one node to other node.
 *  Copyright (C) 2021 Sourabh J Choure
 * 
 *  I promise that the work presented below is of my own.
 * 
 *  To make it FOSS Compliant, this software is free to use.
 */

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
using namespace std;

#define NUMNODES 10

struct FileDescriptor
{
    // Store the name of the file
    string inputFileName;
    string outputFileName;

    // File Descriptors
    fstream input;
    ofstream output;
};

class NodeRecord
{
public:
    NodeRecord(){};
    ~NodeRecord(){};

    // Total number of nodes
    size_t numNodes = 0;

    // Channels of Controller
    FileDescriptor *channels;

    // Topology Links
    int topologyLinks[NUMNODES][NUMNODES] = {{0}};

    // Counter to check if the node is not responding
    int nodeNotResponding[NUMNODES] = {0};

    // Create the channels
    void createChannels();
};

void NodeRecord::createChannels()
{
    channels = new FileDescriptor[numNodes];

    for (size_t i = 0; i < numNodes; i++)
    {
        // Give a name to the files
        channels[i].inputFileName = string("output_") + char('0' + i);
        channels[i].outputFileName = string("input_") + char('0' + i);

        // Create the files
        channels[i].input.open(channels[i].inputFileName.c_str(), ios::in);
        channels[i].output.open(channels[i].outputFileName.c_str(), ios::out | ios::app);

        if (channels[i].input.fail())
        {
            cout << "Controller: Node " << i << " No input file" << endl;
            exit(1);
        }
        if (channels[i].output.fail())
        {
            cout << "Controller: Node " << i << " No output file" << endl;
            exit(1);
        }
    }
}

class Controller
{
public:
    Controller(size_t duration) : duration(duration)
    {
        setChannel(); // topology
        createNodeChannels(); // Node channels
    };
    ~Controller(){};

    // Duration
    size_t duration;

    //Hello Protocol
    void sendToNeighbors();

private:
    // Channels of Controller
    FileDescriptor channel;

    // Node Record Entries
    NodeRecord nodes;

    // Init Channels
    void setChannel();

    //Create New Channels
    void createNodeChannels();

    //Parse the strings
    void parseString(string);

    // Read File
    string readFile(fstream &);
};

void Controller::parseString(string line) //Waring: Single Sequencial Digit Parser only!!
{
    // Store the characters
    char c1 = line[0];
    char c2 = line[2];

    if (unsigned(c1 - '0') + 1 > nodes.numNodes || unsigned(c2 - '0') + 1 > nodes.numNodes)
    {
        if ((c1 - '0') > (c2 - '0'))
            nodes.numNodes = c1 - '0' + 1;
        else
            nodes.numNodes = c2 - '0' + 1;
    }

    nodes.topologyLinks[unsigned(c1 - '0')][unsigned(c2 - '0')] = 1;
}

void Controller::createNodeChannels()
{
    // Check and Parse the topology file
    string line;
    while (getline(channel.input, line) && !channel.input.eof())
    {
        parseString(line);
    }

    // Create the Channels
    nodes.createChannels();
}

void Controller::setChannel()
{
    // Store the name of the file to open
    channel.inputFileName = string("topology");
    channel.outputFileName = "";

    // Open the file as input
    channel.input.open(channel.inputFileName.c_str(), ios::in);
    if (channel.input.fail())
    {
        cout << "No file";
        exit(1);
    }
}

string Controller::readFile(fstream &fd)
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

void Controller::sendToNeighbors()
{
    // Search through the topology links to find the neighbors
    for (size_t i = 0; i < nodes.numNodes; i++)
    {
        // Read the output file of the node for the hello message
        string line = readFile(nodes.channels[i].input);

        // If the read is empty don't transmit
        if (line != "")
        {
            // Go through all the links of that particular nodes
            for (size_t j = 0; j < nodes.numNodes; j++)
            {
                // If the link exist then put the message of that nodes input file
                if (nodes.topologyLinks[i][j])
                {
                    nodes.channels[j].output << line << endl;
                    nodes.channels[j].output.flush(); //force
                }
            }
        }
    }
}

int main(int argc, char *argv[])
{
    //Check number of arguments
    if (argc != 2)
    {
        cout << "too " << (argc < 3 ? "few " : "many ") << "arguments passed" << endl;
        cout << "Requires: Duration" << endl;
        return -1;
    }

    //Convert Char Array to long int
    long int arg = strtol(argv[1], NULL, 10);

    // Let the nodes get init
    sleep(1);

    cout << endl;

    //Create a node
    Controller controller(arg);

    // Start the algo
    for (size_t i = 0; i < controller.duration; i++)
    {
        if (i % 30 == 0) 
            controller.sendToNeighbors();
        if (i % 10 == 0) 
            controller.sendToNeighbors();

        sleep(1);
    }
    cout << "Controller Done" << endl;

    return 0;
}