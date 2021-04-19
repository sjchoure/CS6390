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

struct Routing
{
    Routing(int dest, string dataMessage) : dest(dest), dataMessage(dataMessage){};
    // Destination Node
    int dest;

    // Buffer for the data to be sent
    string dataMessage;

    // Keep track of Incoming Neighbors
    int incomingNeighbors[NUMNODES] = {0};

    // Keep track of Outgoing Neighbors
    int outgoingNeighbors[NUMNODES] = {0};

    // In-tree of a Node
    int intree[NUMNODES][NUMNODES] = {{0}};

    // Check if incoming Neighbors is empty
    bool isINempty();
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

    // Compute the Hello Messages
    void computeHello();

    // Compute the intree Messages
    void computeIntree();
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
    string buffer = "Intree " + to_string(ID) + " Something too";
    channel.output << buffer << endl;
    channel.output.flush();
}

void Node::computeHello()
{
    // Refresh the Incoming Neighbors
    fill(msg.incomingNeighbors, msg.incomingNeighbors + 10, 0);

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

    cout << "Node " << ID << endl;
    for (size_t i = 0; i < NUMNODES; i++)
        cout << msg.incomingNeighbors[i] << " ";
    cout << endl;
}

void Node::computeIntree()
{
    // Read the inpute file
    // Update the intree array
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

        // TODO
    }
}

void Node::processInputFile()
{
    // Counter to check when it is time to expect input
    static size_t timer = 0;

    if (timer != 0)
    {
        // Check for Hello Message
        if (timer % 32 == 0 || timer == 2)
        {
            computeHello();
        }

        // Check for Intree Message
        if (timer % 12 == 0 || timer == 2)
        {
            computeIntree();
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
        //if (i % 15)

        // Read the Input file and update the received file if neccessary
        node.processInputFile();
        sleep(1);
    }

    cout << "Node " << node.ID << " Done" << endl;

    return 0;
}