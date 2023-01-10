package hpcc.simulations.Experiment1;

@namespace(inet);

import inet.node.inet.StandardHost;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.Router;
import ned.DatarateChannel;
import ned.IBidirectionalChannel;

network simplenetwork
{
    parameters:
        @display("bgb=512,395");
        int numberOfClients = default(1);
        int numberOfServers = default(1);
        int numberOfNormalFlows = default(1);
        int numberOfLongerFlows = default(0);
        int numberOfRouters = default(1);
    types:
        channel ethernetline extends DatarateChannel //look at 
        {
            delay = 1ms;
            datarate = 1Gbps;
        }
        channel bottleneckline extends DatarateChannel
        {
            delay = 0.5ms;
            datarate = 1Gbps;  //125000 bytes BDP = 83 packets
        }
    submodules:
        client[numberOfClients]: StandardHost {
            @display("p=68,71,m,n,$numberOfClients,150");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=450,350");
        }
        server[numberOfServers]: StandardHost {
            @display("p=431,50,m,n,$numberOfServers,150");
        }
        router1: Router {
            @display("p=183,50,m,n,$numberOfRouters,150");
        }
        router2: Router {
            @display("p=300,50,m,n,$numberOfRouters,150");
        }
    connections:
        for i=0..sizeof(client)-1 {
            client[i].pppg++ <--> ethernetline <--> router1.pppg++;
        }

        for n=0..sizeof(server)-1 {
            server[n].pppg++ <--> ethernetline <--> router2.pppg++;
        }

        router1.pppg++ <--> bottleneckline <--> router2.pppg++;
}
