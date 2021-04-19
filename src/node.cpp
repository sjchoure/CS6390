#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <unistd.h>

using namespace std;

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
    int incomingNeighbors[10] = {0};

    // Keep track of Outgoing Neighbors
    int outgoingNeighbors[10] = {0};

    // In-tree of a Node
    int intree[10][10] = {{0}};
};

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

void Node::computeHello()
{
    // Refresh the Incoming Neighbors
    fill(msg.incomingNeighbors,msg.incomingNeighbors+10,0);

    // Read the Input file to check for the message
    // and then update the incoming neighbors
    string line;
    while ((line = readFile(channel.input)) != "")
    {
        //Store the node number in char form
        char c = line[6];

        // Update the Incoming Neighbors
        msg.incomingNeighbors[c-'0'] = 1;
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
        //if (i % 10)

        // Send Data message every 15 seconds
        //if (i % 15)

        // Read the Input file and update the received file if neccessary
        node.processInputFile();
        sleep(1);
    }

    cout << "Node " << node.ID << " Done" << endl;

    return 0;
}