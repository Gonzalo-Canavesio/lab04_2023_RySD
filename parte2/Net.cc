#ifndef NET
#define NET

#include <string.h>
#include <omnetpp.h>
#include <packet_m.h>
#include <map>
#include <vector>
#include <queue>

using namespace std;

#define MAXN 10000

using namespace omnetpp;

class Net: public cSimpleModule {
private:
    // Routing table is a map of destination and what interface to use
    std::map<int, int> routingTable;
    // Graph 
    std::vector<int> graph[MAXN];
    // Info received
    std::map<int, bool> infoReceived;
    bool ready;
    int numInterfaces;
    std::map<int, int> neighbors;
    int numNeighborsKnown;

public:
    Net();
    virtual ~Net();
protected:
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage *msg);
};

Define_Module(Net);

#endif /* NET */

Net::Net() {
}

Net::~Net() {
}

void Net::initialize() {
    ready = false;
    numInterfaces = gateSize("toLnk");
    
    numNeighborsKnown = 0;
    for(int i = 0; i < numInterfaces; i++) {
        // I send Hello messages to all my neighbors
        Packet *pkt = new Packet("Hello");
        pkt->setSource(this->getParentModule()->getIndex());
        pkt->setDestination(0);
        pkt->setHopCount(0);
        pkt->setKind(3);
        send(pkt, "toLnk$o", i);
    }

}

void Net::finish() {
}

void Net::handleMessage(cMessage *msg) {

    // All msg (events) on net are packets
    Packet *pkt = (Packet *) msg;

    // If the packet is a Hello message, answer with a HelloAck
    if (pkt->getKind() == 3) {
        Packet *pktAck = new Packet("HelloAck");
        pktAck->setSource(this->getParentModule()->getIndex());
        pktAck->setDestination(pkt->getSource());
        pktAck->setHopCount(0);
        pktAck->setKind(4);
        send(pktAck, "toLnk$o", pkt->getArrivalGate()->getIndex());
        delete (pkt);
        return;

    } else if(pkt->getKind() == 4) {
        // If the packet is a HelloAck
        numNeighborsKnown++;
        // Add the neighbor to the graph
        graph[this->getParentModule()->getIndex()].push_back(pkt->getSource());
        neighbors[pkt->getSource()] = pkt->getArrivalGate()->getIndex();
        infoReceived[pkt->getSource()] = false;

        // If I know all my neighbors, I'm ready to send the info to all packets
        if(numNeighborsKnown == numInterfaces) {
            // Send packet to neighbors resuming the network
            Packet *pkt = new Packet("Info");
            pkt->setSource(this->getParentModule()->getIndex());
            pkt->setDestination(0);
            pkt->setHopCount(0);
            pkt->setKind(5);
            pkt->setAmountNeighbours(numInterfaces);
            for(int i = 0; i < numInterfaces; i++) {
                pkt->setNeighbours(i, graph[this->getParentModule()->getIndex()][i]);
            }
            for(int i = 0; i < numInterfaces; i++) {
                send(pkt->dup(), "toLnk$o", i);
            }
        }
        delete (pkt);
        return;

    } else if(pkt->getKind() == 5) {
        // If the packet is a Info
        // If I already received the info, ignore
        if(infoReceived[pkt->getSource()]) {
            delete (pkt);
            return;
        } else {
            // If not, add the info to the routing table
            infoReceived[pkt->getSource()] = true;
            
            // Add the info to the graph
            for(int i = 0; i < pkt->getAmountNeighbours(); i++) {
                graph[pkt->getSource()].push_back(pkt->getNeighbours(i));
                if(pkt->getNeighbours(i) != this->getParentModule()->getIndex()){
                    neighbors[pkt->getNeighbours(i)] = pkt->getArrivalGate()->getIndex();
                }
                infoReceived.insert(std::make_pair(pkt->getNeighbours(i), false));
            }

            // Re-send the packet to all neighbors
            pkt->setHopCount(pkt->getHopCount()+1);
            for(int i = 0; i < numInterfaces; i++) {
                if(i != pkt->getArrivalGate()->getIndex()){
                    send(pkt->dup(), "toLnk$o", i);
                }
            }

            delete (pkt);

            // If I received all the info, I'm ready to send packets
            for(int i = 0; i < infoReceived.size(); i++) {
                if(!infoReceived[i] && i != this->getParentModule()->getIndex()) {
                    return;
                }
            }
            ready = true;

            // Run BFS to find the shortest path to all nodes
            queue<int> q;
            bool visited[MAXN];
            int dist[MAXN];
            int parent[MAXN];
            for(int i = 0; i < MAXN; i++) {
                visited[i] = false;
                dist[i] = 0;
                parent[i] = -1;
            }
            q.push(this->getParentModule()->getIndex());
            visited[this->getParentModule()->getIndex()] = true;
            while(!q.empty()) {
                int u = q.front();
                q.pop();
                for(int i = 0; i < graph[u].size(); i++) {
                    int v = graph[u][i];
                    if(!visited[v]) {
                        visited[v] = true;
                        dist[v] = dist[u] + 1;
                        parent[v] = u;
                        q.push(v);
                    }
                }
            }

            // Add the shortest path to the routing table
            for(int i = 0; i < MAXN; i++) {
                if(dist[i] != 0) {
                    int next = i;
                    while(dist[next] != 1) {
                        next = parent[next];
                    }
                    routingTable[i] = neighbors[next];
                }
            }
            // Print the routing table
            for(int i = 0; i < MAXN; i++) {
                if(routingTable.count(i) != 0) {
                    EV << "Node " << this->getParentModule()->getIndex() << " to " << i << " through " << routingTable[i] << endl;
                }
            }

            // Print the graph
            for(int i = 0; i < MAXN; i++) {
                if(graph[i].size() != 0) {
                    EV << "Node " << i << " has neighbors: ";
                    for(int j = 0; j < graph[i].size(); j++) {
                        EV << graph[i][j] << " ";
                    }
                    EV << endl;
                }
            }
            
        }

        return;
    } else {
        // If this node is the final destination, send to App
        if (pkt->getDestination() == this->getParentModule()->getIndex()) {
            send(pkt, "toApp$o");
        }
        // If not, forward the packet
        else {
            pkt->setHopCount(pkt->getHopCount()+1);
            send(pkt, "toLnk$o", routingTable[pkt->getDestination()]);
        }
    }
}
