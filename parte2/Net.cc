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
    // Tabla de ruteo
    std::map<int, int> routingTable;
    // Graph 
    std::vector<int> graph[MAXN];
    // Info received
    std::map<int, bool> infoReceived;
    // Estoy listo para enviar y recibir paquetes
    bool ready;
    // Numero de interfaces que tiene el modulo
    int numInterfaces;
    // Numero de vecinos conocidos
    int numNeighborsKnown;
    // Tabla de vecinos y cual es la interfaz que me lleva a ellos
    std::map<int, int> neighbors;


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
    ready = false; // No estoy listo para enviar o recibir paquetes
    int numPosiblesInterfaces = gateSize("toLnk"); // Obtengo el numero de interfaces que tiene el modulo
    numInterfaces = 0; // Inicializo el numero de interfaces realmente validas en 0
    numNeighborsKnown = 0; // Inicializo el numero de vecinos conocidos en 0

    // Creo el paquete de Hello para enviarlo a todos mis vecinos
    Packet *pkt = new Packet("Hello");
    pkt->setSource(this->getParentModule()->getIndex()); // Establezco el origen como mi numero de nodo
    pkt->setDestination(0); // Establezco el destino en 0 ya que no conozco el indice de mis vecinos
    pkt->setHopCount(0); 
    pkt->setKind(3); // Establezco el tipo de mensaje como Hello (3)

    // Envio el paquete de Hello por todas las interfaces
    for(int i = 0; i < numPosiblesInterfaces; i++) {
        // Verifico que la interfaz este conectada a un nodo/sea valida (Caso contrario me da error la simulacion)
        if(getParentModule()->getSubmodule("lnk",i)->gate("toOut$o")->getNextGate()->isConnectedOutside()) {
            numInterfaces++;
            send(pkt->dup(), "toLnk$o", i);
        }
    }

    delete (pkt);

}

void Net::finish() {
}

void Net::handleMessage(cMessage *msg) {

    Packet *pkt = (Packet *) msg;

    // Si el paquete es un Hello respondo con un HelloAck
    if (pkt->getKind() == 3) {

        // Creo el paquete de respuesta
        Packet *pktAck = new Packet("HelloAck");
        pktAck->setSource(this->getParentModule()->getIndex()); // Establezco el origen como mi numero de nodo
        pktAck->setDestination(pkt->getSource()); // Establezco el destino como el nodo que me envio el mensaje Hello
        pktAck->setHopCount(0); 
        pktAck->setKind(4); // Establezco el tipo de mensaje como HelloAck (4)

        // Envio el paquete de respuesta por la misma interfaz que me llego el mensaje Hello
        send(pktAck, "toLnk$o", pkt->getArrivalGate()->getIndex());

        delete (pkt);

    // Si el paquete es un HelloAck
    } else if(pkt->getKind() == 4) {
        
        // Incremento el numero de vecinos conocidos
        numNeighborsKnown++;
        // Agrego el vecino a mi grafo
        graph[this->getParentModule()->getIndex()].push_back(pkt->getSource());

        // Establezco cual es la interfaz que me lleva a ese vecino
        neighbors[pkt->getSource()] = pkt->getArrivalGate()->getIndex();

        // Agrego el vecino a la lista de nodos que me falta recibir el paquete "Info"
        infoReceived[pkt->getSource()] = false;

        // Si ya recibi todos los paquetes "HelloAck" de todos mis vecinos envio el paquete "Info" usando inundacion
        if(numNeighborsKnown == numInterfaces) { 
            // Creo el paquete de Info
            Packet *pktInfo = new Packet("Info");
            pktInfo->setSource(this->getParentModule()->getIndex());
            pktInfo->setDestination(0); // Establezco el destino en 0 ya que el paquete usará inundacion asi que no importa
            pktInfo->setHopCount(0);
            pktInfo->setKind(5); // Establezco el tipo de mensaje como Info (5)
            pktInfo->setNeighboursArraySize(numNeighborsKnown); // Establezco el tamaño del arreglo de vecinos del paquete
            
            // Agrego todos mis vecinos al paquete
            for(int i = 0; i < numNeighborsKnown; i++) {
                pktInfo->setNeighbours(i, graph[this->getParentModule()->getIndex()][i]);
            }
            
            // Envio el paquete por todas las interfaces
            for(int i = 0; i < numNeighborsKnown; i++) {
                send(pktInfo->dup(), "toLnk$o", neighbors[graph[this->getParentModule()->getIndex()][i]]);
            }

            delete (pktInfo);

        }

        delete (pkt);

    // Si el paquete es un Info
    } else if(pkt->getKind() == 5) {
        // Si ya recibi el paquete de ese nodo lo ignoro
        if(infoReceived[pkt->getSource()]) {
            
            delete (pkt);

        } else {
            // Si no recibi info de ese nodo ahora marco como que ya la recibi
            infoReceived[pkt->getSource()] = true;
            
            // Agrego la informacion de los vecinos 
            for(int i = 0; i < pkt->getNeighboursArraySize(); i++) {
                // Agrego el vecino del nodo a mi grafo
                graph[pkt->getSource()].push_back(pkt->getNeighbours(i));

                // Agrego el vecino a la lista de nodos que me falta recibir el paquete "Info" si no lo habia recibido antes
                infoReceived.insert(std::make_pair(pkt->getNeighbours(i), false)); 
            }

            pkt->setHopCount(pkt->getHopCount()+1);

            // Reenvio el paquete por todas las interfaces excepto por la que me llego
            for(int i = 0; i < numInterfaces; i++) {
                if(neighbors[graph[this->getParentModule()->getIndex()][i]] != pkt->getArrivalGate()->getIndex()){
                    send(pkt->dup(), "toLnk$o", neighbors[graph[this->getParentModule()->getIndex()][i]]);
                }
            }

            delete (pkt);

            // Si ya recibi todos los paquetes "Info" de todos mis vecinos entonces estoy listo para enviar paquetes de datos
            for(int i = 0; i < infoReceived.size(); i++) {
                // Si hay un nodo del cual no recibi info (y no soy yo) entonces no estoy listo
                if(!infoReceived[i] && i != this->getParentModule()->getIndex()) {
                    return;
                }
            }
            ready = true;

            // Ejecuto BFS para calcular distancias minimas desde mi nodo a todos los demás
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

            // Calculo la tabla de ruteo
            for(int i = 0; i < MAXN; i++) {
                if(dist[i] != 0) {
                    int next = i;
                    // Busco cual de mis nodos vecinos es el siguiente en el camino hacia el nodo i
                    while(dist[next] != 1) {
                        next = parent[next];
                    }
                    // Agrego a la tabla de ruteo la interfaz que me lleva al vecino que me lleva al nodo i
                    routingTable[i] = neighbors[next];
                }
            }

            // DEBUG: Imprimo la tabla de ruteo
            for(int i = 0; i < MAXN; i++) {
                if(routingTable.count(i) != 0) {
                    EV << "Node " << this->getParentModule()->getIndex() << " to " << i << " through " << routingTable[i] << endl;
                }
            }

            // DEBUG: Imprimo el grafo
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
    } else if(ready){
        // Si el paquete es para mi lo envio a la aplicacion
        if (pkt->getDestination() == this->getParentModule()->getIndex()) {
            send(pkt, "toApp$o");
        }
        // Si el paquete no es para mi lo reenvio por la interfaz correspondiente
        else {
            pkt->setHopCount(pkt->getHopCount()+1);
            send(pkt, "toLnk$o", routingTable[pkt->getDestination()]);
        }
    }
}
