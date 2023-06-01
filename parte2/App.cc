#ifndef APP
#define APP

#include <string.h>
#include <omnetpp.h>
#include <packet_m.h>

using namespace omnetpp;

class App: public cSimpleModule {
private:
    cMessage *sendMsgEvent;
    cStdDev delayStats;
    cStdDev hopStats;
    cOutVector delayVector;
    int packetsSent;
    int packetsReceived;
public:
    App();
    virtual ~App();
protected:
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage *msg);
};

Define_Module(App);

#endif /* APP */

App::App() {
}

App::~App() {
}

void App::initialize() {

    // If interArrivalTime for this node is higher than 0
    // initialize packet generator by scheduling sendMsgEvent
    if (par("interArrivalTime").doubleValue() != 0) {
        sendMsgEvent = new cMessage("sendEvent");
        scheduleAt(par("interArrivalTime"), sendMsgEvent);
    }

    // Initialize statistics
    delayStats.setName("TotalDelay");
    delayVector.setName("Delay");
    packetsSent = 0;
    packetsReceived = 0;
}

void App::finish() {
    // Record statistics
    recordScalar("Average delay", delayStats.getMean());
    recordScalar("Number of packets", delayStats.getCount());
    recordScalar("hopCount", hopStats.getMean());
    recordScalar("sentPackets", packetsSent);
    recordScalar("receivedPackets", packetsReceived);

}

void App::handleMessage(cMessage *msg) {

    // Esto se ejecuta cuando la app genera un mensaje y tiene que mandarlo a la capa de red
    if (msg == sendMsgEvent) {
        // create new packet
        Packet *pkt = new Packet("packet",this->getParentModule()->getIndex());
        pkt->setByteLength(par("packetByteSize"));
        pkt->setSource(this->getParentModule()->getIndex());
        pkt->setDestination(par("destination"));
        pkt->setHopCount(0);
        pkt->setKind(0);


        // send to net layer
        send(pkt, "toNet$o");

        // compute the new departure time and schedule next sendMsgEvent
        simtime_t departureTime = simTime() + par("interArrivalTime");
        scheduleAt(departureTime, sendMsgEvent);
        packetsSent++;
    }
    // Esto se ejecuta cuando se llega al nodo destino
    else {
        // compute delay and record statistics
        simtime_t delay = simTime() - msg->getCreationTime();
        Packet *pkt = (Packet *) msg;
        delayStats.collect(delay);
        hopStats.collect(pkt->getHopCount());
        delayVector.record(delay);
        // delete msg
        delete (msg);
        packetsReceived++;
    }

}
